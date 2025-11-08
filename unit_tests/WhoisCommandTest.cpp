#include "TestFixture.hpp"
#include "WhoisCommand.hpp"
#include "Channel.hpp"
#include <unistd.h> // for sleep

class WhoisCommandTest : public CommandTest {
  protected:
    WhoisCommand *whoisCmd;
    TestClient *targetClient;

    virtual void SetUp() {
        CommandTest::SetUp();
        whoisCmd = new WhoisCommand(server);
        
        // client1 is the one executing WHOIS
        registerClient(client1, "UserA");

        // targetClient is the one being looked up
        targetClient = new TestClient(12, "target.host");
        server->addTestClient(targetClient);
        registerClient(targetClient, "UserB");
        targetClient->setUsername("userb_username"); // Use a distinct username
        targetClient->setRealname("Test User"); // Set real name for the test

        // Have UserB join two channels, one as an operator
        Channel *chan1 = new Channel("#chan1");
        server->addChannel(chan1);
        chan1->addMember(targetClient);
        chan1->addOperator(targetClient);
        targetClient->addChannel(chan1);

        Channel *chan2 = new Channel("#chan2");
        server->addChannel(chan2);
        chan2->addMember(targetClient);
        targetClient->addChannel(chan2);

        client1->receivedMessages.clear();
    }

    virtual void TearDown() {
        delete whoisCmd;
        CommandTest::TearDown();
    }
};

TEST_F(WhoisCommandTest, Whois_Success) {
    // Make the target client idle for a bit
    targetClient->setLastActivityTime(time(NULL) - 10); // 10 seconds idle

    args.push_back("UserB");
    whoisCmd->execute(client1, args);

    ASSERT_GE(client1->receivedMessages.size(), 4);

    // 1. RPL_WHOISUSER (311)
    std::string whoisuser_reply = client1->receivedMessages[0];
    EXPECT_NE(whoisuser_reply.find("311"), std::string::npos);
    EXPECT_NE(whoisuser_reply.find("UserB"), std::string::npos); // nick
    EXPECT_NE(whoisuser_reply.find("userb_username"), std::string::npos); // user
    EXPECT_NE(whoisuser_reply.find("target.host"), std::string::npos); // host
    EXPECT_NE(whoisuser_reply.find("Test User"), std::string::npos); // real name (from registerClient helper)

    // 2. RPL_WHOISCHANNELS (319)
    std::string whoischannels_reply = client1->receivedMessages[1];
    EXPECT_NE(whoischannels_reply.find("319"), std::string::npos);
    EXPECT_NE(whoischannels_reply.find("UserB"), std::string::npos);
    // Order of channels is not guaranteed, so check for both
    EXPECT_NE(whoischannels_reply.find("@#chan1"), std::string::npos);
    EXPECT_NE(whoischannels_reply.find("#chan2"), std::string::npos);

    // 3. RPL_WHOISIDLE (317)
    std::string whoisidle_reply = client1->receivedMessages[2];
    EXPECT_NE(whoisidle_reply.find("317"), std::string::npos);
    EXPECT_NE(whoisidle_reply.find("UserB"), std::string::npos);
    // Check that idle time is reported (e.g., " 10 ")
    EXPECT_NE(whoisidle_reply.find(" 10 "), std::string::npos);

    // 4. RPL_ENDOFWHOIS (318)
    std::string endofwhois_reply = client1->receivedMessages[3];
    EXPECT_NE(endofwhois_reply.find("318"), std::string::npos);
    EXPECT_NE(endofwhois_reply.find("UserB"), std::string::npos);
}

TEST_F(WhoisCommandTest, Whois_NoSuchNick) {
    args.push_back("NonExistentUser");
    whoisCmd->execute(client1, args);

    ASSERT_EQ(client1->receivedMessages.size(), 1);
    std::vector<std::string> params;
    params.push_back("NonExistentUser");
    std::string expected_reply =
        formatReply(server->getServerName(), client1->getNickname(), ERR_NOSUCHNICK, params) + "\r\n";
    EXPECT_EQ(client1->getLastMessage(), expected_reply);
}

TEST_F(WhoisCommandTest, Whois_NoNicknameGiven) {
    // No arguments
    whoisCmd->execute(client1, args);

    ASSERT_EQ(client1->receivedMessages.size(), 1);
    std::string expected_reply =
        formatReply(server->getServerName(), client1->getNickname(), ERR_NONICKNAMEGIVEN, std::vector<std::string>()) + "\r\n";
    EXPECT_EQ(client1->getLastMessage(), expected_reply);
}
