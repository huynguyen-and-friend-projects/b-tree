// NOLINTBEGIN
#include "b-tree.hxx"
#include <concepts>
#include <gtest/gtest.h>
// #include <string>
// #include <type_traits>
// #include <utility>

namespace bt = my_b_tree;

TEST(b_tree, obvious) {
    ASSERT_TRUE(std::movable<int>);
    const bt::BTree<int, 1> test_tree{};

    ASSERT_FALSE(test_tree.find(2).has_value());
    ASSERT_FALSE(test_tree.contains(69420));
}

// insert and remove stress-testing is now moved to fuzztest
TEST(b_tree, insert) {
    bt::BTree<int, 1> test_tree{};
    test_tree.insert(69);
    test_tree.insert(420);
    test_tree.insert(666);
    // current tree
    //    [ 420 ]
    //
    // [ 69 ] [ 666 ]
    test_tree.insert(13);
    test_tree.insert(7);
    // current tree
    //    [ 13 420 ]
    //
    // [ 7 ] [ 69 ] [ 666 ]
    test_tree.insert(70);
    test_tree.insert(74);
    // current tree
    //          [ 70 ]
    //
    //      [ 13 ] [ 420 ]
    //
    // [ 7 ] [ 69 ] [ 74 ] [ 666 ]
    ASSERT_TRUE(test_tree.contains(69));
    ASSERT_TRUE(test_tree.contains(420));
    ASSERT_TRUE(test_tree.contains(666));
    ASSERT_TRUE(test_tree.contains(13));
    ASSERT_TRUE(test_tree.contains(7));
    ASSERT_TRUE(test_tree.contains(70));
    ASSERT_TRUE(test_tree.contains(74));
}

TEST(b_tree, leaf_remove) {
    bt::BTree<int, 2> test_tree{};
    for (int num = 1; num < 11; ++num) {
        test_tree.insert(num);
    }
    // current tree:
    //   [3         6]
    //
    // [1 2] [4 5]  [7 8 9 10]
    ASSERT_TRUE(test_tree.remove(1));
    ASSERT_FALSE(test_tree.find(1).has_value());
    // current tree:
    //  [   6       ]
    //
    // [2 3 4 5]    [7 8 9 10]
    ASSERT_TRUE(test_tree.remove(3));
    ASSERT_TRUE(test_tree.remove(7));
    ASSERT_TRUE(test_tree.remove(2));
    // current tree:
    //  [   6       ]
    //
    // [4 5]    [8 9 10]
    ASSERT_FALSE(test_tree.find(3).has_value());
    ASSERT_FALSE(test_tree.find(2).has_value());
    ASSERT_FALSE(test_tree.find(7).has_value());
    ASSERT_TRUE(test_tree.remove(4));
    // current tree:
    //  [   8     ]
    //
    // [5 6]    [9 10]
    ASSERT_TRUE(test_tree.remove(9));
    // current tree:
    // [5 6 8 10]
}

TEST(b_tree, nonleaf_remove) {
    bt::BTree<int, 2> test_tree{};
    for (int num = 1; num < 30; ++num) {
        test_tree.insert(num);
    }
    // place a breakpoint on this comment to see how the tree is at this stage
    ASSERT_TRUE(test_tree.remove(3));
    ASSERT_FALSE(test_tree.contains(3));
    ASSERT_TRUE(test_tree.remove(12));
    ASSERT_FALSE(test_tree.contains(12));

    ASSERT_TRUE(test_tree.remove(18));
    ASSERT_FALSE(test_tree.contains(18));
    ASSERT_TRUE(test_tree.remove(16));
    ASSERT_FALSE(test_tree.contains(16));

    ASSERT_TRUE(test_tree.remove(6));
    ASSERT_FALSE(test_tree.contains(6));

    ASSERT_TRUE(test_tree.remove(9));
    ASSERT_FALSE(test_tree.contains(9));

    ASSERT_TRUE(test_tree.remove(5));
    ASSERT_FALSE(test_tree.contains(5));
}

TEST(b_tree, copy) {
    bt::BTree<int, 4> test_tree{};
    for (int i = 0; i < 10; ++i) {
        test_tree.insert(i);
    }
    auto copy_test_tree{test_tree};
    for (int i = 0; i < 10; ++i) {
        ASSERT_TRUE(copy_test_tree.contains(i));
        ASSERT_TRUE(test_tree.contains(i));
    }
    copy_test_tree.insert(69);
    ASSERT_TRUE(copy_test_tree.find(69).has_value());
    ASSERT_FALSE(test_tree.find(69).has_value());
}

TEST(b_tree, move) {
    bt::BTree<int, 4> test_tree{};
    for (int i = 0; i < 10; ++i) {
        test_tree.insert(i);
    }
    auto move_test_tree{std::move(test_tree)};
    for (int i = 0; i < 10; ++i) {
        ASSERT_TRUE(move_test_tree.contains(i));
    }
}

TEST(b_tree, insert_nontrival_copy) {
    bt::BTree<std::string, 4> test_tree{};
    ASSERT_FALSE(std::is_trivially_copyable_v<std::string>);
    ASSERT_TRUE(std::is_trivially_copyable_v<const char*>);
    std::string sus = "Never gonna give you up";
    test_tree.insert_copy(sus);
    test_tree.insert("Never gonna let you down");
    ASSERT_TRUE(test_tree.contains("Never gonna give you up"));
    ASSERT_STREQ(sus.c_str(), "Never gonna give you up");
    ASSERT_FALSE(test_tree.insert(std::move(sus)));
    ASSERT_STREQ(sus.c_str(), "Never gonna give you up");

    std::string another_sus = "We know each other for so long";
    ASSERT_TRUE(test_tree.insert(std::move(another_sus)));
    ASSERT_STREQ(another_sus.c_str(), "");
}
// NOLINTEND
