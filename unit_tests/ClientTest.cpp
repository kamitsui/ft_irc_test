#include "gtest/gtest.h"
#include "Client.hpp"

TEST(ClientTest, Constructor) {
    Client client(1, "127.0.0.1");
    EXPECT_EQ(client.getFd(), 1);
    EXPECT_EQ(client.getHostname(), "127.0.0.1");
    EXPECT_EQ(client.getNickname(), "");
    EXPECT_EQ(client.getUsername(), "");
    EXPECT_EQ(client.getRealname(), "");
    EXPECT_FALSE(client.isAuthenticated());
    EXPECT_FALSE(client.isRegistered());
}

TEST(ClientTest, SettersAndGetters) {
    Client client(2, "localhost");
    client.setNickname("testnick");
    client.setUsername("testuser");
    client.setRealname("Test User");
    client.setAuthenticated(true);

    EXPECT_EQ(client.getNickname(), "testnick");
    EXPECT_EQ(client.getUsername(), "testuser");
    EXPECT_EQ(client.getRealname(), "Test User");
    EXPECT_TRUE(client.isAuthenticated());

    // isRegistered depends on nickname, username and authenticated status
    EXPECT_FALSE(client.isRegistered());
    client.setRegistered(true);
    EXPECT_TRUE(client.isRegistered());
}
