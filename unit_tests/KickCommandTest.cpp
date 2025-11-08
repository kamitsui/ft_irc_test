#include "KickCommand.hpp"
#include "Channel.hpp"
#include "TestFixture.hpp"

class KickCommandTest : public CommandTest {
  protected:
    Channel *channel;
    TestClient *client3; // Kicked user

    virtual void SetUp() {
        CommandTest::SetUp();
        registerClient(client1, "Operator"); // client1 is the operator
        registerClient(client2, "Member");   // client2 is a regular member
        client3 = new TestClient(12, "client3.host");
        server->addTestClient(client3);
        registerClient(client3, "Target"); // client3 is the target to be kicked

        // Setup channel
        channel = new Channel("#test");
        server->addChannel(channel);

        // Add members and make client1 an operator
        channel->addMember(client1);
        channel->addMember(client2);
        channel->addMember(client3);
        channel->addOperator(client1);

        client1->receivedMessages.clear();
        client2->receivedMessages.clear();
        client3->receivedMessages.clear();
    }
};

TEST_F(KickCommandTest, Kick_Success) {
    args.push_back("#test");
    args.push_back("Target");
    args.push_back("Optional reason");
    kickCmd->execute(client1, args);

    // 1. Target is no longer a member
    EXPECT_FALSE(channel->isMember(client3));

    // 2. All remaining members (Operator, Member) and the kicked user receive the KICK message
    std::string expected_msg = std::string(":Operator!user@client1.host KICK #test Target :Optional reason") + "\r\n";

    EXPECT_EQ(client1->getLastMessage(), expected_msg);
    EXPECT_EQ(client2->getLastMessage(), expected_msg);
    EXPECT_EQ(client3->getLastMessage(), expected_msg);
}

TEST_F(KickCommandTest, Kick_NotAnOperator) {
    args.push_back("#test");
    args.push_back("Target");
    kickCmd->execute(client2, args); // Member (not op) tries to kick Target

    // Target should still be a member
    EXPECT_TRUE(channel->isMember(client3));

    // Member receives ERR_CHANOPRIVSNEEDED
    std::vector<std::string> params;
    params.push_back("#test");
    std::string expected_reply =
        formatReply(server->getServerName(), client2->getNickname(), ERR_CHANOPRIVSNEEDED, params);
    EXPECT_EQ(client2->getLastMessage(), expected_reply);
}

TEST_F(KickCommandTest, Kick_TargetUserNotInChannel) {
    // Target leaves the channel first
    channel->removeMember(client3);

    args.push_back("#test");
    args.push_back("Target");
    kickCmd->execute(client1, args);

    // Operator receives ERR_USERNOTINCHANNEL
    std::vector<std::string> params;
    params.push_back("Target");
    params.push_back("#test");
    std::string expected_reply =
        formatReply(server->getServerName(), client1->getNickname(), ERR_USERNOTINCHANNEL, params);
    EXPECT_EQ(client1->getLastMessage(), expected_reply);
}

TEST_F(KickCommandTest, Kick_NoSuchChannel) {
    args.push_back("#nonexistent");
    args.push_back("Target");
    kickCmd->execute(client1, args);

    std::vector<std::string> params;
    params.push_back("#nonexistent");
    std::string expected_reply =
        formatReply(server->getServerName(), client1->getNickname(), ERR_NOSUCHCHANNEL, params);
    EXPECT_EQ(client1->getLastMessage(), expected_reply);
}

TEST_F(KickCommandTest, Kick_NeedMoreParams) {
    args.push_back("#test"); // Missing target user
    kickCmd->execute(client1, args);

    std::vector<std::string> params;
    params.push_back("KICK");
    std::string expected_reply =
        formatReply(server->getServerName(), client1->getNickname(), ERR_NEEDMOREPARAMS, params);
    EXPECT_EQ(client1->getLastMessage(), expected_reply);
}

TEST_F(KickCommandTest, Kick_NoSuchNick) {
    args.push_back("#test");
    args.push_back("NoSuchNick");
    kickCmd->execute(client1, args);

    std::vector<std::string> params;
    params.push_back("NoSuchNick");
    std::string expected_reply = formatReply(server->getServerName(), client1->getNickname(), ERR_NOSUCHNICK, params);
    EXPECT_EQ(client1->getLastMessage(), expected_reply);
}
