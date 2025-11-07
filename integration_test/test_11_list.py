import pytest
from client_helper import get_client, HOST, PORT

def test_list_command(pytestconfig):
    """
    Tests the LIST command functionality.
    1. Client A connects.
    2. Client B connects, creates #chan1 and joins.
    3. Client C connects, creates #chan2 and joins.
    4. Client A sends LIST and receives a list of both channels.
    5. Client A sends LIST #chan1 and receives info only for #chan1.
    """
    # 1. Connect clients
    client_a = get_client('ClientA')
    client_b = get_client('ClientB')
    client_c = get_client('ClientC')
    
    # Wait for welcome messages
    client_a.wait_for_welcome()
    client_b.wait_for_welcome()
    client_c.wait_for_welcome()

    # 2. Client B creates and joins #chan1
    client_b.send_cmd("JOIN #chan1")
    client_b.wait_for_message("366") # End of NAMES

    # 3. Client C creates and joins #chan2, and sets a topic
    client_c.send_cmd("JOIN #chan2")
    client_c.wait_for_message("366") # End of NAMES
    client_c.send_cmd("TOPIC #chan2 :This is a test topic")
    client_c.wait_for_message("TOPIC")

    # 4. Client A sends LIST
    client_a.send_cmd("LIST")
    
    # Wait for the full list
    # Expected replies: 321 (List start), 322 (List item) x2, 323 (List end)
    reply_a = client_a.wait_for_message("323", full_message=True) # Wait for List end
    
    # Assertions for the full list
    assert "321" in reply_a
    assert "322 ClientA #chan1 1" in reply_a
    assert "322 ClientA #chan2 1 :This is a test topic" in reply_a
    assert "323 ClientA :End of /LIST" in reply_a

    # 5. Client A sends LIST for a specific channel
    client_a.send_cmd("LIST #chan2")
    
    # Wait for the specific list
    reply_b = client_a.wait_for_message("323", full_message=True)
    
    # Assertions for the specific list
    assert "321" in reply_b
    assert "#chan1" not in reply_b # Should not contain info for #chan1
    assert "322 ClientA #chan2 1 :This is a test topic" in reply_b
    assert "323 ClientA :End of /LIST" in reply_b

    # Clean up
    client_a.send_cmd("QUIT")
    client_b.send_cmd("QUIT")
    client_c.send_cmd("QUIT")
