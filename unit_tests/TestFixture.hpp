#ifndef TEST_FIXTURE_HPP
#define TEST_FIXTURE_HPP

#include "Channel.hpp"
#include "Client.hpp"
#include "CommandManager.hpp"
#include "JoinCommand.hpp"
#include "NickCommand.hpp"
#include "PartCommand.hpp"
#include "PassCommand.hpp"
#include "PrivmsgCommand.hpp"
#include "NoticeCommand.hpp"
#include "NamesCommand.hpp"
#include "TopicCommand.hpp"
#include "ModeCommand.hpp"
#include "KickCommand.hpp"
#include "InviteCommand.hpp"
#include "Replies.hpp"
#include "Server.hpp"
#include "UserCommand.hpp"
#include "gtest/gtest.h"
#include <string>
#include <vector>

/**
 * @brief Client::sendMessage をオーバーライドし、
 * 送信されたメッセージを内部のベクタに保存するモッククラス
 */
class TestClient : public Client {
  public:
    std::vector<std::string> receivedMessages;

    TestClient(int fd, const std::string &hostname) : Client(fd, hostname) {}

    virtual ~TestClient() {}

    // sendMessageをオーバーライド
    // --- 修正前 ---
    // virtual void sendMessage(const std::string &message) const {
    //     // const_castが必要になるが、テスト目的のため許容する
    //     const_cast<TestClient *>(this)->receivedMessages.push_back(message);
    // }
    // --- 修正箇所 ---
    virtual void sendMessage(const std::string &message) const {
        // 本物のClient::sendMessageと同様に、\r\n を付加して保存する
        std::string fullMessage = message + "\r\n";
        const_cast<TestClient *>(this)->receivedMessages.push_back(fullMessage);
    }

    // 最後に受信したメッセージを取得 (テスト用ヘルパー)
    std::string getLastMessage() const {
        if (receivedMessages.empty()) {
            return "";
        }
        return receivedMessages[receivedMessages.size() - 1];
    }

    // テスト用に最終アクティビティ時刻を設定する
    void setLastActivityTime(time_t newTime) {
        this->_lastActivityTime = newTime;
    }
};

/**
 * @brief 全てのコマンドテストで使用する共通フィクスチャ
 */
class CommandTest : public ::testing::Test {
  protected:
    Server *server;
    TestClient *client1; // メインのテスト対象クライアント
    TestClient *client2; // ニックネーム衝突やPRIVMSGの相手用

    // 各コマンドのインスタンス
    PassCommand *passCmd;
    NickCommand *nickCmd;
    UserCommand *userCmd;
    JoinCommand *joinCmd;
    PrivmsgCommand *privmsgCmd;
    PartCommand *partCmd;
    NoticeCommand *noticeCmd;
    NamesCommand *namesCmd;
    TopicCommand *topicCmd;
    ModeCommand *modeCmd;
    KickCommand *kickCmd;
    InviteCommand *inviteCmd;

    std::vector<std::string> args;

    virtual void SetUp() {
        // 1. サーバーをリセットして初期化
        Server::resetInstance();
        server = Server::getInstance(6667, "pass123");

        // 2. テスト用クライアントを作成 (FDは適当な負でない値)
        client1 = new TestClient(10, "client1.host");
        client2 = new TestClient(11, "client2.host");

        // 3. サーバーにクライアントを"接続"
        server->addTestClient(client1);
        server->addTestClient(client2);

        // 4. コマンドインスタンスを作成
        passCmd = new PassCommand(server);
        nickCmd = new NickCommand(server);
        userCmd = new UserCommand(server);
        joinCmd = new JoinCommand(server);
        privmsgCmd = new PrivmsgCommand(server);
        partCmd = new PartCommand(server);
        noticeCmd = new NoticeCommand(server);
        namesCmd = new NamesCommand(server);
        topicCmd = new TopicCommand(server);
        modeCmd = new ModeCommand(server);
        kickCmd = new KickCommand(server);
        inviteCmd = new InviteCommand(server);

        // 5. 引数ベクタをクリア
        args.clear();
    }

    virtual void TearDown() {
        delete passCmd;
        delete nickCmd;
        delete userCmd;
        delete joinCmd;
        delete privmsgCmd;
        delete partCmd;
        delete noticeCmd;
        delete namesCmd;
        delete topicCmd;
        delete modeCmd;
        delete kickCmd;
        delete inviteCmd;

        // ClientはServerが所有権を持つため、Serverのデストラクタに任せる
        Server::resetInstance();
    }

    // テスト用ヘルパー: クライアントを認証・登録済みにする
    void registerClient(TestClient *client, const std::string &nick) {
        client->setAuthenticated(true);
        client->setNickname(nick);
        client->setUsername("user");
        client->setRegistered(true);
    }
};

/**
 * @brief 全てのコマンドテストで使用する共通フィクスチャ
 */
class CommandManagerTest : public ::testing::Test {
  protected:
    Server *server;
    CommandManager *cmdManager;
    TestClient *client1; // メインのテスト対象クライアント
    TestClient *client2; // ニックネーム衝突やPRIVMSGの相手用

    std::vector<std::string> args;

    virtual void SetUp() {
        // 1. サーバーをリセットして初期化
        Server::resetInstance();
        server = Server::getInstance(6667, "pass123");

        // 2. テスト用クライアントを作成 (FDは適当な負でない値)
        client1 = new TestClient(10, "client1.host");
        client2 = new TestClient(11, "client2.host");

        // 3. サーバーにクライアントを"接続"
        server->addTestClient(client1);
        server->addTestClient(client2);

        // 4. 引数ベクタをクリア
        args.clear();

        // コマンドマネージャーインスタンスを作成
        cmdManager = new CommandManager(server);
    }

    virtual void TearDown() {
        delete cmdManager;

        // ClientはServerが所有権を持つため、Serverのデストラクタに任せる
        Server::resetInstance();
    }

    // テスト用ヘルパー: クライアントを認証・登録済みにする
    void registerClient(TestClient *client, const std::string &nick) {
        client->setAuthenticated(true);
        client->setNickname(nick);
        client->setUsername("user");
        client->setRegistered(true);
    }
};
#include <iostream>
#include <string>
#include <vector>

// テンプレートクラスの宣言
template <typename T> class DebugVector {
  public:
    // コンストラクタ
    explicit DebugVector(const std::vector<T> &vec) : m_vec(vec) {}

    // フレンド関数として、ストリーミング演算子をオーバーロード
    friend std::ostream &operator<<(std::ostream &os, const DebugVector &dv) {
        os << "[ ";
        for (const auto &elem : dv.m_vec) {
            os << elem << " ";
        }
        os << "]";
        return os;
    }

  private:
    const std::vector<T> &m_vec;
};

// 例外処理：文字列ベクターの場合、引用符付きで出力するように特殊化
template <> class DebugVector<std::string> {
  public:
    explicit DebugVector(const std::vector<std::string> &vec) : m_vec(vec) {}

    friend std::ostream &operator<<(std::ostream &os, const DebugVector &dv) {
        os << "[ ";
        for (const auto &elem : dv.m_vec) {
            os << "\"" << elem << "\" ";
        }
        os << "]";
        return os;
    }

  private:
    const std::vector<std::string> &m_vec;
};

#endif
