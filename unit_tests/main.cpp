#include "gtest/gtest.h"

// 個々のテストファイルはGTestのフレームワークに登録されるため、
// ここで明示的にincludeする必要はありません。
// コンパイル時に全てのテストソースファイルを含めることで、
// GTestがそれらを自動的に発見します。

// Server.o がリンク時に参照するために g_running の実体を定義する。
// ユニットテストでは mainLoop() を実行しないため、
// この値が false になることは想定しない（シグナルハンドラも不要）。
volatile bool g_running = true;
// void signalHandler(int signum) {
//     (void)signum;
//     g_running = false;
// }

int main(int argc, char **argv) {
    // Initializes Google Test. This must be called before RUN_ALL_TESTS().
    ::testing::InitGoogleTest(&argc, argv);
    // Runs all registered tests.
    return RUN_ALL_TESTS();
}
