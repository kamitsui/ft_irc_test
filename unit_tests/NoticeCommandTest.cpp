#include "NoticeCommand.hpp"
#include "Channel.hpp"
#include "TestFixture.hpp"

class NoticeCommandTest : public CommandTest {};

TEST_F(NoticeCommandTest, NoticeToUser) {
    registerClient(client1, "sender");
    registerClient(client2, "receiver");

    args.push_back("receiver");
    args.push_back("Hello there!");
    noticeCmd->execute(client1, args);

    std::string expected_msg = std::string(client1->getPrefix() + " NOTICE receiver :Hello there!") + "\r\n";
    EXPECT_EQ(client2->getLastMessage(), expected_msg);
}

TEST_F(NoticeCommandTest, NoticeToChannel) {
    registerClient(client1, "sender");
    registerClient(client2, "receiver");

    Channel *channel = new Channel("#test");
    server->addChannel(channel);
    channel->addMember(client1);
    channel->addMember(client2);

    args.push_back("#test");
    args.push_back("Channel notice");
    noticeCmd->execute(client1, args);

    std::string expected_msg = std::string(client1->getPrefix() + " NOTICE #test :Channel notice") + "\r\n";
    EXPECT_EQ(client2->getLastMessage(), expected_msg);
    // 送信者自身には送られない
    EXPECT_NE(client1->getLastMessage(), expected_msg);
}

TEST_F(NoticeCommandTest, NoErrorForNonExistentNick) {
    registerClient(client1, "sender");
    size_t messages_before = client1->receivedMessages.size();

    args.push_back("nonexistent");
    args.push_back("This should not cause an error");
    noticeCmd->execute(client1, args);

    size_t messages_after = client1->receivedMessages.size();
    EXPECT_EQ(messages_before, messages_after);
}

TEST_F(NoticeCommandTest, NoErrorForNonExistentChannel) {
    registerClient(client1, "sender");
    size_t messages_before = client1->receivedMessages.size();

    args.push_back("#nonexistent");
    args.push_back("This should not cause an error");
    noticeCmd->execute(client1, args);

    size_t messages_after = client1->receivedMessages.size();
    EXPECT_EQ(messages_before, messages_after);
}

TEST_F(NoticeCommandTest, NoArgs) {
    registerClient(client1, "sender");
    size_t messages_before = client1->receivedMessages.size();

    noticeCmd->execute(client1, args);

    size_t messages_after = client1->receivedMessages.size();
    EXPECT_EQ(messages_before, messages_after);
}

TEST_F(NoticeCommandTest, OneArg) {
    registerClient(client1, "sender");
    size_t messages_before = client1->receivedMessages.size();

    args.push_back("receiver");
    noticeCmd->execute(client1, args);

    size_t messages_after = client1->receivedMessages.size();
    EXPECT_EQ(messages_before, messages_after);
}
