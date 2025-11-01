import pytest
from client_helper import IRCClient
from conftest import SERVER_HOST, SERVER_PORT, SERVER_PASSWORD

# conftest.py の irc_server フィクスチャが自動的に適用される

def test_client_registration(irc_server):
    """
    クライアントがサーバーに接続し、正しく登録できるかテストする
    """
    client = IRCClient(SERVER_PORT, "UserA")
    client.connect()

    # client_helper の register メソッドがRPL_WELCOMEの受信を保証する
    client.register(SERVER_PASSWORD, "User A Realname")

    # 登録が成功したことを（暗黙的に）確認
    assert client.nick == "UserA"

    client.close()

def test_nick_in_use(irc_server):
    """
    既に使用されているニックネームを登録しようとした場合、
    ERR_NICKNAMEINUSE (433) が返るかテストする
    """
    client1 = IRCClient(SERVER_PORT, "UserA")
    client1.connect()
    client1.register(SERVER_PASSWORD)

    client2 = IRCClient(SERVER_PORT, "UserA") # 同じNickで接続
    client2.connect()
    client2.send(f"PASS {SERVER_PASSWORD}")
    client2.send(f"NICK UserA")
    client2.send(f"USER userb 0 * :User B")

    ### NICK, USER プロトコル
    # NICK <nickname>
    # USER <username> <hostname> <servername> :<realname>
    ###

    # 433 (ERR_NICKNAMEINUSE) を待つ
    msg = client2.wait_for_command("433")

    assert msg is not None, "ERR_NICKNAMEINUSE (433) が返りませんでした"
    # IRCプロトコル 433 ERR_NICKNAMEINUSE のフォーマット
    # :server_name 433 <client> <nick> :<reason>
    # 例
    # --- Client A ---
    # nick UserA
    # user user a * 0 :User A
    # :ergo.test 001 UserA :Welcome to
    # --- client B --- isNotRegistered
    # user user b * 0 :User A
    # :ergo.test 433 * UserA :Nickname is already in use
    assert msg["args"] == ["*", "UserA", "Nickname is already in use"]

    # これは間違い
    # assert msg["args"] == ["UserA", "Nickname is already in use"]

    client1.close()
    client2.close()

def test_invalid_password(irc_server):
    """
    不正なパスワードで登録しようとした場合、
    ERR_PASSWDMISMATCH (464) が返るかテストする
    """
    client = IRCClient(SERVER_PORT, "UserC")
    client.connect()

    client.send("PASS wrongpass")
    client.send("NICK UserC")
    client.send("USER userc 0 * :User C")

    # 464 を待つ
    msg = client.wait_for_command("464")

    assert msg is not None, "ERR_PASSWDMISMATCH (464) が返りませんでした"
    assert "Password incorrect" in msg["args"][-1]

    client.close()
