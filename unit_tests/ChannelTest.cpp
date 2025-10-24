// #include "Channel.hpp"
// #include "Client.hpp"
// #include "gtest/gtest.h"
//
// class ChannelTest : public ::testing::Test {
//   protected:
//     void SetUp() override {
//         // Test objects
//         client1 = new Client(1, "127.0.0.1");
//         client1->setNickname("user1");
//         client1->setUsername("userone");
//         client1->setRegistered(true);
//
//         client2 = new Client(2, "127.0.0.1");
//         client2->setNickname("user2");
//         client2->setUsername("usertwo");
//         client2->setRegistered(true);
//
//         op = new Client(3, "127.0.0.1");
//         op->setNickname("operator");
//         op->setUsername("opone");
//         op->setRegistered(true);
//     }
//
//     void TearDown() override {
//         delete client1;
//         delete client2;
//         delete op;
//     }
//
//     Client *client1;
//     Client *client2;
//     Client *op;
// };
//
// TEST_F(ChannelTest, Constructor) {
//     Channel channel("#test");
//     EXPECT_EQ(channel.getName(), "#test");
//     EXPECT_EQ(channel.getTopic(), "");
//     EXPECT_TRUE(channel.getMembers().empty());
//     // EXPECT_TRUE(channel.getOperators().empty());
// }
//
// TEST_F(ChannelTest, AddUser) {
//     Channel channel("#test");
//     channel.addMember(client1);
//     EXPECT_TRUE(channel.isMember(client1));
//     EXPECT_FALSE(channel.isMember(client2));
//     EXPECT_EQ(channel.getMembers().size(), 1);
// }
//
// TEST_F(ChannelTest, RemoveUser) {
//     Channel channel("#test");
//     channel.addMember(client1);
//     channel.addMember(client2);
//     EXPECT_EQ(channel.getMembers().size(), 2);
//
//     channel.removeMember(client1);
//     EXPECT_FALSE(channel.isMember(client1));
//     EXPECT_TRUE(channel.isMember(client2));
//     EXPECT_EQ(channel.getMembers().size(), 1);
// }
//
//// TEST_F(ChannelTest, AddOperator) {
////     Channel channel("#test");
////     channel.addMember(op);
////     channel.addOperator(op);
////     EXPECT_TRUE(channel.isOperator(op));
////     EXPECT_FALSE(channel.isOperator(client1));
//// }
//
//// TEST_F(ChannelTest, RemoveOperator) {
////     Channel channel("#test");
////     channel.addMember(op);
////     channel.addOperator(op);
////     EXPECT_TRUE(channel.isOperator(op));
////
////     channel.removeOperator(op);
////     EXPECT_FALSE(channel.isOperator(op));
//// }
//
// TEST_F(ChannelTest, SetAndGetTopic) {
//    Channel channel("#test");
//    channel.setTopic("This is a test topic.");
//    EXPECT_EQ(channel.getTopic(), "This is a test topic.");
//}
//
//// TEST_F(ChannelTest, GetUserList) {
////     Channel channel("#test");
////     channel.addMember(client1);
////     channel.addMember(op);
////     channel.addOperator(op);
////
////     std::string userList = channel.getUserList();
////     // Order might vary, so check for presence of both nicknames
////     EXPECT_NE(userList.find("user1"), std::string::npos);
////     EXPECT_NE(userList.find("@operator"), std::string::npos);
//// }

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
    ASSERT_EQ(client1->getLastMessage(), "Hello\r\n");
    ASSERT_EQ(client2->getLastMessage(), "Hello\r\n");
}

TEST_F(ChannelTest, Broadcast_ExcludeClient) {
    channel->broadcast("Hello", client1);     // client1 を除く
    ASSERT_EQ(client1->getLastMessage(), ""); // client1には送信されない
    ASSERT_EQ(client2->getLastMessage(), "Hello\r\n");
}

TEST_F(ChannelTest, MemberManagement) {
    ASSERT_TRUE(channel->isMember(client1));
    ASSERT_TRUE(channel->isMember(client2));

    channel->removeMember(client1);
    ASSERT_FALSE(channel->isMember(client1));
    ASSERT_TRUE(channel->isMember(client2));
}
