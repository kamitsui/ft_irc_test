# ft_irc_test

このリポジトリは ft_irc プロジェクトの中でクローンして使います。
```
ft_irc
├── Makefile
├── inc
├── src
└── test      # <- ここにクローンします。
```

```sh
git clone git@github.com:kamitsui/ft_irc_test.git test
```

## 各テストコードの説明

```sh
ft_irc_test/
│
├── scripts/                # IRCクライアント irssi を使ったテスト
│
├── unit_tests/             # 各クラスを単体でテストするコード
│   ├── test_parser.cpp
│   └── ...
│
├── integration_tests/          # 複数クラスを組み合わせてテストするコード
│   ├── conftest.py             # 各テスト実行前にサーバーを起動するスクリプト
│   ├── client_helper.py        # ソケット通信・IRCコマンドのためのヘルパークラス
│   ├── test_01_connection.py   # クライアント接続と登録の基本動作
│   ├── test_01_connection.py   # メッセージ送信
│   └── ...
│
└── performance_tests/      # 性能や耐久性をテストするコード （未検証）
    ├── irc_load_test.py
    └── ...
```

## 使い方

* `ft_irc` のMakefileにテストを呼び出す設置をします。（以下は例）
```
# REPOSITORY for Test
TEST_DIR = test
URL_TEST_REPO = "https://github.com/kamitsui/ft_irc_test.git"
UNIT_TEST_TARGET = $(TEST_DIR)/unit_tests/ft_irc_unittest

# Unit Test
unit_test:
ifeq ($(shell test -d $(TEST_DIR) && echo exist),)
	@echo "Please clone the test repository"
	$(call ASK_AND_EXECUTE_ON_YES, git clone $(URL_TEST_REPO) $(TEST_DIR))
endif
	$(MAKE) -C $(TEST_DIR)/unit_tests
.PHONY: unit_test

# Integration Test
test:
	@echo "Building server for testing..."
	$(MAKE) CXXFLAGS="$(CXXFLAGS) -DTEST_BUILD" re
    $(MAKE) -C $(TEST_DIR)/integration_tests
.PHONY: test

# Rule for removing object & dependency files
clean:
	rm -rf $(OBJ_DIR) $(DEP_DIR) $(TEST_DIR)/objs $(TEST_DIR)/.deps
.PHONY: clean

# Rule for removing Target & others
fclean: clean
	rm -f $(NAME) $(UNIT_TEST_TARGET)
.PHONY: fclean
```

* PING,PONGのタイムアウトをテスト時は短い時間でテストします。
> `integration_tests/test_04_ping_pong.py`では、２秒以内に応答がないとFAILEDになる判定です。
>
> そのため`make test`では、統合テストを実行する前に、`ircserv`ビルド時に`TEST_BUILD`変数を定義しています。
>
> 参考として、PING/PONGが定義されているコードに以下を追記すると、正しく判定ができるようになります。
>
> ```cpp
> // Define PING/PONG timeouts based on build type
> #ifdef TEST_BUILD
> #define PING_TIMEOUT 1 // Longer timeout for testing
> #define PONG_TIMEOUT 2
> #else
> #define PING_TIMEOUT 120 // Standard timeout for production
> #define PONG_TIMEOUT 20
> #endif
> ```

* `ft_irc` のディレクトリから `make test` を実行すると、自動的にこのテストリポジトリがクローンされ、各テストが実行されます。

## 動作環境

* 校舎PCの`goinfre`に`googletest`をインストールする必要があります。
> `goinfre` に `brew` をインストールする方法 ... mfunyuさんの [config](https://github.com/mfunyu/config) を参考
>
> `googletest` インストール方法 ... `brew install googletest` （ **５分程度時間がかかります。** ）

* 個人PCで `googletest` を動かす場合、以下のコマンドで`.env`ファイルを作成してください。
> ```sh
> cp env_example unit_tests/.env
> ```
> 
> `.env` ファイルで `googletest` のインストール先を指定できます。
> ```
> # Google Test Path
> # My environ (Ubuntu22.04) : Define this variable in .env file.
> CXXFLAGS = -Wall -Wextra -Werror -std=c++14 -I/usr/include -pthread
> LIBS = -L/usr/lib -lgtest -lgtest_main
> ```

* `scripts` のテストは `tmux` コマンドをインストールする必要があります。

## その他

* 詳しい情報については、[こちら(未完)]()のページを参照ください。
* まだまだ未完成な点がありますので、問題点・改善点などありましたら Issues またはプルリクエストのご協力いただけますと幸いです。
