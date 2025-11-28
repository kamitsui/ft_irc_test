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

    // 1. JOIN通知, 2. RPL_NOTOPIC, 3. RPL_NAMREPLY, 4. RPL_ENDOFNAMES の4つが届く

    ASSERT_EQ(client1->receivedMessages.size(), static_cast<std::string::size_type>(4));

    // 1. JOIN通知
    std::string expected_join_msg = client1->getPrefix() + " JOIN " + "#new" + "\r\n";
    EXPECT_EQ(client1->receivedMessages[0], expected_join_msg);

    // 2. RPL_NOTOPIC (331)
    std::vector<std::string> notopic_args;
    notopic_args.push_back("#new");
    std::string expected_notopic =
        formatReply(server->getServerName(), client1->getNickname(), RPL_NOTOPIC, notopic_args);
    EXPECT_EQ(client1->receivedMessages[1], expected_notopic);

    // 3. RPL_NAMREPLY (353)
    std::vector<std::string> namreply_args;
    namreply_args.push_back("#new");
    namreply_args.push_back("@" + client1->getNickname()); // 最初のメンバーはオペレータ
    std::string expected_namreply =
        formatReply(server->getServerName(), client1->getNickname(), RPL_NAMREPLY, namreply_args);
    EXPECT_EQ(client1->receivedMessages[2], expected_namreply);

    // 4. RPL_ENDOFNAMES (366)
    std::vector<std::string> endofnames_args;
    endofnames_args.push_back("#new");
    std::string expected_endofnames =
        formatReply(server->getServerName(), client1->getNickname(), RPL_ENDOFNAMES, endofnames_args);
    EXPECT_EQ(client1->receivedMessages[3], expected_endofnames);
}

TEST_F(ChannelCommandsTest, Join_ExistingChannel) {
    // client2 が先に #test に参加
    args.push_back("#test");
    joinCmd->execute(client2, args);
    client2->receivedMessages.clear(); // client2 のJOIN通知はクリア

    // client1 が #test に参加
    joinCmd->execute(client1, args);

    // client2 に client1 のJOINが通知される
    EXPECT_EQ(client2->getLastMessage(), std::string(":User1!user@client1.host JOIN #test") + "\r\n");
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
    ASSERT_EQ(client2->getLastMessage(),
              std::string(":User1!user@client1.host PRIVMSG #test :Hello Channel!") + "\r\n");
}

TEST_F(ChannelCommandsTest, Privmsg_ToUser) {
    args.push_back("User2"); // client2 のニックネーム
    args.push_back("Hello User2!");
    privmsgCmd->execute(client1, args);

    ASSERT_TRUE(client1->receivedMessages.empty());
    ASSERT_EQ(client2->getLastMessage(), std::string(":User1!user@client1.host PRIVMSG User2 :Hello User2!") + "\r\n");
}

TEST_F(ChannelCommandsTest, Privmsg_NotOnChannel) {
    // #test を作成 (client2のみ参加)
    args.push_back("#test");
    joinCmd->execute(client2, args);

    // client1 は参加していない
    client1->receivedMessages.clear();
    args.clear();
    args.push_back("#test");
    args.push_back("Am I here?");
    privmsgCmd->execute(client1, args);

    // +n モードがデフォルトで設定されていないため、メッセージは送信されるはず
    // (CommandUtils.cpp の変更により、+n がない場合は外部メッセージを許可するようになった)
    ASSERT_EQ(client2->getLastMessage(), std::string(":User1!user@client1.host PRIVMSG #test :Am I here?") + "\r\n");
    ASSERT_TRUE(client1->receivedMessages.empty()); // 送信者には返信なし
}

TEST_F(ChannelCommandsTest, Privmsg_NotOnChannel_WithNoExternalMessagesMode) {
    // #test を作成 (client2のみ参加)
    args.push_back("#test");
    joinCmd->execute(client2, args);

    // client2 (オペレーター) が +n モードを設定
    args.clear();
    args.push_back("#test");
    args.push_back("+n");
    modeCmd->execute(client2, args);
    client2->receivedMessages.clear(); // Clear mode broadcast message

    // client1 は参加していない
    client1->receivedMessages.clear();
    args.clear();
    args.push_back("#test");
    args.push_back("Am I here?");
    privmsgCmd->execute(client1, args);

    // ERR_CANNOTSENDTOCHAN が返る
    std::vector<std::string> params;
    params.push_back("#test");
    std::string expected_reply =
        formatReply(server->getServerName(), client1->getNickname(), ERR_CANNOTSENDTOCHAN, params);
    EXPECT_EQ(client1->getLastMessage(), expected_reply);
    ASSERT_TRUE(client2->receivedMessages.empty()); // client2にはメッセージが届かない
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

// --- Multiple Channel Operations ---

TEST_F(ChannelCommandsTest, Join_MultipleChannels) {
    args.push_back("#chan1,#chan2");
    joinCmd->execute(client1, args);

    Channel *ch1 = server->getChannel("#chan1");
    Channel *ch2 = server->getChannel("#chan2");

    ASSERT_TRUE(ch1 != NULL);
    ASSERT_TRUE(ch2 != NULL);
    EXPECT_TRUE(ch1->isMember(client1));
    EXPECT_TRUE(ch2->isMember(client1));
    EXPECT_EQ(client1->getChannels().size(), static_cast<std::string::size_type>(2));
}

TEST_F(ChannelCommandsTest, Part_MultipleChannels) {
    // Setup: client1 joins #chan1 and #chan2
    Channel *ch1 = new Channel("#chan1");
    Channel *ch2 = new Channel("#chan2");
    server->addChannel(ch1);
    server->addChannel(ch2);
    ch1->addMember(client1);
    client1->addChannel(ch1);
    ch2->addMember(client1);
    client1->addChannel(ch2);

    args.push_back("#chan1,#chan2");
    partCmd->execute(client1, args);

    EXPECT_FALSE(ch1->isMember(client1));
    EXPECT_FALSE(ch2->isMember(client1));
    EXPECT_TRUE(client1->getChannels().empty());
}

TEST_F(ChannelCommandsTest, Join_ZeroPartsAllChannels) {
    // Setup: client1 joins #chan1 and #chan2
    Channel *ch1 = new Channel("#chan1");
    Channel *ch2 = new Channel("#chan2");
    server->addChannel(ch1);
    server->addChannel(ch2);
    ch1->addMember(client1);
    client1->addChannel(ch1);
    ch2->addMember(client1);
    client1->addChannel(ch2);

    args.push_back("0");
    joinCmd->execute(client1, args);

    EXPECT_FALSE(ch1->isMember(client1));
    EXPECT_FALSE(ch2->isMember(client1));
    EXPECT_TRUE(client1->getChannels().empty());
}
