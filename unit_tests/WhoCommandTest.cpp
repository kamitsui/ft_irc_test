#include "TestFixture.hpp"
#include "WhoCommand.hpp"
#include "Channel.hpp"

class WhoCommandTest : public CommandTest {
  protected:
    WhoCommand *whoCmd;

    virtual void SetUp() {
        CommandTest::SetUp();
        whoCmd = new WhoCommand(server);
        registerClient(client1, "UserA"); // Operator
        registerClient(client2, "UserB"); // Regular member

        // Client3: Server-only client
        TestClient *client3 = new TestClient(12, "client3.host");
        server->addTestClient(client3);
        registerClient(client3, "UserC");

        // Setup channel
        Channel *channel = new Channel("#test");
        server->addChannel(channel);
        channel->addMember(client1);
        channel->addMember(client2);
        channel->addOperator(client1);
        client1->addChannel(channel);
        client2->addChannel(channel);

        client1->receivedMessages.clear();
        client2->receivedMessages.clear();
        client3->receivedMessages.clear();
    }

    virtual void TearDown() {
        delete whoCmd;
        CommandTest::TearDown();
    }
};

TEST_F(WhoCommandTest, Who_ChannelMembers) {
    args.push_back("#test");
    whoCmd->execute(client1, args);

    ASSERT_EQ(client1->receivedMessages.size(), static_cast<std::string::size_type>(3)); // 2 RPL_WHOREPLY + 1 RPL_ENDOFWHO

    std::string expected_reply_a_str;
    {
        std::vector<std::string> params;
        params.push_back("#test");
        params.push_back("UserA");
        params.push_back("client1.host");
        params.push_back("irc.myserver.com");
        params.push_back("UserA");
        params.push_back("H@");
        params.push_back("0");
        params.push_back("UserA");
        expected_reply_a_str = formatReply(server->getServerName(), client1->getNickname(), RPL_WHOREPLY, params);
    }
    EXPECT_EQ(client1->receivedMessages[0], expected_reply_a_str);

    std::string expected_reply_b_str;
    {
        std::vector<std::string> params;
        params.push_back("#test");
        params.push_back("UserB");
        params.push_back("client2.host");
        params.push_back("irc.myserver.com");
        params.push_back("UserB");
        params.push_back("H");
        params.push_back("0");
        params.push_back("UserB");
        expected_reply_b_str = formatReply(server->getServerName(), client1->getNickname(), RPL_WHOREPLY, params);
    }
    EXPECT_EQ(client1->receivedMessages[1], expected_reply_b_str);

    // Check RPL_ENDOFWHO
    std::string expected_end_reply_str;
    {
        std::vector<std::string> params;
        params.push_back("#test");
        expected_end_reply_str = formatReply(server->getServerName(), client1->getNickname(), RPL_ENDOFWHO, params);
    }
    EXPECT_EQ(client1->receivedMessages[2], expected_end_reply_str);}

TEST_F(WhoCommandTest, Who_SpecificUserInChannel) {
    args.push_back("UserB");
    whoCmd->execute(client1, args);

    ASSERT_EQ(client1->receivedMessages.size(), static_cast<std::string::size_type>(2));

    // Check RPL_WHOREPLY for UserB
    std::string expected_reply_b_str;
    {
        std::vector<std::string> params;
        params.push_back("#test");
        params.push_back("UserB");
        params.push_back("client2.host");
        params.push_back("irc.myserver.com");
        params.push_back("UserB");
        params.push_back("H");
        params.push_back("0");
        params.push_back("UserB");
        expected_reply_b_str = formatReply(server->getServerName(), client1->getNickname(), RPL_WHOREPLY, params);
    }
    EXPECT_EQ(client1->receivedMessages[0], expected_reply_b_str);

    // Check RPL_ENDOFWHO
    std::string expected_end_reply_str;
    {
        std::vector<std::string> params;
        params.push_back("UserB");
        expected_end_reply_str = formatReply(server->getServerName(), client1->getNickname(), RPL_ENDOFWHO, params);
    }
    EXPECT_EQ(client1->receivedMessages[1], expected_end_reply_str);
}

TEST_F(WhoCommandTest, Who_SpecificUserNotInChannelButOnServer) {
    // UserC is on server but not in #test
    args.push_back("UserC");
    whoCmd->execute(client1, args);

    ASSERT_EQ(client1->receivedMessages.size(), static_cast<std::string::size_type>(2));

    // Check RPL_WHOREPLY for UserC
    std::string expected_reply_c_str;
    {
        std::vector<std::string> params;
        params.push_back("*"); // Channel is '*' for server-only users
        params.push_back("UserC");
        params.push_back("client3.host");
        params.push_back("irc.myserver.com");
        params.push_back("UserC");
        params.push_back("H");
        params.push_back("0");
        params.push_back("UserC");
        expected_reply_c_str = formatReply(server->getServerName(), client1->getNickname(), RPL_WHOREPLY, params);
    }
    EXPECT_EQ(client1->receivedMessages[0], expected_reply_c_str);

    // Check RPL_ENDOFWHO
    std::string expected_end_reply_str;
    {
        std::vector<std::string> params;
        params.push_back("UserC");
        expected_end_reply_str = formatReply(server->getServerName(), client1->getNickname(), RPL_ENDOFWHO, params);
    }
    EXPECT_EQ(client1->receivedMessages[1], expected_end_reply_str);
}

TEST_F(WhoCommandTest, Who_NonExistentTarget) {
    args.push_back("NonExistentUser");
    whoCmd->execute(client1, args);

    ASSERT_EQ(client1->receivedMessages.size(), static_cast<std::string::size_type>(1)); // Only RPL_ENDOFWHO

    std::string expected_end_reply_str;
    {
        std::vector<std::string> params;
        params.push_back("NonExistentUser");
        expected_end_reply_str = formatReply(server->getServerName(), client1->getNickname(), RPL_ENDOFWHO, params);
    }
    EXPECT_EQ(client1->receivedMessages[0], expected_end_reply_str);
}

TEST_F(WhoCommandTest, Who_NoArgs_ListsAllUsers) {
    // Add another channel and client to ensure comprehensive listing
    TestClient *client4 = new TestClient(13, "client4.host");
    server->addTestClient(client4);
    registerClient(client4, "UserD");
    Channel *chan2 = new Channel("#chan2");
    server->addChannel(chan2);
    chan2->addMember(client4);
    client4->addChannel(chan2);

    args.clear(); // No arguments
    whoCmd->execute(client1, args);

    // Expect replies for UserA, UserB, UserC, UserD + ENDOFWHO
    ASSERT_EQ(client1->receivedMessages.size(), static_cast<std::string::size_type>(5));

    std::string expected_reply_a_str;
    {
        std::vector<std::string> params;
        params.push_back("#test");
        params.push_back("UserA");
        params.push_back("client1.host");
        params.push_back("irc.myserver.com");
        params.push_back("UserA");
        params.push_back("H@");
        params.push_back("0");
        params.push_back("UserA");
        expected_reply_a_str = formatReply(server->getServerName(), client1->getNickname(), RPL_WHOREPLY, params);
    }
    EXPECT_EQ(client1->receivedMessages[0], expected_reply_a_str);

    std::string expected_reply_b_str;
    {
        std::vector<std::string> params;
        params.push_back("#test");
        params.push_back("UserB");
        params.push_back("client2.host");
        params.push_back("irc.myserver.com");
        params.push_back("UserB");
        params.push_back("H");
        params.push_back("0");
        params.push_back("UserB");
        expected_reply_b_str = formatReply(server->getServerName(), client1->getNickname(), RPL_WHOREPLY, params);
    }
    EXPECT_EQ(client1->receivedMessages[1], expected_reply_b_str);

    std::string expected_reply_c_str;
    {
        std::vector<std::string> params;
        params.push_back("*");
        params.push_back("UserC");
        params.push_back("client3.host");
        params.push_back("irc.myserver.com");
        params.push_back("UserC");
        params.push_back("H");
        params.push_back("0");
        params.push_back("UserC");
        expected_reply_c_str = formatReply(server->getServerName(), client1->getNickname(), RPL_WHOREPLY, params);
    }
    EXPECT_EQ(client1->receivedMessages[2], expected_reply_c_str);

    std::string expected_reply_d_str;
    {
        std::vector<std::string> params;
        params.push_back("#chan2");
        params.push_back("UserD");
        params.push_back("client4.host");
        params.push_back("irc.myserver.com");
        params.push_back("UserD");
        params.push_back("H");
        params.push_back("0");
        params.push_back("UserD");
        expected_reply_d_str = formatReply(server->getServerName(), client1->getNickname(), RPL_WHOREPLY, params);
    }
    EXPECT_EQ(client1->receivedMessages[3], expected_reply_d_str);

    std::string expected_end_reply_str;
    {
        std::vector<std::string> params;
        params.push_back(client1->getNickname());
        expected_end_reply_str = formatReply(server->getServerName(), client1->getNickname(), RPL_ENDOFWHO, params);
    }
    EXPECT_EQ(client1->receivedMessages[4], expected_end_reply_str);
}
