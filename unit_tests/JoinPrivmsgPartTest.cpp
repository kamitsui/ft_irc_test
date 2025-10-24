#include "TestFixture.hpp"

// JOIN, PRIVMSG, PART は相互作用が多いため、まとめる
class ChannelCommandsTest : public CommandTest {
  protected:
    virtual void SetUp() {
        CommandTest::SetUp();
        // client1, client2 を両方とも登録済みにする
        registerClient(client1, "User1");
        registerClient(client2, "User2");
    }
};

TEST_F(ChannelCommandsTest, Join_NewChannel) {
    args.push_back("#new");
    joinCmd->execute(client1, args);

    Channel *ch = server->getChannel("#new");
    ASSERT_TRUE(ch != NULL);
    ASSERT_TRUE(ch->isMember(client1));
    ASSERT_NE(client1->getLastMessage().find(RPL_ENDOFNAMES), std::string::npos);  // NAMESリストが返る
    ASSERT_NE(client1->receivedMessages[0].find("JOIN :#new"), std::string::npos); // 自分へのJOIN通知
}

TEST_F(ChannelCommandsTest, Join_ExistingChannel) {
    // client2 が先に #test に参加
    args.push_back("#test");
    joinCmd->execute(client2, args);
    client2->receivedMessages.clear(); // client2 のJOIN通知はクリア

    // client1 が #test に参加
    joinCmd->execute(client1, args);

    // client2 に client1 のJOINが通知される
    ASSERT_NE(client2->getLastMessage().find(":User1!user@client1.host JOIN :#test"), std::string::npos);
}

TEST_F(ChannelCommandsTest, Privmsg_ToChannel) {
    // 2人を #test に参加させる
    args.push_back("#test");
    joinCmd->execute(client1, args);
    joinCmd->execute(client2, args);

    // client1 が発言
    client1->receivedMessages.clear();
    client2->receivedMessages.clear();
    args.clear();
    args.push_back("#test");
    args.push_back("Hello Channel!");
    privmsgCmd->execute(client1, args);

    // 自分(client1)には送信されない
    ASSERT_TRUE(client1->receivedMessages.empty());
    // 相手(client2)に送信される
    ASSERT_EQ(client2->getLastMessage(), ":User1!user@client1.host PRIVMSG #test :Hello Channel!\r\n");
}

TEST_F(ChannelCommandsTest, Privmsg_ToUser) {
    args.push_back("User2"); // client2 のニックネーム
    args.push_back("Hello User2!");
    privmsgCmd->execute(client1, args);

    ASSERT_TRUE(client1->receivedMessages.empty());
    ASSERT_EQ(client2->getLastMessage(), ":User1!user@client1.host PRIVMSG User2 :Hello User2!\r\n");
}

TEST_F(ChannelCommandsTest, Privmsg_NotOnChannel) {
    // #test を作成 (client2のみ参加)
    args.push_back("#test");
    joinCmd->execute(client2, args);

    // client1 は参加していない
    args.clear();
    args.push_back("#test");
    args.push_back("Am I here?");
    privmsgCmd->execute(client1, args);

    // ERR_CANNOTSENDTOCHAN が返る
    ASSERT_NE(client1->getLastMessage().find(ERR_CANNOTSENDTOCHAN), std::string::npos);
}

TEST_F(ChannelCommandsTest, Part_Success) {
    args.push_back("#test");
    joinCmd->execute(client1, args);
    joinCmd->execute(client2, args);
    client1->receivedMessages.clear();
    client2->receivedMessages.clear();

    // client1 が退出
    args.clear();
    args.push_back("#test");
    partCmd->execute(client1, args);

    Channel *ch = server->getChannel("#test");
    ASSERT_TRUE(ch != NULL);
    ASSERT_FALSE(ch->isMember(client1)); // 退出
    ASSERT_TRUE(ch->isMember(client2));  // client2は残る

    // 退出メッセージが自分と相手にブロードキャストされる
    ASSERT_NE(client1->getLastMessage().find(":User1!user@client1.host PART #test"), std::string::npos);
    ASSERT_NE(client2->getLastMessage().find(":User1!user@client1.host PART #test"), std::string::npos);
}

TEST_F(ChannelCommandsTest, Part_LastMember) {
    args.push_back("#test");
    joinCmd->execute(client1, args);

    // client1 が退出 (最後のメンバー)
    args.clear();
    args.push_back("#test");
    partCmd->execute(client1, args);

    // チャンネルが削除される
    ASSERT_TRUE(server->getChannel("#test") == NULL);
}
