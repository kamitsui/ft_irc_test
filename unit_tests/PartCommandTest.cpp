#include "gtest/gtest.h"
#include "PartCommand.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "Replies.hpp"

class PartCommandTest : public ::testing::Test {
protected:
    void SetUp() override {
        server = new Server("password");
        client = new Client(1, "127.0.0.1");
        client->setAuthenticated(true);
        client->setNickname("testnick");
        client->setUsername("testuser");
        client->setRealname("Test User");
        client->setRegistered(true);
        partCmd = new PartCommand();

        channel1 = new Channel("#channel1");
        channel1->addUser(client);
        server->addChannel(channel1);

        channel2 = new Channel("#channel2");
        channel2->addUser(client);
        server->addChannel(channel2);
    }

    void TearDown() override {
        delete server;
        delete client;
        delete partCmd;
        // Channels are deleted by server's destructor
    }

    Server* server;
    Client* client;
    PartCommand* partCmd;
    Channel* channel1;
    Channel* channel2;
};

TEST_F(PartCommandTest, PartFromOneChannel) {
    std::vector<std::string> args;
    args.push_back("#channel1");

    partCmd->execute(server, client, args);

    EXPECT_FALSE(channel1->isUserInChannel(client));
    EXPECT_TRUE(client->getSendBuffer().find("PART #channel1") != std::string::npos);
}

TEST_F(PartCommandTest, PartFromMultipleChannels) {
    std::vector<std::string> args;
    args.push_back("#channel1,#channel2");

    partCmd->execute(server, client, args);

    EXPECT_FALSE(channel1->isUserInChannel(client));
    EXPECT_FALSE(channel2->isUserInChannel(client));
    EXPECT_TRUE(client->getSendBuffer().find("PART #channel1") != std::string::npos);
    EXPECT_TRUE(client->getSendBuffer().find("PART #channel2") != std::string::npos);
}

TEST_F(PartCommandTest, PartWithReason) {
    std::vector<std::string> args;
    args.push_back("#channel1");
    args.push_back(":Leaving for a break");

    partCmd->execute(server, client, args);

    EXPECT_FALSE(channel1->isUserInChannel(client));
    EXPECT_TRUE(client->getSendBuffer().find("PART #channel1 :Leaving for a break") != std::string::npos);
}

TEST_F(PartCommandTest, NotEnoughParams) {
    std::vector<std::string> args; // No arguments

    partCmd->execute(server, client, args);

    std::string expected_reply = ERR_NEEDMOREPARAMS(client->getNickname(), "PART");
    EXPECT_EQ(client->getSendBuffer(), expected_reply);
}

TEST_F(PartCommandTest, NotOnChannel) {
    Channel* nonMemberChannel = new Channel("#nonmember");
    server->addChannel(nonMemberChannel);

    std::vector<std::string> args;
    args.push_back("#nonmember");

    partCmd->execute(server, client, args);

    std::string expected_reply = ERR_NOTONCHANNEL(client->getNickname(), "#nonmember");
    EXPECT_EQ(client->getSendBuffer(), expected_reply);
    // nonMemberChannel is deleted by server's destructor
}

TEST_F(PartCommandTest, NotRegistered) {
    client->setRegistered(false);
    std::vector<std::string> args;
    args.push_back("#channel1");

    partCmd->execute(server, client, args);

    EXPECT_FALSE(client->getSendBuffer().empty()); // Expect an error
}
