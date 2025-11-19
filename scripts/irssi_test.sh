#!/bin/bash

PORT="6667"

#SCRIPT_DIR=$(dirname "$0")
#echo "スクリプトのディレクトリ: ${SCRIPT_DIR}"
SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)
#echo "スクリプトの絶対パス: ${SCRIPT_DIR}" # デバッグ
#exit

# エラーが発生した場合や未定義の変数を使用した場合にスクリプトを終了する
set -eu

# --- 設定項目 ---
# tmuxのセッション名
readonly SESSION_NAME="irc_test_session_3pane"
# ログファイル名
readonly LOG_FILE="tmux_test.log"
# 一時的なレイアウト設定ファイル名
readonly LAYOUT_FILE="tmux_layout.conf"
# ペイン番号
readonly LOG_PANE=0
readonly SERVER_PANE=1
readonly CLIENT_PANE=2
# 各コマンド実行後の基本的な待機時間（秒）
readonly WAIT_SECONDS=2
# 実行ファイル
if [ $# == 1 ] && [ "$1" == "docker" ]; then
  TARGET="docker compose -f $SCRIPT_DIR/../docker/docker-compose.yml exec irc-server /app/ircserv"
  readonly SERVER_ADDRESS="irc-server"
  shift # Remove 'docker' from arguments so the rest can be passed
else
  readonly TARGET="$SCRIPT_DIR/../../ircserv"
  readonly SERVER_ADDRESS="localhost"
fi
# パスワード
readonly PASS="password"

# --- 関数定義 ---

# スクリプト終了時に実行されるクリーンアップ関数
cleanup() {
  echo "INFO: クリーンアップを実行します..."
  stop_server
  # tmuxサーバーごと終了させて、完全にクリーンな状態に戻す
  tmux kill-server 2>/dev/null || true
  # 一時ファイルを削除
  rm -f "$LOG_FILE" "$LAYOUT_FILE"
  echo "INFO: クリーンアップが完了しました。"
}

# tmuxセッションを開始し、3つのペインを準備する関数
setup_tmux() {
  echo "INFO: tmuxセッションをセットアップします..."

  # --- 根本対策: 環境の完全リセット ---
  unset TMUX
  tmux kill-server 2>/dev/null || true
  touch "$LOG_FILE"

  # --- 最終戦略: 設定ファイルからのレイアウト読み込み ---
  # 1. tmuxに実行させるコマンドを一時ファイルに書き出す
  echo "INFO: レイアウト設定ファイルを作成します..."
  cat > "$LAYOUT_FILE" << EOL
# Enable vim mode
set-window-option -g mode-keys vi

# Enable mouse support (for resizing panes and selecting panes with the mouse)
set -g mouse on

# Split
bind | split-window -h
bind - split-window -v

# Resize pane
bind H resize-pane -L 2
bind J resize-pane -D 2
bind K resize-pane -U 2
bind L resize-pane -R 2

# Rename window
bind R command-prompt "rename-window '%%'"

# Other
bind c new-window
bind , command-prompt "split-window -v '%%'"
bind . command-prompt "split-window -h '%%'"
bind d detach-client
bind x kill-session
# 新しいデタッチセッションを作成
new-session -d -s "$SESSION_NAME" -n "Test" -x 120 -y 30
# ペインを分割
split-window -v -l 70%
split-window -h -l 50%
# 最初のペイン(ログ)でコマンドを実行
send-keys -t "$SESSION_NAME:0.$LOG_PANE" "tail -f $LOG_FILE" C-m
EOL

  # 2. tmuxサーバーを起動し、設定ファイルを読み込ませる
  echo "INFO: tmuxサーバーを起動し、設定ファイルを読み込みます..."
  # -f /dev/null を指定して、ユーザー個人の設定ファイル(~/.tmux.conf)を無視させる
  #tmux -f /dev/null start-server
  tmux -f "$LAYOUT_FILE" start-server
  # サーバーがソケットを作成し、準備が整うのを待つ
  #sleep 0.5
  #tmux source-file "$LAYOUT_FILE"

  # レイアウトが安定するまで待機
  sleep 1

  echo "INFO: tmuxのセットアップが完了しました。"
}

# 指定したペインでコマンドを実行し、待機する関数
exec_command() {
  local pane_index=$1
  local command_to_run=$2
  local wait_time=${3:-$WAIT_SECONDS}

  echo "INFO: [ペイン$pane_index] コマンド実行: '$command_to_run'"
  tmux send-keys -t "$SESSION_NAME:0.$pane_index" "$command_to_run" C-m
  sleep "$wait_time"
}

# 指定したペインに制御文字（Ctrl+Cなど）を送信し、待機する関数
send_ctrl_char() {
  local pane_index=$1
  local ctrl_char=$2
  local wait_time=${3:-$WAIT_SECONDS}

  echo "INFO: [ペイン$pane_index] Ctrl+${ctrl_char#C-} を送信"
  tmux send-keys -t "$SESSION_NAME:0.$pane_index" "$ctrl_char"
  sleep "$wait_time"
}

# サーバー側の操作を行う関数
run_server_operations() {
  echo "--- サーバー側の操作を開始 ---"
  exec_command $SERVER_PANE "$TARGET $PORT $PASS" 5
  echo "--- サーバーが起動しました ---"
}

# クライアント側の操作を行う関数
run_client_operations() {
  echo "--- クライアント側の操作を開始 ---"

  # 1回目の接続
  exec_command $CLIENT_PANE "docker compose -f $SCRIPT_DIR/../docker/docker-compose.yml exec irssi-client irssi" 3
  exec_command $CLIENT_PANE "/SERVER ADD -nocap $SERVER_ADDRESS $PORT $PASS" 2
  exec_command $CLIENT_PANE "/connect $SERVER_ADDRESS" 2
  # pass not command
  exec_command $CLIENT_PANE "/pass $PASS" 2
  exec_command $CLIENT_PANE "/nick test_nick" 2
  exec_command $CLIENT_PANE "/user test_user" 2
  exec_command $CLIENT_PANE "/join #general" 2
  send_ctrl_char $CLIENT_PANE "C-d"
  exec_command $CLIENT_PANE 'echo $?'
  return
  #exit
  exec_command $CLIENT_PANE "USER guest 0 * :Guest User"
  exec_command $CLIENT_PANE "NICK guest"
  send_ctrl_char $CLIENT_PANE "C-d"

  # 2回目の接続
  exec_command $CLIENT_PANE "nc localhost $PORT" 3
  exec_command $CLIENT_PANE "Hello again, server!"
  send_ctrl_char $CLIENT_PANE "C-d"

  # 3回目の接続
  exec_command $CLIENT_PANE "nc localhost $PORT" 3
  exec_command $CLIENT_PANE "This is the final message."
  send_ctrl_char $CLIENT_PANE "C-c"
  exec_command $CLIENT_PANE 'echo $?'

  echo "--- クライアント側の操作が完了しました ---"
}

# サーバーを停止する関数
stop_server() {
  echo "--- サーバーを停止します ---"
  send_ctrl_char $SERVER_PANE "C-c"
  exec_command $SERVER_PANE 'echo $?'
  echo "--- サーバーが停止しました ---"
}

# メインのテストシナリオを定義する関数
# この関数はバックグラウンドで実行される
run_test_in_background() {
  # サーバーの起動を少し待つ
  sleep 1
  echo "##################################################"
  echo "INFO: テストシナリオを開始します..."
  echo "##################################################"

  run_server_operations
  run_client_operations
  stop_server

  echo "##################################################"
  echo "INFO: テストが完了しました。"
  echo "INFO: このセッションから抜けるには 'exit' と入力するか、"
  echo "INFO: tmuxのデタッチキー (Ctrl+b dなど) を押してください。"
  echo "INFO: スクリプトを終了すると、このセッションは破棄されます。"
  echo "##################################################"
}

# メイン処理
main() {
  # スクリプト終了時にcleanup関数を呼び出すように設定
  trap cleanup EXIT

  # 1. tmuxの準備
  setup_tmux

  # 2. メインの処理をバックグラウンドで実行し、出力をログファイルにリダイレクト
  #    `&` をつけることで、この処理の完了を待たずに次に進む
  run_test_in_background > "$LOG_FILE" 2>&1 &

  # 3. テストが実行されているセッションに、すぐにアタッチする
  echo "INFO: tmuxセッションにアタッチします。テストの実行をお楽しみください..."
  sleep 1 # アタッチ前の最終準備時間
  tmux attach-session -t "$SESSION_NAME"
}

# --- スクリプトの実行 ---
main
