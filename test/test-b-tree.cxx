#include "b-tree.hxx"
#include <concepts>
#include <gtest/gtest.h>
#include <string>
#include <type_traits>
#include <utility>

namespace bt = my_b_tree;

TEST(b_tree, obvious) {
    ASSERT_TRUE(std::movable<int>);
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
    // NOTE: MIN_DEG so small runs pretty damn slow
    bt::BTree<int, 1> test_tree{};
    test_tree.insert(69); // NOLINT
    ASSERT_TRUE(test_tree.contains(69));
    test_tree.insert(13); // NOLINT
    ASSERT_TRUE(test_tree.contains(13));
    test_tree.insert(42); // NOLINT
    ASSERT_TRUE(test_tree.contains(42));
    test_tree.insert(77); // NOLINT
    ASSERT_TRUE(test_tree.contains(77));
    test_tree.insert(420); // NOLINT
    ASSERT_TRUE(test_tree.contains(420));

    // this shan't get inserted
    ASSERT_FALSE(test_tree.insert(77));
    ASSERT_TRUE(test_tree.contains(77));

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
}

TEST(b_tree, insert_hard_mode) { // NOLINT
    // nice
    bt::BTree<int, 69> test_tree{};      // NOLINT
    for (int i = -6666; i < 6666; ++i) { // NOLINT
        test_tree.insert(i);
    }
    for (int i = -6666; i < 6666; ++i) { // NOLINT
        ASSERT_TRUE(test_tree.contains(i));
    }
}

TEST(b_tree, leaf_remove) { // NOLINT
    bt::BTree<int, 2> test_tree{};
    for (int num = 1; num < 11; ++num) { // NOLINT
        test_tree.insert(num);
    }
    // current tree:
    //   [3         6]
    //    /    |     \
    // [1 2] [4 5]  [7 8 9 10]
    ASSERT_TRUE(test_tree.remove(1));
    ASSERT_FALSE(test_tree.find(1).has_value());
    // current tree:
    //  [   6       ]
    //  /           \
    // [2 3 4 5]    [7 8 9 10]
    ASSERT_TRUE(test_tree.remove(3));
    ASSERT_TRUE(test_tree.remove(7));
    ASSERT_TRUE(test_tree.remove(2));
    // current tree:
    //  [   6       ]
    //  /           \
    // [4 5]    [8 9 10]
    ASSERT_FALSE(test_tree.find(3).has_value());
    ASSERT_FALSE(test_tree.find(2).has_value());
    ASSERT_FALSE(test_tree.find(7).has_value());
    ASSERT_TRUE(test_tree.remove(4));
    // current tree:
    //  [   8     ]
    //  /         \
    // [5 6]    [9 10]
    ASSERT_TRUE(test_tree.remove(9));
    // current tree:
    // [5 6 8 10]
}

TEST(b_tree, nonleaf_remove) { // NOLINT
    bt::BTree<int, 2> test_tree{};
    for (int num = 1; num < 30; ++num) { // NOLINT
        test_tree.insert(num);
    }
    // place a breakpoint on this comment to see how the tree is at this stage
    ASSERT_TRUE(test_tree.remove(3));
    ASSERT_FALSE(test_tree.contains(3));
    ASSERT_TRUE(test_tree.remove(12));
    ASSERT_FALSE(test_tree.contains(12));
    ASSERT_TRUE(test_tree.remove(18));
    ASSERT_FALSE(test_tree.contains(18));
}

TEST(b_tree, copy) {
    bt::BTree<int, 4> test_tree{};
    for (int i = 0; i < 10; ++i) { // NOLINT
        test_tree.insert(i);
    }
    auto copy_test_tree{test_tree};
    for (int i = 0; i < 10; ++i) { // NOLINT
        ASSERT_TRUE(copy_test_tree.contains(i));
        ASSERT_TRUE(test_tree.contains(i));
    }
    copy_test_tree.insert(69); // NOLINT
    ASSERT_TRUE(copy_test_tree.find(69).has_value());
    ASSERT_FALSE(test_tree.find(69).has_value());
}

TEST(b_tree, move) {
    bt::BTree<int, 4> test_tree{};
    for (int i = 0; i < 10; ++i) { // NOLINT
        test_tree.insert(i);
    }
    auto move_test_tree{std::move(test_tree)};
    for (int i = 0; i < 10; ++i) { // NOLINT
        ASSERT_TRUE(move_test_tree.contains(i));
    }
}

TEST(b_tree, insert_nontrival_copy) {
    bt::BTree<std::string, 4> test_tree{};
    ASSERT_FALSE(std::is_trivially_copyable_v<std::string>);
    std::string sus = "Never gonna give you up";
    test_tree.insert_copy(sus); // NOLINT
    test_tree.insert("Never gonna let you down");
    ASSERT_TRUE(test_tree.contains("Never gonna give you up"));
    ASSERT_STREQ(sus.c_str(), "Never gonna give you up");
    ASSERT_FALSE(test_tree.insert(std::move(sus)));       // NOLINT
    ASSERT_STREQ(sus.c_str(), "Never gonna give you up"); // NOLINT

    std::string another_sus = "We know each other for so long";
    ASSERT_TRUE(test_tree.insert(std::move(another_sus))); // NOLINT
    ASSERT_STREQ(another_sus.c_str(), "");                 // NOLINT
}
