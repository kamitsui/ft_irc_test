import socket
import threading
import time
import random

SERVER_HOST = '127.0.0.1' # ホストマシンのIPまたはlocalhost
SERVER_PORT = 6667
NUM_CLIENTS = 1000 # 同時接続数

def run_client(client_id):
    nick = f"user_{client_id}_{random.randint(1000, 9999)}"
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((SERVER_HOST, SERVER_PORT))
        
        # ユーザー登録
        s.send(f"NICK {nick}\r\n".encode('utf-8'))
        s.send(f"USER {nick} 0 * :{nick} user\r\n".encode('utf-8'))
        
        # 応答を少しだけ読み取る（確認）
        # response = s.recv(4096).decode('utf-8')
        # print(f"Client {client_id} connected and registered.")
        
        # チャンネルに参加
        s.send(f"JOIN #test_channel\r\n".encode('utf-8'))

        # ここで一定時間接続を維持し続ける
        time.sleep(300) # 5分間接続を維持

        s.close()
    except Exception as e:
        print(f"Client {client_id} error: {e}")

# 負荷テストの実行
print(f"Starting load test with {NUM_CLIENTS} clients...")
threads = []
for i in range(NUM_CLIENTS):
    t = threading.Thread(target=run_client, args=(i,))
    threads.append(t)
    t.start()
    # 接続を分散させるために少し待機
    # time.sleep(0.01) 

for t in threads:
    t.join()

print("Load test finished.")
