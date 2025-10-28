import pytest
from client_helper import IRCClient
from conftest import SERVER_HOST, SERVER_PORT, SERVER_PASSWORD

# conftest.py の irc_server フィクスチャが自動的に適用される

@pytest.fixture(scope="function")
def registered_clients(irc_server):
    """
    テスト用に登録済みのクライアントを2つ準備するフィクスチャ
    """
    client1 = IRCClient(SERVER_PORT, "UserA")
    client1.connect()
    client1.register(SERVER_PASSWORD)

    client2 = IRCClient(SERVER_PORT, "UserB")
    client2.connect()
    client2.register(SERVER_PASSWORD)

    # 2つのクライアントをタプルでテスト関数に渡す
    yield client1, client2

    # テスト終了後、クリーンアップ
    client1.close()
    client2.close()


def test_join_and_names(registered_clients):
    """
    クライアントがチャンネルに参加し、NAMESリストが正しく返るか
    """
    client1, client2 = registered_clients

    # Client1 が #test に参加
    client1.send("JOIN #test")

    # JOIN応答 (RPL_NAMREPLY と RPL_ENDOFNAMES) を確認
    names_msg = client1.wait_for_command("353") # RPL_NAMREPLY
    end_names_msg = client1.wait_for_command("366") # RPL_ENDOFNAMES

    assert names_msg is not None, "RPL_NAMREPLY (353) が受信できませんでした"
    assert names_msg["args"] == ["UserA", "=", "#test", "UserA"]

    assert end_names_msg is not None, "RPL_ENDOFNAMES (366) が受信できませんでした"
    assert end_names_msg["args"] == ["UserA", "#test", "End of /NAMES list."]

def test_join_notification_and_privmsg(registered_clients):
    """
    2クライアントがチャンネルに参加し、一方が送信したメッセージを
    もう一方が受信できるかテストする
    """
    client1, client2 = registered_clients

    # Client1 が #test に参加
    client1.send("JOIN #test")
    client1.wait_for_command("366") # JOIN完了を待つ

    # Client2 が #test に参加
    client2.send("JOIN #test")

    # Client1 は Client2 のJOIN通知を受信するはず
    join_notification = client1.wait_for_command("JOIN")
    assert join_notification is not None, "Client1がClient2のJOIN通知を受信できませんでした"
    assert join_notification["prefix"].startswith("UserB!UserB@")
    assert join_notification["args"] == ["#test"]

    # Client1 がメッセージを送信
    client1.send("PRIVMSG #test :Hello UserB!")

    # Client2 がメッセージを受信
    privmsg = client2.wait_for_command("PRIVMSG")
    assert privmsg is not None, "Client2がPRIVMSGを受信できませんでした"
    assert privmsg["prefix"].startswith("UserA!UserA@")
    assert privmsg["args"] == ["#test", "Hello UserB!"]

