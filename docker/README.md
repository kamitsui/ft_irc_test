# docker を使ったIRCクライアント・IRCサーバー

## IRC クライアント

IRCサーバーの実装を検証するために、軽量でCLIベースのIRCクライアントである **irssi** を使います。

### irssiの起動方法 (Docker)

```bash
docker run -it --rm \
  --name=irssi \
  --network host \
  irssi
```

* `--network host`: ホストのネットワークスタックを共有します。
* これにより、コンテナ内から `localhost:<あなたのIRCサーバーのポート番号>` でIRCサーバーに接続できます。

* **注意:** `--network host` は、コンテナがホストのネットワークインターフェースを直接使用することを意味します。これにより、コンテナ内のirssiがホスト上で動作しているIRCサーバーに `localhost` 経由で接続できるようになります。セキュリティ上の理由から、本番環境での使用は推奨されませんが、開発・検証目的では非常に便利です。

1. irssi内でのIRCサーバーへの接続
```
/connect localhost <あなたのIRCサーバーのポート番号>
# 例
# /connect localhost 6667
```

2. コマンド操作

* フォーマット
```
/nick <your nick name>
/user <user name> <host name> <server name> :<real name>
/join #<channel name>
/quit
```

* 例
```
/nick testuser
/user testuser 0 * :Test User
/join #general
/quit
```

## IRCサーバー

### Ergo

```sh
docker compose -f docker-compose.ergo.yml up -d
```
> port は `6668` に設定
>
> モダン, IRCv3に対応

### ngircd

```sh
docker compose -f docker-compose.ngircd.yml up -d
```
> 初回起動時は`config/ngircd.conf`が作成される。
>
> port は `6669` に設定

### 不採用

**amd64アーキテクチャ向けのみ。**

* [テストサーバー](https://github.com/irccom/test-servers)
> ```sh
> docker run -d -p 6670:6667 irccom/ircd-hybrid
> ```
> 
> ```sh
> docker run -d -p 6671:6667 jtbouse/hybrid-ircd
> ```

* [テストスクリプトランナー](https://github.com/irccom/script-runner)
> 使える？　IRCデーモン上で動く？らしい。
> これを ft_irc でも使えないか？

### 保留

```
docker-compose.yml
ircd.yaml
```
