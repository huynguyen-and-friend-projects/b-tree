#include "b-tree.hxx"
#include <cstddef>
#include <cstdint>
#include <exception>
#include <mutex>

// NOLINTBEGIN
namespace bt = my_b_tree;

static std::mutex guard;

static bt::BTree<int, 4> test_tree{};

namespace {
void btree_remove(const uint8_t* data, std::size_t size) {
    const std::lock_guard<std::mutex> lock(guard);
    for (std::size_t offset = 0; offset < size; offset += sizeof(int)) {
        if (offset + 4 >= size - 1) {
            return;
        }
        // HACK: int assumed to be 4 bytes here
        const int key =
            static_cast<int>((data[offset] << 24) | (data[offset + 1] << 16) |
                             (data[offset + 2] << 8) | (data[offset + 3]));
        test_tree.remove(key);
        if(test_tree.contains(key)){
            std::terminate();
        }
    }
}

void btree_insert(const uint8_t* data, std::size_t size) {
    const std::lock_guard<std::mutex> lock(guard);
    for (std::size_t offset = 0; offset < size; offset += sizeof(int)) {
        if (offset + 4 >= size - 1) {
            return;
        }
        // HACK: int assumed to be 4 bytes here
        const int key =
            static_cast<int>((data[offset] << 24) | (data[offset + 1] << 16) |
                             (data[offset + 2] << 8) | (data[offset + 3]));
        if (!test_tree.contains(key)) {
            test_tree.insert(key);
        }
        if (!test_tree.contains(key)) {
            std::terminate();
        }
    }
}

} // namespace

extern "C" auto LLVMFuzzerTestOneInput(const uint8_t* data,
                                       std::size_t size) -> int {
    btree_insert(data, size);
    btree_remove(data, size);
    return 0;
}
// NOLINTEND
