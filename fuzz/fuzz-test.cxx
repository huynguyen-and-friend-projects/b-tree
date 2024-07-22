#include "b-tree.hxx"
#include <iterator>
#include <print>

namespace bt = my_b_tree;

static bt::BTree<int, 4> test_tree{}; // NOLINT

static size_t counter = 0; // NOLINT

static void btree_insert(const uint8_t* data, size_t size) {
    for (size_t offset = 0; offset < size; ++offset) {
        int key = static_cast<int>(*std::next(data, static_cast<long>(offset)));
        test_tree.insert(key);
    }
}

static void btree_remove(const uint8_t* data, size_t size) {
    for (size_t offset = 0; offset < size; ++offset) {
        test_tree.remove( // NOLINT
            static_cast<int>(*std::next(data, static_cast<long>(offset))));
    }
}

extern "C" auto LLVMFuzzerTestOneInput(const uint8_t* data,
                                       size_t size) -> int {
    if (counter % 5 == 1) { // NOLINT
        btree_remove(data, size);
    } else {
        btree_insert(data, size);
    }
    if (counter == ULONG_MAX) { // NOLINT
        return 0;
    }
    ++counter;
    return 0;
}
