#include "b-tree.hxx"
#include <gtest/gtest.h>

namespace bt = my_b_tree;

TEST(b_tree, obvi) {
    bt::BTree<int, 2> test_tree{};
    ASSERT_TRUE(test_tree.get_root()->is_leaf());
    ASSERT_EQ(test_tree.get_root()->key_count(), 0);
    ASSERT_EQ(test_tree.get_root()->children_count(), 0);
    ASSERT_FALSE(test_tree.find_space_constrained(69).has_value());
    ASSERT_FALSE(test_tree.find(69).has_value());
}
