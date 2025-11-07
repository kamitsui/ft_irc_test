#include "TestFixture.hpp"
#include "TopicCommand.hpp"
#include "Channel.hpp"

class TopicCommandTest : public CommandTest {
  protected:
    Channel* channel;

    virtual void SetUp() {
        CommandTest::SetUp();
        registerClient(client1, "UserA");
        registerClient(client2, "UserB");

        // Setup channel and add client1
        channel = new Channel("#test");
        server->addChannel(channel);
        channel->addMember(client1);
        client1->addChannel(channel);
        client1->receivedMessages.clear();
    }
};

TEST_F(TopicCommandTest, Topic_SetTopic) {
    args.push_back("#test");
    args.push_back("This is the new topic.");
    topicCmd->execute(client1, args);

    // Check that the topic was set
    EXPECT_EQ(channel->getTopic(), "This is the new topic.");

    // Check for the TOPIC message broadcast
    ASSERT_FALSE(client1->receivedMessages.empty());
    std::string topicMsg = client1->getLastMessage();
    EXPECT_NE(topicMsg.find("TOPIC #test :This is the new topic."), std::string::npos);
}

TEST_F(TopicCommandTest, Topic_SetEmptyTopic) {
    // First set a topic
    channel->setTopic("An old topic.");
    client1->receivedMessages.clear();

    // Then clear it
    args.push_back("#test");
    args.push_back("");
    topicCmd->execute(client1, args);

    EXPECT_EQ(channel->getTopic(), "");
    ASSERT_FALSE(client1->receivedMessages.empty());
    std::string topicMsg = client1->getLastMessage();
    EXPECT_NE(topicMsg.find("TOPIC #test :"), std::string::npos);
}

TEST_F(TopicCommandTest, Topic_ViewTopic) {
    channel->setTopic("A readable topic.");
    client1->receivedMessages.clear();

    args.push_back("#test");
    topicCmd->execute(client1, args);

    ASSERT_EQ(client1->receivedMessages.size(), 1);
    std::vector<std::string> params;
    params.push_back("#test");
    params.push_back("A readable topic.");
    std::string expected_reply =
        formatReply(server->getServerName(), client1->getNickname(), RPL_TOPIC, params) + "\r\n";
    EXPECT_EQ(client1->getLastMessage(), expected_reply);
}

TEST_F(TopicCommandTest, Topic_NoTopicSet) {
    // Topic is empty by default
    args.push_back("#test");
    topicCmd->execute(client1, args);

    ASSERT_EQ(client1->receivedMessages.size(), 1);
    std::vector<std::string> params;
    params.push_back("#test");
    std::string expected_reply =
        formatReply(server->getServerName(), client1->getNickname(), RPL_NOTOPIC, params) + "\r\n";
    EXPECT_EQ(client1->getLastMessage(), expected_reply);
}

TEST_F(TopicCommandTest, Topic_NeedMoreParams) {
    topicCmd->execute(client1, args);

    ASSERT_EQ(client1->receivedMessages.size(), 1);
    std::vector<std::string> params;
    params.push_back("TOPIC");
    std::string expected_reply =
        formatReply(server->getServerName(), client1->getNickname(), ERR_NEEDMOREPARAMS, params) +
        "\r\n";
    EXPECT_EQ(client1->getLastMessage(), expected_reply);
}

TEST_F(TopicCommandTest, Topic_NotOnChannel) {
    // client2 is not on the channel
    args.push_back("#test");
    topicCmd->execute(client2, args); // View topic attempt

    ASSERT_EQ(client2->receivedMessages.size(), 1);
    std::vector<std::string> params;
    params.push_back("#test");
    std::string expected_reply =
        formatReply(server->getServerName(), client2->getNickname(), ERR_NOTONCHANNEL, params) +
        "\r\n";
    EXPECT_EQ(client2->getLastMessage(), expected_reply);
}

TEST_F(TopicCommandTest, Topic_NoSuchChannel) {
    args.push_back("#nonexistent");
    topicCmd->execute(client1, args);

    ASSERT_EQ(client1->receivedMessages.size(), 1);
    std::vector<std::string> params;
    params.push_back("#nonexistent");
    std::string expected_reply =
        formatReply(server->getServerName(), client1->getNickname(), ERR_NOSUCHCHANNEL, params) +
        "\r\n";
    EXPECT_EQ(client1->getLastMessage(), expected_reply);
}
