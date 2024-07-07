#include "b-tree.hxx"
#include <gtest/gtest.h>

namespace bt = my_b_tree;

static constexpr std::size_t TEST_TREE_MIN_DEG = 2;

TEST(b_tree, obvi) {
    bt::BTree<int, TEST_TREE_MIN_DEG> test_tree{};
    ASSERT_TRUE(test_tree.get_root().is_leaf());
    ASSERT_EQ(test_tree.get_root().key_count(), 0);
    ASSERT_EQ(test_tree.get_root().children_count(), 0);
    ASSERT_FALSE(test_tree.find(69).has_value());
    test_tree.find(420).has_value(); // NOLINT(*magic-numbers)
}
