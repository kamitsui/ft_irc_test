import pytest
from client_helper import IRCClient
from conftest import SERVER_PORT, SERVER_PASSWORD

def test_names_command_single_channel(irc_server):
    """
    Tests NAMES command for a single channel.
    """
    client1 = IRCClient(SERVER_PORT, "user1")
    client2 = IRCClient(SERVER_PORT, "user2")
    client1.connect()
    client2.connect()
    client1.register(SERVER_PASSWORD)
    client2.register(SERVER_PASSWORD)

    client1.send("JOIN #test_chan")
    client1.wait_for_command("366") # End of NAMES
    client2.send("JOIN #test_chan")
    client2.wait_for_command("366") # End of NAMES

    # Clear client1's buffer
    while client1.get_message(timeout=0.1) is not None: pass

    client1.send("NAMES #test_chan")

    names_reply = client1.wait_for_command("353")
    end_names_reply = client1.wait_for_command("366")

    assert names_reply is not None
    assert names_reply["command"] == "353"
    assert names_reply["args"][0] == "user1"
    assert names_reply["args"][1] == "="
    assert names_reply["args"][2] == "#test_chan"
    # Order of users might vary, so check for presence
    users_in_channel = names_reply["args"][3].split()
    assert "@user1" in users_in_channel or "user1" in users_in_channel
    assert "user2" in users_in_channel

    assert end_names_reply is not None
    assert end_names_reply["command"] == "366"
    assert end_names_reply["args"][0] == "user1"
    assert end_names_reply["args"][1] == "#test_chan"
    assert end_names_reply["args"][2] == "End of NAMES list"

    client1.close()
    client2.close()

def test_names_command_multiple_channels(irc_server):
    """
    Tests NAMES command for multiple channels.
    """
    client1 = IRCClient(SERVER_PORT, "user1")
    client2 = IRCClient(SERVER_PORT, "user2")
    client1.connect()
    client2.connect()
    client1.register(SERVER_PASSWORD)
    client2.register(SERVER_PASSWORD)

    client1.send("JOIN #chan1,#chan2")
    client1.wait_for_command("366")
    client1.wait_for_command("366")
    client2.send("JOIN #chan1") # client2 only joins #chan1
    client2.wait_for_command("366")

    # Clear client1's buffer
    while client1.get_message(timeout=0.1) is not None: pass

    client1.send("NAMES #chan1,#chan2")

    # Expect 2x 353 and 2x 366 replies
    replies = []
    for _ in range(4):
        msg = client1.get_message(timeout=2) # Use get_message with a timeout
        if msg:
            replies.append(msg)
        else:
            pytest.fail("Did not receive all 4 expected replies for NAMES command")

    assert len(replies) == 4

    # Check for #chan1 replies
    chan1_names = [r for r in replies if r.get("command") == "353" and r["args"][2] == "#chan1"]
    chan1_end = [r for r in replies if r.get("command") == "366" and r["args"][1] == "#chan1"]
    assert len(chan1_names) == 1
    assert len(chan1_end) == 1
    users_in_chan1 = chan1_names[0]["args"][3].split()
    assert "@user1" in users_in_chan1 or "user1" in users_in_chan1
    assert "user2" in users_in_chan1

    # Check for #chan2 replies
    chan2_names = [r for r in replies if r.get("command") == "353" and r["args"][2] == "#chan2"]
    chan2_end = [r for r in replies if r.get("command") == "366" and r["args"][1] == "#chan2"]
    assert len(chan2_names) == 1
    assert len(chan2_end) == 1
    users_in_chan2 = chan2_names[0]["args"][3].split()
    assert "@user1" in users_in_chan2 or "user1" in users_in_chan2
    assert "user2" not in users_in_chan2 # user2 is not in #chan2

    client1.close()
    client2.close()

def test_names_command_no_parameters_lists_all_channels(irc_server):
    """
    Tests NAMES command without parameters lists all visible channels.
    """
    client1 = IRCClient(SERVER_PORT, "user1")
    client2 = IRCClient(SERVER_PORT, "user2")
    client1.connect()
    client2.connect()
    client1.register(SERVER_PASSWORD)
    client2.register(SERVER_PASSWORD)

    client1.send("JOIN #chanA,#chanB")
    client1.wait_for_command("366")
    client1.wait_for_command("366")
    client2.send("JOIN #chanA")
    client2.wait_for_command("366")

    # Clear client1's buffer
    while client1.get_message(timeout=0.1) is not None: pass

    client1.send("NAMES") # No parameters

    # Expect replies for all channels (2x 353 and 2x 366)
    replies = []
    for _ in range(4):
        msg = client1.get_message(timeout=2) # Use get_message with a timeout
        if msg:
            replies.append(msg)
        else:
            pytest.fail("Did not receive all 4 expected replies for NAMES command")
    
    assert len(replies) == 4

    # Check for #chanA replies
    chanA_names = [r for r in replies if r.get("command") == "353" and r["args"][2] == "#chanA"]
    chanA_end = [r for r in replies if r.get("command") == "366" and r["args"][1] == "#chanA"]
    assert len(chanA_names) == 1
    assert len(chanA_end) == 1
    users_in_chanA = chanA_names[0]["args"][3].split()
    assert "@user1" in users_in_chanA or "user1" in users_in_chanA
    assert "user2" in users_in_chanA

    # Check for #chanB replies
    chanB_names = [r for r in replies if r.get("command") == "353" and r["args"][2] == "#chanB"]
    chanB_end = [r for r in replies if r.get("command") == "366" and r["args"][1] == "#chanB"]
    assert len(chanB_names) == 1
    assert len(chanB_end) == 1
    users_in_chanB = chanB_names[0]["args"][3].split()
    assert "@user1" in users_in_chanB or "user1" in users_in_chanB
    assert "user2" not in users_in_chanB # user2 is not in #chanB

    client1.close()
    client2.close()
