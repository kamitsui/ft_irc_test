#include "TestFixture.hpp"

TEST_F(CommandTest, Nick_Success_BeforeUser) {
    client1->setAuthenticated(true); // PASSは通過済みとする
    args.push_back("NewNick");
    nickCmd->execute(client1, args);
    ASSERT_EQ(client1->getNickname(), "NewNick");
    ASSERT_FALSE(client1->isRegistered()); // USERがまだなので未登録
}

TEST_F(CommandTest, Nick_Success_CompletesRegistration) {
    client1->setAuthenticated(true);
    client1->setUsername("user"); // USERは通過済み
    //client1->setHostname("host");// ?
    args.push_back("NewNick");
    nickCmd->execute(client1, args);

    ASSERT_EQ(client1->getNickname(), "NewNick");
    ASSERT_TRUE(client1->isRegistered());

    ASSERT_EQ(client1->receivedMessages.size(), 2);

    // 001 RPL_WELCOME
    std::vector<std::string> welcome_args;
    welcome_args.push_back(client1->getNickname());
    welcome_args.push_back(client1->getPrefix());
    std::string expected_welcome =
        formatReply(server->getServerName(), client1->getNickname(), RPL_WELCOME, welcome_args) +
        "\r\n";
    EXPECT_EQ(client1->receivedMessages[0], expected_welcome);

    // 002 RPL_YOURHOST
    std::vector<std::string> yourhost_args;
    yourhost_args.push_back(client1->getNickname());
    yourhost_args.push_back(server->getServerName());
    std::string expected_yourhost =
        formatReply(server->getServerName(), client1->getNickname(), RPL_YOURHOST, yourhost_args) +
        "\r\n";
    EXPECT_EQ(client1->receivedMessages[1], expected_yourhost);
}

TEST_F(CommandTest, Nick_NotAuthenticated) {
    args.push_back("NewNick");
    nickCmd->execute(client1, args);
    ASSERT_EQ(client1->getNickname(), ""); // 認証前は無視される
}

TEST_F(CommandTest, Nick_NicknameInUse) {
    registerClient(client2, "TakenNick");
    client1->setAuthenticated(true);
    client1->setNickname("user1");

    args.push_back("TakenNick");
    nickCmd->execute(client1, args);
    ASSERT_NE(client1->getNickname(), "TakenNick"); // ニックネームは変わらない

    std::vector<std::string> params;
    params.push_back("TakenNick");
    std::string expected_reply =
        formatReply(server->getServerName(), "*", ERR_NICKNAMEINUSE, params) + "\r\n";
    EXPECT_EQ(client1->getLastMessage(), expected_reply);
}

TEST_F(CommandTest, Nick_ErroneousNickname) {
    client1->setAuthenticated(true);
    client1->setNickname("user1");
    args.push_back("1InvalidNick"); // 数字で始まる
    nickCmd->execute(client1, args);

    std::vector<std::string> params;
    params.push_back("1InvalidNick");
    std::string expected_reply =
        formatReply(server->getServerName(), "*", ERR_ERRONEUSNICKNAME, params) + "\r\n";
    EXPECT_EQ(client1->getLastMessage(), expected_reply);
}
