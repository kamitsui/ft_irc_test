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

    std::vector<std::string> args;
    args.push_back("UNKNOWNCMD");
    std::string expected_reply =
        formatReply(server->getServerName(), client1->getNickname(), ERR_UNKNOWNCOMMAND, args) +
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
        formatReply(server->getServerName(), client1->getNickname(), ERR_NONICKNAMEGIVEN) + "\r\n";
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

TEST_F(CommandManagerTest, ExecutePongCommandUpdatesActivityTime) {
    registerClient(client1, "client1_nick");

    // 1. 最終アクティビティ時刻を意図的に古く設定
    time_t oldTime = time(NULL) - 10;
    client1->setLastActivityTime(oldTime);
    ASSERT_EQ(client1->getLastActivityTime(), oldTime);

    // 2. PONG コマンドを実行
    cmdManager->parseAndExecute(client1, "PONG :some_token");

    // 3. 最終アクティビティ時刻が更新されたことを確認
    EXPECT_GT(client1->getLastActivityTime(), oldTime);
}

TEST_F(CommandManagerTest, RemoveClientCleansUpChannels) {
    registerClient(client1, "client1_nick");
    registerClient(client2, "client2_nick");

    // 1. client1 と client2 を #test チャンネルに参加させる
    Channel *channel = new Channel("#test");
    server->addChannel(channel);
    channel->addMember(client1);
    channel->addMember(client2);
    client1->addChannel(channel);
    client2->addChannel(channel);

    ASSERT_TRUE(channel->isMember(client1));
    ASSERT_TRUE(channel->isMember(client2));
    ASSERT_EQ(channel->getMembers().size(), 2);

    // 2. client1 をサーバーから削除
    server->removeClient(client1->getFd());

    // 3. client1 がチャンネルから削除され、client2 は残っていることを確認
    EXPECT_FALSE(channel->isMember(client1));
    EXPECT_TRUE(channel->isMember(client2));
    EXPECT_EQ(channel->getMembers().size(), 1);
}
