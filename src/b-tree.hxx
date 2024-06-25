#ifndef B_TREE_PROTO_H
#define B_TREE_PROTO_H

#include <array>
#include <concepts>
#include <cstddef>
#include <memory>
#include <optional>

template <typename T>
concept Num =
    std::integral<T> || std::floating_point<T> || std::equality_comparable<T>;

namespace my_b_tree {
template <Num T> class node {
  public:
    static constexpr size_t MAX_KEYS = 5;
    static constexpr size_t MAX_CHILDREN = MAX_KEYS + 1;

    node() = default;

    node(node&& to_move) = default;
    auto operator=(node&& to_move) -> node<T>& = default;

    node(const node& to_copy) = default;
    auto operator=(const node& to_copy) -> node<T>& = default;

    ~node() = default;

    /**
     * @brief Whether this node has no children.
     *
     * @return true if this node has zero children, false otherwise
     */
    auto is_leaf() -> bool { return this->n_keys == 0; }

    /**
     * @brief Finds the node containing the specified value, and return that
     * node if found.
     *
     * @param val
     * @return 
     */
    auto find(T val) -> std::optional<node<T>*> {
        auto ret = find_(this, val);
        if (ret.has_value()) {
            return ret.value();
        }
        return std::nullopt;
    }

  private:
    size_t n_keys = 0;
    std::array<T, MAX_KEYS> keys{};
    std::array<std::shared_ptr<node<T>>, MAX_CHILDREN> children{};

    static auto find_(node<T>* curr, T val) -> std::optional<node*> {
        size_t pos = 0;

        for (; pos < curr->n_keys && val > curr->keys[pos]; ++pos) {
        }

        if (pos < curr->n_keys && val == curr->keys[pos]) {
            return curr;
        }

        if (curr->is_leaf()) {
            return std::nullopt;
        }

        return find_(curr->children[pos].get(), val);
    }
};
} // namespace my_b_tree

#endif // !B_TREE_PROTO_H
