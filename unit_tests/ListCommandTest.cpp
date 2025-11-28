#include "TestFixture.hpp"
#include "ListCommand.hpp"
#include "Channel.hpp"

class ListCommandTest : public CommandTest {
  protected:
    ListCommand *listCmd;

    virtual void SetUp() {
        CommandTest::SetUp();
        listCmd = new ListCommand(server);
        registerClient(client1, "UserA");
    }

    virtual void TearDown() {
        delete listCmd;
        CommandTest::TearDown();
    }
};

TEST_F(ListCommandTest, List_WithChannels) {
    // Setup: Create two channels
    Channel *chan1 = new Channel("#chan1");
    chan1->addMember(client1);
    chan1->setTopic("Topic for chan1");
    server->addChannel(chan1);

    Channel *chan2 = new Channel("#chan2");
    server->addChannel(chan2); // No members, no topic

    args.clear();
    listCmd->execute(client1, args);

    ASSERT_EQ(client1->receivedMessages.size(), static_cast<std::string::size_type>(4));

    // 1. RPL_LISTSTART (321)
    EXPECT_NE(client1->receivedMessages[0].find("321"), std::string::npos);

    // 2. RPL_LIST for #chan1 (322)
    std::string list_chan1 = client1->receivedMessages[1];
    EXPECT_NE(list_chan1.find("322"), std::string::npos);
    EXPECT_NE(list_chan1.find("#chan1"), std::string::npos);
    EXPECT_NE(list_chan1.find("1"), std::string::npos); // Member count
    EXPECT_NE(list_chan1.find("Topic for chan1"), std::string::npos);

    // 3. RPL_LIST for #chan2 (322)
    std::string list_chan2 = client1->receivedMessages[2];
    EXPECT_NE(list_chan2.find("322"), std::string::npos);
    EXPECT_NE(list_chan2.find("#chan2"), std::string::npos);
    EXPECT_NE(list_chan2.find("0"), std::string::npos); // Member count

    // 4. RPL_LISTEND (323)
    EXPECT_NE(client1->receivedMessages[3].find("323"), std::string::npos);
}

TEST_F(ListCommandTest, List_NoChannels) {
    args.clear();
    listCmd->execute(client1, args);

    ASSERT_EQ(client1->receivedMessages.size(), static_cast<std::string::size_type>(2));
    EXPECT_NE(client1->receivedMessages[0].find("321"), std::string::npos); // RPL_LISTSTART
    EXPECT_NE(client1->receivedMessages[1].find("323"), std::string::npos); // RPL_LISTEND
}

TEST_F(ListCommandTest, List_SpecificChannel) {
    // Setup: Create two channels
    Channel *chan1 = new Channel("#chan1");
    chan1->setTopic("Topic for chan1");
    server->addChannel(chan1);
    Channel *chan2 = new Channel("#chan2");
    server->addChannel(chan2);

    args.push_back("#chan1");
    listCmd->execute(client1, args);

    ASSERT_EQ(client1->receivedMessages.size(), static_cast<std::string::size_type>(3));
    EXPECT_NE(client1->receivedMessages[0].find("321"), std::string::npos);
    
    std::string list_reply = client1->receivedMessages[1];
    EXPECT_NE(list_reply.find("#chan1"), std::string::npos);
    EXPECT_EQ(list_reply.find("#chan2"), std::string::npos); // Should not contain info about #chan2
    
    EXPECT_NE(client1->receivedMessages[2].find("323"), std::string::npos);
}

TEST_F(ListCommandTest, List_MultipleSpecificChannels) {
    // Setup
    Channel *chan1 = new Channel("#chan1");
    server->addChannel(chan1);
    Channel *chan2 = new Channel("#chan2");
    server->addChannel(chan2);
    Channel *chan3 = new Channel("#chan3");
    server->addChannel(chan3);

    args.push_back("#chan1,#chan3");
    listCmd->execute(client1, args);

    ASSERT_EQ(client1->receivedMessages.size(), static_cast<std::string::size_type>(4));
    EXPECT_NE(client1->receivedMessages[0].find("321"), std::string::npos);
    
    std::string combined_replies = client1->receivedMessages[1] + client1->receivedMessages[2];
    EXPECT_NE(combined_replies.find("#chan1"), std::string::npos);
    EXPECT_NE(combined_replies.find("#chan3"), std::string::npos);
    EXPECT_EQ(combined_replies.find("#chan2"), std::string::npos);

    EXPECT_NE(client1->receivedMessages[3].find("323"), std::string::npos);
}

TEST_F(ListCommandTest, List_NonExistentChannel) {
    args.push_back("#nonexistent");
    listCmd->execute(client1, args);

    // RFC specifies that LIST for a non-existent channel should just not return a RPL_LIST for it.
    // So it should be the same as an empty list.
    ASSERT_EQ(client1->receivedMessages.size(), static_cast<std::string::size_type>(2));
    EXPECT_NE(client1->receivedMessages[0].find("321"), std::string::npos);
    EXPECT_NE(client1->receivedMessages[1].find("323"), std::string::npos);
}
