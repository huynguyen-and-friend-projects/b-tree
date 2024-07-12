#include "b-tree.hxx"
#include <gtest/gtest.h>

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

TEST(b_tree, insert_medium_mode) { // NOLINT
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
    test_tree.insert(666);    // NOLINT
    test_tree.insert(69420);  // NOLINT
    test_tree.insert(12345);  // NOLINT
    test_tree.insert(-12345); // NOLINT
    test_tree.insert(-77);    // NOLINT
    test_tree.insert(-222);   // NOLINT
    test_tree.insert(-288);   // NOLINT
    test_tree.insert(-139);   // NOLINT
    test_tree.insert(-334);   // NOLINT
    test_tree.insert(-969);   // NOLINT
    ASSERT_TRUE(test_tree.contains(420));
    ASSERT_TRUE(test_tree.contains(666));
    ASSERT_TRUE(test_tree.contains(69420));
    ASSERT_TRUE(test_tree.contains(12345));
    ASSERT_TRUE(test_tree.contains(-12345));
    ASSERT_TRUE(test_tree.contains(-77));
    ASSERT_TRUE(test_tree.contains(-222));
    ASSERT_TRUE(test_tree.contains(-288));
    ASSERT_TRUE(test_tree.contains(-139));
    ASSERT_TRUE(test_tree.contains(-334));
    ASSERT_TRUE(test_tree.contains(-969));
    for (int i = -6666; i < -5555; ++i) {
        test_tree.insert(std::move(i));
        ASSERT_TRUE(test_tree.contains(i));
    }
}

TEST(b_tree, insert_hard_mode) { // NOLINT
    // nice
    bt::BTree<int, 69> test_tree{}; // NOLINT
    for (int i = -6666; i < 6666; ++i) {
        test_tree.insert(std::move(i));
        ASSERT_TRUE(test_tree.contains(i));
    }
    for (int i = -6666; i < 6666; ++i) { // NOLINT
        ASSERT_TRUE(test_tree.contains(i));
    }
}
