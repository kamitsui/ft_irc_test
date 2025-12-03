#import pytest
#import socket
#import time
#import select
#from client_helper import IRCClient, SERVER_HOST
#from conftest import SERVER_PORT, SERVER_PASSWORD
#
## 一時停止機能をサポートする拡張IRCクライアント
#class PausedIRCClient(IRCClient):
#    def __init__(self, port, nick="paused_client"):
#        super().__init__(port, nick)
#        self._is_paused = False
#        self._pause_duration = 0 # 受信を一時停止する時間 (秒)
#        self._pause_start_time = 0
#
#    def pause_receiving(self, duration=0):
#        """クライアントの受信を一時停止する。durationが0の場合、手動で再開するまで停止。"""
#        print(f"[{self.nick}] --- PAUSING RECEIVING for {duration} seconds ---")
#        self._is_paused = True
#        self._pause_duration = duration
#        self._pause_start_time = time.time()
#
#    def resume_receiving(self):
#        """クライアントの受信を再開する。"""
#        print(f"[{self.nick}] --- RESUMING RECEIVING ---")
#        self._is_paused = False
#        self._pause_duration = 0
#        self._pause_start_time = 0
#
#    def _recv_raw(self, timeout=1.0):
#        """一時停止中は実際のrecvを呼び出さないようにオーバーライド"""
#        if self._is_paused:
#            if self._pause_duration > 0 and (time.time() - self._pause_start_time) > self._pause_duration:
#                self.resume_receiving() # 指定期間経過したら自動再開
#            else:
#                # 一時停止中は何も読み取らず、タイムアウトをシミュレート
#                return None
#        return super()._recv_raw(timeout)
#
## テスト: 一時停止したクライアントへのメッセージフラッド
#def test_paused_client_flood(irc_server):
#    sender_client = IRCClient(SERVER_PORT, nick="sender")
#    paused_client = PausedIRCClient(SERVER_PORT, nick="receiver")
#
#    sender_client.connect()
#    paused_client.connect()
#
#    sender_client.register(SERVER_PASSWORD)
#    paused_client.register(SERVER_PASSWORD)
#
#    # チャンネルに参加
#    channel_name = "#testchannel"
#    sender_client.send(f"JOIN {channel_name}")
#    paused_client.send(f"JOIN {channel_name}")
#
#    # JOIN応答を待つ
#    sender_client.wait_for_command("JOIN", timeout=2)
#    paused_client.wait_for_command("JOIN", timeout=2)
#    # NAMESリストなども受信する可能性があるので、少し待機
#    sender_client.get_message(timeout=1)
#    paused_client.get_message(timeout=1)
#
#
#    # paused_clientの受信を一時停止
#    paused_client.pause_receiving() # 手動で再開するまで停止
#
#    # sender_clientから大量のメッセージをフラッド
#    num_messages = 2000 # 十分な量
#    message_prefix = "FLOOD_MSG_"
#    print(f"[{sender_client.nick}] Sending {num_messages} messages to {channel_name}...")
#    for i in range(num_messages):
#        sender_client.send(f"PRIVMSG {channel_name} :{message_prefix}{i}")
#        # 短い間隔で送信し、ソケットバッファが満杯になる機会を与える
#        time.sleep(0.001)
#
#    print(f"[{sender_client.nick}] Finished sending all flood messages.")
#
#    # サーバーがハングアップしていないことを間接的に確認
#    # sender_clientがQUITコマンドを送信し、応答を受け取れることを確認
#    sender_client.send("QUIT :Flood Test Done")
#    quit_response = sender_client.wait_for_command("ERROR", timeout=5) # QUIT後のエラー応答や切断を待つ
#    assert quit_response is not None or sender_client.get_message(timeout=1) is None, "Sender client should be able to quit, indicating server is not hung."
#    sender_client.close()
#
#    # paused_clientの受信を再開
#    paused_client.resume_receiving()
#
#    # paused_clientが蓄積されたすべてのメッセージを受信するまで待機
#    received_messages_count = 0
#    start_time = time.time()
#    all_received_content = []
#
#    print(f"[{paused_client.nick}] Waiting for flood messages...")
#    while received_messages_count < num_messages and (time.time() - start_time < 30): # 30秒のタイムアウト
#        msg = paused_client.get_message(timeout=0.1) # 短いタイムアウトでポーリング
#        if msg:
#            print(f"[{paused_client.nick}] Received: {msg}")
#            if msg["command"] == "PRIVMSG" and msg["args"][0] == channel_name and message_prefix in msg["args"][1]:
#                received_messages_count += 1
#                all_received_content.append(msg["args"][1])
#        # else: msg is None (timeout for get_message)
#
#    print(f"[{paused_client.nick}] Total messages received: {received_messages_count}")
#
#    assert received_messages_count == num_messages, \
#        f"Expected to receive {num_messages} messages, but got {received_messages_count}"
#
#    # メッセージの順序を検証
#    for i in range(num_messages):
#        expected_content = f"{message_prefix}{i}"
#        assert expected_content in all_received_content[i], \
#            f"Message at index {i} is not in expected order or content: {all_received_content[i]}"
#
#    paused_client.close()
