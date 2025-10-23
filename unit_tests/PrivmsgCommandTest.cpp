#include "gtest/gtest.h"
#include "PrivmsgCommand.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "Replies.hpp"

class PrivmsgCommandTest : public ::testing::Test {
protected:
    void SetUp() override {
        server = new Server("password");
        
        sender = new Client(1, "127.0.0.1");
        sender->setAuthenticated(true);
        sender->setNickname("sender");
        sender->setUsername("sender_user");
        sender->setRegistered(true);
        server->addClient(sender);

        receiver = new Client(2, "127.0.0.1");
        receiver->setAuthenticated(true);
        receiver->setNickname("receiver");
        receiver->setUsername("receiver_user");
        receiver->setRegistered(true);
        server->addClient(receiver);

        channel = new Channel("#testchan");
        channel->addUser(sender);
        channel->addUser(receiver);
        server->addChannel(channel);

        privmsgCmd = new PrivmsgCommand();
    }

    void TearDown() override {
        delete server;
        // clients and channel are deleted by server's destructor
        delete privmsgCmd;
    }

    Server* server;
    Client* sender;
    Client* receiver;
    Channel* channel;
    PrivmsgCommand* privmsgCmd;
};

TEST_F(PrivmsgCommandTest, SendToChannel) {
    std::vector<std::string> args;
    args.push_back("#testchan");
    args.push_back(":Hello channel!");

    privmsgCmd->execute(server, sender, args);

    // The sender should not receive their own message back in their send buffer
    EXPECT_TRUE(sender->getSendBuffer().empty());
    // The receiver in the channel should get the message
    std::string expected_msg = ":" + sender->getPrefix() + " PRIVMSG #testchan :Hello channel!\r\n";
    EXPECT_EQ(receiver->getSendBuffer(), expected_msg);
}

TEST_F(PrivmsgCommandTest, SendToUser) {
    std::vector<std::string> args;
    args.push_back("receiver");
    args.push_back(":Hello receiver!");

    privmsgCmd->execute(server, sender, args);

    EXPECT_TRUE(sender->getSendBuffer().empty());
    std::string expected_msg = ":" + sender->getPrefix() + " PRIVMSG receiver :Hello receiver!\r\n";
    EXPECT_EQ(receiver->getSendBuffer(), expected_msg);
}

TEST_F(PrivmsgCommandTest, NotEnoughParams) {
    std::vector<std::string> args; // No arguments

    privmsgCmd->execute(server, sender, args);

    std::string expected_reply = ERR_NEEDMOREPARAMS(sender->getNickname(), "PRIVMSG");
    EXPECT_EQ(sender->getSendBuffer(), expected_reply);
}

TEST_F(PrivmsgCommandTest, NoTextToSend) {
    std::vector<std::string> args;
    args.push_back("#testchan");

    privmsgCmd->execute(server, sender, args);

    std::string expected_reply = ERR_NOTEXTTOSEND(sender->getNickname());
    EXPECT_EQ(sender->getSendBuffer(), expected_reply);
}

TEST_F(PrivmsgCommandTest, NoSuchChannel) {
    std::vector<std::string> args;
    args.push_back("#no-such-channel");
    args.push_back(":test");

    privmsgCmd->execute(server, sender, args);

    std::string expected_reply = ERR_NOSUCHCHANNEL(sender->getNickname(), "#no-such-channel");
    EXPECT_EQ(sender->getSendBuffer(), expected_reply);
}

TEST_F(PrivmsgCommandTest, NoSuchNick) {
    std::vector<std::string> args;
    args.push_back("no-such-nick");
    args.push_back(":test");

    privmsgCmd->execute(server, sender, args);

    std::string expected_reply = ERR_NOSUCHNICK(sender->getNickname(), "no-such-nick");
    EXPECT_EQ(sender->getSendBuffer(), expected_reply);
}

TEST_F(PrivmsgCommandTest, CannotSendToChannel) {
    channel->removeUser(sender); // Remove sender from channel
    std::vector<std::string> args;
    args.push_back("#testchan");
    args.push_back(":test");

    privmsgCmd->execute(server, sender, args);

    std::string expected_reply = ERR_CANNOTSENDTOCHAN(sender->getNickname(), "#testchan");
    EXPECT_EQ(sender->getSendBuffer(), expected_reply);
}

TEST_F(PrivmsgCommandTest, NotRegistered) {
    sender->setRegistered(false);
    std::vector<std::string> args;
    args.push_back("receiver");
    args.push_back(":test");

    privmsgCmd->execute(server, sender, args);

    EXPECT_FALSE(sender->getSendBuffer().empty()); // Expect an error
}
