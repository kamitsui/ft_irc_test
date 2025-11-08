#include "InviteCommand.hpp"
#include "Channel.hpp"
#include "Replies.hpp"
#include "TestFixture.hpp"

class InviteCommandTest : public CommandTest {
  protected:
    Channel *channel;
    TestClient *client3; // Target client for invite

    virtual void SetUp() {
        CommandTest::SetUp();
        registerClient(client1, "Inviter");     // Client who sends INVITE
        registerClient(client2, "OtherMember"); // Another member in the channel
        client3 = new TestClient(12, "client3.host");
        server->addTestClient(client3);
        registerClient(client3, "Invitee"); // Client to be invited

        // Setup channel
        channel = new Channel("#test");
        server->addChannel(channel);

        // Add inviter and other member to channel
        channel->addMember(client1);
        channel->addMember(client2);
        channel->addOperator(client1); // Inviter is operator

        client1->addChannel(channel);
        client2->addChannel(channel);

        client1->receivedMessages.clear();
        client2->receivedMessages.clear();
        client3->receivedMessages.clear();
    }
};

TEST_F(InviteCommandTest, Invite_Success) {
    args.push_back("Invitee");
    args.push_back("#test");
    inviteCmd->execute(client1, args);

    // Invitee should receive the INVITE message
    std::string expected_invite_msg = std::string(":Inviter!user@client1.host INVITE Invitee :#test") + "\r\n";
    EXPECT_EQ(client3->getLastMessage(), expected_invite_msg);

    // Invitee should be in the channel's invited list
    EXPECT_TRUE(channel->isInvitedUser(client3));

    // Inviter should not receive any error or confirmation (RFC behavior)
    EXPECT_TRUE(client1->receivedMessages.empty());
}

TEST_F(InviteCommandTest, Invite_NeedMoreParams) {
    args.push_back("Invitee"); // Missing channel name
    inviteCmd->execute(client1, args);

    std::vector<std::string> params;
    params.push_back("INVITE");
    std::string expected_reply =
        formatReply(server->getServerName(), client1->getNickname(), ERR_NEEDMOREPARAMS, params);
    EXPECT_EQ(client1->getLastMessage(), expected_reply);
}

TEST_F(InviteCommandTest, Invite_NoSuchNick) {
    args.push_back("NonExistentNick");
    args.push_back("#test");
    inviteCmd->execute(client1, args);

    std::vector<std::string> params;
    params.push_back("NonExistentNick");
    std::string expected_reply = formatReply(server->getServerName(), client1->getNickname(), ERR_NOSUCHNICK, params);
    EXPECT_EQ(client1->getLastMessage(), expected_reply);
}

TEST_F(InviteCommandTest, Invite_NoSuchChannel) {
    args.push_back("Invitee");
    args.push_back("#nonexistent");
    inviteCmd->execute(client1, args);

    std::vector<std::string> params;
    params.push_back("#nonexistent");
    std::string expected_reply =
        formatReply(server->getServerName(), client1->getNickname(), ERR_NOSUCHCHANNEL, params);
    EXPECT_EQ(client1->getLastMessage(), expected_reply);
}

TEST_F(InviteCommandTest, Invite_NotOnChannel) {
    // client2 is not in the channel
    channel->removeMember(client1); // Remove inviter from channel
    client1->removeChannel(channel);

    args.push_back("Invitee");
    args.push_back("#test");
    inviteCmd->execute(client1, args);

    std::vector<std::string> params;
    params.push_back("#test");
    std::string expected_reply = formatReply(server->getServerName(), client1->getNickname(), ERR_NOTONCHANNEL, params);
    EXPECT_EQ(client1->getLastMessage(), expected_reply);
}

TEST_F(InviteCommandTest, Invite_UserOnChannel) {
    channel->addMember(client3); // Invitee is already in channel
    client3->addChannel(channel);

    args.push_back("Invitee");
    args.push_back("#test");
    inviteCmd->execute(client1, args);

    std::vector<std::string> params;
    params.push_back("Invitee");
    params.push_back("#test");
    std::string expected_reply =
        formatReply(server->getServerName(), client1->getNickname(), ERR_USERONCHANNEL, params);
    EXPECT_EQ(client1->getLastMessage(), expected_reply);
}

TEST_F(InviteCommandTest, Invite_InviteOnlyChannel_NotOperator) {
    channel->setMode('i', true);      // Set channel to invite-only
    channel->removeOperator(client1); // Remove operator status from inviter

    args.push_back("Invitee");
    args.push_back("#test");
    inviteCmd->execute(client1, args);

    std::vector<std::string> params;
    params.push_back("#test");
    std::string expected_reply =
        formatReply(server->getServerName(), client1->getNickname(), ERR_CHANOPRIVSNEEDED, params);
    EXPECT_EQ(client1->getLastMessage(), expected_reply);
}

TEST_F(InviteCommandTest, Invite_InviteOnlyChannel_AsOperator) {
    channel->setMode('i', true); // Set channel to invite-only
    // client1 is already operator from SetUp

    args.push_back("Invitee");
    args.push_back("#test");
    inviteCmd->execute(client1, args);

    // Invitee should receive the INVITE message
    std::string expected_invite_msg = std::string(":Inviter!user@client1.host INVITE Invitee :#test") + "\r\n";
    EXPECT_EQ(client3->getLastMessage(), expected_invite_msg);

    // Invitee should be in the channel's invited list
    EXPECT_TRUE(channel->isInvitedUser(client3));
}
