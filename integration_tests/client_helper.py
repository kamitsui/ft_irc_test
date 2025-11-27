"""ネットワーク通信と非同期I/O操作をサポートするモジュール。"""
import socket
import select
import time

SERVER_HOST = "127.0.0.1" # conftest.py と合わせる

class IRCClient:
    """
    ソケット通信をラップし、IRCコマンドの送受信を容易にするヘルパークラス。
    RFC1459メッセージの解析機能を追加し、テストの可読性を向上させます。
    """
    def __init__(self, port, nick="client"):
        self.host = SERVER_HOST
        self.port = port
        self.nick = nick
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket.settimeout(2.0) # タイムアウトを少し長めに
        self.buffer = ""
        self.message_queue = [] # メッセージキューを追加
        self.auto_pong = True

    def connect(self):
        """サーバーにソケット接続を確立します。"""
        print(f"[{self.nick}] Connecting to {self.host}:{self.port}...")
        self.socket.connect((self.host, self.port))

    def send(self, command_str):
        """IRCコマンド文字列をサーバーに送信し、CRLFを付加してエンコードする。"""
        print(f"[{self.nick}] C -> S: {command_str}")
        self.socket.sendall((command_str + "\r\n").encode('utf-8'))

    def _recv_raw(self, timeout=1.0):
        """内部用の受信ヘルパー"""
        ready_to_read, _, _ = select.select([self.socket], [], [], timeout)
        if not ready_to_read:
            return None # タイムアウト

        try:
            data = self.socket.recv(4096).decode('utf-8', errors='ignore')
            if not data:
                print(f"[{self.nick}] S -> C: (Connection closed by server)")
                return "" # 接続切断
            print(f"[{self.nick}] S -> C RAW: {data.strip()}")
            return data
        except socket.timeout:
            return None
        except Exception as e:
            print(f"[{self.nick}] Recv error: {e}")
            return ""

    def get_message(self, timeout=1.0):
        """
        メッセージキューまたはネットワークからIRCメッセージを取得する。
        PINGは自動で処理される。
        """
        # 1. メッセージキューにメッセージがあれば、それを返す
        if self.message_queue:
            return self.message_queue.pop(0)

        start_time = time.time()
        while time.time() - start_time < timeout:
            # 2. バッファから全ての完全なメッセージを解析し、キューに入れる
            while "\r\n" in self.buffer:
                line, self.buffer = self.buffer.split("\r\n", 1)
                msg = self.parse_message(line)
                if msg:
                    self.message_queue.append(msg) # すべてのメッセージをキューに追加する
                    if msg["command"] == "PING" and self.auto_pong:
                        self._handle_ping(msg) # PONG応答は引き続き行う
            
            # 3. キューにメッセージが入ったら、最初のものを返す
            if self.message_queue:
                return self.message_queue.pop(0)

            # 4. バッファが不完全な場合、ネットワークから読み込む
            raw_data = self._recv_raw(0.1)
            if raw_data is None: # タイムアウト
                time.sleep(0.01)
                continue
            if raw_data == "": # 接続切断
                return None
            self.buffer += raw_data
        
        return None # 総合タイムアウト

    def wait_for_command(self, expected_command, timeout=2.0):
        """
        指定されたコマンド (例: "001" や "PRIVMSG") を受信するまで待機する。
        """
        start_time = time.time()
        while time.time() - start_time < timeout:
            msg = self.get_message(timeout=0.2)
            if msg and msg["command"] == expected_command:
                return msg

        print(f"[{self.nick}] Timeout: Did not receive command '{expected_command}'")
        return None

    def _handle_ping(self, ping_msg):
        """
        PINGメッセージに応答する。
        """
        token = ""
        if "args" in ping_msg and ping_msg["args"]:
            token = ping_msg["args"][0]
        self.send(f"PONG :{token}")
        #if "args" in ping_msg and ping_msg["args"]:
        #    token = ping_msg["args"][0]
        #else:
        #    self.send("PONG :irc.myserver.com") # デフォルトのサーバー名

    def register(self, password, user_real_name="Test User"):
        """クライアント登録を自動で行う"""
        self.send(f"PASS {password}")
        self.send(f"NICK {self.nick}")
        self.send(f"USER {self.nick} 0 * :{user_real_name}")

        expected_registration_commands = ["001", "002"]
        received_registration_messages = []
        start_time = time.time()
        print(f"[{self.nick}] Debug: Starting registration loop.")
        while len(received_registration_messages) < 2 and (time.time() - start_time < 5): # タイムアウトは調整
            msg = self.get_message(timeout=0.1)
            if msg:
                print(f"[{self.nick}] Debug: get_message returned: {msg}")
            if msg and msg["command"] in expected_registration_commands:
                received_registration_messages.append(msg)
                print(f"[{self.nick}] Debug: Appended {msg['command']}. Current received: {[m['command'] for m in received_registration_messages]}")
            elif msg:
                print(f"[{self.nick}] Debug: Skipped unexpected msg: {msg}")
            # else: msg is None (timeout for get_message)
        print(f"[{self.nick}] Debug: Exiting registration loop. Final received: {[m['command'] for m in received_registration_messages]}")
        
        # 001と002が両方存在することを確認
        assert any(msg["command"] == "001" for msg in received_registration_messages), \
            f"[{self.nick}] 登録失敗: RPL_WELCOME (001) が受信できませんでした"
        assert any(msg["command"] == "002" for msg in received_registration_messages), \
            f"[{self.nick}] 登録失敗: RPL_YOURHOST (002) が受信できませんでした"
        print(f"[{self.nick}] Debug: Registration assertions passed.")
        # 003と004もオプションでチェック
        # assert any(msg["command"] == "003" for msg in received_registration_messages), \
        #     f"[{self.nick}] 登録失敗: RPL_CREATED (003) が受信できませんでした"
        # assert any(msg["command"] == "004" for msg in received_registration_messages), \
        #     f"[{self.nick}] 登録失敗: RPL_MYINFO (004) が受信できませんでした"

        print(f"[{self.nick}] 登録完了 (Nick: {self.nick})")

    def close(self):
        """ソケット接続を閉じます。"""
        print(f"[{self.nick}] Closing connection.")
        self.socket.close()

    @staticmethod
    def parse_message(raw_line):
        """
        生のIRCメッセージをパースして辞書で返す
        :prefix COMMAND arg1 arg2 :trailing
        """
        line = raw_line.strip()
        if not line:
            return None

        parts = line.split(' ')
        prefix_dict = {}

        if parts[0].startswith(':'):
            prefix_str = parts.pop(0)[1:]
            if '!' in prefix_str and '@' in prefix_str:
                nick, rest = prefix_str.split('!', 1)
                user, host = rest.split('@', 1)
                prefix_dict = {"nick": nick, "user": user, "host": host, "raw": prefix_str}
            else:
                prefix_dict = {"server": prefix_str, "raw": prefix_str}


        command = parts.pop(0).upper()

        args = []
        trailing = None

        while parts:
            if parts[0].startswith(':'):
                trailing = ' '.join(parts)[1:]
                break
            else:
                args.append(parts.pop(0))

        if trailing:
            args.append(trailing)

        return {"prefix": prefix_dict, "command": command, "args": args}
