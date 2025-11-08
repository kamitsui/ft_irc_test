import pytest
from client_helper import IRCClient
from conftest import SERVER_PORT, SERVER_PASSWORD

@pytest.fixture(scope="function")
def clients_for_mode_test(irc_server):
    """
    Sets up three clients:
    - userA: Joins #test and becomes an operator.
    - userB: Joins #test as a regular user.
    - userC: Registered but not in any channel.
    """
    client_a = IRCClient(SERVER_PORT, "userA")
    client_b = IRCClient(SERVER_PORT, "userB")
    client_c = IRCClient(SERVER_PORT, "userC")
    
    client_a.connect()
    client_b.connect()
    client_c.connect()

    client_a.register(SERVER_PASSWORD)
    client_b.register(SERVER_PASSWORD)
    client_c.register(SERVER_PASSWORD)

    # userA creates the channel, should become an op automatically
    client_a.send("JOIN #test")
    client_a.wait_for_command("366") # Wait for JOIN to complete

    # userB joins as a regular user
    client_b.send("JOIN #test")
    client_b.wait_for_command("366")

    # Clear buffers
    while client_a.get_message(timeout=0.1) is not None: pass
    while client_b.get_message(timeout=0.1) is not None: pass
    while client_c.get_message(timeout=0.1) is not None: pass

    yield client_a, client_b, client_c

    client_a.close()
    client_b.close()
    client_c.close()

def test_channel_creator_is_operator(clients_for_mode_test):
    """
    Tests that the first user in a channel is automatically made an operator.
    This is verified by checking if their nick has an '@' prefix in NAMES list.
    """
    client_a, _, _ = clients_for_mode_test
    
    client_a.send("NAMES #test")
    names_reply = client_a.wait_for_command("353")
    
    assert names_reply is not None
    users_in_channel = names_reply["args"][3].split()
    assert "@userA" in users_in_channel
    assert "userB" in users_in_channel

def test_op_can_op_another_user(clients_for_mode_test):
    """
    Tests if an operator can grant operator status to another user.
    """
    client_a, client_b, _ = clients_for_mode_test

    # userA (op) gives op status to userB
    client_a.send("MODE #test +o userB")

    # Both A and B should receive the MODE broadcast
    mode_broadcast_a = client_a.wait_for_command("MODE")
    mode_broadcast_b = client_b.wait_for_command("MODE")

    assert mode_broadcast_a is not None and mode_broadcast_a["args"] == ["#test", "+o", "userB"]
    assert mode_broadcast_b is not None and mode_broadcast_b["args"] == ["#test", "+o", "userB"]

    # Verify userB is now an op
    client_a.send("NAMES #test")
    names_reply = client_a.wait_for_command("353")
    users_in_channel = names_reply["args"][3].split()
    assert "@userA" in users_in_channel
    assert "@userB" in users_in_channel

def test_op_can_deop_another_user(clients_for_mode_test):
    """
    Tests if an operator can remove operator status from another user.
    """
    client_a, client_b, _ = clients_for_mode_test

    # First, make userB an operator
    client_a.send("MODE #test +o userB")
    client_b.wait_for_command("MODE") # Wait for confirmation
    
    # Now, userA (op) removes op status from userB
    client_a.send("MODE #test -o userB")
    client_b.wait_for_command("MODE") # Wait for de-op broadcast

    # Verify userB is no longer an op
    client_a.send("NAMES #test")
    names_reply = client_a.wait_for_command("353")
    users_in_channel = names_reply["args"][3].split()
    assert "@userA" in users_in_channel
    assert "userB" in users_in_channel
    assert "@userB" not in users_in_channel

def test_non_op_cannot_change_mode(clients_for_mode_test):
    """
    Tests that a non-operator cannot change channel modes.
    """
    _, client_b, _ = clients_for_mode_test

    # userB (non-op) tries to op userC (who is not in the channel, but that doesn't matter here)
    client_b.send("MODE #test +o userC")

    # Expect ERR_CHANOPRIVSNEEDED
    error_reply = client_b.wait_for_command("482")
    assert error_reply is not None
    assert error_reply["args"] == ["userB", "#test", "You're not an operator on this channel"]

def test_mode_errors(clients_for_mode_test):
    """
    Tests various error conditions for the MODE command.
    """
    client_a, _, client_c = clients_for_mode_test

    # ERR_USERNOTINCHANNEL (441) - target user is not in the channel
    client_a.send("MODE #test +o userC")
    error_reply = client_a.wait_for_command("441")
    assert error_reply is not None
    assert error_reply["args"] == ["userA", "userC", "#test", "They aren't on that channel"]

    # ERR_NOSUCHNICK (401) - target user does not exist
    client_a.send("MODE #test +o nonExistentUser")
    error_reply = client_a.wait_for_command("401")
    assert error_reply is not None
    assert error_reply["args"] == ["userA", "nonExistentUser", "No such nick/channel"]

    # ERR_UNKNOWNMODE (472)
    client_a.send("MODE #test +z userB")
    error_reply = client_a.wait_for_command("472")
    assert error_reply is not None
    assert error_reply["args"] == ["userA", "z", "is unknown mode char to me for #test"]

def test_mode_topic_protection(clients_for_mode_test):
    """
    Tests channel mode +t (topic protection).
    - An operator should be able to set the topic.
    - A non-operator should not be able to set the topic.
    """
    client_a, client_b, _ = clients_for_mode_test

    # 1. Operator sets +t mode
    client_a.send("MODE #test +t")
    mode_broadcast = client_b.wait_for_command("MODE")
    assert mode_broadcast is not None and mode_broadcast["args"] == ["#test", "+t"]

    # 2. Non-operator (userB) fails to set topic
    client_b.send("TOPIC #test :New topic by non-op")
    error_reply = client_b.wait_for_command("482") # ERR_CHANOPRIVSNEEDED
    assert error_reply is not None
    assert error_reply["args"] == ["userB", "#test", "You're not an operator on this channel"]

    # 3. Operator (userA) successfully sets topic
    client_a.send("TOPIC #test :New topic by op")
    topic_broadcast = client_b.wait_for_command("TOPIC")
    assert topic_broadcast is not None
    assert topic_broadcast["args"] == ["#test", "New topic by op"]

    # 4. Operator unsets +t mode
    client_a.send("MODE #test -t")
    client_b.wait_for_command("MODE")

    # 5. Non-operator (userB) can now set the topic
    while client_a.get_message(timeout=0.1) is not None: pass # Clear buffer
    client_b.send("TOPIC #test :Final topic by non-op")
    topic_broadcast = client_a.wait_for_command("TOPIC")
    assert topic_broadcast is not None
    assert topic_broadcast["args"] == ["#test", "Final topic by non-op"]

def test_mode_no_external_messages(clients_for_mode_test):
    """
    Tests channel mode +n (no external messages).
    - A user not in the channel should not be able to send messages.
    """
    client_a, _, client_c = clients_for_mode_test

    # 1. Operator sets +n mode
    client_a.send("MODE #test +n")
    client_a.wait_for_command("MODE") # Wait for self-broadcast

    # 2. External user (userC) fails to send PRIVMSG
    client_c.send("PRIVMSG #test :Hello from outside")
    error_reply = client_c.wait_for_command("404") # ERR_CANNOTSENDTOCHAN
    assert error_reply is not None
    assert error_reply["args"] == ["userC", "#test", "Cannot send to channel"]

    # 3. Operator unsets +n mode
    client_a.send("MODE #test -n")
    client_a.wait_for_command("MODE")

    # 4. External user (userC) can now send PRIVMSG
    client_c.send("PRIVMSG #test :Hello again from outside")
    message_from_c = client_a.wait_for_command("PRIVMSG")
    assert message_from_c is not None
    assert message_from_c["prefix"]["nick"] == "userC"
    assert message_from_c["args"] == ["#test", "Hello again from outside"]

def test_mode_channel_key(clients_for_mode_test):
    """
    Tests channel mode +k (channel key/password).
    - Operator sets a key.
    - Non-operator cannot join without the key.
    - Non-operator can join with the correct key.
    - Operator unsets the key.
    - Non-operator can join without a key.
    """
    client_a, _, client_c = clients_for_mode_test

    # 1. Operator sets +k mode with a key
    client_a.send("MODE #test +k mysecret")
    client_a.wait_for_command("MODE")

    # 2. External user (userC) fails to join without key
    client_c.send("JOIN #test")
    error_reply = client_c.wait_for_command("475") # ERR_BADCHANNELKEY
    assert error_reply is not None
    assert error_reply["args"] == ["userC", "#test", "Cannot join channel (+k)"]

    # 3. External user (userC) fails to join with incorrect key
    client_c.send("JOIN #test wrongkey")
    error_reply = client_c.wait_for_command("475") # ERR_BADCHANNELKEY
    assert error_reply is not None

    # 4. External user (userC) successfully joins with correct key
    client_c.send("JOIN #test mysecret")
    join_reply = client_c.wait_for_command("JOIN")
    assert join_reply is not None
    assert join_reply["args"] == ["#test"]
    client_c.wait_for_command("366") # End of NAMES

    # 5. Operator unsets +k mode
    client_a.send("MODE #test -k mysecret")
    client_a.wait_for_command("MODE")

    # 6. userC parts the channel to test re-joining
    client_c.send("PART #test")
    client_c.wait_for_command("PART")

    # 7. External user (userC) can now join without a key
    client_c.send("JOIN #test")
    join_reply = client_c.wait_for_command("JOIN")
    assert join_reply is not None
    assert join_reply["args"] == ["#test"]

def test_mode_user_limit(clients_for_mode_test):
    """
    Tests channel mode +l (user limit).
    - Operator sets a limit.
    - Additional users cannot join if limit is reached.
    - Operator unsets the limit.
    - Additional users can join.
    """
    client_a, client_b, client_c = clients_for_mode_test

    # 1. Operator sets +l mode with a limit of 2 (userA and userB are already in)
    client_a.send("MODE #test +l 2")
    client_a.wait_for_command("MODE")

    # 2. External user (userC) fails to join because limit is reached
    client_c.send("JOIN #test")
    error_reply = client_c.wait_for_command("471") # ERR_CHANNELISFULL
    assert error_reply is not None
    assert error_reply["args"] == ["userC", "#test", "Cannot join channel (+l)"]

    # 3. Operator unsets +l mode
    client_a.send("MODE #test -l")
    client_a.wait_for_command("MODE")

    # 4. External user (userC) can now join
    client_c.send("JOIN #test")
    join_reply = client_c.wait_for_command("JOIN")
    assert join_reply is not None
    assert join_reply["args"] == ["#test"]

def test_mode_invite_only(clients_for_mode_test):
    """
    Tests channel mode +i (invite-only).
    - Operator sets invite-only mode.
    - Non-invited user cannot join.
    - Invited user can join.
    """
    client_a, _, client_c = clients_for_mode_test

    # 1. Operator sets +i mode
    client_a.send("MODE #test +i")
    client_a.wait_for_command("MODE")

    # 2. External user (userC) fails to join (not invited)
    client_c.send("JOIN #test")
    error_reply = client_c.wait_for_command("473") # ERR_INVITEONLYCHAN
    assert error_reply is not None
    assert error_reply["args"] == ["userC", "#test", "Cannot join channel (+i)"]

    # 3. Operator invites userC
    client_a.send("INVITE userC #test")
    invite_msg = client_c.wait_for_command("INVITE")
    assert invite_msg is not None

    # 4. Invited user (userC) can now join
    client_c.send("JOIN #test")
    join_reply = client_c.wait_for_command("JOIN")
    assert join_reply is not None
    assert join_reply["args"] == ["#test"]
