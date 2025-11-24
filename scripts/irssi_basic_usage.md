## irssi 基本ガイド

irssiは、ターミナルベースのIRC（Internet Relay Chat）クライアントです。軽量で高機能、カスタマイズ性が高いのが特徴で、SSH経由でリモートサーバー上で実行し続けることも可能です。

### 1. irssiのインストール

* ローカルにインストールする場合
> ほとんどのLinuxディストリビューションでは、パッケージマネージャーを使って簡単にインストールできます。
> 
> ```bash
> # Debian/Ubuntuの場合
> sudo apt update
> sudo apt install irssi
> 
> # Fedora/CentOSの場合
> sudo dnf install irssi
> 
> # macOS (Homebrew)の場合
> brew install irssi
> ```

* 三井PC（M2 macOS）では、`test/docker/docker-compose.yml` でビルドされた`irssi-client`コンテナを使って動作確認します。

* 校舎PCはデフォルトでインストールされているため、ローカルホスト間で動作確認を行います。

### 2. irssiの起動

ターミナルで以下のコマンドを実行します。

```bash
irssi
```

irssiが起動すると、白い画面にステータスウィンドウが表示されます。

### 3. IRCサーバーへの接続

`irssi` からIRCサーバーに接続するには、`/connect` コマンドを使用します。

```bash
/connect <hostname> [port] [password]
```

* `irssi-client` のコンテナから `irc-server`のコンテナへ接続する場合

```bash
/server add -nocap irc-server 6667 password
/connect irc-server
```

* 校舎環境：ローカルホスト間で`irssi`を起動して、`ircserv` に接続する場合

```bash
/connect localhost 6667 password
```

接続に成功すると、ステータスウィンドウに接続メッセージが表示されます。

### 4. ニックネーム (Nickname) / ユーザーネーム（Username） の設定

接続後、ニックネームを設定します。これはIRC上でのあなたの表示名になります。

```bash
/set nick <新しいニックネーム>
```

**例:**

```bash
/set nick MyCoolNick
```

ユーザーネーム（Username）の設定も同様の操作です。

### 5. チャンネルへの参加と離脱

#### チャンネルへの参加 (JOIN)

チャンネルに参加するには `/join` コマンドを使用します。

```bash
/join #<チャンネル名> [チャンネルキー (パスワード)]
```

**例:**

```bash
/join #mychannel
/join #secretchannel mykey
```

チャンネルに参加すると、新しいウィンドウが開き、そのチャンネルの会話が表示されます。

**パスワードは非対応** : 実装したIRCサーバーはチャンネルに対してパスワード設定機能はサポートしていません。

#### チャンネルからの離脱 (PART)

チャンネルから離脱するには `/part` コマンドを使用します。

```bash
/part #<チャンネル名> [離脱メッセージ]
```

**例:**

```bash
/part #mychannel
/part #mychannel Leaving now!
```

### 6. メッセージの送信

#### チャンネルメッセージ

チャンネルウィンドウで直接メッセージを入力してEnterキーを押すと、そのチャンネルにメッセージが送信されます。

```
Hello everyone in this channel!
```

#### プライベートメッセージ (PRIVMSG)

特定のユーザーにプライベートメッセージを送るには `/msg` コマンドを使用します。

```bash
/msg <ニックネーム> <メッセージ>
```

**例:**

```bash
/msg JohnDoe Hi John, how are you?
```

### 7. ウィンドウの操作

irssiは複数のウィンドウを扱います（ステータス、各チャンネル、プライベートメッセージなど）。

-   **ウィンドウの切り替え:** `Alt` + `[数字]` (例: `Alt` + `1` でウィンドウ1へ)
-   **次のウィンドウへ:** `Ctrl` + `p` または `Alt` + `→`
-   **前のウィンドウへ:** `Ctrl` + `n` または `Alt` + `←`
-   **ウィンドウの閉じる:** `/window close` (ステータスウィンドウは閉じられません)

### 8. その他の便利なコマンド

-   **ユーザーリストの表示:** チャンネルウィンドウで `/names`
-   **チャンネルリストの表示:** `/list -yes`
-   **ユーザー情報の確認:** `/whois <ニックネーム>`
-   **トピックの設定:** チャンネルオペレーターとして `/topic #<チャンネル名> :<新しいトピック>`
-   **チャンネルモードの設定:** チャンネルオペレーターとして `/mode #<チャンネル名> <モード>` (例: `/mode #mychannel +i`)
-   **ユーザーのキック:** チャンネルオペレーターとして `/kick #<チャンネル名> <ニックネーム> [理由]`
-   **ユーザーの招待:** チャンネルオペレーターとして `/invite <ニックネーム> #<チャンネル名>`

### 9. irssiの終了

irssiを終了するには `/quit` コマンドを使用します。

```bash
/quit [終了メッセージ]
```

**例:**

```bash
/quit See you later!
```

### 10. 設定の保存

`irssi` で行った設定（接続情報、自動参加チャンネルなど）を保存するには、`/save` コマンドを使用します。

```bash
/save
```

これにより、設定が `~/.irssi/config` に保存され、次回 `irssi` 起動時にロードされます。
