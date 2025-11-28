// #include "Client.hpp"
// #include "gtest/gtest.h"
//
// TEST(ClientTest, Constructor) {
//     Client client(1, "127.0.0.1");
//     EXPECT_EQ(client.getFd(), 1);
//     EXPECT_EQ(client.getHostname(), "127.0.0.1");
//     EXPECT_EQ(client.getNickname(), "");
//     EXPECT_EQ(client.getUsername(), "");
//     // EXPECT_EQ(client.getRealname(), "");
//     EXPECT_FALSE(client.isAuthenticated());
//     EXPECT_FALSE(client.isRegistered());
// }
//
// TEST(ClientTest, SettersAndGetters) {
//     Client client(2, "localhost");
//     client.setNickname("testnick");
//     client.setUsername("testuser");
//     // client.setRealname("Test User");
//     client.setAuthenticated(true);
//
//     EXPECT_EQ(client.getNickname(), "testnick");
//     EXPECT_EQ(client.getUsername(), "testuser");
//     // EXPECT_EQ(client.getRealname(), "Test User");
//     EXPECT_TRUE(client.isAuthenticated());
//
//     // isRegistered depends on nickname, username and authenticated status
//     EXPECT_FALSE(client.isRegistered());
//     client.setRegistered(true);
//     EXPECT_TRUE(client.isRegistered());
// }

#include "Client.hpp"
#include "gtest/gtest.h"

// Clientクラスのバッファ管理機能のためのテストフィクスチャ
class ClientBufferTest : public ::testing::Test {
  protected:
    Client *client;

    virtual void SetUp() {
        // テスト用のClientインスタンス (FDは-1でOK)
        client = new Client(-1, "testhost.local");
    }

    virtual void TearDown() { delete client; }
};

// バッファにCRLF区切りの単一行を追加
TEST_F(ClientBufferTest, ReadLine_SingleLineCRLF) {
    client->appendBuffer("CMD arg1 arg2\r\n");
    ASSERT_EQ(client->readLineFromBuffer(), "CMD arg1 arg2");
    ASSERT_EQ(client->readLineFromBuffer(), ""); // バッファは空
}

// バッファにLF区切りの単一行を追加 (ncなどへの対応)
TEST_F(ClientBufferTest, ReadLine_SingleLineLF) {
    client->appendBuffer("CMD arg1 arg2\n");
    ASSERT_EQ(client->readLineFromBuffer(), "CMD arg1 arg2");
    ASSERT_EQ(client->readLineFromBuffer(), "");
}

// バッファに複数行を一度に追加
TEST_F(ClientBufferTest, ReadLine_MultipleLines) {
    client->appendBuffer("LINE1\r\nLINE2\r\nLINE3\n");
    ASSERT_EQ(client->readLineFromBuffer(), "LINE1");
    ASSERT_EQ(client->readLineFromBuffer(), "LINE2");
    ASSERT_EQ(client->readLineFromBuffer(), "LINE3");
    ASSERT_EQ(client->readLineFromBuffer(), "");
}

// バッファに不完全な行が残るケース
TEST_F(ClientBufferTest, ReadLine_PartialLine) {
    client->appendBuffer("LINE1\r\nPARTIAL");
    ASSERT_EQ(client->readLineFromBuffer(), "LINE1");
    // 不完全な行は読み込まれない
    ASSERT_EQ(client->readLineFromBuffer(), "");
    // 追加のデータが来て初めて読み込まれる
    client->appendBuffer("_DATA\n");
    ASSERT_EQ(client->readLineFromBuffer(), "PARTIAL_DATA");
    ASSERT_EQ(client->readLineFromBuffer(), "");
}

// バッファが空のケース
TEST_F(ClientBufferTest, ReadLine_Empty) { ASSERT_EQ(client->readLineFromBuffer(), ""); }

// TEST: appendToSendBufferとgetSendBufferの基本機能
TEST_F(ClientBufferTest, SendBuffer_AppendAndGet) {
    ASSERT_TRUE(client->getSendBuffer().empty());

    client->appendToSendBuffer("HELLO");
    ASSERT_EQ(client->getSendBuffer(), "HELLO");

    client->appendToSendBuffer(" WORLD");
    ASSERT_EQ(client->getSendBuffer(), "HELLO WORLD");
}

// TEST: sendBufferからのデータ削除 (部分送信のシミュレーション)
TEST_F(ClientBufferTest, SendBuffer_RemoveSentData) {
    client->appendToSendBuffer("FULL_MESSAGE_TO_SEND");
    //ASSERT_EQ(client->getSendBuffer().length(), 20);
    EXPECT_EQ(client->getSendBuffer().length(), static_cast<std::string::size_type>(20));

    // 最初の5バイトが送信されたと仮定
    client->removeSentData(5);
    //ASSERT_EQ(client->getSendBuffer().c_str(), "MESSAGE_TO_SEND");
    ASSERT_STREQ(client->getSendBuffer().c_str(), "MESSAGE_TO_SEND");
    ASSERT_EQ(client->getSendBuffer().length(), static_cast<std::string::size_type>(15));

    // 残り全てが送信されたと仮定
    client->removeSentData(15);
    ASSERT_TRUE(client->getSendBuffer().empty());
}

// TEST: sendBufferが空の時にデータ削除を試みる
TEST_F(ClientBufferTest, SendBuffer_RemoveFromEmpty) {
    ASSERT_TRUE(client->getSendBuffer().empty());

    client->removeSentData(0); // 0バイト削除
    ASSERT_TRUE(client->getSendBuffer().empty());

    client->removeSentData(100); // 空文字列から100バイト削除を試みる
    ASSERT_TRUE(client->getSendBuffer().empty());
}
