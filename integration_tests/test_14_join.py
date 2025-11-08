import pytest
from client_helper import IRCClient
from conftest import SERVER_PORT, SERVER_PASSWORD

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

    yield client1, client2

    client1.close()
    client2.close()

def test_join_password_protected_channel_correct_key(registered_clients):
    client1, client2 = registered_clients

    # Client1 creates a key-protected channel
    client1.send("JOIN #keychan")
    client1.wait_for_command("366")
    client1.send("MODE #keychan +k mykey")
    client1.wait_for_command("MODE")

    # Client2 joins with correct key
    client2.send("JOIN #keychan mykey")
    join_msg = client2.wait_for_command("JOIN")
    assert join_msg is not None
    assert join_msg["args"] == ["#keychan"]

def test_join_password_protected_channel_incorrect_key(registered_clients):
    client1, client2 = registered_clients

    # Client1 creates a key-protected channel
    client1.send("JOIN #keychan")
    client1.wait_for_command("366")
    client1.send("MODE #keychan +k mykey")
    client1.wait_for_command("MODE")

    # Client2 tries to join with incorrect key
    client2.send("JOIN #keychan wrongkey")
    error_reply = client2.wait_for_command("475") # ERR_BADCHANNELKEY
    assert error_reply is not None
    assert error_reply["args"] == ["UserB", "#keychan", "Cannot join channel (+k)"]

def test_join_password_protected_channel_no_key(registered_clients):
    client1, client2 = registered_clients

    # Client1 creates a key-protected channel
    client1.send("JOIN #keychan")
    client1.wait_for_command("366")
    client1.send("MODE #keychan +k mykey")
    client1.wait_for_command("MODE")

    # Client2 tries to join without key
    client2.send("JOIN #keychan")
    error_reply = client2.wait_for_command("475") # ERR_BADCHANNELKEY
    assert error_reply is not None
    assert error_reply["args"] == ["UserB", "#keychan", "Cannot join channel (+k)"]

def test_join_limit_channel_full(registered_clients):
    client1, client2 = registered_clients

    # Client1 creates a limit-protected channel with limit 1
    client1.send("JOIN #limitchan")
    client1.wait_for_command("366")
    client1.send("MODE #limitchan +l 1")
    client1.wait_for_command("MODE")

    # Client2 tries to join, but channel is full
    client2.send("JOIN #limitchan")
    error_reply = client2.wait_for_command("471") # ERR_CHANNELISFULL
    assert error_reply is not None
    assert error_reply["args"] == ["UserB", "#limitchan", "Cannot join channel (+l)"]

def test_join_invite_only_channel_not_invited(registered_clients):
    client1, client2 = registered_clients

    # Client1 creates an invite-only channel
    client1.send("JOIN #inviteonly")
    client1.wait_for_command("366")
    client1.send("MODE #inviteonly +i")
    client1.wait_for_command("MODE")

    # Client2 tries to join, but is not invited
    client2.send("JOIN #inviteonly")
    error_reply = client2.wait_for_command("473") # ERR_INVITEONLYCHAN
    assert error_reply is not None
    assert error_reply["args"] == ["UserB", "#inviteonly", "Cannot join channel (+i)"]

def test_join_invite_only_channel_invited(registered_clients):
    client1, client2 = registered_clients

    # Client1 creates an invite-only channel
    client1.send("JOIN #inviteonly")
    client1.wait_for_command("366")
    client1.send("MODE #inviteonly +i")
    client1.wait_for_command("MODE")

    # Client1 invites Client2
    client1.send("INVITE UserB #inviteonly")
    client2.wait_for_command("INVITE")

    # Client2 joins (now invited)
    client2.send("JOIN #inviteonly")
    join_msg = client2.wait_for_command("JOIN")
    assert join_msg is not None
    assert join_msg["args"] == ["#inviteonly"]
