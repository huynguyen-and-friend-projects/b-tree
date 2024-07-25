#include "b-tree.hxx"
#include <cstddef>
#include <cstdint>
#include <exception>
#include <mutex>
#include <vector>

// NOLINTBEGIN
namespace bt = my_b_tree;

static std::mutex guard;

static bt::BTree<int, 4> test_tree{};

static std::vector<int> data_vec{};

namespace {
void btree_remove() {
    for(const auto& key : data_vec) {
        test_tree.remove(key);
    }
}

void btree_insert(const uint8_t* data, std::size_t size) {
    const std::lock_guard<std::mutex> lock(guard);
    for (std::size_t offset = 0; offset < size; offset += sizeof(int)) {
        if (offset + 4 >= size - 1) {
            btree_remove();
            return;
        }
        // HACK: int assumed to be 4 bytes here
        const int key =
            static_cast<int>((data[offset] << 24) | (data[offset + 1] << 16) |
                             (data[offset + 2] << 8) | (data[offset + 3]));
        if (!test_tree.contains(key)) {
            data_vec.push_back(key);
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
    return 0;
}
// NOLINTEND
