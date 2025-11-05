#include "TestFixture.hpp"
#include "NamesCommand.hpp"
#include "Channel.hpp"

class NamesCommandTest : public CommandTest {};

TEST_F(NamesCommandTest, NamesCommandReturnsCorrectListForOneChannel) {
    registerClient(client1, "UserA");
    registerClient(client2, "UserB");

    // Setup channel and members
    Channel* channel = new Channel("#test");
    server->addChannel(channel);
    channel->addMember(client1);
    channel->addMember(client2);
    client1->addChannel(channel);
    client2->addChannel(channel);

    // Clear client1's messages from JOIN
    client1->receivedMessages.clear();

    // Execute NAMES command for #test
    args.push_back("#test");
    namesCmd->execute(client1, args);

    // Expect RPL_NAMREPLY (353) and RPL_ENDOFNAMES (366)
    ASSERT_EQ(client1->receivedMessages.size(), 2);

    // Check RPL_NAMREPLY
    std::string namesReply = client1->receivedMessages[0];
    EXPECT_NE(namesReply.find(RPL_NAMREPLY), std::string::npos);
    EXPECT_NE(namesReply.find("#test"), std::string::npos);
    EXPECT_NE(namesReply.find("UserA"), std::string::npos);
    EXPECT_NE(namesReply.find("UserB"), std::string::npos);

    // Check RPL_ENDOFNAMES
    std::string endNamesReply = client1->receivedMessages[1];
    EXPECT_NE(endNamesReply.find(RPL_ENDOFNAMES), std::string::npos);
    EXPECT_NE(endNamesReply.find("#test"), std::string::npos);
}

TEST_F(NamesCommandTest, NamesCommandReturnsCorrectListForMultipleChannels) {
    registerClient(client1, "UserA");
    registerClient(client2, "UserB");

    // Setup channels and members
    Channel* chan1 = new Channel("#chan1");
    Channel* chan2 = new Channel("#chan2");
    server->addChannel(chan1);
    server->addChannel(chan2);

    chan1->addMember(client1);
    client1->addChannel(chan1);
    chan1->addMember(client2);
    client2->addChannel(chan1);

    chan2->addMember(client1);
    client1->addChannel(chan2);

    // Clear client1's messages from JOIN
    client1->receivedMessages.clear();

    // Execute NAMES command for #chan1,#chan2
    args.push_back("#chan1,#chan2");
    namesCmd->execute(client1, args);

    // Expect 4 messages (2x RPL_NAMREPLY, 2x RPL_ENDOFNAMES)
    ASSERT_EQ(client1->receivedMessages.size(), 4);

    // Check #chan1 replies
    std::string namesReply1 = client1->receivedMessages[0];
    std::string endNamesReply1 = client1->receivedMessages[1];
    EXPECT_NE(namesReply1.find(RPL_NAMREPLY), std::string::npos);
    EXPECT_NE(namesReply1.find("#chan1"), std::string::npos);
    EXPECT_NE(namesReply1.find("UserA"), std::string::npos);
    EXPECT_NE(namesReply1.find("UserB"), std::string::npos);
    EXPECT_NE(endNamesReply1.find(RPL_ENDOFNAMES), std::string::npos);
    EXPECT_NE(endNamesReply1.find("#chan1"), std::string::npos);

    // Check #chan2 replies
    std::string namesReply2 = client1->receivedMessages[2];
    std::string endNamesReply2 = client1->receivedMessages[3];
    EXPECT_NE(namesReply2.find(RPL_NAMREPLY), std::string::npos);
    EXPECT_NE(namesReply2.find("#chan2"), std::string::npos);
    EXPECT_NE(namesReply2.find("UserA"), std::string::npos);
    EXPECT_EQ(namesReply2.find("UserB"), std::string::npos); // UserB is not in #chan2
    EXPECT_NE(endNamesReply2.find(RPL_ENDOFNAMES), std::string::npos);
    EXPECT_NE(endNamesReply2.find("#chan2"), std::string::npos);
}

TEST_F(NamesCommandTest, NamesCommandForNonExistentChannel) {
    registerClient(client1, "UserA");
    client1->receivedMessages.clear();

    args.push_back("#nonexistent");
    namesCmd->execute(client1, args);

    // Should still receive RPL_ENDOFNAMES for the non-existent channel
    ASSERT_EQ(client1->receivedMessages.size(), 1);
    std::string endNamesReply = client1->receivedMessages[0];
    EXPECT_NE(endNamesReply.find(RPL_ENDOFNAMES), std::string::npos);
    EXPECT_NE(endNamesReply.find("#nonexistent"), std::string::npos);
}

TEST_F(NamesCommandTest, NamesCommandWithoutParametersListsAllChannels) {
    registerClient(client1, "UserA");
    registerClient(client2, "UserB");

    // Setup channels and members
    Channel* chan1 = new Channel("#chan1");
    Channel* chan2 = new Channel("#chan2");
    server->addChannel(chan1);
    server->addChannel(chan2);

    chan1->addMember(client1);
    client1->addChannel(chan1);
    chan1->addMember(client2);
    client2->addChannel(chan1);

    chan2->addMember(client1);
    client1->addChannel(chan2);

    client1->receivedMessages.clear();

    // Execute NAMES command without parameters
    args.clear();
    namesCmd->execute(client1, args);

    // Expect replies for all channels (2x RPL_NAMREPLY, 2x RPL_ENDOFNAMES)
    ASSERT_EQ(client1->receivedMessages.size(), 4);

    // Check for replies for both channels (order might vary, so check content)
    bool foundChan1Names = false;
    bool foundChan2Names = false;
    bool foundChan1End = false;
    bool foundChan2End = false;

    for (size_t i = 0; i < client1->receivedMessages.size(); ++i) {
        const std::string& msg = client1->receivedMessages[i];
        if (msg.find(RPL_NAMREPLY) != std::string::npos) {
            if (msg.find("#chan1") != std::string::npos) {
                foundChan1Names = true;
                EXPECT_NE(msg.find("UserA"), std::string::npos);
                EXPECT_NE(msg.find("UserB"), std::string::npos);
            } else if (msg.find("#chan2") != std::string::npos) {
                foundChan2Names = true;
                EXPECT_NE(msg.find("UserA"), std::string::npos);
                EXPECT_EQ(msg.find("UserB"), std::string::npos); // UserB is not in #chan2
            }
        } else if (msg.find(RPL_ENDOFNAMES) != std::string::npos) {
            if (msg.find("#chan1") != std::string::npos) {
                foundChan1End = true;
            } else if (msg.find("#chan2") != std::string::npos) {
                foundChan2End = true;
            }
        }
    }

    EXPECT_TRUE(foundChan1Names);
    EXPECT_TRUE(foundChan2Names);
    EXPECT_TRUE(foundChan1End);
    EXPECT_TRUE(foundChan2End);
}
