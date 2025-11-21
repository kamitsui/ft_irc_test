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

    # サーバーからの切断を待つ (最大5秒)
    start_time = time.time()
    disconnected = False
    while time.time() - start_time < 5:
        try:
            # 切断されたソケットにデータを送信しようとするとエラーになることを期待
            client.send("DUMMY_COMMAND") # ダミーコマンドを送信
        except (socket.error, ConnectionResetError, BrokenPipeError) as e:
            print(f"[Quitter] Detected disconnection: {e}")
            disconnected = True
            break
        except socket.timeout:
            # タイムアウトは接続がまだ生きている可能性があるのでループを続ける
            pass
        time.sleep(0.1) # 短時間待機して再試行

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
    assert quit_notification["prefix"]["raw"] == f"Quitter!Quitter@127.0.0.1"
    assert quit_notification["command"] == "QUIT"
    assert quit_notification["args"] == [quit_message]

    client_quitter.close()
    client_receiver.close()

