import pytest
from client_helper import IRCClient
from conftest import SERVER_PORT, SERVER_PASSWORD

@pytest.fixture(scope="function")
def clients_for_invite_test(irc_server):
    client_inviter = IRCClient(SERVER_PORT, "Inviter")
    client_invitee = IRCClient(SERVER_PORT, "Invitee")
    client_other = IRCClient(SERVER_PORT, "OtherUser")

    client_inviter.connect()
    client_invitee.connect()
    client_other.connect()

    client_inviter.register(SERVER_PASSWORD)
    client_invitee.register(SERVER_PASSWORD)
    client_other.register(SERVER_PASSWORD)

    # Inviter creates the channel and becomes operator
    client_inviter.send("JOIN #test_channel")
    client_inviter.wait_for_command("366")

    # OtherUser joins the channel as a regular member
    client_other.send("JOIN #test_channel")
    client_other.wait_for_command("366")

    # Clear buffers
    for client in [client_inviter, client_invitee, client_other]:
        while client.get_message(timeout=0.1) is not None: pass

    yield client_inviter, client_invitee, client_other

    for client in [client_inviter, client_invitee, client_other]:
        client.close()

def test_invite_success(clients_for_invite_test):
    inviter, invitee, _ = clients_for_invite_test

    inviter.send("INVITE Invitee #test_channel")

    # Invitee should receive the INVITE message
    invite_msg = invitee.wait_for_command("INVITE")
    assert invite_msg is not None
    assert invite_msg["prefix"]["nick"] == "Inviter"
    assert invite_msg["args"] == ["Invitee", "#test_channel"]

    # Inviter should not receive any error or confirmation (RFC behavior)
    assert inviter.get_message(timeout=0.2) is None

def test_invite_errors(clients_for_invite_test):
    inviter, invitee, other_user = clients_for_invite_test

    # ERR_NEEDMOREPARAMS (461)
    inviter.send("INVITE Invitee") # Missing channel
    error_reply = inviter.wait_for_command("461")
    assert error_reply is not None
    assert error_reply["args"] == ["Inviter", "INVITE", "Not enough parameters"]

    # ERR_NOSUCHNICK (401)
    inviter.send("INVITE NonExistentUser #test_channel")
    error_reply = inviter.wait_for_command("401")
    assert error_reply is not None
    assert error_reply["args"] == ["Inviter", "NonExistentUser", "No such nick/channel"]

    # ERR_NOTONCHANNEL (442) - Inviter not in channel
    inviter.send("PART #test_channel")
    inviter.wait_for_command("PART")
    inviter.send("INVITE Invitee #test_channel")
    error_reply = inviter.wait_for_command("442")
    assert error_reply is not None
    assert error_reply["args"] == ["Inviter", "#test_channel", "You're not on that channel"]
    inviter.send("JOIN #test_channel") # Rejoin for subsequent tests
    inviter.wait_for_command("366")

    # ERR_USERONCHANNEL (443) - Invitee already in channel
    invitee.send("JOIN #test_channel")
    invitee.wait_for_command("366")
    inviter.send("INVITE Invitee #test_channel")
    error_reply = inviter.wait_for_command("443")
    assert error_reply is not None
    assert error_reply["args"] == ["Inviter", "Invitee", "#test_channel", "is already on channel"]
    invitee.send("PART #test_channel") # Part for subsequent tests
    invitee.wait_for_command("PART")

    # ERR_CHANOPRIVSNEEDED (482) - Invite-only channel, inviter not op
    # チャンネルの状態をリセットし、Inviterがオペレーターであることを保証
    inviter.send("PART #test_channel")
    inviter.wait_for_command("PART")
    other_user.send("PART #test_channel")
    other_user.wait_for_command("PART")
    invitee.send("PART #test_channel") # 以前のテストで参加している可能性があるので念のため
    invitee.wait_for_command("PART", timeout=0.5)

    # Inviterがチャンネルを再作成し、オペレーターになる
    inviter.send("JOIN #test_channel")
    inviter.wait_for_command("366")

    # OtherUserが一般メンバーとして参加
    other_user.send("JOIN #test_channel")
    other_user.wait_for_command("366")

    # バッファをクリア
    for client in [inviter, invitee, other_user]:
        while client.get_message(timeout=0.1) is not None: pass

    # Inviter (オペレーター) がチャンネルを招待制 (+i) に設定
    inviter.send("MODE #test_channel +i")
    inviter.wait_for_command("MODE") # Inviter自身へのブロードキャスト
    other_user.wait_for_command("MODE") # OtherUserへのブロードキャスト

    # OtherUser (非オペレーター) がInviteeを招待制チャンネルに招待しようとする
    other_user.send("INVITE Invitee #test_channel")
    error_reply = other_user.wait_for_command("482") # ERR_CHANOPRIVSNEEDED
    assert error_reply is not None
    assert error_reply["args"] == ["OtherUser", "#test_channel", "You're not an operator on this channel"]

    # 次のテストのためにチャンネルモードをリセット
    inviter.send("MODE #test_channel -i")
    inviter.wait_for_command("MODE")

def test_invite_only_channel_op_can_invite(clients_for_invite_test):
    inviter, invitee, _ = clients_for_invite_test

    # Set channel to invite-only
    inviter.send("MODE #test_channel +i")
    inviter.wait_for_command("MODE")

    # Inviter (who is op) invites invitee
    inviter.send("INVITE Invitee #test_channel")
    invite_msg = invitee.wait_for_command("INVITE")
    assert invite_msg is not None

    # Invited user can now join
    invitee.send("JOIN #test_channel")
    join_msg = invitee.wait_for_command("JOIN")
    assert join_msg is not None
    assert join_msg["args"] == ["#test_channel"]
