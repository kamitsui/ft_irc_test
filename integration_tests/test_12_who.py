import pytest
from client_helper import IRCClient
from conftest import SERVER_PORT, SERVER_PASSWORD
import time

def test_who_command(irc_server):
    """
    Tests the WHO command functionality.
    1. Clients A, B, C, D connect and register.
    2. Client A (operator) and B (regular) join #test_channel.
    3. Client C is registered but not in any channel.
    4. Client D sends WHO #test_channel and verifies A and B's info.
    5. Client D sends WHO ClientC and verifies C's info.
    6. Client D sends WHO non_existent_nick and verifies only RPL_ENDOFWHO.
    """
    # 1. Connect and register clients
    client_a = IRCClient(SERVER_PORT, "ClientA")
    client_b = IRCClient(SERVER_PORT, "ClientB")
    client_c = IRCClient(SERVER_PORT, "ClientC")
    client_d = IRCClient(SERVER_PORT, "ClientD")
    
    client_a.connect()
    client_b.connect()
    client_c.connect()
    client_d.connect()

    client_a.register(SERVER_PASSWORD)
    client_b.register(SERVER_PASSWORD)
    client_c.register(SERVER_PASSWORD)
    client_d.register(SERVER_PASSWORD)

    # 2. Client A (operator) and B (regular) join #test_channel
    client_a.send("JOIN #test_channel")
    assert client_a.wait_for_command("366") is not None, "ClientA failed to join #test_channel"
    client_b.send("JOIN #test_channel")
    assert client_b.wait_for_command("366") is not None, "ClientB failed to join #test_channel"

    # Make ClientA an operator (it should be by default as first joiner, but explicitly set for clarity)
    # This is handled by the server logic, first joiner is op.

    # 3. Client C is registered but not in any channel (already done)

    # 4. Client D sends WHO #test_channel and verifies A and B's info.
    client_d.send("WHO #test_channel")
    
    who_replies = []
    # Expect 2 RPL_WHOREPLY (352) messages
    for _ in range(2):
        msg = client_d.wait_for_command("352")
        assert msg is not None, "Did not receive expected RPL_WHOREPLY (352) for #test_channel"
        who_replies.append(msg)
    
    assert client_d.wait_for_command("315") is not None, "Did not receive RPL_ENDOFWHO (315) for #test_channel"

    # Verify ClientA (operator) and ClientB (regular member)
    client_a_found = False
    client_b_found = False
    for reply in who_replies:
        # Correct indices are: [0]=target_client_nick (ClientD), [1]=channel, [2]=user, [3]=host, [4]=server, [5]=nick, [6]=status+hopcount+realname
        if reply['args'][2] == "ClientA" and reply['args'][5] == "ClientA" and "H@" in reply['args'][6]:
            client_a_found = True
        if reply['args'][2] == "ClientB" and reply['args'][5] == "ClientB" and "H" in reply['args'][6] and "@" not in reply['args'][6]:
            client_b_found = True
    
    assert client_a_found, "ClientA's WHO reply not found or incorrect status for operator"
    assert client_b_found, "ClientB's WHO reply not found or incorrect status for regular member"

    # 5. Client D sends WHO ClientC and verifies C's info.
    client_d.send("WHO ClientC")
    
    msg_c = client_d.wait_for_command("352")
    assert msg_c is not None, "Did not receive RPL_WHOREPLY (352) for ClientC"
    assert client_d.wait_for_command("315") is not None, "Did not receive RPL_ENDOFWHO (315) for ClientC"

    # Verify ClientC (server-only)
    assert msg_c['args'][1] == "*", "ClientC's channel should be *"
    assert msg_c['args'][2] == "ClientC", "ClientC's user field incorrect"
    assert msg_c['args'][5] == "ClientC", "ClientC's nick field incorrect"
    assert "H" in msg_c['args'][6] and "@" not in msg_c['args'][6], "ClientC's status flags incorrect"

    # 6. Client D sends WHO non_existent_nick and verifies only RPL_ENDOFWHO.
    client_d.send("WHO non_existent_nick")
    
    # Expect only RPL_ENDOFWHO (315)
    msg_end = client_d.wait_for_command("315")
    assert msg_end is not None, "Did not receive RPL_ENDOFWHO (315) for non_existent_nick"
    assert msg_end['args'][1] == "non_existent_nick", "RPL_ENDOFWHO parameter incorrect for non_existent_nick"    
    # Ensure no other messages are received
    assert client_d.get_message(timeout=0.1) is None, "Received unexpected message after RPL_ENDOFWHO for non_existent_nick"

    # Clean up
    client_a.close()
    client_b.close()
    client_c.close()
    client_d.close()
