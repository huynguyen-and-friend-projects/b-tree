#include "b-tree.hxx"
#include <gtest/gtest.h>
#include <iostream>
#include <ranges>

namespace bt = my_b_tree;

// TEST(b_tree, obvi) { // NOLINT
//     bt::BTree<int, 2> test_tree{};
//     ASSERT_TRUE(test_tree.get_root()->is_leaf());
//     ASSERT_EQ(test_tree.get_root()->key_count(), 0);
//     ASSERT_EQ(test_tree.get_root()->children_count(), 0);
//     ASSERT_FALSE(test_tree.find(69).has_value());
// }

TEST(b_tree, insert) { // NOLINT
    bt::BTree<int, 2> test_tree{};
    // PASSED
    test_tree.insert(4);
    ASSERT_TRUE(test_tree.find(4).has_value());
    test_tree.insert(5); // NOLINT
    ASSERT_TRUE(test_tree.find(5).has_value());
    test_tree.insert(6); // NOLINT
    ASSERT_TRUE(test_tree.find(6).has_value());
    test_tree.insert(3); // NOLINT
    ASSERT_TRUE(test_tree.find(3).has_value());
    test_tree.insert(7); // NOLINT
    ASSERT_TRUE(test_tree.find(7).has_value());
    test_tree.insert(8); // NOLINT
    ASSERT_TRUE(test_tree.find(8).has_value());
    test_tree.insert(1); // NOLINT
    ASSERT_TRUE(test_tree.find(1).has_value());

    test_tree.insert(12); // NOLINT
    ASSERT_TRUE(test_tree.find(12).has_value());
    test_tree.insert(16); // NOLINT
    ASSERT_TRUE(test_tree.find(16).has_value());
    test_tree.insert(-3); // NOLINT
    ASSERT_TRUE(test_tree.find(-3).has_value());
    test_tree.insert(21); // NOLINT
    ASSERT_TRUE(test_tree.find(21).has_value());
    std::cout << "Finish first round of torture\n";

    // stress testing on the extreme positive
    for (const int num : std::views::iota(64, 129)) {
        test_tree.insert(num);
    }
    for (const int num : std::views::iota(64, 129)) {
        ASSERT_TRUE(test_tree.find(num).has_value());
    }
    // stress testing on the extreme negative
    for (const int num : std::views::iota(-128, -63)) {
        test_tree.insert(num);
    }
    for (const int num : std::views::iota(-128, -63)) {
        ASSERT_TRUE(test_tree.find(num).has_value());
    }
}
