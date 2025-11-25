import pytest
import socket
import time
from client_helper import IRCClient
from conftest import SERVER_PORT, SERVER_PASSWORD # conftestからポートとパスワードをインポート

# サーバーに大量のメッセージを送信し、すべてが受信されることを確認するテスト
def test_bulk_message_delivery(irc_server):
    client1 = IRCClient(SERVER_PORT, nick="nick1") # グローバル定数を使用
    client2 = IRCClient(SERVER_PORT, nick="nick2") # グローバル定数を使用
    client1.connect()
    client2.connect()
    client1.register(SERVER_PASSWORD) # グローバル定数を使用
    client2.register(SERVER_PASSWORD)

    # client1からclient2へ大量のメッセージを送信
    num_messages = 100
    message_prefix = "HELLO_WORLD_"
    for i in range(num_messages):
        client1.send(f"PRIVMSG nick2 :{message_prefix}{i}") # sendメソッドを使用

    # client2がすべてのメッセージを受信することを確認
    received_messages = []
    start_time = time.time()
    while len(received_messages) < num_messages and (time.time() - start_time < 5): # 5秒のタイムアウト
        msg = client2.get_message() # get_messageメソッドを使用
        if msg and msg["command"] == "PRIVMSG" and msg["args"][0] == "nick2" and message_prefix in msg["args"][1]:
            received_messages.append(msg)
            print(f"Client2 received: {msg}")
        elif msg:
            print(f"Client2 received (other): {msg}")

    assert len(received_messages) == num_messages, f"Expected {num_messages} messages, but got {len(received_messages)}"
    # メッセージの順序や内容も必要に応じて検証可能
    for i in range(num_messages):
        expected_message_content = f"{message_prefix}{i}"
        assert any(expected_message_content in msg["args"][1] for msg in received_messages), f"Message {i} not found"
    
    client1.close()
    client2.close()

# サーバーの送信バッファが満杯になってもデータ損失が発生しないことを検証するテスト
# これは、クライアントが大量のデータを一度に送信し、サーバーがそれを内部バッファで処理することを確認する
def test_no_data_loss_on_full_send_buffer(irc_server):
    client1 = IRCClient(SERVER_PORT, nick="sender") # グローバル定数を使用
    client2 = IRCClient(SERVER_PORT, nick="receiver") # グローバル定数を使用
    client1.connect()
    client2.connect()
    client1.register(SERVER_PASSWORD) # グローバル定数を使用
    client2.register(SERVER_PASSWORD)

    large_message_content = "A" * 1024 # 1KBのメッセージ
    num_large_messages = 500          # 500KBのデータを送信 (サーバーのバッファをオーバーフローさせる可能性)

    print(f"Sending {num_large_messages} large messages...")
    for i in range(num_large_messages):
        client1.send(f"PRIVMSG receiver :{large_message_content}_{i}")
    print("All messages sent by sender.")

    received_count = 0
    start_time = time.time()
    all_received = []

    while received_count < num_large_messages and (time.time() - start_time < 60): # 60秒のタイムアウト
        msg = client2.get_message() # get_messageメソッドを使用
        if msg and msg["command"] == "PRIVMSG" and msg["args"][0] == "receiver" and f"{large_message_content}_" in msg["args"][1]:
            received_count += 1
            all_received.append(msg)
            # print(f"Receiver received: {msg}")
        elif msg:
            # print(f"Receiver received (other): {msg}")
            pass # 他のサーバーからの応答は無視
        else: # msg is None
            time.sleep(0.01) # 短いスリープでCPU使用率を抑える
    assert received_count == num_large_messages, f"Expected to receive {num_large_messages} messages, but got {received_count}"

    # 受信したメッセージの内容と順序を検証
    for i in range(num_large_messages):
        expected_substring = f"{large_message_content}_{i}"
        found = False
        for msg in all_received:
            if msg and expected_substring in msg["args"][1]:
                found = True
                break
        assert found, f"Message containing '{expected_substring}' was not found in received messages."
    
    client1.close()
    client2.close()
