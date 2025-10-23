#include "gtest/gtest.h"
#include "JoinCommand.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "Replies.hpp"

class JoinCommandTest : public ::testing::Test {
protected:
    void SetUp() override {
        server = new Server("password");
        client = new Client(1, "127.0.0.1");
        client->setAuthenticated(true);
        client->setNickname("testnick");
        client->setUsername("testuser");
        client->setRealname("Test User");
        client->setRegistered(true);
        joinCmd = new JoinCommand();
    }

    void TearDown() override {
        delete server;
        delete client;
        delete joinCmd;
    }

    Server* server;
    Client* client;
    JoinCommand* joinCmd;
};

TEST_F(JoinCommandTest, JoinNewChannel) {
    std::vector<std::string> args;
    args.push_back("#newchannel");

    joinCmd->execute(server, client, args);

    Channel* channel = server->getChannelByName("#newchannel");
    ASSERT_NE(channel, (Channel*)NULL);
    EXPECT_TRUE(channel->isUserInChannel(client));
    EXPECT_TRUE(channel->isOperator(client)); // First user becomes operator
    EXPECT_TRUE(client->getSendBuffer().find("JOIN #newchannel") != std::string::npos);
}

TEST_F(JoinCommandTest, JoinExistingChannel) {
    Channel* existingChannel = new Channel("#existing");
    server->addChannel(existingChannel);

    std::vector<std::string> args;
    args.push_back("#existing");

    joinCmd->execute(server, client, args);

    EXPECT_TRUE(existingChannel->isUserInChannel(client));
    EXPECT_TRUE(client->getSendBuffer().find("JOIN #existing") != std::string::npos);
}

TEST_F(JoinCommandTest, NotEnoughParams) {
    std::vector<std::string> args; // No arguments

    joinCmd->execute(server, client, args);

    std::string expected_reply = ERR_NEEDMOREPARAMS(client->getNickname(), "JOIN");
    EXPECT_EQ(client->getSendBuffer(), expected_reply);
}

TEST_F(JoinCommandTest, JoinPasswordProtectedChannel_CorrectPassword) {
    Channel* protectedChannel = new Channel("#protected");
    protectedChannel->setMode(Channel::MODE_KEY, true);
    protectedChannel->setPassword("chanpass");
    server->addChannel(protectedChannel);

    std::vector<std::string> args;
    args.push_back("#protected");
    args.push_back("chanpass");

    joinCmd->execute(server, client, args);

    EXPECT_TRUE(protectedChannel->isUserInChannel(client));
    EXPECT_TRUE(client->getSendBuffer().find("JOIN #protected") != std::string::npos);
}

TEST_F(JoinCommandTest, JoinPasswordProtectedChannel_IncorrectPassword) {
    Channel* protectedChannel = new Channel("#protected");
    protectedChannel->setMode(Channel::MODE_KEY, true);
    protectedChannel->setPassword("chanpass");
    server->addChannel(protectedChannel);

    std::vector<std::string> args;
    args.push_back("#protected");
    args.push_back("wrongpass");

    joinCmd->execute(server, client, args);

    EXPECT_FALSE(protectedChannel->isUserInChannel(client));
    std::string expected_reply = ERR_BADCHANNELKEY(client->getNickname(), "#protected");
    EXPECT_EQ(client->getSendBuffer(), expected_reply);
}

TEST_F(JoinCommandTest, JoinInviteOnlyChannel_NotInvited) {
    Channel* inviteOnlyChannel = new Channel("#inviteonly");
    inviteOnlyChannel->setMode(Channel::MODE_INVITE_ONLY, true);
    server->addChannel(inviteOnlyChannel);

    std::vector<std::string> args;
    args.push_back("#inviteonly");

    joinCmd->execute(server, client, args);

    EXPECT_FALSE(inviteOnlyChannel->isUserInChannel(client));
    std::string expected_reply = ERR_INVITEONLYCHAN(client->getNickname(), "#inviteonly");
    EXPECT_EQ(client->getSendBuffer(), expected_reply);
}

TEST_F(JoinCommandTest, JoinInviteOnlyChannel_Invited) {
    Channel* inviteOnlyChannel = new Channel("#inviteonly");
    inviteOnlyChannel->setMode(Channel::MODE_INVITE_ONLY, true);
    inviteOnlyChannel->inviteUser(client->getNickname());
    server->addChannel(inviteOnlyChannel);

    std::vector<std::string> args;
    args.push_back("#inviteonly");

    joinCmd->execute(server, client, args);

    EXPECT_TRUE(inviteOnlyChannel->isUserInChannel(client));
    EXPECT_TRUE(client->getSendBuffer().find("JOIN #inviteonly") != std::string::npos);
}

TEST_F(JoinCommandTest, JoinFullChannel) {
    Channel* fullChannel = new Channel("#full");
    fullChannel->setMode(Channel::MODE_LIMIT, true);
    fullChannel->setUserLimit(1);
    Client* otherClient = new Client(2, "127.0.0.1");
    otherClient->setNickname("other");
    otherClient->setUsername("otheruser");
    otherClient->setRegistered(true);
    fullChannel->addUser(otherClient);
    server->addClient(otherClient);
    server->addChannel(fullChannel);

    std::vector<std::string> args;
    args.push_back("#full");

    joinCmd->execute(server, client, args);

    EXPECT_FALSE(fullChannel->isUserInChannel(client));
    std::string expected_reply = ERR_CHANNELISFULL(client->getNickname(), "#full");
    EXPECT_EQ(client->getSendBuffer(), expected_reply);
    delete otherClient;
}

TEST_F(JoinCommandTest, NotRegistered) {
    client->setRegistered(false);
    std::vector<std::string> args;
    args.push_back("#channel");

    joinCmd->execute(server, client, args);

    EXPECT_FALSE(client->getSendBuffer().empty()); // Expect an error
}
