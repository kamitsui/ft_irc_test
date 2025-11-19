#!/bin/env python3

import socket
import time

# --- 設定項目 ---
# 接続するIRCサーバーの情報
SERVER = "127.0.0.1"  # サーバーのIPアドレス or ホスト名
PORT = 6668           # サーバーのポート番号
#PORT = 6667           # サーバーのポート番号
NICKNAME = "py-test"  # 使用するニックネーム
USERNAME = "pytest"   # 使用するユーザー名
REALNAME = "Python Test Script" # ユーザーのフルネーム

# --- スクリプト本体 ---
def main():
    # ソケットを作成
    irc_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    
    try:
        # サーバーに接続
        print(f"[*] サーバーに接続中: {SERVER}:{PORT}...")
        irc_socket.connect((SERVER, PORT))
        print("[+] 接続成功！")

        # サーバーからの応答を待つため、少し待機
        time.sleep(1)

        # ニックネームを送信
        # IRCプロトコルでは、コマンドの末尾に \r\n が必要
        nick_command = f"NICK {NICKNAME}\r\n"
        irc_socket.send(nick_command.encode("utf-8"))
        print(f">>> 送信: {nick_command.strip()}")

        # ユーザー情報を送信
        user_command = f"USER {USERNAME} 0 * :{REALNAME}\r\n"
        irc_socket.send(user_command.encode("utf-8"))
        print(f">>> 送信: {user_command.strip()}")

        # サーバーからの応答を監視
        # サーバーからのPINGに対してPONGを返すことで、接続を維持する
        # サーバーからのウェルカムメッセージ(001)が来たら登録成功とみなす
        is_registered = False
        while not is_registered:
            # 4096バイトまでのデータを受信
            response = irc_socket.recv(4096).decode("utf-8")
            if not response:
                print("[!] サーバーからの応答がありません。接続が切れた可能性があります。")
                break
            
            # 受信したデータを表示
            print(f"<<< 受信: {response.strip()}")

            # PING/PONG 応答
            if response.startswith("PING"):
                pong_response = response.replace("PING", "PONG")
                irc_socket.send(pong_response.encode("utf-8"))
                print(f">>> 送信: {pong_response.strip()}")

            # RPL_WELCOME (001) を受信したら登録完了
            # 応答の中に " 001 " という文字列があるかで判定
            if f" 001 {NICKNAME} " in response:
                print("\n[+] IRCサーバーへの登録が確認できました。")
                is_registered = True
                time.sleep(2) # 確認のため少し待機

    except ConnectionRefusedError:
        print(f"[!] 接続が拒否されました。サーバーが起動しているか、アドレスとポートが正しいか確認してください。")
    except Exception as e:
        print(f"[!] エラーが発生しました: {e}")
    finally:
        # 接続を終了
        if irc_socket:
            print("\n[*] サーバーから切断します...")
            quit_command = "QUIT :Test finished.\r\n"
            try:
                irc_socket.send(quit_command.encode("utf-8"))
                print(f">>> 送信: {quit_command.strip()}")
            except Exception as e:
                print(f"[!] QUITコマンド送信中にエラー: {e}")
            finally:
                irc_socket.close()
                print("[+] 切断完了。")

if __name__ == "__main__":
    main()

# Run the python test
#python3 irc_connect_test.py
