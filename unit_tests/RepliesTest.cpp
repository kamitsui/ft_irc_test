#include "gtest/gtest.h"
#include "Replies.hpp"

// Helper to construct a full prefix for comparison
std::string get_full_prefix(const std::string& nickname, const std::string& username, const std::string& hostname) {
    return ":" + nickname + "!" + username + "@" + hostname;
}

TEST(RepliesTest, WelcomeReply) {
    std::string server_name = "ft_irc";
    std::string nickname = "testnick";
    std::string username = "testuser";
    std::string hostname = "testhost";
    std::string client_prefix = ":" + nickname + "!" + username + "@" + hostname;
    std::string expected = ":" + server_name + " 001 " + nickname + " :Welcome to the Internet Relay Network " + client_prefix + "\r\n";
    EXPECT_EQ(formatReply(server_name, nickname, RPL_WELCOME, {client_prefix}), expected);
}

TEST(RepliesTest, ErrNoSuchNickReply) {
    std::string server_name = "ft_irc";
    std::string client_nick = "sender";
    std::string target_nick = "nonexistent";
    std::string expected = ":" + server_name + " 401 " + client_nick + " " + target_nick + " :No such nick/channel\r\n";
    EXPECT_EQ(formatReply(server_name, client_nick, ERR_NOSUCHNICK, {target_nick}), expected);
}

TEST(RepliesTest, ErrNeedMoreParamsReply) {
    std::string server_name = "ft_irc";
    std::string client_nick = "testnick";
    std::string command = "JOIN";
    std::string expected = ":" + server_name + " 461 " + client_nick + " " + command + " :Not enough parameters\r\n";
    EXPECT_EQ(formatReply(server_name, client_nick, ERR_NEEDMOREPARAMS, {command}), expected);
}

TEST(RepliesTest, ErrPasswdMismatchReply) {
    std::string server_name = "ft_irc";
    std::string client_nick = "testnick";
    std::string expected = ":" + server_name + " 464 " + client_nick + " :Password incorrect\r\n";
    EXPECT_EQ(formatReply(server_name, client_nick, ERR_PASSWDMISMATCH, {}), expected);
}

TEST(RepliesTest, ErrNicknameInUseReply) {
    std::string server_name = "ft_irc";
    std::string client_nick = "testnick";
    std::string used_nick = "usednick";
    std::string expected = ":" + server_name + " 433 " + client_nick + " " + used_nick + " :Nickname is already in use\r\n";
    EXPECT_EQ(formatReply(server_name, client_nick, ERR_NICKNAMEINUSE, {used_nick}), expected);
}

TEST(RepliesTest, ErrAlreadyRegistredReply) {
    std::string server_name = "ft_irc";
    std::string client_nick = "testnick";
    std::string expected = ":" + server_name + " 462 " + client_nick + " :Unauthorized command (already registered)\r\n";
    EXPECT_EQ(formatReply(server_name, client_nick, ERR_ALREADYREGISTRED, {}), expected);
}

TEST(RepliesTest, ErrUnknownCommandReply) {
    std::string server_name = "ft_irc";
    std::string client_nick = "testnick";
    std::string command = "UNKNOWN";
    std::string expected = ":" + server_name + " 421 " + client_nick + " " + command + " :Unknown command\r\n";
    EXPECT_EQ(formatReply(server_name, client_nick, ERR_UNKNOWNCOMMAND, {command}), expected);
}

TEST(RepliesTest, ErrNoNicknameGivenReply) {
    std::string server_name = "ft_irc";
    std::string client_nick = "testnick";
    std::string expected = ":" + server_name + " 431 " + client_nick + " :No nickname given\r\n";
    EXPECT_EQ(formatReply(server_name, client_nick, ERR_NONICKNAMEGIVEN, {}), expected);
}

TEST(RepliesTest, ErrErroneusNicknameReply) {
    std::string server_name = "ft_irc";
    std::string client_nick = "testnick";
    std::string erroneous_nick = "bad!nick";
    std::string expected = ":" + server_name + " 432 " + client_nick + " " + erroneous_nick + " :Erroneous nickname\r\n";
    EXPECT_EQ(formatReply(server_name, client_nick, ERR_ERRONEUSNICKNAME, {erroneous_nick}), expected);
}

TEST(RepliesTest, ErrNoSuchChannelReply) {
    std::string server_name = "ft_irc";
    std::string client_nick = "testnick";
    std::string channel_name = "#nonexistent";
    std::string expected = ":" + server_name + " 403 " + client_nick + " " + channel_name + " :No such channel\r\n";
    EXPECT_EQ(formatReply(server_name, client_nick, ERR_NOSUCHCHANNEL, {channel_name}), expected);
}

TEST(RepliesTest, ErrCannotSendToChanReply) {
    std::string server_name = "ft_irc";
    std::string client_nick = "testnick";
    std::string channel_name = "#private";
    std::string expected = ":" + server_name + " 404 " + client_nick + " " + channel_name + " :Cannot send to channel\r\n";
    EXPECT_EQ(formatReply(server_name, client_nick, ERR_CANNOTSENDTOCHAN, {channel_name}), expected);
}

TEST(RepliesTest, ErrNotOnChannelReply) {
    std::string server_name = "ft_irc";
    std::string client_nick = "testnick";
    std::string channel_name = "#notmember";
    std::string expected = ":" + server_name + " 442 " + client_nick + " " + channel_name + " :You're not on that channel\r\n";
    EXPECT_EQ(formatReply(server_name, client_nick, ERR_NOTONCHANNEL, {channel_name}), expected);
}

TEST(RepliesTest, ErrBadChannelKeyReply) {
    std::string server_name = "ft_irc";
    std::string client_nick = "testnick";
    std::string channel_name = "#keychan";
    std::string expected = ":" + server_name + " 475 " + client_nick + " " + channel_name + " :Cannot join channel (+k)\r\n";
    EXPECT_EQ(formatReply(server_name, client_nick, ERR_BADCHANNELKEY, {channel_name}), expected);
}

TEST(RepliesTest, ErrInviteOnlyChanReply) {
    std::string server_name = "ft_irc";
    std::string client_nick = "testnick";
    std::string channel_name = "#inviteonly";
    std::string expected = ":" + server_name + " 473 " + client_nick + " " + channel_name + " :Cannot join channel (+i)\r\n";
    EXPECT_EQ(formatReply(server_name, client_nick, ERR_INVITEONLYCHAN, {channel_name}), expected);
}

TEST(RepliesTest, ErrChannelIsFullReply) {
    std::string server_name = "ft_irc";
    std::string client_nick = "testnick";
    std::string channel_name = "#fullchan";
    std::string expected = ":" + server_name + " 471 " + client_nick + " " + channel_name + " :Cannot join channel (+l)\r\n";
    EXPECT_EQ(formatReply(server_name, client_nick, ERR_CHANNELISFULL, {channel_name}), expected);
}

TEST(RepliesTest, ErrNoTextToSendReply) {
    std::string server_name = "ft_irc";
    std::string client_nick = "testnick";
    std::string expected = ":" + server_name + " 412 " + client_nick + " :No text to send\r\n";
    EXPECT_EQ(formatReply(server_name, client_nick, ERR_NOTEXTTOSEND, {}), expected);
}

TEST(RepliesTest, ErrUserOnChannelReply) {
    std::string server_name = "ft_irc";
    std::string client_nick = "inviter";
    std::string target_nick = "invitee";
    std::string channel_name = "#channel";
    std::string expected = ":" + server_name + " 443 " + client_nick + " " + target_nick + " " + channel_name + " :is already on channel\r\n";
    EXPECT_EQ(formatReply(server_name, client_nick, ERR_USERONCHANNEL, {target_nick, channel_name}), expected);
}
