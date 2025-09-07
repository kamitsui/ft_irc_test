#include "PmergeMe.hpp"
#include "gtest/gtest.h"
#include <algorithm> // for std::sort
#include <deque>
#include <vector>

// --- Test Fixture for PmergeMe ---
// テストごとに共通のセットアップを行うためのクラス
class PmergeMeTest : public ::testing::Test {
  protected:
    PmergeMe pmm; // 各テストでPmergeMeのインスタンスを使用
};

// --- Helper function to check if a container is sorted ---
template <typename Container> bool is_sorted(const Container &c) {
    for (size_t i = 0; i + 1 < c.size(); ++i) {
        if (c[i] > c[i + 1]) {
            return false;
        }
    }
    return true;
}

// --- Vector Tests ---

TEST_F(PmergeMeTest, VectorNormalCaseOdd) {
    std::vector<int> vec;
    vec.push_back(3);
    vec.push_back(5);
    vec.push_back(9);
    vec.push_back(7);
    vec.push_back(4);

    pmm.mergeInsertSort(vec);
    ASSERT_TRUE(is_sorted(vec));
}

TEST_F(PmergeMeTest, VectorNormalCaseEven) {
    std::vector<int> vec;
    vec.push_back(6);
    vec.push_back(5);
    vec.push_back(3);
    vec.push_back(1);
    vec.push_back(8);
    vec.push_back(7);

    pmm.mergeInsertSort(vec);
    ASSERT_TRUE(is_sorted(vec));
}

TEST_F(PmergeMeTest, VectorBoundaryCaseEmpty) {
    std::vector<int> vec;
    pmm.mergeInsertSort(vec);
    ASSERT_TRUE(is_sorted(vec));
    ASSERT_TRUE(vec.empty());
}

TEST_F(PmergeMeTest, VectorBoundaryCaseSingleElement) {
    std::vector<int> vec;
    vec.push_back(42);
    pmm.mergeInsertSort(vec);
    ASSERT_EQ(vec.size(), 1ul);
    ASSERT_EQ(vec[0], 42);
}

TEST_F(PmergeMeTest, VectorEdgeCaseAlreadySorted) {
    std::vector<int> vec;
    for (int i = 1; i <= 10; ++i)
        vec.push_back(i);

    pmm.mergeInsertSort(vec);
    ASSERT_TRUE(is_sorted(vec));
}

TEST_F(PmergeMeTest, VectorEdgeCaseReverseSorted) {
    std::vector<int> vec;
    for (int i = 10; i >= 1; --i)
        vec.push_back(i);

    pmm.mergeInsertSort(vec);
    ASSERT_TRUE(is_sorted(vec));
}

TEST_F(PmergeMeTest, VectorEdgeCaseWithDuplicates) {
    std::vector<int> vec;
    vec.push_back(5);
    vec.push_back(8);
    vec.push_back(2);
    vec.push_back(5);
    vec.push_back(2);

    std::vector<int> expected = vec;
    std::sort(expected.begin(), expected.end());

    // std::cout << "vec: ";
    // for (const auto &num : vec) {
    //     std::cout << num << " ";
    // }
    // std::cout << std::endl;

    // std::cout << "exp: ";
    // for (const auto &num : expected) {
    //     std::cout << num << " ";
    // }
    // std::cout << std::endl;

    pmm.mergeInsertSort(vec);
    ASSERT_EQ(vec, expected);
}

TEST_F(PmergeMeTest, VectorEdgeCaseAllSame) {
    std::vector<int> vec(10, 7); // 10個の7
    std::vector<int> expected = vec;

    pmm.mergeInsertSort(vec);
    ASSERT_EQ(vec, expected);
}

// --- Deque Tests ---

TEST_F(PmergeMeTest, DequeNormalCaseOdd) {
    std::deque<int> deq;
    deq.push_back(3);
    deq.push_back(5);
    deq.push_back(9);
    deq.push_back(7);
    deq.push_back(4);

    pmm.mergeInsertSort(deq);
    ASSERT_TRUE(is_sorted(deq));
}

TEST_F(PmergeMeTest, DequeNormalCaseEven) {
    std::deque<int> deq;
    deq.push_back(6);
    deq.push_back(5);
    deq.push_back(3);
    deq.push_back(1);
    deq.push_back(8);
    deq.push_back(7);

    pmm.mergeInsertSort(deq);
    ASSERT_TRUE(is_sorted(deq));
}

TEST_F(PmergeMeTest, DequeBoundaryCaseEmpty) {
    std::deque<int> deq;
    pmm.mergeInsertSort(deq);
    ASSERT_TRUE(is_sorted(deq));
    ASSERT_TRUE(deq.empty());
}

TEST_F(PmergeMeTest, DequeBoundaryCaseSingleElement) {
    std::deque<int> deq;
    deq.push_back(42);
    pmm.mergeInsertSort(deq);
    ASSERT_EQ(deq.size(), 1ul);
    ASSERT_EQ(deq[0], 42);
}

TEST_F(PmergeMeTest, DequeEdgeCaseAlreadySorted) {
    std::deque<int> deq;
    for (int i = 1; i <= 10; ++i)
        deq.push_back(i);

    pmm.mergeInsertSort(deq);
    ASSERT_TRUE(is_sorted(deq));
}

TEST_F(PmergeMeTest, DequeEdgeCaseReverseSorted) {
    std::deque<int> deq;
    for (int i = 10; i >= 1; --i)
        deq.push_back(i);

    pmm.mergeInsertSort(deq);
    ASSERT_TRUE(is_sorted(deq));
}
