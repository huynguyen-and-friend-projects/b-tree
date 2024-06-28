#include "b-tree.hxx"
#include <gtest/gtest.h>

namespace bt = my_b_tree;
TEST(the_tree, contain) {
    bt::b_tree test_tree{3};
    ASSERT_FALSE(test_tree.contains(1));
    ASSERT_FALSE(test_tree.contains(-1));
    ASSERT_FALSE(test_tree.contains(0));
    ASSERT_FALSE(test_tree.contains(INT_MAX));
    ASSERT_FALSE(test_tree.contains(INT_MIN));
}
