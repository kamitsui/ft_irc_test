import pytest
from client_helper import IRCClient
from conftest import SERVER_PORT, SERVER_PASSWORD
import time

def test_whois_command(irc_server):
    """
    Tests the WHOIS command functionality.
    1. Clients A and B connect and register.
    2. Client A joins #chan1 (as operator) and #chan2 (as regular member).
    3. Client B sends WHOIS for ClientA and verifies the responses.
    4. Client B sends WHOIS for a non-existent user and verifies the error.
    """
    # 1. Connect and register clients
    client_a = IRCClient(SERVER_PORT, "ClientA")
    client_b = IRCClient(SERVER_PORT, "ClientB")
    
    client_a.connect()
    client_b.connect()

    client_a.register(SERVER_PASSWORD, "User A Real Name")
    client_b.register(SERVER_PASSWORD)

    # 2. Client A joins channels
    client_a.send("JOIN #chan1")
    assert client_a.wait_for_command("366") is not None, "ClientA failed to join #chan1"
    client_a.send("JOIN #chan2")
    assert client_a.wait_for_command("366") is not None, "ClientA failed to join #chan2"

    # Wait a moment for server to process joins
    time.sleep(0.2)

    # 3. Client B sends WHOIS for ClientA
    client_b.send("WHOIS ClientA")

    # Collect all WHOIS replies until RPL_ENDOFWHOIS (318)
    whois_replies = {}
    while True:
        msg = client_b.get_message(timeout=1.0)
        assert msg is not None, "Timed out waiting for WHOIS replies"
        whois_replies[msg['command']] = msg
        if msg['command'] == "318": # RPL_ENDOFWHOIS
            break
    
    # --- Verify the replies ---
    
    # RPL_WHOISUSER (311)
    assert "311" in whois_replies, "Did not receive RPL_WHOISUSER (311)"
    reply311 = whois_replies["311"]
    assert reply311['args'][1] == "ClientA" # Nick
    assert reply311['args'][2] == "ClientA" # User
    assert "User A Real Name" in reply311['args'][5] # Real Name

    # RPL_WHOISCHANNELS (319)
    assert "319" in whois_replies, "Did not receive RPL_WHOISCHANNELS (319)"
    reply319 = whois_replies["319"]
    assert reply319['args'][1] == "ClientA" # Nick
    # Channel list order is not guaranteed
    assert "@#chan1" in reply319['args'][2]
    assert " #chan2" in reply319['args'][2] or reply319['args'][2].endswith("#chan2")


    # RPL_WHOISIDLE (317) - Just check it exists and has the right format
    assert "317" in whois_replies, "Did not receive RPL_WHOISIDLE (317)"
    reply317 = whois_replies["317"]
    assert reply317['args'][1] == "ClientA" # Nick
    assert reply317['args'][3] == "seconds idle"

    # RPL_ENDOFWHOIS (318)
    assert "318" in whois_replies, "Did not receive RPL_ENDOFWHOIS (318)"
    reply318 = whois_replies["318"]
    assert reply318['args'][1] == "ClientA" # Nick
    assert "End of WHOIS list" in reply318['args'][2]

    # 4. Client B sends WHOIS for a non-existent user
    client_b.send("WHOIS NonExistentUser")
    
    err_msg = client_b.wait_for_command("401") # ERR_NOSUCHNICK
    assert err_msg is not None, "Did not receive ERR_NOSUCHNICK (401)"
    assert err_msg['args'][1] == "NonExistentUser"
    assert "No such nick/channel" in err_msg['args'][2]

    # Clean up
    client_a.close()
    client_b.close()
