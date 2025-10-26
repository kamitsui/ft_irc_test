import pytest
import socket
import subprocess
import time
import select

# --- サーバー設定 (Makefileや起動コマンドに合わせてください) ---
SERVER_EXECUTABLE = "../../ircserv" # ircserv実行ファイルへのパス
SERVER_PORT = 6668 # ユニットテストとは別のポートを推奨
SERVER_PASSWORD = "testpass"
SERVER_HOST = "127.0.0.1"

# --- Pytest フィクスチャ (テストの準備と後片付け) ---

@pytest.fixture(scope="function")
def irc_server():
    """
    各テスト関数の実行前にサーバーを起動し、
    テスト終了後にサーバーを停止するフィクスチャ
    """
    # サーバープロセスを起動
    server_process = subprocess.Popen(
        [SERVER_EXECUTABLE, str(SERVER_PORT), SERVER_PASSWORD],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )
    
    # サーバーが起動するのを待つ (簡易的な待機)
    time.sleep(0.2) 
    
    # サーバーが起動しているか確認 (起動直後に死んでいないか)
    if server_process.poll() is not None:
        stderr_output = server_process.stderr.read()
        raise RuntimeError(f"サーバーの起動に失敗しました: {stderr_output}")

    # テスト本体 (yield) にサーバープロセスを渡す
    yield server_process

    # テスト終了後、サーバープロセスを停止
    server_process.terminate()
    try:
        # 終了待機 (タイムアウト付き)
        server_process.wait(timeout=1.0)
    except subprocess.TimeoutExpired:
        print("サーバーが時間内に終了しなかったため、強制終了します。")
        server_process.kill()
        
    print("\n--- Server stdout ---")
    print(server_process.stdout.read())
    print("--- Server stderr ---")
    print(server_process.stderr.read())

# --- テスト用IRCクライアントヘルパークラス ---

class IRCClient:
    """
    ソケット通信をラップし、IRCコマンドの送受信を容易にするヘルパークラス
    """
    def __init__(self, host, port):
        self.host = host
        self.port = port
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket.settimeout(1.0) # 1秒のタイムアウト
        self.buffer = b""

    def connect(self):
        self.socket.connect((self.host, self.port))

    def send(self, command_str):
        print(f"C -> S: {command_str}")
        self.socket.sendall((command_str + "\r\n").encode('utf-8'))

    def recv(self, timeout=0.5):
        """
        サーバーからの応答を受信する。複数行まとめて返すことがある。
        """
        # selectを使ってデータが来るまで待機
        ready_to_read, _, _ = select.select([self.socket], [], [], timeout)
        if not ready_to_read:
            return None # タイムアウト

        try:
            data = self.socket.recv(4096)
            if not data:
                return None # 接続切断
            print(f"S -> C: {data.decode('utf-8', errors='ignore')}")
            return data.decode('utf-8', errors='ignore')
        except socket.timeout:
            return None
        except Exception as e:
            print(f"Recv error: {e}")
            return None
            
    def register(self, password, nick, user):
        """クライアント登録を自動で行う"""
        self.send(f"PASS {password}")
        self.send(f"NICK {nick}")
        self.send(f"USER {user} 0 * :{user}")
        # 登録応答(001など)を待つ
        response = self.recv(timeout=1.0)
        return response

    def close(self):
        self.socket.close()


# --- 統合テストケース ---

def test_client_registration(irc_server):
    """
    クライアントがサーバーに接続し、正しく登録できるかテストする
    """
    client = IRCClient(SERVER_HOST, SERVER_PORT)
    client.connect()
    
    response = client.register(SERVER_PASSWORD, "UserA", "usera")
    
    assert response is not None, "サーバーから応答がありませんでした"
    assert "001 UserA :Welcome" in response, "RPL_WELCOME (001) が受信できませんでした"
    assert "002 UserA :Your host is" in response, "RPL_YOURHOST (002) が受信できませんでした"
    
    client.close()

def test_nick_in_use(irc_server):
    """
    既に使用されているニックネームを登録しようとした場合、
    ERR_NICKNAMEINUSE (433) が返るかテストする
    """
    client1 = IRCClient(SERVER_HOST, SERVER_PORT)
    client1.connect()
    client1.register(SERVER_PASSWORD, "UserA", "usera")

    client2 = IRCClient(SERVER_HOST, SERVER_PORT)
    client2.connect()
    # UserAと同じニックネームで登録しようとする
    response = client2.register(SERVER_PASSWORD, "UserA", "userb")

    assert response is not None, "サーバーから応答がありませんでした"
    assert "433 UserA :Nickname is already in use" in response, "ERR_NICKNAMEINUSE (433) が返りませんでした"
    
    client1.close()
    client2.close()

def test_join_and_privmsg(irc_server):
    """
    2クライアントがチャンネルに参加し、一方が送信したメッセージを
    もう一方が受信できるかテストする
    """
    client1 = IRCClient(SERVER_HOST, SERVER_PORT)
    client1.connect()
    client1.register(SERVER_PASSWORD, "UserA", "usera")
    
    client2 = IRCClient(SERVER_HOST, SERVER_PORT)
    client2.connect()
    client2.register(SERVER_PASSWORD, "UserB", "userb")

    # Client1 が #test に参加
    client1.send("JOIN #test")
    client1.recv() # JOINの応答を読み飛ばす

    # Client2 が #test に参加
    client2.send("JOIN #test")
    # Client2 は Client1 のJOIN通知と、自身のJOIN応答を受信するはず
    response_c2 = client2.recv(timeout=1.0) 
    assert ":UserA" in response_c2, "Client2がClient1のJOIN通知を受信できませんでした"
    assert "353 UserB = #test :UserA UserB" in response_c2 or "353 UserB = #test :UserB UserA" in response_c2, "RPL_NAMREPLY (353) が正しくありません"

    # Client1 がメッセージを送信
    client1.send("PRIVMSG #test :Hello UserB!")
    
    # Client2 がメッセージを受信
    response_c2 = client2.recv(timeout=1.0)
    assert response_c2 is not None, "Client2がPRIVMSGを受信できませんでした"
    assert ":UserA!usera@127.0.0.1 PRIVMSG #test :Hello UserB!" in response_c2, "PRIVMSGの形式が正しくありません"

    client1.close()
    client2.close()
