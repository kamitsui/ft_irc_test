#include "gtest/gtest.h"
#include "CommandManager.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "Replies.hpp"

class CommandManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        server = new Server("password");
        client = new Client(1, "127.0.0.1");
        client->setAuthenticated(true);
        client->setNickname("old_nick");
        client->setUsername("user");
        client->setRegistered(true);
        server->addClient(client);

        cmdManager = new CommandManager();
    }

    void TearDown() override {
        delete server;
        // client is deleted by server's destructor
        delete cmdManager;
    }

    Server* server;
    Client* client;
    CommandManager* cmdManager;
};

TEST_F(CommandManagerTest, ExecuteSimpleCommand) {
    cmdManager->executeCommand(server, client, "NICK new_nick");
    EXPECT_EQ(client->getNickname(), "new_nick");
}

TEST_F(CommandManagerTest, ExecuteCommandWithTrailingArg) {
    Client* receiver = new Client(2, "127.0.0.1");
    receiver->setNickname("receiver");
    receiver->setUsername("receiver_user");
    receiver->setRegistered(true);
    server->addClient(receiver);

    cmdManager->executeCommand(server, client, "PRIVMSG receiver :Hello there!");
    
    std::string expected_msg = ":" + client->getPrefix() + " PRIVMSG receiver :Hello there!\r\n";
    EXPECT_EQ(receiver->getSendBuffer(), expected_msg);
}

TEST_F(CommandManagerTest, ExecuteUnknownCommand) {
    cmdManager->executeCommand(server, client, "UNKNOWNCMD some args");
    
    std::string expected_reply = ERR_UNKNOWNCOMMAND(client->getNickname(), "UNKNOWNCMD");
    EXPECT_EQ(client->getSendBuffer(), expected_reply);
}

TEST_F(CommandManagerTest, ExecuteCaseInsensitiveCommand) {
    cmdManager->executeCommand(server, client, "nick case_insensitive_nick");
    EXPECT_EQ(client->getNickname(), "case_insensitive_nick");
}

TEST_F(CommandManagerTest, ExecuteEmptyCommand) {
    cmdManager->executeCommand(server, client, "");
    EXPECT_TRUE(client->getSendBuffer().empty());
}

TEST_F(CommandManagerTest, ExecuteCommandWithoutArgs) {
    // This tests that the CommandManager correctly passes an empty argument list
    // to the command, which should then handle the error.
    cmdManager->executeCommand(server, client, "NICK");
    
    std::string expected_reply = ERR_NONICKNAMEGIVEN(client->getNickname());
    EXPECT_EQ(client->getSendBuffer(), expected_reply);
}
