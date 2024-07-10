/**
 * @file b-tree.hxx
 * @brief Declaration of public members/functions of b_tree
 */

#ifndef B_TREE_PROTO_H
#define B_TREE_PROTO_H

#include <array>
#include <climits>
#include <concepts>
#include <cstddef>
#include <memory>
#include <optional>
#include <utility>

namespace my_b_tree {

template <typename T>
concept BTreeTypenameConcept =
    std::equality_comparable<T> || std::is_move_assignable_v<T>;

// HACK: forward decl of BTree so that BTreeNode could use it

/**
 * @brief A B-tree.
 *
 * The maximum number of keys in each node of the B-tree is 2*min_deg,
 * and the maximum number of children nodes of each node is 2*min_deg + 1
 *
 */
template <BTreeTypenameConcept T, std::size_t min_deg> class BTree;

template <BTreeTypenameConcept T, std::size_t min_deg> class BTreeNode {
  private:
    /**
     * @brief The array of keys stored inside this instance of BTreeNode.
     *
     * There are always at least 1 more key than there are children.
     */
    std::array<T, (2 * min_deg) - 1> keys{0};
    /**
     * @brief The array of children pointers from this instance of BTreeNode.
     *
     * There are always at least 1 more key than there are children.
     * If the BTreeNode is leaf, there is no children.
     */
    std::array<std::unique_ptr<BTreeNode>, 2 * min_deg> children{0};

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
     * @brief Splits the current node into two, if it's full.
     *
     * If the current node is not full, this function simply does nothing.
     *
     * @param curr_bt The current BTree. This is passed in in case a new root is
     * created
     * @return true if the current node is split, false otherwise.
     */
    auto split(BTree<T, min_deg>* curr_bt) -> bool {
        if (!is_full()) {
            return false;
        }

        // root node
        if (!parent.has_value()) {
            std::unique_ptr<T> median = std::move(keys[n_keys / 2]);
            std::unique_ptr<BTreeNode> new_root =
                std::make_unique<BTreeNode>(nullptr, 0);
            this->parent = new_root.get();
            std::unique_ptr<BTreeNode> new_node =
                std::make_unique<BTreeNode>(this->parent, 1);

            // insert median into new root
            new_root->keys[0] = std::move(median);

            // move the keys and children larger than the median from the old
            // child to the new child
            std::size_t new_node_i = 0;
            for (std::size_t i = (n_keys / 2) + 1; i < n_keys; i++) {
                new_node->keys[new_node_i] = std::move(this->keys[i]);
                new_node->children[new_node_i] = std::move(this->children[i]);
                new_node->children[new_node_i]->index = new_node_i;
                ++new_node_i;
                --(this->n_keys);
            }

            curr_bt->root = std::move(new_root);

            return true;
        }

        parent.value()->split();
        std::unique_ptr<T> median = std::move(keys[n_keys / 2]);
        // assignment happens here because parent.value()->split() may change
        // this node's parent
        BTreeNode* curr_parent = parent.value();
        // at this point, curr_parent is guaranteed to have at least 1 spare
        // slot

        for (std::size_t i = curr_parent->n_keys; i > index; --i) {
            // move a child and the key just smaller than this child
            curr_parent->children[i + 1] = std::move(curr_parent->children[i]);
            curr_parent->keys[(i + 1) - 1] =
                std::move(curr_parent->keys[i - 1]);
            ++curr_parent->children[i + 1]->index;
        }
        // insert median to parent
        curr_parent->keys[index] = std::move(median);

        curr_parent->children[this->index + 1] =
            std::make_unique<BTreeNode>(curr_parent, this->index + 1);
        BTreeNode* new_node = curr_parent->children[this->index + 1].get();

        // move the keys and children larger than the median from the old child
        // to the new child
        std::size_t new_node_i = 0;
        for (std::size_t i = (n_keys / 2) + 1; i < n_keys; i++) {
            new_node->keys[new_node_i] = std::move(this->keys[i]);
            new_node->children[new_node_i] = std::move(this->children[i]);
            new_node->children[new_node_i]->index = new_node_i;
            ++new_node_i;
            --(this->n_keys);
        }

        return true;
    }

    friend class BTree<T, min_deg>;

  public:
    /* boilerplate */

    constexpr explicit BTreeNode(BTreeNode* parent, std::size_t idx)
        : index(idx) {
        if (parent == nullptr) {
            this->parent = std::nullopt;
        }
        this->parent = parent;
    };

    constexpr BTreeNode(BTreeNode&&) = default;
    constexpr BTreeNode(const BTreeNode&) = default;
    constexpr auto operator=(BTreeNode&&) -> BTreeNode& = default;
    constexpr auto operator=(const BTreeNode&) -> BTreeNode& = default;
    // HACK: C++20 and above only for constexpr destructor
    constexpr ~BTreeNode() = default;

    /**
     * @brief Returns the number of keys in this instance of BTreeNode
     *
     * @return The number of keys in this instance of BTreeNode
     */
    [[nodiscard]] auto constexpr key_count() const -> std::size_t {
        return n_keys;
    }
    /**
     * @brief Returns the number of children in this instance of BTreeNode
     *
     * @return The number of children in this instance of BTreeNode
     */
    [[nodiscard]] auto constexpr children_count() const -> std::size_t {
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
        return find(key).has_value();
    }

    /**
     * @brief Finds an instance of BTreeNode that is either this instance or a
     * descendant that contains the value specified.
     *
     * @param key
     * @return the pointer to the BTreeNode containing the key
     */
    [[nodiscard]] auto constexpr find(T key) const
        -> std::optional<std::pair<const BTreeNode*, std::size_t>> {
        long long int pos = n_keys / 2;
        long long int lptr = 0;
        long long int rptr = n_keys - 1;
        while (lptr <= rptr) {
            if (key == keys[pos]) {
                // HACK: forces make_pair to use the copy version
                return std::make_pair<const BTreeNode*, std::size_t>(
                    this, static_cast<std::size_t>(pos));
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
        return children[pos]->find(key);
    }

    /**
     * @brief Finds an instance of BTreeNode that is either this instance or a
     * descendant that contains the value specified.
     *
     * WARNING: only use this if you really, really value 96 bits, otherwise,
     * the normal "find" is faster
     *
     * @param val
     * @return the pointer to the BTreeNode containing the key
     */
    [[nodiscard]] auto constexpr find_space_constrained(T key) const
        -> std::optional<std::pair<const BTreeNode*, std::size_t>> {
        std::size_t pos = n_keys / 2;
        std::size_t lptr = 0;
        std::size_t rptr = n_keys - 1;

        // binary search with a flavor of trauma
        while (lptr <= rptr) {
            if (key == keys[pos]) {
                // HACK: forces make_pair to use the copy version
                return std::make_pair<const BTreeNode*, std::size_t>(
                    this,
                    (std::size_t)pos); // NOLINT
            }
            if (key > keys[pos]) {
                if (lptr == ULONG_MAX) [[unlikely]] {
                    // INFO: at this point, pos = ULONG_MAX
                    break;
                }
                lptr = pos + 1;
            } else {
                if (pos == 0) [[unlikely]] {
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
        return children[pos].get()->find_space_constrained(key);
    }
};

template <BTreeTypenameConcept T, std::size_t min_deg> class BTree {
  private:
    /**
     * @brief This instance of BTree's root. At the beginning, the root has no
     * key
     */
    std::unique_ptr<BTreeNode<T, min_deg>> root =
        std::make_unique<BTreeNode<T, min_deg>>(nullptr, 0);

    friend class BTreeNode<T, min_deg>;

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
        return root.get();
    }

    [[nodiscard]] auto constexpr find(T val) const
        -> std::optional<std::pair<const BTreeNode<T, min_deg>*, std::size_t>> {
        return root->find(val);
    }

    [[nodiscard]] auto constexpr find_space_constrained(T val) const
        -> std::optional<std::pair<const BTreeNode<T, min_deg>*, std::size_t>> {
        return root->find_space_constrained(val);
    }
};

} // namespace my_b_tree

#endif //! B_TREE_PROTO_H
