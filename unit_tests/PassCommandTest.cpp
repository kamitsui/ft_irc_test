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
