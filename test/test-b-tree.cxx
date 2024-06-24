#include "b-tree.hxx"
#include <gtest/gtest.h>

TEST(my_suite, check_gtest) {
    std::cout << "This should compile\n";
    ASSERT_EQ(1, 1);
}

TEST(my_b_tree_test, check_is_leaf) {
    std::unique_ptr<my_b_tree::node<int>> test_tree_node =
        std::make_unique<my_b_tree::node<int>>();

    ASSERT_TRUE(test_tree_node->is_leaf());
}
