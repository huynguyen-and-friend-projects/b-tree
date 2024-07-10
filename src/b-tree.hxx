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
    std::array<T, (2 * min_deg) - 1> keys_{0};
    /**
     * @brief The array of children pointers from this instance of BTreeNode.
     *
     * There are always at least 1 more key than there are children.
     * If the BTreeNode is leaf, there is no children.
     */
    std::array<std::unique_ptr<BTreeNode>, 2 * min_deg> children_{0};

    /**
     * @brief Parent of this instance of BTreeNode. If this instance is the tree
     * root, parent is nullptr parent is nullptr (or in this case,
     * !parent.has_value()).
     *
     */
    BTreeNode* parent_;
    /**
     * @brief Number of keys currently stored by this instance of BTreeNode
     */
    std::size_t n_keys_{0};
    // TODO: add n_children as the way to count children
    std::size_t n_children_{0};
    /**
     * @brief The index of this instance of BTreeNode's pointer stored inside
     * its parent's children pointer array.
     */
    std::size_t index_;

    /**
     * @brief Splits the current node into two, if it's full.
     *
     * If the current node is not full, this function simply does nothing.
     *
     * @param curr_bt The current BTree. This is passed in in case a new root is
     * created
     * @return true if the current node is split, false otherwise.
     */
    constexpr auto split_(BTree<T, min_deg>& curr_bt) -> bool {
        if (!is_full()) {
            return false;
        }

        // root node
        if (parent_ == nullptr) {
            T median = std::move(this->keys_[n_keys_ / 2]);

            std::unique_ptr<BTreeNode> new_root =
                std::make_unique<BTreeNode>(nullptr, 0);
            // insert median into new root
            new_root->keys_[0] = std::move(median);
            ++new_root->n_keys_;
            // add this and a new node as the new root's children
            new_root->children_[0] = std::move(curr_bt.root);
            new_root->children_[1] =
                std::make_unique<BTreeNode>(new_root.get(), 1);
            new_root->n_children_ = 2;

            this->parent_ = new_root.get();
            BTreeNode* new_node = new_root->children_[1].get();

            // keys_[n_keys_ / 2] is now invalid, and so
            // children_[n_keys_ / 2 + 1] needs to move to the new node
            if (children_[(n_keys_ / 2) + 1] != nullptr) {
                new_node->children_[0] =
                    std::move(children_[(n_keys_ / 2) + 1]);
                new_node->children_[0]->index_ = 0;
                new_node->children_[0]->parent_ = new_node;
                ++new_node->n_children_;
                --this->n_children_;
            }
            --(this->n_keys_);

            // move the keys and child larger than the median from the old
            // child to the new child
            std::size_t new_node_i = 0;
            // HACK: n_keys_ + 1 because the supposed median slot is now empty
            std::size_t idx = (n_keys_ / 2) + 1;
            std::size_t max_i = n_keys_ + 1;
            for (; idx < max_i; idx++) {
                new_node->keys_[new_node_i] = std::move(this->keys_[idx]);
                --this->n_keys_;
                ++new_node_i;
                ++new_node->n_keys_;
                if (this->children_[idx + 1] == nullptr) {
                    continue;
                }
                new_node->children_[new_node->n_children_] =
                    std::move(this->children_[idx + 1]);
                new_node->children_[new_node->n_children_]->index_ =
                    new_node->n_children_;
                new_node->children_[new_node->n_children_]->parent_ = new_node;
                ++new_node->n_children_;
                --this->n_children_;
            }

            curr_bt.root = std::move(new_root);
            this->parent_ = curr_bt.root.get();
            new_node->parent_ = curr_bt.root.get();

            return true;
        }

        parent_->split_(curr_bt);
        T median = std::move(keys_[n_keys_ / 2]);

        // make room to insert median
        for (std::size_t i = parent_->n_keys_; i > index_; --i) {
            // move a child and the key just smaller than this child
            parent_->children_[i + 1] = std::move(parent_->children_[i]);
            parent_->keys_[(i + 1) - 1] = std::move(parent_->keys_[i - 1]);
            ++parent_->children_[i + 1]->index_;
        }
        // insert median to parent
        parent_->keys_[index_] = median;
        ++parent_->n_keys_;

        parent_->children_[this->index_ + 1] =
            std::make_unique<BTreeNode>(parent_, this->index_ + 1);
        ++parent_->n_children_;
        BTreeNode* new_node = parent_->children_[this->index_ + 1].get();

        // keys_[n_keys_ / 2] is now invalid, and so
        // children_[n_keys_ / 2 + 1] needs to move to the new node
        if (children_[(n_keys_ / 2) + 1] != nullptr) {
            new_node->children_[0] = std::move(children_[(n_keys_ / 2) + 1]);
            new_node->children_[0]->index_ = 0;
            new_node->children_[0]->parent_ = new_node;
            --this->n_children_;
            ++new_node->n_children_;
        }

        --this->n_keys_;

        // move the keys and children larger than the median from the old child
        // to the new child
        std::size_t new_node_i = 0;
        for (std::size_t idx = (n_keys_ / 2) + 1; idx < n_keys_ + 1; idx++) {
            new_node->keys_[new_node_i] = std::move(this->keys_[idx]);
            ++new_node_i;
            --(this->n_keys_);
            new_node->n_keys_++;

            if (this->children_[idx + 1] == nullptr) {
                continue;
            }
            new_node->children_[new_node->n_children_] =
                std::move(this->children_[idx + 1]);
            new_node->children_[new_node->n_children_]->index_ =
                new_node->n_children_;
            new_node->children_[new_node->n_children_]->parent_ = new_node;

            ++new_node->n_children_;
            --this->n_children_;
        }

        return true;
    }

    /**
     * @brief Inserts the specified value into this node. Must only be called
     * when this node is leaf
     *
     * @param curr_bt
     * @param val
     */
    constexpr auto insert_(BTree<T, min_deg>& curr_bt, T val) -> bool {
        if (this->split_(curr_bt)) {
            return curr_bt.insert(val);
        }
        // the current node is guaranteed to be non-full here

        // search for the spot
        long long int lptr = 0;
        long long int rptr = n_keys_ - 1;
        long long int mid = (lptr + rptr) / 2;

        while (lptr <= rptr) {
            if (keys_[mid] == val) {
                return false;
            }
            if (keys_[mid] < val) {
                lptr = mid + 1;
            } else {
                rptr = mid - 1;
            }
            mid = (lptr + rptr) / 2;
        }

        // here, the two pointers would be at somewhere like:
        // ... | < val   | > val    |       | ...
        //       rptr      lptr
        // and we want to insert right after the right pointer.

        // HACK: this is possible because right is, at minimum, -1
        const long long int insert_pos = rptr + 1;

        // make room for new element
        for (long long int i = n_keys_; i > insert_pos; --i) {
            keys_[i] = std::move(keys_[i - 1]);
        }
        // insert
        keys_[insert_pos] = std::move(val);
        ++n_keys_;
        return true;
    }

    friend class BTree<T, min_deg>;

  public:
    /* boilerplate */

    constexpr explicit BTreeNode(BTreeNode* parent, std::size_t idx)
        : parent_(parent), index_(idx){};

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
    [[nodiscard]] constexpr auto key_count() const -> std::size_t {
        return n_keys_;
    }
    /**
     * @brief Returns the number of children in this instance of BTreeNode
     *
     * @return The number of children in this instance of BTreeNode
     */
    [[nodiscard]] constexpr auto children_count() const -> std::size_t {
        return n_children_;
    }
    /**
     * @brief Returns whether this instance of BTreeNode is a leaf node
     *
     * @return Whether this instance of BTreeNode is a leaf node
     */
    [[nodiscard]] constexpr auto is_leaf() const -> bool {
        return n_children_ == 0;
    }

    /**
     * @brief Returns whether this instance of BTreeNode is already full
     *
     * @return Whether this instance of BTreeNode is already full
     */
    [[nodiscard]] constexpr auto is_full() const -> bool {
        return n_keys_ == keys_.size();
    }

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
    [[nodiscard]] constexpr auto find(T key) const
        -> std::optional<std::pair<const BTreeNode*, std::size_t>> {
        long long int lptr = 0;
        long long int rptr = (n_keys_ == 0) ? -1 : n_keys_ - 1;
        long long int pos = (lptr + rptr) / 2;
        while (lptr <= rptr) {
            if (key == keys_[pos]) {
                // HACK: forces make_pair to use the copy version
                return std::make_pair<const BTreeNode*, std::size_t>(
                    this, static_cast<std::size_t>(pos));
            }
            if (key > keys_[pos]) {
                lptr = pos + 1;
            } else {
                rptr = pos - 1;
            }
            pos = (lptr + rptr) / 2;
        }
        if (is_leaf()) {
            return std::nullopt;
        }
        return children_[rptr + 1]->find(key);
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
    [[nodiscard]] constexpr auto get_root() -> const BTreeNode<T, min_deg>* {
        return root.get();
    }

    [[nodiscard]] constexpr auto
    find(T val) const -> std::optional<const BTreeNode<T, min_deg>*> {
        auto ret = root->find(val);
        if (ret.has_value()) {
            return ret.value().first;
        }
        return std::nullopt;
    }

    constexpr auto insert(T val) -> bool {
        auto curr_node = root.get();
        long long int mid, lptr, rptr; // NOLINT
        while (!curr_node->is_leaf()) {
            lptr = 0;
            rptr = (curr_node->n_keys_ == 0) ? -1 : curr_node->n_keys_ - 1;
            mid = (lptr + rptr) / 2;

            while (lptr <= rptr) {
                if (curr_node->keys_[mid] == val) {
                    return false;
                }
                if (curr_node->keys_[mid] < val) {
                    lptr = mid + 1;
                } else {
                    rptr = mid - 1;
                }
                mid = (lptr + rptr) / 2;
            }

            curr_node = curr_node->children_[rptr + 1].get();
        }
        return curr_node->insert_(*this, val);
    }
};

} // namespace my_b_tree

#endif //! B_TREE_PROTO_H
