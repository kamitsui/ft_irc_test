#include "TestFixture.hpp"
#include "NamesCommand.hpp"
#include "Channel.hpp"
#include <algorithm>

class NamesCommandTest : public CommandTest {};

TEST_F(NamesCommandTest, NamesCommandReturnsCorrectListForOneChannel) {
    registerClient(client1, "UserA");
    registerClient(client2, "UserB");

    Channel* channel = new Channel("#test");
    server->addChannel(channel);
    channel->addMember(client1);
    channel->addMember(client2);
    channel->addOperator(client1); // UserA is an operator
    client1->addChannel(channel);
    client2->addChannel(channel);
    client1->receivedMessages.clear();

    args.push_back("#test");
    namesCmd->execute(client1, args);

    ASSERT_EQ(client1->receivedMessages.size(), static_cast<std::string::size_type>(2));

    // Check RPL_NAMREPLY (353)
    std::string namesReply = client1->receivedMessages[0];
    EXPECT_NE(namesReply.find("353"), std::string::npos);
    EXPECT_NE(namesReply.find("#test"), std::string::npos);
    EXPECT_NE(namesReply.find("@UserA"), std::string::npos);
    EXPECT_NE(namesReply.find("UserB"), std::string::npos);

    // Check RPL_ENDOFNAMES (366)
    std::string endNamesReply = client1->receivedMessages[1];
    EXPECT_NE(endNamesReply.find("366"), std::string::npos);
    EXPECT_NE(endNamesReply.find("#test"), std::string::npos);
}

TEST_F(NamesCommandTest, NamesCommandReturnsCorrectListForMultipleChannels) {
    registerClient(client1, "UserA");
    registerClient(client2, "UserB");

    Channel* chan1 = new Channel("#chan1");
    server->addChannel(chan1);
    chan1->addMember(client1);
    chan1->addOperator(client1);
    chan1->addMember(client2);
    client1->addChannel(chan1);
    client2->addChannel(chan1);

    Channel* chan2 = new Channel("#chan2");
    server->addChannel(chan2);
    chan2->addMember(client1);
    chan2->addOperator(client1);
    client1->addChannel(chan2);

    client1->receivedMessages.clear();

    args.push_back("#chan1,#chan2");
    namesCmd->execute(client1, args);

    ASSERT_EQ(client1->receivedMessages.size(), static_cast<std::string::size_type>(4));

    // Collect replies for each channel
    std::string chan1_namereply, chan1_endofnames;
    std::string chan2_namereply, chan2_endofnames;

    for (size_t i = 0; i < client1->receivedMessages.size(); ++i) {
        if (client1->receivedMessages[i].find("#chan1") != std::string::npos) {
            if (client1->receivedMessages[i].find("353") != std::string::npos)
                chan1_namereply = client1->receivedMessages[i];
            else if (client1->receivedMessages[i].find("366") != std::string::npos)
                chan1_endofnames = client1->receivedMessages[i];
        } else if (client1->receivedMessages[i].find("#chan2") != std::string::npos) {
            if (client1->receivedMessages[i].find("353") != std::string::npos)
                chan2_namereply = client1->receivedMessages[i];
            else if (client1->receivedMessages[i].find("366") != std::string::npos)
                chan2_endofnames = client1->receivedMessages[i];
        }
    }

    // Verify #chan1 replies
    EXPECT_FALSE(chan1_namereply.empty());
    EXPECT_NE(chan1_namereply.find("@UserA"), std::string::npos);
    EXPECT_NE(chan1_namereply.find("UserB"), std::string::npos);
    EXPECT_FALSE(chan1_endofnames.empty());

    // Verify #chan2 replies
    EXPECT_FALSE(chan2_namereply.empty());
    EXPECT_NE(chan2_namereply.find("@UserA"), std::string::npos);
    EXPECT_EQ(chan2_namereply.find("UserB"), std::string::npos);
    EXPECT_FALSE(chan2_endofnames.empty());
}

TEST_F(NamesCommandTest, NamesCommandForNonExistentChannel) {
    registerClient(client1, "UserA");
    client1->receivedMessages.clear();

    args.push_back("#nonexistent");
    namesCmd->execute(client1, args);

    ASSERT_EQ(client1->receivedMessages.size(), static_cast<std::string::size_type>(1));

    std::string endNamesReply = client1->receivedMessages[0];
    EXPECT_NE(endNamesReply.find("366"), std::string::npos);
    EXPECT_NE(endNamesReply.find("#nonexistent"), std::string::npos);
}

TEST_F(NamesCommandTest, NamesCommandWithoutParametersListsAllChannels) {
    registerClient(client1, "UserA");
    registerClient(client2, "UserB");

    Channel* chan1 = new Channel("#chan1");
    server->addChannel(chan1);
    chan1->addMember(client1);
    chan1->addOperator(client1);
    chan1->addMember(client2);
    client1->addChannel(chan1);
    client2->addChannel(chan1);

    Channel* chan2 = new Channel("#chan2");
    server->addChannel(chan2);
    chan2->addMember(client1);
    chan2->addOperator(client1);
    client1->addChannel(chan2);

    client1->receivedMessages.clear();

    args.clear();
    namesCmd->execute(client1, args);

    ASSERT_EQ(client1->receivedMessages.size(), static_cast<std::string::size_type>(4));

    // Collect replies for each channel
    std::string chan1_namereply, chan1_endofnames;
    std::string chan2_namereply, chan2_endofnames;

    for (size_t i = 0; i < client1->receivedMessages.size(); ++i) {
        if (client1->receivedMessages[i].find("#chan1") != std::string::npos) {
            if (client1->receivedMessages[i].find("353") != std::string::npos)
                chan1_namereply = client1->receivedMessages[i];
            else if (client1->receivedMessages[i].find("366") != std::string::npos)
                chan1_endofnames = client1->receivedMessages[i];
        } else if (client1->receivedMessages[i].find("#chan2") != std::string::npos) {
            if (client1->receivedMessages[i].find("353") != std::string::npos)
                chan2_namereply = client1->receivedMessages[i];
            else if (client1->receivedMessages[i].find("366") != std::string::npos)
                chan2_endofnames = client1->receivedMessages[i];
        }
    }

    // Verify #chan1 replies
    EXPECT_FALSE(chan1_namereply.empty());
    EXPECT_NE(chan1_namereply.find("@UserA"), std::string::npos);
    EXPECT_NE(chan1_namereply.find("UserB"), std::string::npos);
    EXPECT_FALSE(chan1_endofnames.empty());

    // Verify #chan2 replies
    EXPECT_FALSE(chan2_namereply.empty());
    EXPECT_NE(chan2_namereply.find("@UserA"), std::string::npos);
    EXPECT_EQ(chan2_namereply.find("UserB"), std::string::npos);
    EXPECT_FALSE(chan2_endofnames.empty());
}
