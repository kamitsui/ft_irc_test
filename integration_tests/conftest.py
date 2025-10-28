import pytest
import subprocess
import time
import socket
import select

# --- サーバー設定 ---
SERVER_EXECUTABLE = "../../ircserv"
SERVER_PORT = 6668
SERVER_PASSWORD = "testpass"
SERVER_HOST = "127.0.0.1"

@pytest.fixture(scope="function")
def irc_server(request):
    """
    各テスト関数の実行前にサーバーを起動し、
    テスト終了後にサーバーを停止するフィクスチャ。
    
    `request.param` を使用して、テストごとに異なる引数で
    サーバーを起動することも将来的に可能。
    """
    print(f"\nStarting ircserv on {SERVER_HOST}:{SERVER_PORT}...")
    
    server_process = subprocess.Popen(
        [SERVER_EXECUTABLE, str(SERVER_PORT), SERVER_PASSWORD],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )
    
    # サーバーが起動するのを待つ
    time.sleep(0.3) 
    
    if server_process.poll() is not None:
        stderr_output = server_process.stderr.read()
        raise RuntimeError(f"サーバーの起動に失敗しました: {stderr_output}")

    # テスト本体 (yield) にプロセスを渡す
    yield server_process

    # テスト終了後、サーバープロセスを停止
    print("\nShutting down ircserv...")
    server_process.terminate()
    try:
        server_process.wait(timeout=1.0)
    except subprocess.TimeoutExpired:
        print("サーバーが時間内に終了しなかったため、強制終了します。")
        server_process.kill()
        
    # テスト失敗時にサーバーのログを出力する
    if request.node.rep_call.failed:
        print("\n--- Server stdout (on test failure) ---")
        print(server_process.stdout.read())
        print("--- Server stderr (on test failure) ---")
        print(server_process.stderr.read())

@pytest.hookimpl(tryfirst=True, hookwrapper=True)
def pytest_runtest_makereport(item, call):
    """
    テストが失敗したかどうかを判定し、フィクスチャで
    ログを出力できるようにするためのフック
    """
    outcome = yield
    rep = outcome.get_result()
    setattr(item, "rep_" + rep.when, rep)
