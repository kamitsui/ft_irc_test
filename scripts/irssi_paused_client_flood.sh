#!/bin/bash

PORT="6667"

SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)

# エラーが発生した場合や未定義の変数を使用した場合にスクリプトを終了する
set -eu

# --- 設定項目 ---
# tmuxのセッション名
readonly SESSION_NAME="irc_test_session_4pane"
# ログファイル名
readonly LOG_FILE="tmux_test.log"
# ペイン番号
readonly LOG_PANE=0
readonly SERVER_PANE=1
readonly SENDER_PANE=2
readonly RECEIVER_PANE=3
# 各コマンド実行後の基本的な待機時間（秒）
readonly WAIT_SECONDS=2
# フラッドメッセージの数
readonly FLOOD_MESSAGE_COUNT=500
# 実行ファイル
if [ $# == 1 ] && [ "$1" == "docker" ]; then
  readonly TARGET="docker compose -f $SCRIPT_DIR/../docker/docker-compose.yml exec irc-server /app/ircserv"
  readonly OPTION="-nocap"
  readonly SERVER_ADDRESS="irc-server"
  readonly IRSSI="docker compose -f $SCRIPT_DIR/../docker/docker-compose.yml exec irssi-client irssi"
  shift # Remove 'docker' from arguments so the rest can be passed
else
  readonly TARGET="$SCRIPT_DIR/../../ircserv"
  readonly OPTION=""
  readonly SERVER_ADDRESS="localhost"
  readonly IRSSI='irssi'
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
  rm -f "$LOG_FILE"
  echo "INFO: クリーンアップが完了しました。"
}

# tmuxセッションを開始し、4つのペインを準備する関数
setup_tmux() {
  echo "INFO: tmuxセッションをセットアップします..."

  # --- 根本対策: 環境の完全リセット ---
  unset TMUX
  tmux kill-server 2>/dev/null || true

  echo "INFO: レイアウト設定を作成し、tmuxサーバーを起動します..."

  # 4つのペインを構成 (2x2グリッド)
  # Pane 0: Top-Left (Log)
  # Pane 1: Top-Right (Server)
  # Pane 2: Bottom-Left (Sender Client)
  # Pane 3: Bottom-Right (Receiver Client)

  tmux -f /dev/null new-session -d -s "$SESSION_NAME" -n "Test" -x 120 -y 40 \
    "tail -f $LOG_FILE" \;
    split-window -h "exec $TARGET $PORT $PASS" \;
    split-window -v -t "$SESSION_NAME:0.0" "exec $IRSSI" \;
    split-window -v -t "$SESSION_NAME:0.1" "exec $IRSSI"

  # ペインのサイズ調整（必要に応じて）
  # この分割方法では、デフォルトでほぼ均等になるはず

  # ペイン名を分かりやすく設定
  tmux rename-pane -t "$SESSION_NAME:0.$LOG_PANE" "Log"
  tmux rename-pane -t "$SESSION_NAME:0.$SERVER_PANE" "Server"
  tmux rename-pane -t "$SESSION_NAME:0.$SENDER_PANE" "Sender Client"
  tmux rename-pane -t "$SESSION_NAME:0.$RECEIVER_PANE" "Receiver Client"

  # レイアウトが安定するまで待機
  sleep 2

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

# サーバーを停止する関数
stop_server() {
  echo "--- サーバーを停止します ---"
  send_ctrl_char $SERVER_PANE "C-c"
  sleep 1 # サーバーが終了するのを待つ
  echo "--- サーバーが停止しました ---"
}

# メインのテストシナリオを定義する関数
run_test_scenario() {
  echo "##################################################"
  echo "INFO: テストシナリオを開始します..."
  echo "##################################################"

  # サーバーの起動 (setup_tmuxで既に起動済みなので待機のみ)
  echo "INFO: サーバー起動を待機中..."
  sleep 5 # サーバーが完全に起動するのを待つ

  # Senderクライアントの接続とチャンネル参加
  echo "--- Senderクライアントの操作 ---"
  exec_command $SENDER_PANE "/SERVER ADD $OPTION $SERVER_ADDRESS $PORT $PASS"
  exec_command $SENDER_PANE "/connect $SERVER_ADDRESS"
  exec_command $SENDER_PANE "/nick sender_client"
  exec_command $SENDER_PANE "/join #testchannel"
  sleep 2 # 参加メッセージが処理されるのを待つ

  # Receiverクライアントの接続とチャンネル参加
  echo "--- Receiverクライアントの操作 ---"
  exec_command $RECEIVER_PANE "/SERVER ADD $OPTION $SERVER_ADDRESS $PORT $PASS"
  exec_command $RECEIVER_PANE "/connect $SERVER_ADDRESS"
  exec_command $RECEIVER_PANE "/nick receiver_client"
  exec_command $RECEIVER_PANE "/join #testchannel"
  sleep 2 # 参加メッセージが処理されるのを待つ

  # Receiverクライアントを一時停止 (Ctrl+Z)
  echo "--- Receiverクライアントを一時停止します (Ctrl+Z) ---"
  send_ctrl_char $RECEIVER_PANE "C-z"
  sleep 2

  # Senderクライアントからフラッドメッセージを送信
  echo "--- Senderクライアントからフラッドメッセージを送信中... (${FLOOD_MESSAGE_COUNT}通) ---"
  for i in $(seq 1 $FLOOD_MESSAGE_COUNT); do
    exec_command $SENDER_PANE "/msg #testchannel :FLOOD_MSG_${i}" 0.01 # 短い待機でフラッド
  done
  echo "--- フラッドメッセージの送信が完了しました ---"
  sleep 5 # サーバーがメッセージを処理するのを待つ

  # Senderクライアントの操作性の確認 (QUIT)
  echo "--- Senderクライアントが正常にQUITできるか確認 ---"
  exec_command $SENDER_PANE "/quit :Sender Done"
  sleep 2

  # Receiverクライアントを再開 (fg)
  echo "--- Receiverクライアントを再開します (fg) ---"
  exec_command $RECEIVER_PANE "fg"
  sleep 10 # 蓄積されたメッセージが表示されるのを待つ

  echo "##################################################"
  echo "INFO: テストシナリオが完了しました。"
  echo "##################################################"
  echo ""
  echo "=================================================="
  echo "=== 検証結果を手動で確認してください ==="
  echo "=================================================="
  echo "1. Serverペインでサーバーがハングアップしていないことを確認。"
  echo "2. Receiver Clientペインで、一時停止中に送信された"
  echo "   ${FLOOD_MESSAGE_COUNT}個のメッセージがすべて表示され、"
  echo "   順序が正しいことを確認。"
  echo "3. Receiver Clientペインで、再開後に通常の操作(\"/names\", \"/who\"など)"
  echo "   が継続できることを確認。"
  echo "4. 必要に応じて、サーバープロセスのメモリ使用量を監視し、"
  echo "   メモリリークが発生していないことを確認してください。"
  echo "   (例: 別のターミナルで 'top -p <ircservのPID>' を実行)"
  echo ""
  echo "このtmuxセッションから抜けるには 'exit' と入力するか、"
  echo "tmuxのデタッチキー (Ctrl+b dなど) を押してください。"
  echo "スクリプトを終了すると、このセッションは破棄されます。"
}

# メイン処理
main() {
  # スクリプト終了時にcleanup関数を呼び出すように設定
  trap cleanup EXIT

  # ログファイルが存在しない場合は作成
  touch "$LOG_FILE"

  # 1. tmuxの準備
  setup_tmux

  # 2. メインのテストシナリオを実行し、その出力をログファイルにリダイレクト
  #    このスクリプト自体の `echo` 出力がログファイルに書き込まれるため、
  #    LOG_PANE で `tail -f` することで実行状況を確認できる
  run_test_scenario > "$LOG_FILE" 2>&1 &

  # 3. テストが実行されているセッションに、すぐにアタッチする
  echo "INFO: tmuxセッションにアタッチします。テストの実行をお楽しみください..."
  sleep 1 # アタッチ前の最終準備時間
  tmux attach-session -t "$SESSION_NAME"
}

# --- スクリプトの実行 ---
main