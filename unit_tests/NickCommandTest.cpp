// #include "gtest/gtest.h"
// #include "NickCommand.hpp"
// #include "Server.hpp"
// #include "Client.hpp"
// #include "Replies.hpp"
//
// class NickCommandTest : public ::testing::Test {
// protected:
//     void SetUp() override {
//         server = new Server("password");
//         client1 = new Client(1, "127.0.0.1");
//         client1->setAuthenticated(true); // Assume authenticated for most tests
//
//         client2 = new Client(2, "127.0.0.1");
//         client2->setNickname("existing_nick");
//         server->addClient(client2);
//
//         nickCmd = new NickCommand();
//     }
//
//     void TearDown() override {
//         delete server;
//         delete client1;
//         // client2 is deleted by server's destructor
//         delete nickCmd;
//     }
//
//     Server* server;
//     Client* client1;
//     Client* client2;
//     NickCommand* nickCmd;
// };
//
// TEST_F(NickCommandTest, SetValidNickname) {
//     std::vector<std::string> args;
//     args.push_back("new_nick");
//
//     nickCmd->execute(server, client1, args);
//
//     EXPECT_EQ(client1->getNickname(), "new_nick");
//     EXPECT_TRUE(client1->getSendBuffer().empty());
// }
//
// TEST_F(NickCommandTest, NoNicknameGiven) {
//     std::vector<std::string> args;
//
//     nickCmd->execute(server, client1, args);
//
//     std::string expected_reply = ERR_NONICKNAMEGIVEN(client1->getNickname());
//     EXPECT_EQ(client1->getSendBuffer(), expected_reply);
// }
//
// TEST_F(NickCommandTest, ErroneousNickname) {
//     std::vector<std::string> args;
//     args.push_back("invalid-nick!");
//
//     nickCmd->execute(server, client1, args);
//
//     std::string expected_reply = ERR_ERRONEUSNICKNAME(client1->getNickname(), "invalid-nick!");
//     EXPECT_EQ(client1->getSendBuffer(), expected_reply);
// }
//
// TEST_F(NickCommandTest, NicknameInUse) {
//     std::vector<std::string> args;
//     args.push_back("existing_nick");
//
//     nickCmd->execute(server, client1, args);
//
//     std::string expected_reply = ERR_NICKNAMEINUSE(client1->getNickname(), "existing_nick");
//     EXPECT_EQ(client1->getSendBuffer(), expected_reply);
// }
//
// TEST_F(NickCommandTest, NotAuthenticated) {
//     client1->setAuthenticated(false);
//     std::vector<std::string> args;
//     args.push_back("any_nick");
//
//     nickCmd->execute(server, client1, args);
//
//     // Behavior for unauthenticated users can vary.
//     // Assuming it sends an error or just ignores.
//     // A common approach is to check that the nickname was NOT set.
//     EXPECT_EQ(client1->getNickname(), "");
//     // You might expect a specific error reply here, like ERR_NOTREGISTERED
//     // For now, we'll just check if the send buffer is not empty.
//     EXPECT_FALSE(client1->getSendBuffer().empty());
// }
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
    args.push_back("NewNick");
    nickCmd->execute(client1, args);

    ASSERT_EQ(client1->getNickname(), "NewNick");
    ASSERT_TRUE(client1->isRegistered()); // 登録完了

    // デバッグ出力 (これは残しても構いません)
    // std::cout << "client1_recievedMessages: " << DebugVector<std::string>(client1->receivedMessages) << std::endl;

    // --- 修正箇所 ---
    // メッセージが空でないことを確認
    ASSERT_FALSE(client1->receivedMessages.empty());
    // 0番目のメッセージに RPL_WELCOME ("001") が含まれていることを確認
    ASSERT_NE(client1->receivedMessages[0].find(RPL_WELCOME), std::string::npos);

    // (オプション) 最後のメッセージが RPL_YOURHOST ("002") であることも確認できます
    ASSERT_NE(client1->getLastMessage().find(RPL_YOURHOST), std::string::npos);
}

TEST_F(CommandTest, Nick_NotAuthenticated) {
    args.push_back("NewNick");
    nickCmd->execute(client1, args);
    ASSERT_EQ(client1->getNickname(), ""); // 認証前は無視される
}

TEST_F(CommandTest, Nick_NicknameInUse) {
    registerClient(client2, "TakenNick");
    client1->setAuthenticated(true);

    args.push_back("TakenNick");
    nickCmd->execute(client1, args);
    ASSERT_NE(client1->getNickname(), "TakenNick"); // ニックネームは変わらない
    ASSERT_NE(client1->getLastMessage().find(ERR_NICKNAMEINUSE), std::string::npos);
}

TEST_F(CommandTest, Nick_ErroneousNickname) {
    client1->setAuthenticated(true);
    args.push_back("1InvalidNick"); // 数字で始まる
    nickCmd->execute(client1, args);
    ASSERT_NE(client1->getLastMessage().find(ERR_ERRONEUSNICKNAME), std::string::npos);
}
