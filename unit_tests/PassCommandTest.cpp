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
    std::string expected_reply = formatReply(server->getServerName(), client1->getNickname(),
                                             ERR_PASSWDMISMATCH, std::vector<std::string>()) +
                                 "\r\n";
    EXPECT_EQ(client1->getLastMessage(), expected_reply);
}

TEST_F(CommandTest, Pass_NoPassword) {
    passCmd->execute(client1, args);
    ASSERT_FALSE(client1->isAuthenticated());
    std::vector<std::string> params;
    params.push_back("PASS");
    std::string expected_reply =
        formatReply(server->getServerName(), client1->getNickname(), ERR_NEEDMOREPARAMS, params) +
        "\r\n";
    EXPECT_EQ(client1->getLastMessage(), expected_reply);
}

TEST_F(CommandTest, Pass_AlreadyRegistered) {
    registerClient(client1, "user1");
    args.push_back("pass123");
    passCmd->execute(client1, args);
    std::string expected_reply = formatReply(server->getServerName(), client1->getNickname(),
                                             ERR_ALREADYREGISTRED, std::vector<std::string>()) +
                                 "\r\n";
    EXPECT_EQ(client1->getLastMessage(), expected_reply);
}
