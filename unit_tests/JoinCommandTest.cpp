#include "TestFixture.hpp"
#include "JoinCommand.hpp"
#include "Channel.hpp"
#include "Replies.hpp"

class JoinCommandTest : public CommandTest {
protected:
    virtual void SetUp() {
        CommandTest::SetUp();
        registerClient(client1, "UserA");
        registerClient(client2, "UserB");
    }
};

TEST_F(JoinCommandTest, JoinNewChannel) {
    args.push_back("#newchannel");
    joinCmd->execute(client1, args);

    Channel* channel = server->getChannel("#newchannel");
    ASSERT_NE(channel, (Channel*)NULL);
    EXPECT_TRUE(channel->isMember(client1));
    EXPECT_TRUE(channel->isOperator(client1)); // First user becomes operator
    
    // Check for JOIN message and other replies
    ASSERT_EQ(client1->receivedMessages.size(), 4);
    EXPECT_NE(client1->receivedMessages[0].find("JOIN #newchannel"), std::string::npos);
}

TEST_F(JoinCommandTest, JoinExistingChannel) {
    Channel* existingChannel = new Channel("#existing");
    server->addChannel(existingChannel);
    existingChannel->addMember(client2);
    client2->addChannel(existingChannel);

    args.push_back("#existing");
    joinCmd->execute(client1, args);

    EXPECT_TRUE(existingChannel->isMember(client1));
    // client2 should receive JOIN notification for client1
    EXPECT_NE(client2->getLastMessage().find("JOIN #existing"), std::string::npos);
}

TEST_F(JoinCommandTest, NotEnoughParams) {
    joinCmd->execute(client1, args); // No arguments

    std::vector<std::string> params;
    params.push_back("JOIN");
    std::string expected_reply = formatReply(server->getServerName(), client1->getNickname(), ERR_NEEDMOREPARAMS, params) + "\r\n";
    EXPECT_EQ(client1->getLastMessage(), expected_reply);
}

TEST_F(JoinCommandTest, JoinPasswordProtectedChannel_CorrectPassword) {
    Channel* protectedChannel = new Channel("#protected");
    server->addChannel(protectedChannel);
    protectedChannel->setMode('k', true);
    protectedChannel->setKey("chanpass");

    args.push_back("#protected");
    args.push_back("chanpass");
    joinCmd->execute(client1, args);

    EXPECT_TRUE(protectedChannel->isMember(client1));
    EXPECT_NE(client1->getLastMessage().find("JOIN #protected"), std::string::npos);
}

TEST_F(JoinCommandTest, JoinPasswordProtectedChannel_IncorrectPassword) {
    Channel* protectedChannel = new Channel("#protected");
    server->addChannel(protectedChannel);
    protectedChannel->setMode('k', true);
    protectedChannel->setKey("chanpass");

    args.push_back("#protected");
    args.push_back("wrongpass");
    joinCmd->execute(client1, args);

    EXPECT_FALSE(protectedChannel->isMember(client1));
    std::vector<std::string> params;
    params.push_back("#protected");
    std::string expected_reply = formatReply(server->getServerName(), client1->getNickname(), ERR_BADCHANNELKEY, params) + "\r\n";
    EXPECT_EQ(client1->getLastMessage(), expected_reply);
}

TEST_F(JoinCommandTest, JoinPasswordProtectedChannel_NoPasswordProvided) {
    Channel* protectedChannel = new Channel("#protected");
    server->addChannel(protectedChannel);
    protectedChannel->setMode('k', true);
    protectedChannel->setKey("chanpass");

    args.push_back("#protected"); // Missing password argument
    joinCmd->execute(client1, args);

    EXPECT_FALSE(protectedChannel->isMember(client1));
    std::vector<std::string> params;
    params.push_back("#protected");
    std::string expected_reply = formatReply(server->getServerName(), client1->getNickname(), ERR_BADCHANNELKEY, params) + "\r\n";
    EXPECT_EQ(client1->getLastMessage(), expected_reply);
}

TEST_F(JoinCommandTest, JoinInviteOnlyChannel_NotInvited) {
    Channel* inviteOnlyChannel = new Channel("#inviteonly");
    server->addChannel(inviteOnlyChannel);
    inviteOnlyChannel->setMode('i', true);

    args.push_back("#inviteonly");
    joinCmd->execute(client1, args);

    EXPECT_FALSE(inviteOnlyChannel->isMember(client1));
    std::vector<std::string> params;
    params.push_back("#inviteonly");
    std::string expected_reply = formatReply(server->getServerName(), client1->getNickname(), ERR_INVITEONLYCHAN, params) + "\r\n";
    EXPECT_EQ(client1->getLastMessage(), expected_reply);
}

TEST_F(JoinCommandTest, JoinInviteOnlyChannel_Invited) {
    Channel* inviteOnlyChannel = new Channel("#inviteonly");
    server->addChannel(inviteOnlyChannel);
    inviteOnlyChannel->setMode('i', true);
    inviteOnlyChannel->addInvitedUser(client1);

    args.push_back("#inviteonly");
    joinCmd->execute(client1, args);

    EXPECT_TRUE(inviteOnlyChannel->isMember(client1));
    EXPECT_FALSE(inviteOnlyChannel->isInvitedUser(client1)); // Should be removed from invited list
    EXPECT_NE(client1->getLastMessage().find("JOIN #inviteonly"), std::string::npos);
}

TEST_F(JoinCommandTest, JoinFullChannel) {
    Channel* fullChannel = new Channel("#full");
    server->addChannel(fullChannel);
    fullChannel->setMode('l', true);
    fullChannel->setLimit(1);
    fullChannel->addMember(client2); // client2 takes the only spot
    client2->addChannel(fullChannel);

    args.push_back("#full");
    joinCmd->execute(client1, args);

    EXPECT_FALSE(fullChannel->isMember(client1));
    std::vector<std::string> params;
    params.push_back("#full");
    std::string expected_reply = formatReply(server->getServerName(), client1->getNickname(), ERR_CHANNELISFULL, params) + "\r\n";
    EXPECT_EQ(client1->getLastMessage(), expected_reply);
}

TEST_F(JoinCommandTest, NotRegistered) {
    client1->setRegistered(false);
    args.push_back("#channel");

    joinCmd->execute(client1, args);

    // NotRegistered clients should not be able to execute commands that require registration
    // The server should handle this before command execution, or the command itself should check.
    // For now, we expect an error reply, likely ERR_NOTREGISTERED if implemented, or no action.
    // Based on current CommandManager, it might send ERR_UNKNOWNCOMMAND if not registered.
    // However, for JOIN, it should be ERR_NOTREGISTERED (451) or similar if implemented.
    // Given the current setup, we'll check if the client is NOT added to the channel and receives an error.
    EXPECT_FALSE(server->getChannel("#channel") && server->getChannel("#channel")->isMember(client1));
    // The actual error might vary depending on server-side registration checks.
    // For now, we'll just ensure no successful join and some message is sent.
    EXPECT_FALSE(client1->receivedMessages.empty());
}