# test/scripts/README.md

## IRCチャット動作確認

クラアントソフト（`irssi`）を使った動作確認
```sh
make

# or

make docker
```

## プロトコルベースの動作確認

`nc`コマンドによるIRCプロトコルを直接送る動作確認
```sh
make nc_test

# or

make nc_test docker
```
