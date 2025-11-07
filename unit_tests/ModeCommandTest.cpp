#include "TestFixture.hpp"
#include "ModeCommand.hpp"
#include "Channel.hpp"

class ModeCommandTest : public CommandTest {
  protected:
    Channel* channel;
    TestClient* client3;

    virtual void SetUp() {
        CommandTest::SetUp();
        registerClient(client1, "UserA"); // Will be operator
        registerClient(client2, "UserB");
        client3 = new TestClient(12, "client3.host");
        server->addTestClient(client3);
        registerClient(client3, "UserC");

        // Setup channel
        channel = new Channel("#test");
        server->addChannel(channel);
        
        // Add members and make client1 an operator
        channel->addMember(client1);
        channel->addMember(client2);
        channel->addOperator(client1);

        client1->receivedMessages.clear();
        client2->receivedMessages.clear();
        client3->receivedMessages.clear();
    }
};

TEST_F(ModeCommandTest, Mode_AddOperator_Success) {
    args.push_back("#test");
    args.push_back("+o");
    args.push_back("UserB");
    modeCmd->execute(client1, args); // UserA (op) ops UserB

    EXPECT_TRUE(channel->isOperator(client2));
    
    // Check for broadcast message
    ASSERT_FALSE(client1->receivedMessages.empty());
    EXPECT_NE(client1->getLastMessage().find("MODE #test +o UserB"), std::string::npos);
    ASSERT_FALSE(client2->receivedMessages.empty());
    EXPECT_NE(client2->getLastMessage().find("MODE #test +o UserB"), std::string::npos);
}

TEST_F(ModeCommandTest, Mode_RemoveOperator_Success) {
    channel->addOperator(client2); // Make UserB an op first
    ASSERT_TRUE(channel->isOperator(client2));
    client1->receivedMessages.clear();
    client2->receivedMessages.clear();

    args.push_back("#test");
    args.push_back("-o");
    args.push_back("UserB");
    modeCmd->execute(client1, args); // UserA (op) de-ops UserB

    EXPECT_FALSE(channel->isOperator(client2));
    
    // Check for broadcast message
    ASSERT_FALSE(client1->receivedMessages.empty());
    EXPECT_NE(client1->getLastMessage().find("MODE #test -o UserB"), std::string::npos);
}

TEST_F(ModeCommandTest, Mode_NotAnOperator) {
    args.push_back("#test");
    args.push_back("+o");
    args.push_back("UserC");
    modeCmd->execute(client2, args); // UserB (not op) tries to op UserC

    EXPECT_FALSE(channel->isOperator(client3));
    ASSERT_FALSE(client2->receivedMessages.empty());

    std::vector<std::string> params;
    params.push_back("#test");
    std::string expected_reply = formatReply(server->getServerName(), client2->getNickname(),
                                             ERR_CHANOPRIVSNEEDED, params) +
                                 "\r\n";
    EXPECT_EQ(client2->getLastMessage(), expected_reply);
}

TEST_F(ModeCommandTest, Mode_TargetUserNotInChannel) {
    args.push_back("#test");
    args.push_back("+o");
    args.push_back("UserC"); // UserC is not in the channel
    modeCmd->execute(client1, args);

    EXPECT_FALSE(channel->isOperator(client3));
    ASSERT_FALSE(client1->receivedMessages.empty());

    std::vector<std::string> params;
    params.push_back("UserC");
    params.push_back("#test");
    std::string expected_reply = formatReply(server->getServerName(), client1->getNickname(),
                                             ERR_USERNOTINCHANNEL, params) +
                                 "\r\n";
    EXPECT_EQ(client1->getLastMessage(), expected_reply);
}

TEST_F(ModeCommandTest, Mode_NeedMoreParams) {
    modeCmd->execute(client1, args);
    ASSERT_FALSE(client1->receivedMessages.empty());

    std::vector<std::string> params;
    params.push_back("MODE");
    std::string expected_reply =
        formatReply(server->getServerName(), client1->getNickname(), ERR_NEEDMOREPARAMS, params) +
        "\r\n";
    EXPECT_EQ(client1->getLastMessage(), expected_reply);
}

TEST_F(ModeCommandTest, Mode_UnknownMode) {
    args.push_back("#test");
    args.push_back("+x"); // Unknown mode
    args.push_back("UserB");
    modeCmd->execute(client1, args);

    ASSERT_FALSE(client1->receivedMessages.empty());
    std::vector<std::string> params;
    params.push_back("x");
    params.push_back("#test");
    std::string expected_reply =
        formatReply(server->getServerName(), client1->getNickname(), ERR_UNKNOWNMODE, params) +
        "\r\n";
    EXPECT_EQ(client1->getLastMessage(), expected_reply);
}
