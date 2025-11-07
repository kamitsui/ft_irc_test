import pytest
from client_helper import IRCClient
from conftest import SERVER_PORT, SERVER_PASSWORD

def test_join_multiple_channels(irc_server):
    """
    Tests joining multiple channels with a single JOIN command.
    """
    client = IRCClient(SERVER_PORT, "m_joiner")
    client.connect()
    client.register(SERVER_PASSWORD)

    client.send("JOIN #chan1,#chan2")

    # Check for JOIN confirmations for both channels
    join1 = client.wait_for_command("JOIN")
    names1 = client.wait_for_command("366") # End of NAMES for #chan1
    join2 = client.wait_for_command("JOIN")
    names2 = client.wait_for_command("366") # End of NAMES for #chan2

    assert join1 is not None and "#chan1" in join1["args"]
    assert names1 is not None and "#chan1" in names1["args"]
    assert join2 is not None and "#chan2" in join2["args"]
    assert names2 is not None and "#chan2" in names2["args"]

    client.close()

def test_part_multiple_channels(irc_server):
    """
    Tests parting multiple channels with a single PART command.
    """
    client1 = IRCClient(SERVER_PORT, "m_parter")
    client2 = IRCClient(SERVER_PORT, "observer")
    client1.connect()
    client2.connect()
    client1.register(SERVER_PASSWORD)
    client2.register(SERVER_PASSWORD)

    # Both clients join two channels
    client1.send("JOIN #chan1,#chan2")
    client1.wait_for_command("366") # Wait for first channel
    client1.wait_for_command("366") # Wait for second channel
    client2.send("JOIN #chan1,#chan2")
    client2.wait_for_command("366")
    client2.wait_for_command("366")

    # Clear observer's buffer
    while client2.get_message(timeout=0.1) is not None: pass

    # client1 parts both channels
    client1.send("PART #chan1,#chan2 :Leaving")

    # Check if observer received both PART notifications
    part1 = client2.wait_for_command("PART")
    part2 = client2.wait_for_command("PART")

    assert part1 is not None and part1["args"][0] == "#chan1"
    assert part2 is not None and part2["args"][0] == "#chan2"
    assert part1["prefix"]["nick"] == "m_parter"
    assert part2["prefix"]["nick"] == "m_parter"

    client1.close()
    client2.close()

def test_join_zero_parts_all_channels(irc_server):
    """
    Tests if 'JOIN 0' correctly parts the client from all joined channels.
    """
    client1 = IRCClient(SERVER_PORT, "z_joiner")
    client2 = IRCClient(SERVER_PORT, "observer")
    client1.connect()
    client2.connect()
    client1.register(SERVER_PASSWORD)
    client2.register(SERVER_PASSWORD)

    # Both clients join two channels
    client1.send("JOIN #chan1,#chan2")
    client1.wait_for_command("366")
    client1.wait_for_command("366")
    client2.send("JOIN #chan1,#chan2")
    client2.wait_for_command("366")
    client2.wait_for_command("366")

    # Clear observer's buffer
    while client2.get_message(timeout=0.1) is not None: pass

    # client1 sends 'JOIN 0'
    client1.send("JOIN 0")

    # Check if observer received PART notifications for both channels
    part1 = client2.wait_for_command("PART")
    part2 = client2.wait_for_command("PART")

    assert part1 is not None
    assert part2 is not None
    
    # The order of parting is not guaranteed, so check for both channels
    parted_channels = {part1["args"][0], part2["args"][0]}
    assert "#chan1" in parted_channels
    assert "#chan2" in parted_channels

    client1.close()
    client2.close()
