# docker を使ったIRCクライアント・IRCサーバー

## IRC クライアント

編集中

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

### テストのフレームワーク？

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
