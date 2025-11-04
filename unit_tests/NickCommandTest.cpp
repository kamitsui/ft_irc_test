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
