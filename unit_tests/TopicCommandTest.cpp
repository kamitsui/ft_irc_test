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

    // Expect RPL_TOPIC (332)
    ASSERT_FALSE(client1->receivedMessages.empty());
    std::string topicReply = client1->getLastMessage();
    EXPECT_NE(topicReply.find(RPL_TOPIC), std::string::npos);
    EXPECT_NE(topicReply.find("#test :A readable topic."), std::string::npos);
}

TEST_F(TopicCommandTest, Topic_NoTopicSet) {
    // Topic is empty by default
    args.push_back("#test");
    topicCmd->execute(client1, args);

    // Expect RPL_NOTOPIC (331)
    ASSERT_FALSE(client1->receivedMessages.empty());
    std::string noTopicReply = client1->getLastMessage();
    EXPECT_NE(noTopicReply.find(RPL_NOTOPIC), std::string::npos);
    EXPECT_NE(noTopicReply.find("#test :No topic is set"), std::string::npos);
}

TEST_F(TopicCommandTest, Topic_NeedMoreParams) {
    topicCmd->execute(client1, args);

    ASSERT_FALSE(client1->receivedMessages.empty());
    std::string errorReply = client1->getLastMessage();
    EXPECT_NE(errorReply.find(ERR_NEEDMOREPARAMS), std::string::npos);
    EXPECT_NE(errorReply.find("TOPIC"), std::string::npos);
}

TEST_F(TopicCommandTest, Topic_NotOnChannel) {
    // client2 is not on the channel
    args.push_back("#test");
    args.push_back("A topic from an outsider.");
    topicCmd->execute(client2, args);

    ASSERT_FALSE(client2->receivedMessages.empty());
    std::string errorReply = client2->getLastMessage();
    EXPECT_NE(errorReply.find(ERR_NOTONCHANNEL), std::string::npos);
    EXPECT_NE(errorReply.find("#test"), std::string::npos);
}

TEST_F(TopicCommandTest, Topic_NoSuchChannel) {
    args.push_back("#nonexistent");
    topicCmd->execute(client1, args);

    ASSERT_FALSE(client1->receivedMessages.empty());
    std::string errorReply = client1->getLastMessage();
    EXPECT_NE(errorReply.find(ERR_NOSUCHCHANNEL), std::string::npos);
}
