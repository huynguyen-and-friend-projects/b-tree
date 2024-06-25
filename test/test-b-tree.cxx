#include "b-tree.hxx"
#include <gtest/gtest.h>
#include <climits>

TEST(my_b_tree_test, check_is_leaf) {
    std::unique_ptr<my_b_tree::node<double>> test_tree_node =
        std::make_unique<my_b_tree::node<double>>();

    ASSERT_TRUE(test_tree_node->is_leaf());
}

TEST(my_b_tree_test, check_contains) {
    std::unique_ptr<my_b_tree::node<int>> test_tree_node =
        std::make_unique<my_b_tree::node<int>>();
    
    ASSERT_FALSE(test_tree_node->find(1));
    ASSERT_FALSE(test_tree_node->find(-1));
    ASSERT_FALSE(test_tree_node->find(0));
    ASSERT_FALSE(test_tree_node->find(INT_MAX));
    ASSERT_FALSE(test_tree_node->find(INT_MIN));
}
