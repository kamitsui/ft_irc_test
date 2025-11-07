import pytest
from client_helper import IRCClient
from conftest import SERVER_PORT, SERVER_PASSWORD

@pytest.fixture(scope="function")
def clients_for_kick_test(irc_server):
    """
    Sets up clients for KICK command testing:
    - Operator: Joins #test and is an operator.
    - Member: Joins #test as a regular user.
    - Target: Joins #test as the user to be kicked.
    - NotInChannel: Registered but not in any channel.
    """
    clients = {
        "operator": IRCClient(SERVER_PORT, "Operator"),
        "member": IRCClient(SERVER_PORT, "Member"),
        "target": IRCClient(SERVER_PORT, "Target"),
        "not_in_channel": IRCClient(SERVER_PORT, "NoChanUsr")
    }

    for client in clients.values():
        client.connect()
        client.register(SERVER_PASSWORD)

    # Operator creates the channel
    clients["operator"].send("JOIN #test")
    clients["operator"].wait_for_command("366")

    # Member and Target join
    clients["member"].send("JOIN #test")
    clients["member"].wait_for_command("366")
    clients["target"].send("JOIN #test")
    clients["target"].wait_for_command("366")

    # Clear any post-join messages
    for client in clients.values():
        while client.get_message(timeout=0.1) is not None: pass

    yield clients

    for client in clients.values():
        client.close()

def test_kick_success_with_reason(clients_for_kick_test):
    op = clients_for_kick_test["operator"]
    member = clients_for_kick_test["member"]
    target = clients_for_kick_test["target"]

    op.send("KICK #test Target :Go away!")

    # All three (op, member, target) should receive the KICK message
    kick_msg_op = op.wait_for_command("KICK")
    kick_msg_member = member.wait_for_command("KICK")
    kick_msg_target = target.wait_for_command("KICK")

    assert kick_msg_op["args"] == ["#test", "Target", "Go away!"]
    assert kick_msg_member["args"] == ["#test", "Target", "Go away!"]
    assert kick_msg_target["args"] == ["#test", "Target", "Go away!"]

    # Target should be removed from the channel
    target.send("NAMES #test")
    # Expect no reply or an error, but definitely not a valid NAMES list for #test
    # A simple way to check is to see if they can send a message
    target.send("PRIVMSG #test :Are you still there?")
    error_reply = target.wait_for_command("404") # ERR_CANNOTSENDTOCHAN
    assert error_reply is not None

def test_kick_not_operator(clients_for_kick_test):
    member = clients_for_kick_test["member"]
    target = clients_for_kick_test["target"]

    member.send("KICK #test Target")
    
    error_reply = member.wait_for_command("482") # ERR_CHANOPRIVSNEEDED
    assert error_reply is not None
    assert error_reply["args"] == ["Member", "#test", "You're not an operator on this channel"]

    # Target should still be in the channel, check by sending NAMES
    target.send("NAMES #test")
    names_reply = target.wait_for_command("353")
    assert names_reply is not None

def test_kick_errors(clients_for_kick_test):
    op = clients_for_kick_test["operator"]
    not_in_channel = clients_for_kick_test["not_in_channel"]

    # ERR_USERNOTINCHANNEL (441)
    op.send(f"KICK #test {not_in_channel.nick}")
    error_reply = op.wait_for_command("441")
    assert error_reply is not None
    assert error_reply["args"] == ["Operator", not_in_channel.nick, "#test", "They aren't on that channel"]

    # ERR_NOSUCHNICK (401)
    op.send("KICK #test NonExistentUser")
    error_reply = op.wait_for_command("401")
    assert error_reply is not None
    assert error_reply["args"] == ["Operator", "NonExistentUser", "No such nick/channel"]

    # ERR_NOSUCHCHANNEL (403)
    op.send("KICK #nonexistent Target")
    error_reply = op.wait_for_command("403")
    assert error_reply is not None
    assert error_reply["args"] == ["Operator", "#nonexistent", "No such channel"]

    # ERR_NEEDMOREPARAMS (461)
    op.send("KICK #test")
    error_reply = op.wait_for_command("461")
    assert error_reply is not None
    assert error_reply["args"] == ["Operator", "KICK", "Not enough parameters"]

def test_kick_multiple_users(clients_for_kick_test):
    op = clients_for_kick_test["operator"]
    member = clients_for_kick_test["member"]
    target = clients_for_kick_test["target"]

    # Kick both member and target
    op.send(f"KICK #test {member.nick},{target.nick} :Multi-kick")

    # Check for member's kick message
    kick_msg_for_member = op.wait_for_command("KICK")
    assert kick_msg_for_member["args"] == ["#test", member.nick, "Multi-kick"]
    
    # Check for target's kick message
    kick_msg_for_target = op.wait_for_command("KICK")
    assert kick_msg_for_target["args"] == ["#test", target.nick, "Multi-kick"]

    # Verify both are out of the channel
    member.send("PRIVMSG #test :test")
    error_reply = member.wait_for_command("404")
    assert error_reply is not None

    target.send("PRIVMSG #test :test")
    error_reply = target.wait_for_command("404")
    assert error_reply is not None