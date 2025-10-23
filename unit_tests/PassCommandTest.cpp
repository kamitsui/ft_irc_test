#include "gtest/gtest.h"
#include "PassCommand.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "Replies.hpp"

class PassCommandTest : public ::testing::Test {
protected:
    void SetUp() override {
        server = new Server("password");
        client = new Client(1, "127.0.0.1");
        passCmd = new PassCommand();
    }

    void TearDown() override {
        delete server;
        delete client;
        delete passCmd;
    }

    Server* server;
    Client* client;
    PassCommand* passCmd;
};

TEST_F(PassCommandTest, CorrectPassword) {
    std::vector<std::string> args;
    args.push_back("password");

    passCmd->execute(server, client, args);

    EXPECT_TRUE(client->isAuthenticated());
    // Assuming no reply is sent on successful password
    EXPECT_TRUE(client->getSendBuffer().empty());
}

TEST_F(PassCommandTest, IncorrectPassword) {
    std::vector<std::string> args;
    args.push_back("wrongpassword");

    passCmd->execute(server, client, args);

    EXPECT_FALSE(client->isAuthenticated());
    std::string expected_reply = ERR_PASSWDMISMATCH(client->getNickname());
    EXPECT_EQ(client->getSendBuffer(), expected_reply);
}

TEST_F(PassCommandTest, NotEnoughParams) {
    std::vector<std::string> args; // No arguments

    passCmd->execute(server, client, args);

    EXPECT_FALSE(client->isAuthenticated());
    std::string expected_reply = ERR_NEEDMOREPARAMS(client->getNickname(), "PASS");
    EXPECT_EQ(client->getSendBuffer(), expected_reply);
}

TEST_F(PassCommandTest, AlreadyRegistered) {
    client->setAuthenticated(true);
    client->setNickname("testnick");
    client->setUsername("testuser");
    client->setRegistered(true);

    std::vector<std::string> args;
    args.push_back("password");

    passCmd->execute(server, client, args);

    std::string expected_reply = ERR_ALREADYREGISTRED(client->getNickname());
    EXPECT_EQ(client->getSendBuffer(), expected_reply);
}
