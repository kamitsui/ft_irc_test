import pytest
from irc_test_utils import IrcClient
import time

# pytest fixture to start and stop the IRC server for each test


def test_successful_registration(irc_server):
    client = IrcClient()
    client.connect()

    # Send registration commands
    client.send_command("PASS testpass")
    client.send_command("NICK testnick")
    client.send_command("USER testuser 0 * :Test User")

    # Give server time to process and respond
    time.sleep(0.5)

    responses = client.get_full_response()
    print("Client responses:", responses)

    # Check for RPL_WELCOME (001) message
    # The exact format might vary, but it should contain "001" and the nickname
    assert any("001 testnick" in r for r in responses), "RPL_WELCOME (001) not received"

    client.disconnect()

def test_nick_collision(irc_server):
    client1 = IrcClient()
    client1.connect()
    client1.send_command("PASS testpass")
    client1.send_command("NICK testnick")
    client1.send_command("USER testuser1 0 * :Test User 1")
    time.sleep(0.5)
    responses1 = client1.get_full_response()
    assert any("001 testnick" in r for r in responses1), "Client 1: RPL_WELCOME (001) not received"

    client2 = IrcClient()
    client2.connect()
    client2.send_command("PASS testpass")
    client2.send_command("NICK testnick") # Attempt to use same nickname
    client2.send_command("USER testuser2 0 * :Test User 2")
    time.sleep(0.5)
    responses2 = client2.get_full_response()
    print("Client 2 responses:", responses2)

    # Check for ERR_NICKNAMEINUSE (433)
    assert any("433 testnick" in r for r in responses2), "ERR_NICKNAMEINUSE (433) not received for client 2"

    client1.disconnect()
    client2.disconnect()

def test_missing_password(irc_server):
    client = IrcClient()
    client.connect()

    client.send_command("NICK testnick")
    client.send_command("USER testuser 0 * :Test User")
    time.sleep(0.5)
    responses = client.get_full_response()
    print("Client responses (missing password):", responses)

    # Expect ERR_NOTREGISTERED (451) or similar if PASS is required first
    # Or ERR_PASSWDMISMATCH (464) if server expects a password but none was given
    # For now, let's check for a general error indicating registration failure
    # A more specific check would depend on the server's exact error codes.
    assert not any("001 testnick" in r for r in responses), "RPL_WELCOME (001) received despite missing password"
    assert any("451" in r or "464" in r for r in responses), "No error message for missing password"

    client.disconnect()
