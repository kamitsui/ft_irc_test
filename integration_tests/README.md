# 統合テスト

このディレクトリには、`ft_irc`サーバーの機能とプロトコル準拠を検証するための統合テストスイートが含まれています。`pytest`フレームワークを使用して、実際のIRCサーバーインスタンスとの対話をシミュレートします。

## テストの自動化

```sh
make           # テスト環境構築(.venv)　＋　テスト実行
make install   # テスト環境構築(.venv)
make run       # テスト実行
make clean     # テスト環境削除
```

## 手動でのテスト

* 作業ディレクトリ：`integration_tests/`
* 統合テストを実行する前に、Python仮想環境構築と、必要なPythonパッケージをインストールします。
> ```bash
> python3 -m venv .venv
> source .venv/bin/activate
> pip install -r requirements.txt
> ```

* テストスクリプト全て実行。
> ```bash
> pytest
> ```
> または
> ```bash
> pytest -v -s
> ```

* 特定のテストファイルのみを実行したい場合は、ファイル名を指定します。
> ```bash
> pytest test_registration.py
> ```

* 特定のテスト関数のみを実行したい場合は、`::`で関数名を指定します。
> ```bash
> pytest test_registration.py::test_successful_registration
> ```

## ファイル構成

* `conftest.py`: pytest の設定ファイル。サーバーを起動・停止する共通フィクスチャ (`irc_server`) を定義しています。
* `client_helper.py`: `IRCClient` ヘルパークラス。ソケット通信やIRCメッセージの送受信・解析をラップし、テストコードを簡潔に保ちます。
* `test_01_connection.py`: 接続と登録（`PASS`, `NICK`, `USER`）に関連するテストケース。
* `test_02_messaging.py`: チャンネル参加（`JOIN`）やメッセージ送信（`PRIVMSG`）に関連するテストケース。
* (今後) test_03_channel_ops.py`: `PART`, `TOPIC`, `MODE`, `KICK` などのテストを追加します。


### 不採用のコード

**テスト設定 (`pytest.ini`)**
> `pytest.ini`ファイルは、`pytest`の実行に関する設定を定義します。特に、IRCサーバーの実行可能ファイルのパスとパスワードがここで設定されます。
> 
> ```ini
> [pytest]
> irc_server_executable = ../../ircserv
> irc_server_password = pass
> ```
> * `irc_server_executable`: プロジェクトのルートディレクトリからIRCサーバーの実行可能ファイルへの相対パスを指定します。
> * `irc_server_password`: IRCサーバーの起動時に使用されるパスワードを指定します。
> これらの設定は`conftest.py`フィクスチャによって読み込まれ、テスト実行時にIRCサーバーを適切に起動するために使用されます。

**テストフィクスチャ (`conftest.py`)**
> `conftest.py`ファイルは、`pytest`のフィクスチャを定義し、テストのセットアップとティアダウンを管理します。
> 
> `irc_server`フィクスチャは、各テスト関数の実行前にIRCサーバーを起動し、テスト完了後にサーバーを停止します。これにより、各テストが独立したクリーンなサーバーインスタンスに対して実行されることが保証されます。

**テストユーティリティ (`irc_test_utils.py`)**
> このファイルには、IRCサーバーとクライアントとの対話に必要なヘルパークラスが含まれています。
> *   **`IrcServer`**: IRCサーバープロセスを起動、停止、およびその出力を監視するためのユーティリティクラスです。
> *   **`IrcClient`**: IRCサーバーに接続し、コマンドを送信し、応答を受信するためのシンプルなIRCクライアントクラスです。
> これらのユーティリティは、テストコードを簡潔に保ち、IRCプロトコルとの低レベルな対話を抽象化するのに役立ちます。

**登録テスト (`test_registration.py`)**
> このファイルには、IRCサーバーへのユーザー登録に関連する統合テストが含まれています。
> *   `test_successful_registration`: ユーザーが`PASS`、`NICK`、`USER`コマンドを使用して正常に登録できることを検証します。
> *   `test_nick_collision`: 既に存在するニックネームで登録しようとしたときに、サーバーが`ERR_NICKNAMEINUSE` (433) エラーを正しく返すことを検証します。
> *   `test_missing_password`: パスワードなしで登録しようとしたときに、サーバーが適切なエラー（例: `ERR_NOTREGISTERED` (451) または `ERR_PASSWDMISMATCH` (464)）を返すことを検証します。
> これらのテストは、IRCサーバーのユーザー登録機能がIRCプロトコル仕様に準拠していることを確認するために重要です。
