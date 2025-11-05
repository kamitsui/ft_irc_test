import pytest
from client_helper import IRCClient
from conftest import SERVER_PORT, SERVER_PASSWORD

@pytest.fixture(scope="function")
def clients_in_channel(irc_server):
    """
    Sets up two clients, both registered and joined to a channel '#test'.
    """
    client1 = IRCClient(SERVER_PORT, "userA")
    client2 = IRCClient(SERVER_PORT, "userB")
    client1.connect()
    client2.connect()
    client1.register(SERVER_PASSWORD)
    client2.register(SERVER_PASSWORD)

    client1.send("JOIN #test")
    client1.wait_for_command("366") # Wait for UserA's JOIN to complete
    client2.send("JOIN #test")
    client2.wait_for_command("366") # Wait for UserB's JOIN to complete

    # Clear buffers from join messages
    while client1.get_message(timeout=0.1) is not None: pass
    while client2.get_message(timeout=0.1) is not None: pass

    yield client1, client2

    client1.close()
    client2.close()

def test_topic_view_no_topic(clients_in_channel):
    """
    Tests viewing topic when none is set. Expects RPL_NOTOPIC.
    """
    client1, _ = clients_in_channel
    client1.send("TOPIC #test")

    reply = client1.wait_for_command("331") # RPL_NOTOPIC
    assert reply is not None
    assert reply["args"] == ["userA", "#test", "No topic is set"]

def test_topic_set_and_view(clients_in_channel):
    """
    Tests setting a topic and then viewing it.
    Also tests that other clients in the channel receive the TOPIC broadcast.
    """
    client1, client2 = clients_in_channel

    # Client1 sets the topic
    client1.send("TOPIC #test :This is a cool topic!")

    # Client2 should receive the broadcast
    broadcast = client2.wait_for_command("TOPIC")
    assert broadcast is not None
    assert broadcast["prefix"].startswith("userA!")
    assert broadcast["args"] == ["#test", "This is a cool topic!"]

    # Client1 now views the topic
    client1.send("TOPIC #test")
    reply = client1.wait_for_command("332") # RPL_TOPIC
    assert reply is not None
    assert reply["args"] == ["userA", "#test", "This is a cool topic!"]

def test_join_gets_topic(irc_server):
    """
    Tests that a client joining a channel with a topic receives RPL_TOPIC.
    """
    client1 = IRCClient(SERVER_PORT, "userA")
    client2 = IRCClient(SERVER_PORT, "userB")
    client1.connect()
    client2.connect()
    client1.register(SERVER_PASSWORD)
    client2.register(SERVER_PASSWORD)

    client1.send("JOIN #test")
    client1.wait_for_command("366")
    client1.send("TOPIC #test :Welcome!")
    # Wait for confirmation broadcast to self
    client1.wait_for_command("TOPIC")

    # Now client2 joins
    client2.send("JOIN #test")
    
    # Client2 should receive RPL_TOPIC upon joining
    topic_reply = client2.wait_for_command("332")
    assert topic_reply is not None
    assert topic_reply["args"] == ["userB", "#test", "Welcome!"]

    client1.close()
    client2.close()

def test_topic_errors(irc_server):
    """
    Tests ERR_NOTONCHANNEL and ERR_NEEDMOREPARAMS for TOPIC command.
    """
    client1 = IRCClient(SERVER_PORT, "userA") # In channel
    client2 = IRCClient(SERVER_PORT, "userB") # Not in channel
    client1.connect()
    client2.connect()
    client1.register(SERVER_PASSWORD)
    client2.register(SERVER_PASSWORD)

    client1.send("JOIN #test")
    client1.wait_for_command("366")

    # Test ERR_NOTONCHANNEL
    client2.send("TOPIC #test :Hello from outside")
    err_reply = client2.wait_for_command("442") # ERR_NOTONCHANNEL
    assert err_reply is not None
    assert err_reply["args"] == ["userB", "#test", "You're not on that channel"]

    # Test ERR_NEEDMOREPARAMS
    client1.send("TOPIC")
    err_reply = client1.wait_for_command("461") # ERR_NEEDMOREPARAMS
    assert err_reply is not None
    assert err_reply["args"] == ["userA", "TOPIC", "Not enough parameters"]

    client1.close()
    client2.close()
