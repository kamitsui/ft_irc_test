#include "gtest/gtest.h"

// 個々のテストファイルはGTestのフレームワークに登録されるため、
// ここで明示的にincludeする必要はありません。
// コンパイル時に全てのテストソースファイルを含めることで、
// GTestがそれらを自動的に発見します。

int main(int argc, char **argv) {
    // Initializes Google Test. This must be called before RUN_ALL_TESTS().
    ::testing::InitGoogleTest(&argc, argv);
    // Runs all registered tests.
    return RUN_ALL_TESTS();
}
