#include "CommandManager.hpp"
#include "Client.hpp"
#include "Replies.hpp"
#include "Server.hpp"
#include "TestFixture.hpp"
#include "gtest/gtest.h"

TEST_F(CommandManagerTest, ExecuteSimpleCommand) {
    registerClient(client1, "client1_nick");
    cmdManager->parseAndExecute(client1, "NICK new_nick");
    EXPECT_EQ(client1->getNickname(), "new_nick");
}

TEST_F(CommandManagerTest, ExecuteCommandWithTrailingArg) {
    registerClient(client1, "client1_nick");
    registerClient(client2, "client2");

    cmdManager->parseAndExecute(client1, "PRIVMSG client2 :Hello there!");
    std::string expected_msg = client1->getPrefix() + " PRIVMSG client2 :Hello there!\r\n";
    EXPECT_EQ(client2->getLastMessage(), expected_msg);
}

TEST_F(CommandManagerTest, ExecuteUnknownCommand) {
    registerClient(client1, "client1_nick");
    cmdManager->parseAndExecute(client1, "UNKNOWNCMD some args");

    std::string expected_reply = formatReply(server->getServerName(), client1->getNickname(), ERR_UNKNOWNCOMMAND,
                                             "UNKNOWNCMD :Unknown command") +
                                 "\r\n";
    EXPECT_EQ(client1->getLastMessage(), expected_reply);
}

TEST_F(CommandManagerTest, ExecuteCaseInsensitiveCommand) {
    registerClient(client1, "client1_nick");
    cmdManager->parseAndExecute(client1, "nick case_nick");
    EXPECT_EQ(client1->getNickname(), "case_nick");
}

TEST_F(CommandManagerTest, ExecuteEmptyCommand) {
    registerClient(client1, "client1_nick");
    cmdManager->parseAndExecute(client1, "");
    EXPECT_TRUE(client1->getLastMessage().empty());
}

TEST_F(CommandManagerTest, ExecuteCommandWithoutArgs) {
    registerClient(client1, "client1_nick");
    cmdManager->parseAndExecute(client1, "NICK");

    std::string expected_reply =
        formatReply(server->getServerName(), client1->getNickname(), ERR_NONICKNAMEGIVEN, ":No nickname given") +
        "\r\n";
    EXPECT_EQ(client1->getLastMessage(), expected_reply);
}

TEST_F(CommandManagerTest, ExecuteQuitCommandWithMessage) {
    registerClient(client1, "client1_nick");
    cmdManager->parseAndExecute(client1, "QUIT :Gone to lunch");
    EXPECT_TRUE(client1->isMarkedForDisconnect());
    EXPECT_EQ(client1->getQuitMessage(), "Gone to lunch");
}

TEST_F(CommandManagerTest, ExecuteQuitCommandWithoutMessage) {
    registerClient(client1, "client1_nick");
    cmdManager->parseAndExecute(client1, "QUIT");
    EXPECT_TRUE(client1->isMarkedForDisconnect());
    EXPECT_EQ(client1->getQuitMessage(), "");
}
