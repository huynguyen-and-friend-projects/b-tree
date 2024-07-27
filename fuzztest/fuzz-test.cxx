#include "b-tree.hxx"
#include <cstddef>
#include <cstdint>
#include <exception>
#include <iterator>
#include <vector>

// NOLINTBEGIN
namespace bt = my_b_tree;

static bt::BTree<int, 4> test_tree{};

static std::vector<int> data_vec{};

namespace {
void btree_remove() {
    for (const int key : data_vec) {
        test_tree.remove(key);
    }
    data_vec.clear();
}

void btree_insert(const uint8_t* data, std::size_t size) {
    for (std::size_t offset = 0; offset < size; ++offset) {
        // HACK: int assumed to be 4 bytes here
        const int key =
            static_cast<int>(*std::next(data, static_cast<long>(offset)));
        data_vec.push_back(key);
        test_tree.insert(key);
        if (!test_tree.contains(key)) {
            std::terminate();
        }
    }
    btree_remove();
}

} // namespace

extern "C" auto LLVMFuzzerTestOneInput(const uint8_t* data,
                                       std::size_t size) -> int {
    btree_insert(data, size);
    return 0;
}
// NOLINTEND
