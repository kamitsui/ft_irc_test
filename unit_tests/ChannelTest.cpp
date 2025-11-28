#include "TestFixture.hpp"

class ChannelTest : public CommandTest {
  protected:
    Channel *channel;
    virtual void SetUp() {
        // 親のSetUpを呼び出す
        CommandTest::SetUp();
        channel = new Channel("#test");
        server->addChannel(channel);

        // 3人のクライアントをチャンネルに参加させる
        channel->addMember(client1);
        channel->addMember(client2);
    }

    virtual void TearDown() {
        // ChannelはServerが削除するので、ここでは何もしない
        // (Server::resetInstance() で server ごと削除される)
        CommandTest::TearDown();
    }
};

TEST_F(ChannelTest, Broadcast_AllMembers) {
    channel->broadcast("Hello", NULL);
    ASSERT_EQ(client1->getLastMessage(), "Hello");
    ASSERT_EQ(client2->getLastMessage(), "Hello");
}

TEST_F(ChannelTest, Broadcast_ExcludeClient) {
    channel->broadcast("Hello", client1);     // client1 を除く
    ASSERT_EQ(client1->getLastMessage(), ""); // client1には送信されない
    ASSERT_EQ(client2->getLastMessage(), "Hello");
}

TEST_F(ChannelTest, MemberManagement) {
    // Clear members from SetUp
    channel->removeMember(client1);
    channel->removeMember(client2);
    ASSERT_EQ(channel->getMembers().size(), static_cast<std::string::size_type>(0));

    client1->setNickname("User1");
    client2->setNickname("User2");

    channel->addMember(client1);
    ASSERT_TRUE(channel->isMember(client1));
    ASSERT_EQ(channel->getMembers().size(), static_cast<std::string::size_type>(1));

    channel->addMember(client2);
    ASSERT_TRUE(channel->isMember(client2));
    ASSERT_EQ(channel->getMembers().size(), static_cast<std::string::size_type>(2));

    channel->removeMember(client1);
    ASSERT_FALSE(channel->isMember(client1));
    ASSERT_EQ(channel->getMembers().size(), static_cast<std::string::size_type>(1));
}

TEST_F(ChannelTest, TopicManagement) {
    ASSERT_EQ(channel->getTopic(), ""); // Initial topic should be empty

    channel->setTopic("This is a test topic.");
    ASSERT_EQ(channel->getTopic(), "This is a test topic.");

    channel->setTopic("A new topic.");
    ASSERT_EQ(channel->getTopic(), "A new topic.");
}

TEST_F(ChannelTest, OperatorManagement) {
    // client1 is not an operator initially
    ASSERT_FALSE(channel->isOperator(client1));

    // Add client1 as an operator
    channel->addOperator(client1);
    ASSERT_TRUE(channel->isOperator(client1));
    ASSERT_FALSE(channel->isOperator(client2)); // client2 should still not be an op

    // Remove client1 as an operator
    channel->removeOperator(client1);
    ASSERT_FALSE(channel->isOperator(client1));
}

TEST_F(ChannelTest, ModeManagement) {
    // Initial modes
    ASSERT_FALSE(channel->hasMode('t'));
    ASSERT_FALSE(channel->hasMode('n'));
    ASSERT_EQ(channel->getModes(), "+");

    // Set mode +t
    channel->setMode('t', true);
    ASSERT_TRUE(channel->hasMode('t'));
    ASSERT_FALSE(channel->hasMode('n'));
    ASSERT_EQ(channel->getModes(), "+t");

    // Set mode +n
    channel->setMode('n', true);
    ASSERT_TRUE(channel->hasMode('t'));
    ASSERT_TRUE(channel->hasMode('n'));
    ASSERT_EQ(channel->getModes(), "+nt");

    // Unset mode +t
    channel->setMode('t', false);
    ASSERT_FALSE(channel->hasMode('t'));
    ASSERT_TRUE(channel->hasMode('n'));
    ASSERT_EQ(channel->getModes(), "+n");

    // Try to set an unsupported mode
    channel->setMode('x', true);
    ASSERT_FALSE(channel->hasMode('x'));
    ASSERT_EQ(channel->getModes(), "+n");
}

TEST_F(ChannelTest, KeyManagement) {
    ASSERT_EQ(channel->getKey(), "");
    channel->setKey("testkey");
    ASSERT_EQ(channel->getKey(), "testkey");
    ASSERT_TRUE(channel->checkKey("testkey"));
    ASSERT_FALSE(channel->checkKey("wrongkey"));
}

TEST_F(ChannelTest, LimitManagement) {
    ASSERT_EQ(channel->getLimit(), static_cast<std::string::size_type>(0));
    channel->setLimit(5);
    ASSERT_EQ(channel->getLimit(), static_cast<std::string::size_type>(5));
}

TEST_F(ChannelTest, InvitedUserManagement) {
    ASSERT_FALSE(channel->isInvitedUser(client1));
    channel->addInvitedUser(client1);
    ASSERT_TRUE(channel->isInvitedUser(client1));
    ASSERT_FALSE(channel->isInvitedUser(client2));

    channel->removeInvitedUser(client1);
    ASSERT_FALSE(channel->isInvitedUser(client1));
}
