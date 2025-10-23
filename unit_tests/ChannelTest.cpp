#include "gtest/gtest.h"
#include "Channel.hpp"
#include "Client.hpp"

class ChannelTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Test objects
        client1 = new Client(1, "127.0.0.1");
        client1->setNickname("user1");
        client1->setUsername("userone");
        client1->setRegistered(true);

        client2 = new Client(2, "127.0.0.1");
        client2->setNickname("user2");
        client2->setUsername("usertwo");
        client2->setRegistered(true);

        op = new Client(3, "127.0.0.1");
        op->setNickname("operator");
        op->setUsername("opone");
        op->setRegistered(true);
    }

    void TearDown() override {
        delete client1;
        delete client2;
        delete op;
    }

    Client* client1;
    Client* client2;
    Client* op;
};

TEST_F(ChannelTest, Constructor) {
    Channel channel("#test");
    EXPECT_EQ(channel.getName(), "#test");
    EXPECT_EQ(channel.getTopic(), "");
    EXPECT_TRUE(channel.getUsers().empty());
    EXPECT_TRUE(channel.getOperators().empty());
}

TEST_F(ChannelTest, AddUser) {
    Channel channel("#test");
    channel.addUser(client1);
    EXPECT_TRUE(channel.isUserInChannel(client1));
    EXPECT_FALSE(channel.isUserInChannel(client2));
    EXPECT_EQ(channel.getUsers().size(), 1);
}

TEST_F(ChannelTest, RemoveUser) {
    Channel channel("#test");
    channel.addUser(client1);
    channel.addUser(client2);
    EXPECT_EQ(channel.getUsers().size(), 2);

    channel.removeUser(client1);
    EXPECT_FALSE(channel.isUserInChannel(client1));
    EXPECT_TRUE(channel.isUserInChannel(client2));
    EXPECT_EQ(channel.getUsers().size(), 1);
}

TEST_F(ChannelTest, AddOperator) {
    Channel channel("#test");
    channel.addUser(op);
    channel.addOperator(op);
    EXPECT_TRUE(channel.isOperator(op));
    EXPECT_FALSE(channel.isOperator(client1));
}

TEST_F(ChannelTest, RemoveOperator) {
    Channel channel("#test");
    channel.addUser(op);
    channel.addOperator(op);
    EXPECT_TRUE(channel.isOperator(op));

    channel.removeOperator(op);
    EXPECT_FALSE(channel.isOperator(op));
}

TEST_F(ChannelTest, SetAndGetTopic) {
    Channel channel("#test");
    channel.setTopic("This is a test topic.");
    EXPECT_EQ(channel.getTopic(), "This is a test topic.");
}

TEST_F(ChannelTest, GetUserList) {
    Channel channel("#test");
    channel.addUser(client1);
    channel.addUser(op);
    channel.addOperator(op);

    std::string userList = channel.getUserList();
    // Order might vary, so check for presence of both nicknames
    EXPECT_NE(userList.find("user1"), std::string::npos);
    EXPECT_NE(userList.find("@operator"), std::string::npos);
}
