// #include "gtest/gtest.h"
// #include "PassCommand.hpp"
// #include "Server.hpp"
// #include "Client.hpp"
// #include "Replies.hpp"
//
// class PassCommandTest : public ::testing::Test {
// protected:
//     void SetUp() override {
//         server = new Server("password");
//         client = new Client(1, "127.0.0.1");
//         passCmd = new PassCommand();
//     }
//
//     void TearDown() override {
//         delete server;
//         delete client;
//         delete passCmd;
//     }
//
//     Server* server;
//     Client* client;
//     PassCommand* passCmd;
// };
//
// TEST_F(PassCommandTest, CorrectPassword) {
//     std::vector<std::string> args;
//     args.push_back("password");
//
//     passCmd->execute(server, client, args);
//
//     EXPECT_TRUE(client->isAuthenticated());
//     // Assuming no reply is sent on successful password
//     EXPECT_TRUE(client->getSendBuffer().empty());
// }
//
// TEST_F(PassCommandTest, IncorrectPassword) {
//     std::vector<std::string> args;
//     args.push_back("wrongpassword");
//
//     passCmd->execute(server, client, args);
//
//     EXPECT_FALSE(client->isAuthenticated());
//     std::string expected_reply = ERR_PASSWDMISMATCH(client->getNickname());
//     EXPECT_EQ(client->getSendBuffer(), expected_reply);
// }
//
// TEST_F(PassCommandTest, NotEnoughParams) {
//     std::vector<std::string> args; // No arguments
//
//     passCmd->execute(server, client, args);
//
//     EXPECT_FALSE(client->isAuthenticated());
//     std::string expected_reply = ERR_NEEDMOREPARAMS(client->getNickname(), "PASS");
//     EXPECT_EQ(client->getSendBuffer(), expected_reply);
// }
//
// TEST_F(PassCommandTest, AlreadyRegistered) {
//     client->setAuthenticated(true);
//     client->setNickname("testnick");
//     client->setUsername("testuser");
//     client->setRegistered(true);
//
//     std::vector<std::string> args;
//     args.push_back("password");
//
//     passCmd->execute(server, client, args);
//
//     std::string expected_reply = ERR_ALREADYREGISTRED(client->getNickname());
//     EXPECT_EQ(client->getSendBuffer(), expected_reply);
// }
#include "TestFixture.hpp"

TEST_F(CommandTest, Pass_CorrectPassword) {
    args.push_back("pass123");
    passCmd->execute(client1, args);
    ASSERT_TRUE(client1->isAuthenticated());
    ASSERT_TRUE(client1->receivedMessages.empty()); // 成功時は何も返さない
}

TEST_F(CommandTest, Pass_IncorrectPassword) {
    args.push_back("wrong");
    passCmd->execute(client1, args);
    ASSERT_FALSE(client1->isAuthenticated());
    ASSERT_NE(client1->getLastMessage().find(ERR_PASSWDMISMATCH), std::string::npos);
}

TEST_F(CommandTest, Pass_NoPassword) {
    passCmd->execute(client1, args);
    ASSERT_FALSE(client1->isAuthenticated());
    ASSERT_NE(client1->getLastMessage().find(ERR_NEEDMOREPARAMS), std::string::npos);
}

TEST_F(CommandTest, Pass_AlreadyRegistered) {
    registerClient(client1, "user1");
    args.push_back("pass123");
    passCmd->execute(client1, args);
    ASSERT_NE(client1->getLastMessage().find(ERR_ALREADYREGISTRED), std::string::npos);
}
