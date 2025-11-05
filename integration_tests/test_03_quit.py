import pytest
from client_helper import IRCClient
from conftest import SERVER_HOST, SERVER_PORT, SERVER_PASSWORD
import socket
import time

def test_quit_command_disconnects_client(irc_server):
    """
    QUITコマンドを送信したクライアントがサーバーから切断されるかテストする。
    """
    client = IRCClient(SERVER_PORT, "Quitter")
    client.connect()
    client.register(SERVER_PASSWORD, "Quitter Realname")

    # QUITコマンドを送信
    client.send("QUIT :Leaving for good")

    # サーバーからの切断を待つ
    # 接続が閉じられるとget_messageはNoneを返す
    start_time = time.time()
    disconnected = False
    while time.time() - start_time < 5: # 5秒のタイムアウト
        msg = client.get_message(timeout=0.1)
        if msg is None: # 接続が閉じられたか、メッセージがない
            try:
                # 実際にソケットが閉じられているか確認
                client.socket.recv(1, socket.MSG_PEEK)
            except (socket.error, ConnectionResetError):
                disconnected = True
                break
            except socket.timeout:
                # タイムアウトの場合はまだ接続が生きている可能性があるのでループを続ける
                continue
            # メッセージがないがソケットは閉じられていない場合
            continue
        # ERRORメッセージを受信した場合は表示
        if msg["command"] == "ERROR":
            print(f"[Quitter] S -> C RAW: ERROR {msg.get('args')}")

    assert disconnected, "クライアントは切断されませんでした"
    client.close()

def test_quit_notifies_other_channel_members(irc_server):
    """
    QUITコマンドを送信したクライアントが参加していたチャネルの他のメンバーに
    QUIT通知が送信されるかテストする。
    """
    client_quitter = IRCClient(SERVER_PORT, "Quitter")
    client_receiver = IRCClient(SERVER_PORT, "Receiver")

    client_quitter.connect()
    client_quitter.register(SERVER_PASSWORD, "Quitter Realname")

    client_receiver.connect()
    client_receiver.register(SERVER_PASSWORD, "Receiver Realname")

    # 両方のクライアントを同じチャネルに参加させる
    client_quitter.send("JOIN #test_channel")
    client_quitter.wait_for_command("JOIN") # Quitter自身のJOIN応答
    client_quitter.wait_for_command("331") # No topic
    client_quitter.wait_for_command("353") # NAMES list
    client_quitter.wait_for_command("366") # End of NAMES

    client_receiver.send("JOIN #test_channel")
    client_receiver.wait_for_command("JOIN") # Receiver自身のJOIN応答
    client_receiver.wait_for_command("331") # No topic
    client_receiver.wait_for_command("353") # NAMES list
    client_receiver.wait_for_command("366") # End of NAMES

    # QuitterがQUITコマンドを送信
    quit_message = "I'm out!"
    client_quitter.send(f"QUIT :{quit_message}")

    # ReceiverがQuitterからのQUIT通知を受信するのを待つ
    quit_notification = client_receiver.wait_for_command("QUIT", timeout=5.0)

    assert quit_notification is not None, "ReceiverがQUIT通知を受信しませんでした"
    assert quit_notification["prefix"] == f"Quitter!Quitter@127.0.0.1"
    assert quit_notification["command"] == "QUIT"
    assert quit_notification["args"] == [quit_message]

    client_quitter.close()
    client_receiver.close()

def test_quit_sends_error_to_quitting_client(irc_server):
    """
    QUITコマンドを送信したクライアント自身が、切断直前に
    サーバーからERRORメッセージを受信するかテストする。
    """
    client = IRCClient(SERVER_PORT, "QClient")
    client.connect()
    client.register(SERVER_PASSWORD, "Quitter Realname")

    quit_message = "Bye now"
    client.send(f"QUIT :{quit_message}")

    # サーバーからのERRORメッセージを待つ
    error_msg = client.wait_for_command("ERROR", timeout=3.0)

    assert error_msg is not None, "Quitting clientがERRORメッセージを受信しませんでした"
    
    # ERRORメッセージの形式を確認
    # 例: ERROR :Closing Link: 127.0.0.1 (Bye now)
    assert "Closing Link" in error_msg["args"][0]
    assert quit_message in error_msg["args"][0]

    client.close()

