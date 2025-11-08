import pytest
from client_helper import IRCClient
from conftest import SERVER_PORT, SERVER_PASSWORD
import time

def test_list_command(irc_server):
    """
    Tests the LIST command functionality.
    1. Client A, B, C connect and register.
    2. Client B creates and joins #chan1.
    3. Client C creates and joins #chan2 and sets a topic.
    4. Client A sends LIST and receives a list of both channels.
    5. Client A sends LIST #chan2 and receives info only for #chan2.
    """
    # 1. Connect and register clients
    client_a = IRCClient(SERVER_PORT, "ClientA")
    client_b = IRCClient(SERVER_PORT, "ClientB")
    client_c = IRCClient(SERVER_PORT, "ClientC")
    
    client_a.connect()
    client_b.connect()
    client_c.connect()

    client_a.register(SERVER_PASSWORD)
    client_b.register(SERVER_PASSWORD)
    client_c.register(SERVER_PASSWORD)

    # 2. Client B creates and joins #chan1
    client_b.send("JOIN #chan1")
    assert client_b.wait_for_command("366") is not None, "ClientB failed to join #chan1"

    # 3. Client C creates and joins #chan2, and sets a topic
    client_c.send("JOIN #chan2")
    assert client_c.wait_for_command("366") is not None, "ClientC failed to join #chan2"
    client_c.send("TOPIC #chan2 :This is a test topic")
    # TOPIC broadcast is sent to channel members, not back to the sender.
    # We'll just wait a moment to ensure the server processes it.
    time.sleep(0.2)

    # 4. Client A sends LIST
    client_a.send("LIST")
    
    # Collect all messages until RPL_LISTEND (323)
    list_replies = []
    assert client_a.wait_for_command("321") is not None, "Did not receive RPL_LISTSTART (321)"
    
    # There should be two RPL_LIST messages for the two channels
    for _ in range(2):
        msg = client_a.wait_for_command("322")
        assert msg is not None, "Did not receive expected number of RPL_LIST (322) messages"
        list_replies.append(msg)

    assert client_a.wait_for_command("323") is not None, "Did not receive RPL_LISTEND (323)"

    # Assertions for the full list
    reply_texts = [" ".join(msg['args']) for msg in list_replies]
    
    chan1_found = any("#chan1 1" in text for text in reply_texts)
    chan2_found = any("#chan2 1 This is a test topic" in text for text in reply_texts)
    
    assert chan1_found, "LIST response did not contain #chan1"
    assert chan2_found, "LIST response did not contain #chan2 with topic"

    # 5. Client A sends LIST for a specific channel
    client_a.send("LIST #chan2")
    
    # Wait for the specific list
    assert client_a.wait_for_command("321") is not None
    
    specific_msg = client_a.wait_for_command("322")
    assert specific_msg is not None
    
    assert client_a.wait_for_command("323") is not None

    # Assertions for the specific list
    specific_reply_text = " ".join(specific_msg['args'])
    assert "#chan1" not in specific_reply_text
    assert "#chan2 1 This is a test topic" in specific_reply_text

    # Clean up
    client_a.close()
    client_b.close()
    client_c.close()