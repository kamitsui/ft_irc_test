// #include "gtest/gtest.h"
// #include "UserCommand.hpp"
// #include "Server.hpp"
// #include "Client.hpp"
// #include "Replies.hpp"
//
// class UserCommandTest : public ::testing::Test {
// protected:
//     void SetUp() override {
//         server = new Server("password");
//         client = new Client(1, "127.0.0.1");
//         client->setAuthenticated(true); // Assume authenticated for most tests
//         userCmd = new UserCommand();
//     }
//
//     void TearDown() override {
//         delete server;
//         delete client;
//         delete userCmd;
//     }
//
//     Server* server;
//     Client* client;
//     UserCommand* userCmd;
// };
//
// TEST_F(UserCommandTest, SetValidUserAndRealname) {
//     client->setNickname("testnick"); // Nickname must be set for registration
//     std::vector<std::string> args;
//     args.push_back("testuser");
//     args.push_back("0"); // mode
//     args.push_back("*"); // unused
//     args.push_back(":Test Realname");
//
//     userCmd->execute(server, client, args);
//
//     EXPECT_EQ(client->getUsername(), "testuser");
//     EXPECT_EQ(client->getRealname(), "Test Realname");
//     EXPECT_TRUE(client->isRegistered());
//     EXPECT_TRUE(client->getSendBuffer().empty());
// }
//
// TEST_F(UserCommandTest, NotEnoughParams) {
//     std::vector<std::string> args;
//     args.push_back("testuser");
//     args.push_back("0");
//     args.push_back("*");
//
//     userCmd->execute(server, client, args);
//
//     std::string expected_reply = ERR_NEEDMOREPARAMS(client->getNickname(), "USER");
//     EXPECT_EQ(client->getSendBuffer(), expected_reply);
//     EXPECT_FALSE(client->isRegistered());
// }
//
// TEST_F(UserCommandTest, AlreadyRegistered) {
//     client->setAuthenticated(true);
//     client->setNickname("testnick");
//     client->setUsername("testuser");
//     client->setRealname("Test Realname");
//     client->setRegistered(true);
//
//     std::vector<std::string> args;
//     args.push_back("anotheruser");
//     args.push_back("0");
//     args.push_back("*");
//     args.push_back(":Another Realname");
//
//     userCmd->execute(server, client, args);
//
//     std::string expected_reply = ERR_ALREADYREGISTRED(client->getNickname());
//     EXPECT_EQ(client->getSendBuffer(), expected_reply);
//     // Ensure username/realname are not changed
//     EXPECT_EQ(client->getUsername(), "testuser");
//     EXPECT_EQ(client->getRealname(), "Test Realname");
// }
//
// TEST_F(UserCommandTest, NotAuthenticated) {
//     client->setAuthenticated(false);
//     std::vector<std::string> args;
//     args.push_back("testuser");
//     args.push_back("0");
//     args.push_back("*");
//     args.push_back(":Test Realname");
//
//     userCmd->execute(server, client, args);
//
//     EXPECT_EQ(client->getUsername(), "");
//     EXPECT_EQ(client->getRealname(), "");
//     EXPECT_FALSE(client->isRegistered());
//     // Expect an error reply, e.g., ERR_NOTREGISTERED or similar
//     EXPECT_FALSE(client->getSendBuffer().empty());
// }
#include "TestFixture.hpp"

TEST_F(CommandTest, User_Success_CompletesRegistration) {
    client1->setAuthenticated(true);
    client1->setNickname("NewNick"); // NICKは通過済み

    args.push_back("user");
    args.push_back("0");
    args.push_back("*");
    args.push_back("Real Name");
    userCmd->execute(client1, args);

    ASSERT_EQ(client1->getUsername(), "user");
    ASSERT_TRUE(client1->isRegistered());
    ASSERT_NE(client1->getLastMessage().find(RPL_WELCOME), std::string::npos);
}

TEST_F(CommandTest, User_NeedMoreParams) {
    client1->setAuthenticated(true);
    args.push_back("user");
    userCmd->execute(client1, args); // 引数が足りない
    ASSERT_FALSE(client1->isRegistered());
    ASSERT_NE(client1->getLastMessage().find(ERR_NEEDMOREPARAMS), std::string::npos);
}

TEST_F(CommandTest, User_AlreadyRegistered) {
    registerClient(client1, "user1");
    args.push_back("user");
    args.push_back("0");
    args.push_back("*");
    args.push_back("Real Name");
    userCmd->execute(client1, args);
    ASSERT_NE(client1->getLastMessage().find(ERR_ALREADYREGISTRED), std::string::npos);
}
