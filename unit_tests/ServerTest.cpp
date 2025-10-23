#include "gtest/gtest.h"
#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"

class ServerTest : public ::testing::Test {
protected:
    void SetUp() override {
        server = new Server("secret_password");
        
        client1 = new Client(1, "127.0.0.1");
        client1->setNickname("client1");
        
        client2 = new Client(2, "127.0.0.1");
        client2->setNickname("client2");

        channel = new Channel("#test");
    }

    void TearDown() override {
        delete server;
        // client1, client2 and channel are managed and deleted by the server's destructor
        // if they are added to the server. If not, we might need to delete them manually.
        // In this test setup, server's destructor will handle them.
    }

    Server* server;
    Client* client1;
    Client* client2;
    Channel* channel;
};

TEST_F(ServerTest, PasswordCheck) {
    EXPECT_TRUE(server->checkPassword("secret_password"));
    EXPECT_FALSE(server->checkPassword("wrong_password"));
}

TEST_F(ServerTest, AddAndGetClient) {
    server->addClient(client1);
    server->addClient(client2);

    EXPECT_EQ(server->getClientByFd(1), client1);
    EXPECT_EQ(server->getClientByFd(2), client2);
    EXPECT_EQ(server->getClientByFd(99), (Client*)NULL);

    EXPECT_EQ(server->getClientByNickname("client1"), client1);
    EXPECT_EQ(server->getClientByNickname("client2"), client2);
    EXPECT_EQ(server->getClientByNickname("nonexistent"), (Client*)NULL);
}

TEST_F(ServerTest, RemoveClient) {
    server->addClient(client1);
    server->addClient(client2);

    server->removeClient(client1);

    EXPECT_EQ(server->getClientByFd(1), (Client*)NULL);
    EXPECT_EQ(server->getClientByNickname("client1"), (Client*)NULL);
    EXPECT_NE(server->getClientByFd(2), (Client*)NULL); // client2 should still be there
}

TEST_F(ServerTest, AddAndGetChannel) {
    server->addChannel(channel);
    EXPECT_EQ(server->getChannelByName("#test"), channel);
    EXPECT_EQ(server->getChannelByName("#nonexistent"), (Channel*)NULL);
}

TEST_F(ServerTest, RemoveChannel) {
    server->addChannel(channel);
    server->removeChannel(channel->getName());
    EXPECT_EQ(server->getChannelByName("#test"), (Channel*)NULL);
}

TEST_F(ServerTest, RemoveClientAlsoRemovesFromChannel) {
    server->addClient(client1);
    server->addClient(client2);
    server->addChannel(channel);

    channel->addUser(client1);
    channel->addUser(client2);

    EXPECT_TRUE(channel->isUserInChannel(client1));
    EXPECT_EQ(channel->getUsers().size(), 2);

    server->removeClient(client1); // This should trigger removal from the channel

    EXPECT_FALSE(channel->isUserInChannel(client1));
    EXPECT_EQ(channel->getUsers().size(), 1);
}

TEST_F(ServerTest, RemoveLastUserDestroysChannel) {
    server->addClient(client1);
    server->addChannel(channel);
    channel->addUser(client1);

    EXPECT_NE(server->getChannelByName("#test"), (Channel*)NULL);

    // Removing the last user from the channel should cause the server to delete it.
    channel->removeUser(client1);
    server->removeChannelIfEmpty(channel->getName());

    EXPECT_EQ(server->getChannelByName("#test"), (Channel*)NULL);
}
