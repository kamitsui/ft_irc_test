#include "gtest/gtest.h"
#include "Replies.hpp"

// Helper to construct a full prefix for comparison
std::string get_full_prefix(const std::string& nickname, const std::string& username, const std::string& hostname) {
    return ":" + nickname + "!" + username + "@" + hostname;
}

TEST(RepliesTest, RPL_WELCOME) {
    std::string nickname = "testnick";
    std::string username = "testuser";
    std::string hostname = "testhost";
    std::string expected = ":" + hostname + " 001 " + nickname + " :Welcome to the ft_irc Network, " + nickname + "!" + username + "@" + hostname + "\r\n";
    EXPECT_EQ(Replies::welcome(nickname, username, hostname), expected);
}

TEST(RepliesTest, ERR_NOSUCHNICK) {
    std::string client_nick = "sender";
    std::string target_nick = "nonexistent";
    std::string expected = ":ft_irc 401 " + client_nick + " " + target_nick + " :No such nick/channel\r\n";
    EXPECT_EQ(Replies::errNoSuchNick(client_nick, target_nick), expected);
}

TEST(RepliesTest, ERR_NEEDMOREPARAMS) {
    std::string client_nick = "testnick";
    std::string command = "JOIN";
    std::string expected = ":ft_irc 461 " + client_nick + " " + command + " :Not enough parameters\r\n";
    EXPECT_EQ(Replies::errNeedMoreParams(client_nick, command), expected);
}

TEST(RepliesTest, ERR_PASSWDMISMATCH) {
    std::string client_nick = "testnick";
    std::string expected = ":ft_irc 464 " + client_nick + " :Password incorrect\r\n";
    EXPECT_EQ(Replies::errPasswdMismatch(client_nick), expected);
}

TEST(RepliesTest, ERR_NICKNAMEINUSE) {
    std::string client_nick = "testnick";
    std::string used_nick = "usednick";
    std::string expected = ":ft_irc 433 " + client_nick + " " + used_nick + " :Nickname is already in use\r\n";
    EXPECT_EQ(Replies::errNicknameInUse(client_nick, used_nick), expected);
}

TEST(RepliesTest, ERR_ALREADYREGISTRED) {
    std::string client_nick = "testnick";
    std::string expected = ":ft_irc 462 " + client_nick + " :You may not reregister\r\n";
    EXPECT_EQ(Replies::errAlreadyRegistred(client_nick), expected);
}

TEST(RepliesTest, ERR_UNKNOWNCOMMAND) {
    std::string client_nick = "testnick";
    std::string command = "UNKNOWN";
    std::string expected = ":ft_irc 421 " + client_nick + " " + command + " :Unknown command\r\n";
    EXPECT_EQ(Replies::errUnknownCommand(client_nick, command), expected);
}

TEST(RepliesTest, ERR_NONICKNAMEGIVEN) {
    std::string client_nick = "testnick";
    std::string expected = ":ft_irc 431 " + client_nick + " :No nickname given\r\n";
    EXPECT_EQ(Replies::errNoNicknameGiven(client_nick), expected);
}

TEST(RepliesTest, ERR_ERRONEUSNICKNAME) {
    std::string client_nick = "testnick";
    std::string erroneous_nick = "bad!nick";
    std::string expected = ":ft_irc 432 " + client_nick + " " + erroneous_nick + " :Erroneous nickname\r\n";
    EXPECT_EQ(Replies::errErroneusNickname(client_nick, erroneous_nick), expected);
}

TEST(RepliesTest, ERR_NOSUCHCHANNEL) {
    std::string client_nick = "testnick";
    std::string channel_name = "#nonexistent";
    std::string expected = ":ft_irc 403 " + client_nick + " " + channel_name + " :No such channel\r\n";
    EXPECT_EQ(Replies::errNoSuchChannel(client_nick, channel_name), expected);
}

TEST(RepliesTest, ERR_CANNOTSENDTOCHAN) {
    std::string client_nick = "testnick";
    std::string channel_name = "#private";
    std::string expected = ":ft_irc 404 " + client_nick + " " + channel_name + " :Cannot send to channel\r\n";
    EXPECT_EQ(Replies::errCannotSendToChan(client_nick, channel_name), expected);
}

TEST(RepliesTest, ERR_NOTONCHANNEL) {
    std::string client_nick = "testnick";
    std::string channel_name = "#notmember";
    std::string expected = ":ft_irc 442 " + client_nick + " " + channel_name + " :You're not on that channel\r\n";
    EXPECT_EQ(Replies::errNotOnChannel(client_nick, channel_name), expected);
}

TEST(RepliesTest, ERR_BADCHANNELKEY) {
    std::string client_nick = "testnick";
    std::string channel_name = "#keychan";
    std::string expected = ":ft_irc 475 " + client_nick + " " + channel_name + " :Cannot join channel (+k)\r\n";
    EXPECT_EQ(Replies::errBadChannelKey(client_nick, channel_name), expected);
}

TEST(RepliesTest, ERR_INVITEONLYCHAN) {
    std::string client_nick = "testnick";
    std::string channel_name = "#inviteonly";
    std::string expected = ":ft_irc 473 " + client_nick + " " + channel_name + " :Cannot join channel (+i)\r\n";
    EXPECT_EQ(Replies::errInviteOnlyChan(client_nick, channel_name), expected);
}

TEST(RepliesTest, ERR_CHANNELISFULL) {
    std::string client_nick = "testnick";
    std::string channel_name = "#fullchan";
    std::string expected = ":ft_irc 471 " + client_nick + " " + channel_name + " :Cannot join channel (+l)\r\n";
    EXPECT_EQ(Replies::errChannelIsFull(client_nick, channel_name), expected);
}

TEST(RepliesTest, ERR_NOTEXTTOSEND) {
    std::string client_nick = "testnick";
    std::string expected = ":ft_irc 412 " + client_nick + " :No text to send\r\n";
    EXPECT_EQ(Replies::errNoTextToSend(client_nick), expected);
}
