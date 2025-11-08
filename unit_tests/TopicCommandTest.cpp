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
        channel->addOperator(client1); // Make client1 an operator for topic tests
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
        formatReply(server->getServerName(), client1->getNickname(), RPL_TOPIC, params);
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
        formatReply(server->getServerName(), client1->getNickname(), RPL_NOTOPIC, params);
    EXPECT_EQ(client1->getLastMessage(), expected_reply);
}

TEST_F(TopicCommandTest, Topic_NeedMoreParams) {
    topicCmd->execute(client1, args);

    ASSERT_EQ(client1->receivedMessages.size(), 1);
    std::vector<std::string> params;
    params.push_back("TOPIC");
    std::string expected_reply =
        formatReply(server->getServerName(), client1->getNickname(), ERR_NEEDMOREPARAMS, params);
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
        formatReply(server->getServerName(), client2->getNickname(), ERR_NOTONCHANNEL, params);
    EXPECT_EQ(client2->getLastMessage(), expected_reply);
}

TEST_F(TopicCommandTest, Topic_NoSuchChannel) {
    args.push_back("#nonexistent");
    topicCmd->execute(client1, args);

    ASSERT_EQ(client1->receivedMessages.size(), 1);
    std::vector<std::string> params;
    params.push_back("#nonexistent");
    std::string expected_reply =
        formatReply(server->getServerName(), client1->getNickname(), ERR_NOSUCHCHANNEL, params);
    EXPECT_EQ(client1->getLastMessage(), expected_reply);
}

TEST_F(TopicCommandTest, Topic_SetTopic_WithTopicProtection_AsOperator) {
    channel->setMode('t', true); // Enable topic protection
    client1->receivedMessages.clear();

    args.push_back("#test");
    args.push_back("Operator can change topic.");
    topicCmd->execute(client1, args); // client1 is operator

    EXPECT_EQ(channel->getTopic(), "Operator can change topic.");
    std::string topicMsg = client1->getLastMessage();
    EXPECT_NE(topicMsg.find("TOPIC #test :Operator can change topic."), std::string::npos);
}

TEST_F(TopicCommandTest, Topic_SetTopic_WithTopicProtection_AsNonOperator) {
    channel->setMode('t', true); // Enable topic protection
    channel->addMember(client2); // Add client2 to channel
    client2->addChannel(channel);
    client2->receivedMessages.clear();

    args.push_back("#test");
    args.push_back("Non-operator cannot change topic.");
    topicCmd->execute(client2, args); // client2 is NOT operator

    // Topic should not change
    EXPECT_EQ(channel->getTopic(), ""); // Assuming initial topic is empty

    // Non-operator should receive ERR_CHANOPRIVSNEEDED
    ASSERT_EQ(client2->receivedMessages.size(), 1);
    std::vector<std::string> params;
    params.push_back("#test");
    std::string expected_reply =
        formatReply(server->getServerName(), client2->getNickname(), ERR_CHANOPRIVSNEEDED, params);
    EXPECT_EQ(client2->getLastMessage(), expected_reply);
}
