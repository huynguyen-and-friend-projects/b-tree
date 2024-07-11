#include "b-tree.hxx"
#include <gtest/gtest.h>
#include <ranges>

namespace bt = my_b_tree;

TEST(b_tree, obvious) {
    const bt::BTree<int, 1> test_tree{};
    const bt::BTreeNode<int, 1>* root = test_tree.get_root();

    ASSERT_EQ(root->children_count(), 0);
    ASSERT_EQ(root->max_keys(), 2);
    ASSERT_EQ(root->max_children(), 3);

    ASSERT_FALSE(test_tree.find(2).has_value());
    ASSERT_FALSE(test_tree.contains(69420));
}

TEST(b_tree, insert_easy_mode) {
    bt::BTree<int, 1> test_tree{};
    test_tree.insert(69); // NOLINT
    ASSERT_TRUE(test_tree.contains(69));
}

TEST(b_tree, insert_medium_mode) {
    bt::BTree<int, 1> test_tree{};
    test_tree.insert(69); // NOLINT
    ASSERT_TRUE(test_tree.contains(69));
    test_tree.insert(42); // NOLINT
    ASSERT_TRUE(test_tree.contains(42));
    test_tree.insert(13); // NOLINT
    ASSERT_TRUE(test_tree.contains(13));
    test_tree.insert(77); // NOLINT
    ASSERT_TRUE(test_tree.contains(77));
    test_tree.insert(420); // NOLINT
    ASSERT_TRUE(test_tree.contains(420));
}

TEST(b_tree, insert_hard_mode) {
    bt::BTree<int, 69> big_test_tree{}; // NOLINT
    for (const int num : std::views::iota(-512, 513)) {
        big_test_tree.insert(static_cast<int>(num));
    }
    for (const int num : std::views::iota(-512, 513)) {
        ASSERT_TRUE(big_test_tree.contains(num));
    }
}
