#include "PingCommand.hpp"
#include "TestFixture.hpp"
#include "gtest/gtest.h"

TEST_F(CommandTest, PingCommandRespondsWithPong) {
    registerClient(client1, "test_nick");
    PingCommand pingCmd(server);
    std::vector<std::string> args;
    args.push_back("irc.example.com"); // PINGパラメータ

    pingCmd.execute(client1, args);

    std::string expected_pong = "PONG " + server->getServerName() + " :irc.example.com" + "\r\n";
    EXPECT_EQ(client1->getLastMessage(), expected_pong);
}

TEST_F(CommandTest, PingCommandWithMultipleParametersRespondsWithFirstParameter) {
    registerClient(client1, "test_nick");
    PingCommand pingCmd(server);
    std::vector<std::string> args;
    args.push_back("irc.example.com");
    args.push_back("another.param"); // RFC1459ではPINGは最大2つのパラメータを持つが、PONGは最初のパラメータを返す

    pingCmd.execute(client1, args);

    std::string expected_pong = "PONG " + server->getServerName() + " :irc.example.com" + "\r\n";
    EXPECT_EQ(client1->getLastMessage(), expected_pong);
}

TEST_F(CommandTest, PingCommandWithoutParametersReturnsError) {
    registerClient(client1, "test_nick");
    PingCommand pingCmd(server);
    std::vector<std::string> args;

    pingCmd.execute(client1, args);

    std::string expected_error = ":" + server->getServerName() + " 409 " +
                                 client1->getNickname() + " :No origin specified\r\n";
    EXPECT_EQ(client1->getLastMessage(), expected_error);
}
