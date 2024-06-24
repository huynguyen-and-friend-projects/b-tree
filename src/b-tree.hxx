#ifndef B_TREE_PROTO_H
#define B_TREE_PROTO_H

#include <array>
#include <concepts>
#include <cstddef>
#include <memory>
#include <optional>

template <typename T>
concept num = requires(T val) { std::equality_comparable<T>; };

namespace my_b_tree {
template <typename T> class node {
  public:
    static constexpr size_t MAX_KEYS = 5;
    static constexpr size_t MAX_CHILDREN = MAX_KEYS + 1;

    node<T>() = default;

    node<T>(node<T>&& to_move) = default;
    auto operator=(node<T>&& to_move) -> node<T>& = default;

    node<T>(const node<T>& to_copy) = default;
    auto operator=(const node<T>& to_copy) -> node<T>& = default;

    ~node<T>() = default;

    auto is_leaf() -> bool { return this->n_keys == 0; }

  private:
    size_t n_keys = 0;
    std::array<T, MAX_KEYS> keys{};
    std::array<std::shared_ptr<node<T>>, MAX_CHILDREN> children{};

    auto find_(node<T>* curr, T val) -> std::optional<std::weak_ptr<node<T>>> {
        int pos = 0;

        for(; pos < curr->n_keys && val > curr->keys[pos]; ++pos) {
        }

        if(pos < curr->n_keys && val == curr->keys[pos]){
            return curr;
        }

        if(curr->is_leaf()){
            return std::nullopt;
        }

        return find_(curr->children[pos], val);
    }
};
} // namespace my_b_tree

#endif // !B_TREE_PROTO_H
