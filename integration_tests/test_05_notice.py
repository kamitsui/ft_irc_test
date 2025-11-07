import pytest
from client_helper import IRCClient
from conftest import SERVER_PORT, SERVER_PASSWORD

def test_notice_to_user(irc_server):
    """
    Tests sending a NOTICE from one user to another.
    """
    client1 = IRCClient(SERVER_PORT, "sender")
    client2 = IRCClient(SERVER_PORT, "receiver")
    client1.connect()
    client2.connect()
    client1.register(SERVER_PASSWORD)
    client2.register(SERVER_PASSWORD)

    client1.send("NOTICE receiver :Hello, this is a notice.")
    
    msg = client2.wait_for_command("NOTICE")
    assert msg is not None
    assert msg["prefix"]["nick"] == "sender"
    assert msg["args"] == ["receiver", "Hello, this is a notice."]

    client1.close()
    client2.close()

def test_notice_to_channel(irc_server):
    """
    Tests sending a NOTICE to a channel, expecting other members to receive it.
    """
    client1 = IRCClient(SERVER_PORT, "sender")
    client2 = IRCClient(SERVER_PORT, "receiver")
    client1.connect()
    client2.connect()
    client1.register(SERVER_PASSWORD)
    client2.register(SERVER_PASSWORD)

    client1.send("JOIN #test")
    client1.wait_for_command("366") # End of NAMES
    client2.send("JOIN #test")
    client2.wait_for_command("366") # End of NAMES

    # Clear any JOIN messages from client2's buffer
    while client2.get_message(timeout=0.1) is not None:
        pass

    client1.send("NOTICE #test :This is a channel notice.")

    msg = client2.wait_for_command("NOTICE")
    assert msg is not None
    assert msg["prefix"]["nick"] == "sender"
    assert msg["args"] == ["#test", "This is a channel notice."]

    # Sender should not receive their own notice
    sender_msg = client1.get_message(timeout=0.2)
    assert sender_msg is None or sender_msg["command"] != "NOTICE"

    client1.close()
    client2.close()

def test_notice_to_nonexistent_target_sends_no_error(irc_server):
    """
    Tests that sending a NOTICE to a non-existent user or channel
    does not result in an error message being sent back to the sender.
    """
    client = IRCClient(SERVER_PORT, "sender")
    client.connect()
    client.register(SERVER_PASSWORD)

    # Send NOTICE to a nick that doesn't exist
    client.send("NOTICE nonexistent_nick :Are you there?")
    
    # Send NOTICE to a channel that doesn't exist
    client.send("NOTICE #nonexistent_channel :Anyone here?")

    # We should not receive any error messages (like 401 or 403)
    # We wait for a short period to see if any message arrives.
    msg = client.get_message(timeout=0.5)
    assert msg is None, f"Server sent an unexpected reply: {msg}"

    client.close()
