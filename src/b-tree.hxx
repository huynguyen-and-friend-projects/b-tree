#ifndef B_TREE_PROTO_H
#define B_TREE_PROTO_H

#include <array>
#include <concepts>
#include <cstddef>
#include <functional>
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

    node(node&& to_move) = delete;
    auto operator=(node&& to_move) -> node<T>& = delete;

    node(const node& to_copy) = delete;
    auto operator=(const node& to_copy) -> node<T>& = delete;

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
    auto find(T val) -> std::optional<std::reference_wrapper<node<T>>> {
        auto ret = find_(*this, val);
        if (ret.has_value()) {
            return (ret.value());
        }
        return std::nullopt;
    }

  private:
    size_t n_keys = 0;
    std::array<T, MAX_KEYS> keys{};
    std::array<std::optional<std::shared_ptr<node<T>>>, MAX_CHILDREN> children{};

    static auto find_(std::reference_wrapper<node<T>> curr_ref,
                      T val) -> std::optional<std::reference_wrapper<node<T>>> {
        node<T> curr = curr_ref.get();
        size_t pos = 0;

        for (; pos < curr.n_keys && val > curr.keys[pos]; ++pos) {
        }

        if (pos < curr.n_keys && val == curr.keys[pos]) {
            return curr_ref;
        }

        if (curr.is_leaf() || !curr.children[pos].has_value()) {
            return std::nullopt;
        }

        return find_(*curr.children[pos].value().get(), val);
    }
};
} // namespace my_b_tree

#endif // !B_TREE_PROTO_H
