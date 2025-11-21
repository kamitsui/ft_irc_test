import pytest
import time
from client_helper import IRCClient
from conftest import SERVER_PORT, SERVER_PASSWORD

def test_client_ping_responds_with_pong(irc_server):
    """
    Tests if the server correctly responds with a PONG to a client's PING.
    """
    client = IRCClient(SERVER_PORT, "pinger")
    client.connect()
    client.register(SERVER_PASSWORD)

    ping_token = "test_token_123"
    client.send(f"PING :{ping_token}")

    pong_msg = client.wait_for_command("PONG")
    assert pong_msg is not None, "Server did not respond with PONG"
    assert len(pong_msg["args"]) > 0, "PONG message has no arguments"
    assert pong_msg["args"][-1] == ping_token, f"PONG token mismatch: got {pong_msg['args'][-1]}, expected {ping_token}"

    client.close()

def test_server_sends_ping_on_idle(irc_server):
    """
    Tests if the server sends a PING to an idle client.
    Note: Requires PING_TIMEOUT on server to be set to a short value (e.g., 2s).
    """
    client = IRCClient(SERVER_PORT, "idler")
    client.connect()
    client.register(SERVER_PASSWORD)

    # Wait for longer than the server's PING_TIMEOUT
    time.sleep(3)

    ping_msg = client.wait_for_command("PING")
    assert ping_msg is not None, "Server did not send PING to idle client"

    client.close()

def test_pong_response_keeps_connection_alive(irc_server):
    """
    Tests if responding to a server's PING with a PONG keeps the connection alive.
    Note: Requires short PING_TIMEOUT and PONG_TIMEOUT on server.
    """
    client = IRCClient(SERVER_PORT, "survivor")
    client.connect()
    client.register(SERVER_PASSWORD)

    # 1. Wait for the server's initial PING
    time.sleep(3)
    ping_msg = client.wait_for_command("PING")
    assert ping_msg is not None, "Server did not send PING to idle client"

    # 2. Respond with PONG
    ping_token = ping_msg["args"][-1]
    client.send(f"PONG :{ping_token}")

    # 3. Wait for another PING cycle to ensure connection is not dropped
    time.sleep(3)
    
    # To check if connection is alive, send a command and expect a response
    # (e.g., PING the server again)
    client.send("PING :areyoustillthere")
    pong_reply = client.wait_for_command("PONG")
    assert pong_reply is not None, "Connection appears to be dropped after PONG"

    client.close()

def test_no_pong_response_leads_to_disconnect(irc_server):
    """
    Tests if the server disconnects a client that does not respond to a PING.
    Note: Requires short PING_TIMEOUT and PONG_TIMEOUT on server.
    """
    client = IRCClient(SERVER_PORT, "zombie")
    client.auto_pong = False # PONGを自動送信しない
    client.connect()
    client.register(SERVER_PASSWORD)

    # 1. Wait for the server's PING
    time.sleep(3)
    ping_msg = client.wait_for_command("PING")
    assert ping_msg is not None, "Server did not send PING to idle client"

    # 2. Do NOT respond with PONG. Wait for PONG_TIMEOUT.
    time.sleep(2)

    # 3. Check if the connection is closed by the server.
    # get_message() should return None if the connection is closed.
    message_after_timeout = client.get_message(timeout=1.0)
    assert message_after_timeout is None, "Server did not close the connection after PONG timeout"

    client.close()
