# ft_irc_test

このリポジトリは [ft_irc](https://github.com/kamitsui/ft_irc) プロジェクトのテストケースが入っています。

## 各テストコードの説明

* `unit/` : googletest を使ったクラス単体テスト
* `integration/` : シェルスクリプトによる統合テスト
* `performance/` : シェルスクリプトによる性能テスト（仮）
* `example_cpp09/` : 参考例として、実際に動作するテストコードがあります。

## 使い方

`ft_irc` のディレクトリから `make test` を実行すると、自動的にこのテストリポジトリがクローンされ、各テストが実行されます。

## 動作環境

* 校舎PCの`goinfre`に`googletest`をインストールする必要があります。
> `goinfre` に `brew` をインストールする方法 ... mfunyuさんの [config](https://github.com/mfunyu/config) を参考
>
> `googletest` インストール方法 ... `brew install googletest` （ **５分程度時間がかかります。** ）

* 個人PCで `googletest` を動かす場合、以下のコマンドで`.env`ファイルを作成してください。
> ```sh
> cp env_example .env
> ```
> 
> `.env` ファイルで `googletest` のインストール先を指定できます。
> ```
> # Google Test Path
> # My environ (Ubuntu22.04) : Define this variable in .env file.
> CXXFLAGS = -Wall -Wextra -Werror -std=c++14 -I/usr/include -pthread
> LIBS = -L/usr/lib -lgtest -lgtest_main
> ```
