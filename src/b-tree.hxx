/**
 * @file b-tree.hxx
 * @brief Declaration of public members/functions of b_tree
 */

#ifndef B_TREE_PROTO_H
#define B_TREE_PROTO_H

#include <array>
#include <concepts>
#include <cstddef>
#include <memory>
#include <optional>
#include <climits>

namespace my_b_tree {

template <typename T>
concept EqComparable = std::equality_comparable<T>;

// HACK: forward decl of BTree so that BTreeNode could use it

/**
 * @brief
 *
 */
template <EqComparable T, std::size_t min_deg> class BTree;

template <EqComparable T, std::size_t min_deg> class BTreeNode {
  private:
    /**
     * @brief The array of keys stored inside this instance of BTreeNode.
     *
     * There are always at least 1 more key than there are children.
     */
    std::array<T, 2 * min_deg> keys{0};
    /**
     * @brief The array of children pointers from this instance of BTreeNode.
     *
     * There are always at least 1 more key than there are children.
     * If the BTreeNode is leaf, there is no children.
     */
    std::array<std::unique_ptr<BTreeNode>, 2 * min_deg - 1> children{0};

    /**
     * @brief Parent of this instance of BTreeNode. If this instance is the tree
     * root, parent is nullptr parent is nullptr (or in this case,
     * !parent.has_value()).
     *
     */
    std::optional<BTreeNode*> parent;
    /**
     * @brief Number of keys currently stored by this instance of BTreeNode
     */
    std::size_t n_keys{0};
    /**
     * @brief The index of this instance of BTreeNode's pointer stored inside
     * its parent's children pointer array.
     */
    std::size_t index;
    /**
     * @brief Number of children currently stored by this instance of BTreeNode
     */
    bool leaf{true};

    /**
     * @brief Returns whether this instance of BTreeNode contains the specified
     * key.
     *
     * This is basically find(key).has_value()
     *
     * @param key
     * @return Whether this instance of BTreeNode contains the specified key.
     */
    [[nodiscard]] constexpr auto contains(T key) const -> bool {
        return find_safe(key).has_value();
    }

    /**
     * @brief Splits the current node into two, if it's full.
     *
     * If the current node is not full, this function simply does nothing.
     *
     * @return true if the current node is split, false otherwise.
     */
    auto split() -> bool {
        if (n_keys < keys.size()) {
            return false;
        }

        if (!parent.has_value()) {
            // TODO: finish the split method for root node
            return true;
        }
        return true;
    }

    friend class BTree<T, min_deg>;

  public:
    /* boilerplate */

    explicit BTreeNode(BTreeNode* parent, std::size_t idx) : index(idx) {
        if (parent == nullptr) {
            this->parent = std::nullopt;
        }
        this->parent = parent;
    };

    BTreeNode(BTreeNode&&) = default;
    BTreeNode(const BTreeNode&) = default;
    auto operator=(BTreeNode&&) -> BTreeNode& = default;
    auto operator=(const BTreeNode&) -> BTreeNode& = default;
    ~BTreeNode() = default;

    /**
     * @brief Returns the number of keys in this instance of BTreeNode
     *
     * @return The number of keys in this instance of BTreeNode
     */
    [[nodiscard]] auto key_count() const -> std::size_t { return n_keys; }
    /**
     * @brief Returns the number of children in this instance of BTreeNode
     *
     * @return The number of children in this instance of BTreeNode
     */
    [[nodiscard]] auto children_count() const -> std::size_t {
        if (leaf) {
            return 0;
        }
        return n_keys - 1;
    }
    /**
     * @brief Returns whether this instance of BTreeNode is a leaf node
     *
     * @return Whether this instance of BTreeNode is a leaf node
     */
    [[nodiscard]] auto is_leaf() const -> bool { return leaf; }

    /**
     * @brief Returns whether this instance of BTreeNode is already full
     *
     * @return Whether this instance of BTreeNode is already full
     */
    [[nodiscard]] auto is_full() const -> bool { return n_keys == keys.size(); }

    [[nodiscard]] auto constexpr find(T key) const
        -> std::optional<const BTreeNode*> {
        std::size_t pos = n_keys / 2;
        std::size_t lptr = 0;
        std::size_t rptr = n_keys;

        // binary search with a flavor of trauma
        while (lptr <= rptr) {
            if (key == keys[pos]) {
                return this; // NOLINT
            }
            if (key > keys[pos]) {
                lptr = pos + 1;
            } else {
                rptr = pos - 1;
            }
            pos = (lptr + rptr) / 2;
        }
        if (leaf) {
            return std::nullopt;
        }
        return children[pos].get()->find(key);
    }

    /**
     * @brief Finds an instance of BTreeNode that is either this instance or a
     * descendant that contains the value specified.
     *
     * WARNING: This find method should only be used if the BTree's index is
     * very large (about at least half of ULONG_MAX). Otherwise, use the
     * "normal" find method, since this method has some branching overhead.
     *
     * @param val
     * @return the pointer to the BTreeNode containing
     */
    [[nodiscard]] auto constexpr find_safe(T key) const
        -> std::optional<const BTreeNode*> {
        std::size_t pos = n_keys / 2;
        std::size_t lptr = 0;
        std::size_t rptr = n_keys;

        // binary search with a flavor of trauma
        while (lptr <= rptr) {
            if (key == keys[pos]) {
                return this; // NOLINT
            }
            if (key > keys[pos]) {
                if (lptr == ULONG_MAX) [[unlikely]] {
                    // INFO: at this point, pos = ULONG_MAX
                    break;
                }
                lptr = pos + 1;
            } else {
                if (rptr == 0) [[unlikely]] {
                    // INFO: at this point, pos = 0
                    break;
                }
                rptr = pos - 1;
            }
            // INFO: this is to prevent the extreme case of lptr and rptr being
            // too big that they overflow size_t.
            // HACK: this could be a left bit-shift, which technically can be
            // faster. But I find division more intent-clear.
            pos = lptr / 2 + rptr / 2;
        }
        if (leaf) {
            return std::nullopt;
        }
        return children[pos].get()->find_safe(key);
    }
};

template <EqComparable T, std::size_t min_deg> class BTree {
  private:
    /**
     * @brief This instance of BTree's root. At the beginning, the root has no
     * key
     */
    BTreeNode<T, min_deg> root{nullptr, 0};

  public:
    constexpr BTree() = default;
    constexpr BTree(BTree&&) = default;
    constexpr BTree(const BTree&) = default;
    constexpr auto operator=(BTree&&) -> BTree& = default;
    constexpr auto operator=(const BTree&) -> BTree& = default;
    constexpr ~BTree() = default;

    /**
     * @return A reference to the root node of this instance of BTree
     */
    [[nodiscard]] auto constexpr get_root() -> const BTreeNode<T, min_deg>* {
        return &root;
    }

    [[nodiscard]] auto constexpr find(T val) const
        -> std::optional<const BTreeNode<T, min_deg>*> {
        return root.find(val);
    }

    [[nodiscard]] auto constexpr find_safe(T val) const
        -> std::optional<const BTreeNode<T, min_deg>*> {
        return root.find_safe(val);
    }
};

} // namespace my_b_tree

#endif //! B_TREE_PROTO_H
