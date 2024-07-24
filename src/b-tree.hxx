#ifndef B_TREE_PROTO_H
#define B_TREE_PROTO_H

#include <array>
#include <cassert>
#include <climits>
#include <concepts>
#include <cstddef>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>

namespace my_b_tree {

template <typename T>
concept Key = requires { std::equality_comparable<T>&& std::movable<T>; };

template <Key K, std::size_t MIN_DEG> class BTree;

template <Key K, std::size_t MIN_DEG> class BTreeNode {
  private:
    static_assert(MIN_DEG > 0, "Error, MIN_DEG must be larger than 0");
    // this assert is for bound safety when doing binary search.
    // You could say it's due to laziness
    static_assert(MIN_DEG < ULONG_MAX / 2 - 1,
                  "Error, MIN_DEG must be smaller than ULONG_MAX / 2 - 1");

    static constexpr bool CAN_TRIVIAL_COPY_ = std::is_trivially_copyable_v<K>;

    /**
     * @brief Same as BTree<K, MIN_DEG>::MAX_KEYS
     */
    static constexpr std::size_t MAX_KEYS_ = 2 * MIN_DEG;

    /**
     * @brief Same as BTree<K, MIN_DEG>::MAX_CHILDREN
     */
    static constexpr std::size_t MAX_CHILDREN_ = MAX_KEYS_ + 1;

    /**
     * @brief Keys array.
     */
    std::array<K, MAX_KEYS_ + 1> keys_{};

    /**
     * @brief Number of keys.
     */
    std::size_t n_keys_{0};

    /**
     * @brief Children array.
     */
    std::array<std::unique_ptr<BTreeNode>, MAX_CHILDREN_ + 1> children_{};

    /**
     * @brief Number of children.
     *
     * This variable is necessary because sometimes (eg, during the splitting of
     * a node), n_children_ != n_keys_ + 1.
     */
    std::size_t n_children_{0};

    /**
     * @brief Non-owning pointer to parent
     *
     * NOTE: parent_ == nullptr when node is root.
     */
    BTreeNode* parent_{nullptr};

    /**
     * @brief This node's position inside parent's children pointer array
     *
     * When this node is root (parent_ == nullptr), index_ = 0;
     */
    std::size_t index_{0};

    friend class BTree<K, MIN_DEG>;

    /**
     * @return Whether a split is needed
     */
    [[nodiscard]] auto need_split_() const noexcept -> bool {
        return n_keys_ > MAX_KEYS_;
    }

    /**
     * @return Whether this node has a left neighbour
     */
    [[nodiscard]] auto has_left_() const noexcept -> bool {
        return (!is_root() && index_ > 0);
    }

    /**
     * @return Whether this node has a right neighbour
     */
    [[nodiscard]] auto has_right_() const noexcept -> bool {
        return (!is_root() && index_ < parent_->n_children_ - 1);
    }

    /**
     * Only call when this node has left neighbour
     *
     * @return Non-owning reference to left neighbour
     */
    [[nodiscard]] auto get_left_() noexcept -> BTreeNode& {
        assert(has_left_());
        return *parent_->children_[index_ - 1].get();
    }

    /**
     * Only call when this node has right neighbour
     *
     * @return Non-owning reference to right neighbour
     */
    [[nodiscard]] auto get_right_() noexcept -> BTreeNode& {
        assert(has_right_());
        return *parent_->children_[index_ + 1].get();
    }

    /**
     * @brief Sets parent of this node to be the specified node.
     *
     * This is the preferred (albeit a little slower) way to change parent node.
     *
     * parent pointer must not be pointing to this (obviously)
     *
     * @param parent The specified parent node.
     * @param index The specified index.
     */
    void set_parent_(BTreeNode* parent, std::size_t index) noexcept {
        // in case you don't listen
        assert(parent != this);
        assert(index < MAX_CHILDREN_);

        index_ = index;
        parent_ = parent;
    }

    /**
     * @brief Finds the specified key inside this array.
     *
     * @param key The specified key
     * @return A pair:
     * - First value (bool) is whether the key was found
     * - Second value (long long):
     *   - If key is found, is the index of that key inside the array.
     *   - Else, is the index of the key just smaller than the specified key.
     *      - So, -1 if the key is smaller than every element and n_keys_ - 1
     * when the key is larger than every element.
     */
    [[nodiscard]] auto
    inner_key_find_(std::conditional_t<CAN_TRIVIAL_COPY_, K, const K&> key)
        const noexcept -> std::pair<bool, long long>;
    /**
     * @brief Finds the specified key inside this array and all children's
     * arrays
     *
     * @param key The specified key
     * @return An optional pair:
     * - std::nullopt is key not found.
     * - A pair if found:
     *   - First value is the reference to the node containing the specified
     * key.
     *   - Second value is the index of that key.
     */
    [[nodiscard]] auto
    find_(std::conditional_t<CAN_TRIVIAL_COPY_, K, const K&> key) const noexcept
        -> std::optional<std::pair<const BTreeNode&, std::size_t>>;

    /**
     * @brief (Simply) split the current node into 2, and insert the median into
     * the parent.
     *
     * Should only be called when this node is full.
     *
     * In case new_key is the median, new_key is swapped with the key that would
     * have been the median had new_key not been.
     *
     * In case a new root node is made, curr_bt->root_ is updated.
     *
     * @param curr_bt
     */
    void inner_split_(BTree<K, MIN_DEG>& curr_bt);
    /**
     * @brief Inserts the specified child at the specified index.
     *
     * This won't trigger a split when the number of children surpasses
     * the maximum number of children. As such, after this method, a check
     * whether to split is needed.
     *
     * Must satisfy the following conditions:
     * 1. This node's children array is not full
     * 2. The child passed in isn't a nullptr
     * 3. index < MAX_CHILDREN_ and index > 0
     *
     * @param child The specified child
     * @param index The specified index
     */
    void inner_insert_child_at_(std::unique_ptr<BTreeNode>&& child,
                                std::size_t index);
    /**
     * @brief Inserts the specified key to the specified position in this node's
     * inner array if this node's inner array is not yet full.
     *
     * Basically inner_insert_ but does not need to search for the index to
     * insert.
     *
     * In case of splitting:
     * - If the specified key is smaller than this node's median, insert it into
     * this node. Otherwise, insert into the new node.
     *
     * Must satisfy the following conditions:
     * 1. If this node isn't full, index < MAX_KEYS_
     * 2. If this node is full, index <= MAX_KEYS_
     * 3. index > 0
     *
     * @param key The specified key
     * @param index The specified index
     */
    void inner_insert_key_at_(BTree<K, MIN_DEG>& curr_bt,
                              std::conditional_t<CAN_TRIVIAL_COPY_, K, K&&> key,
                              std::size_t index);
    /**
     * @brief Search for the position inside this node's inner array to insert
     * the specified key, then insert it if this node is not yet full.
     * Otherwise, split this node, then insert the key.
     *
     * In case of splitting:
     * - If the specified key is smaller than this node's median, insert it into
     * this node. Otherwise, insert into the new node.
     *
     * @param key The specified key.
     */
    void inner_insert_(BTree<K, MIN_DEG>& curr_bt,
                       std::conditional_t<CAN_TRIVIAL_COPY_, K, K&&> key);

    /**
     * @brief Remove the specified key out of this leaf's key array
     *
     * In case the remove causes rebalancing after which the root node has 0
     * key, curr_bt's root node is changed.
     *
     * @param key The specified key
     * @return true if key exists (and is removed), false if key doesn't exist.
     */
    auto leaf_remove_(BTree<K, MIN_DEG>& curr_bt,
                      std::conditional_t<CAN_TRIVIAL_COPY_, K, const K&>
                          key) noexcept -> bool;

    /**
     * @brief Either borrow from left or right, or merge.
     *
     * Must only be called when n_keys_ <= MIN_DEG,
     * which also means this node should not be root.
     *
     * In case this node's parent is root and the root just reaches 0 key after
     * rebalancing (specifically, merging), the newly merged node is the new
     * root.
     */
    void leaf_rebalance_(BTree<K, MIN_DEG>& curr_bt) noexcept;

    /**
     * @brief Either borrow from left or right, or merge.
     *
     * Must only be called when n_keys_ <= MIN_DEG,
     * which also means this node should not be root.
     *
     * In case this node's parent is root and the root just reaches 0 key after
     * rebalancing (specifically, merging), the newly merged node is the new
     * root.
     */
    void nonleaf_rebalance_(BTree<K, MIN_DEG>& curr_bt) noexcept;

    /**
     * @brief Remove the specified key out of the inner array.
     *
     * @param key The specified key
     * @return true if the key exists (and is removed), false if the key doesn't
     */
    auto leaf_inner_remove_(std::conditional_t<CAN_TRIVIAL_COPY_, K, const K&>
                                key) noexcept -> bool;

    /**
     * @brief Remove the key at the specified index out of the inner array.
     *
     * index must be between 0 and n_keys_
     *
     * @param index The specified index
     * @return the removed key
     */
    auto leaf_inner_remove_at_(std::size_t index) noexcept
        -> std::conditional_t<CAN_TRIVIAL_COPY_, K, K&&>;

    /**
     * @brief Remove the key at the specified index out of the inner array.
     *
     * index must be between 0 and n_keys_
     *
     * The process is different than that of a leaf_inner_remove_at_.
     * This method finds the smallest element down the right subtree of the
     * to-be-removed key, then replace the removed key with that element, and
     * lastly return the removed key.
     *
     * @param index The specified index
     * @return the removed key
     */
    auto nonleaf_remove_at_(BTree<K, MIN_DEG>& curr_bt,
                            std::size_t index) noexcept
        -> std::conditional_t<CAN_TRIVIAL_COPY_, K, K&&>;

    /**
     * @brief Take the left neighbour's largest key as the new separator between
     * this and the left neighbour, and return the old separator.
     *
     * Must only be called when this is leaf and has left neighbour
     *
     * @return the old separator
     */
    [[nodiscard]] auto leaf_borrow_left_() noexcept
        -> std::conditional_t<CAN_TRIVIAL_COPY_, K, K&&>;

    /**
     * @brief Take the right neighbour's smallest key as the new separator
     * between this and the right neighbour, and return the old separator.
     *
     * Must only be called when this is leaf and has right neighbour
     *
     * @return the old separator
     */
    [[nodiscard]] auto leaf_borrow_right_() noexcept
        -> std::conditional_t<CAN_TRIVIAL_COPY_, K, K&&>;

    /**
     * @brief Take the left neighbour's largest key and the child larger than
     * that key, use the key as the new separator between this and the left
     * neighbour, then return the old separator and the child.
     *
     * @return A pair whose:
     * - First value is the old separator
     * - Second value is the owning pointer to the largest child of the left
     * node.
     */
    [[nodiscard]] auto nonleaf_borrow_left_() noexcept
        -> std::pair<std::conditional_t<CAN_TRIVIAL_COPY_, K, K&&>,
                     std::unique_ptr<BTreeNode>>;

    /**
     * @brief Take the right neighbour's smallest key and the child smaller than
     * that key, use the key as the new separator between this and the right
     * neighbour, then return the old separator and the child.
     *
     * @return A pair whose:
     * - First value is the old separator
     * - Second value is the owning pointer to the smallest child of the right
     * node.
     */
    [[nodiscard]] auto nonleaf_borrow_right_() noexcept
        -> std::pair<std::conditional_t<CAN_TRIVIAL_COPY_, K, K&&>,
                     std::unique_ptr<BTreeNode>>;
    /**
     * @brief Merge this node with its right neighbour
     *
     * Must only be called when this is leaf and both this and
     * its right neighbour cannot remove any more keys without going below the
     * minimum degree.
     */
    void leaf_merge_right_(BTree<K, MIN_DEG>& curr_bt) noexcept;

    /**
     * @brief Merge this node with its right neighbour
     *
     * Must only be called when this is NOT leaf and both this and
     * its right neighbour cannot remove any more keys without going below the
     * minimum degree.
     */
    void nonleaf_merge_right_(BTree<K, MIN_DEG>& curr_bt) noexcept;

  public:
    BTreeNode() noexcept = default;

    BTreeNode(BTreeNode&&) noexcept = default;

    /**
     * WARNING: VERY EXPENSIVE OPERATION.
     */
    BTreeNode(const BTreeNode& cpy);

    auto operator=(BTreeNode&&) noexcept -> BTreeNode& = default;

    /**
     * WARNING: VERY EXPENSIVE OPERATION.
     */
    auto operator=(const BTreeNode& cpy) -> BTreeNode&;

    ~BTreeNode() noexcept = default;

    /**
     * @return Whether this node has no child.
     */
    [[nodiscard]] auto is_leaf() const noexcept -> bool {
        return n_children_ == 0;
    }

    /**
     * @return Whether this node's key array is fully occupied.
     */
    [[nodiscard]] auto is_full() const noexcept -> bool {
        return n_keys_ == MAX_KEYS_;
    }

    /**
     * @return Whether this node is a BTree's root.
     */
    [[nodiscard]] auto is_root() const noexcept -> bool {
        return parent_ == nullptr;
    }
};

template <Key K, std::size_t MIN_DEG> class BTree {
  private:
    static_assert(MIN_DEG > 0, "Error, MIN_DEG must be larger than 0");
    static_assert(MIN_DEG < ULONG_MAX / 2,
                  "Error, MIN_DEG must be smaller than ULONG_MAX / 2");
    /**
     * @brief Root pointer
     */
    std::unique_ptr<BTreeNode<K, MIN_DEG>> root_{new BTreeNode<K, MIN_DEG>};

    static constexpr bool CAN_TRIVIAL_COPY_ = std::is_trivially_copyable_v<K>;

    friend class BTreeNode<K, MIN_DEG>;

  public:
    /**
     * @brief 2 * MIN_DEG
     */
    static constexpr std::size_t MAX_KEYS = 2 * MIN_DEG;

    /**
     * @brief 2 * MIN_DEG + 1
     */
    static constexpr std::size_t MAX_CHILDREN = MAX_KEYS + 1;

    BTree() noexcept = default;

    BTree(BTree&&) noexcept = default;

    /**
     * WARNING: VERY EXPENSIVE OPERATION.
     */
    BTree(const BTree& cpy) {
        // root_ always has value
        BTreeNode<K, MIN_DEG> copy_root = *cpy.root_.get();
        this->root_ =
            std::make_unique<BTreeNode<K, MIN_DEG>>(std::move(copy_root));
    }

    auto operator=(BTree&&) noexcept -> BTree& = default;

    /**
     * WARNING: VERY EXPENSIVE OPERATION.
     */
    auto operator=(const BTree& cpy) -> BTree& {
        if (&cpy == this) {
            return *this;
        }
        // root_ always has value
        assert(cpy.root_ != nullptr);
        BTreeNode<K, MIN_DEG> copy_root = *cpy.root_.get();
        this->root_ =
            std::make_unique<BTreeNode<K, MIN_DEG>>(std::move(copy_root));
    }

    ~BTree() noexcept = default;

    /**
     * @return The pointer to the root of this BTree.
     *
     * The root should not be manually modified.
     */
    [[nodiscard]] auto get_root() const -> const BTreeNode<K, MIN_DEG>* {
        return root_.get();
    }

    /**
     * @brief Finds the node containing the specified key.
     *
     * @param key
     * @return std::nullopt if no node contains the value, a pointer to the node
     * containing the value otherwise.
     */
    [[nodiscard]] auto
    find(std::conditional_t<std::is_trivially_copyable_v<K>, K, const K&> key)
        const noexcept
        -> std::optional<std::pair<const BTreeNode<K, MIN_DEG>&, std::size_t>> {
        auto pair_result = root_->find_(key);
        if (!pair_result.has_value()) {
            return std::nullopt;
        }
        return pair_result;
    }

    /**
     * @param key The specified key
     * @return Whether the BTree contains the specified key.
     */
    [[nodiscard]] auto contains(
        std::conditional_t<std::is_trivially_copyable_v<K>, K, const K&> key)
        const noexcept -> bool {
        return root_->find_(key).has_value();
    }

    /**
     * @brief Inserts the specified key into the BTree.
     *
     * If K is not trivially copyable, K is moved. Otherwise, K is copied.
     *
     * @param key The specified key.
     * @return true if the key is successfully inserted, false if there's
     * already a key of the same value inside.
     *
     * If K is nontrivially copyable and the function returns false, the
     * original variable K passed in would still be intact (aka, not actually
     * moved).
     */
    auto insert(std::conditional_t<std::is_trivially_copyable_v<K>, K, K&&> key)
        -> bool;
    /**
     * @brief Inserts the specified key into the BTree.
     *
     * For trivially copyable K, this is identical to insert(). For nontrivially
     * copyable K, this explicitly copies K, retaining the original variable.
     *
     * NOTE: if K is trivially copyable, the "usual" insert() method is more
     * performant, because at the end, this method just calls insert()
     *
     * @param key The specified key
     * @return true if the key is successfully inserted, false if there's
     * already a key of the same value inside.
     */
    auto
    insert_copy(std::conditional_t<std::is_trivially_copyable_v<K>, K, const K&>
                    key) noexcept
        -> std::enable_if_t<std::is_copy_assignable_v<K>, bool> {
        K pass_in = key;
        return insert(std::move(pass_in));
    }

    /**
     * @brief Removes the specified key into the BTree.
     *
     * @param key The specified key
     * @return true if key exists and is successfully removed, false if key
     * doesn't exist.
     */
    auto
    remove(std::conditional_t<std::is_trivially_copyable_v<K>, K, const K&> key)
        -> bool;
};

template <Key K, std::size_t MIN_DEG>
BTreeNode<K, MIN_DEG>::BTreeNode(const BTreeNode& cpy)
    : n_keys_(cpy.n_keys_), n_children_(cpy.n_children_) {
    for (std::size_t idx = 0; idx < cpy.n_keys_; ++idx) {
        this->keys_[idx] = cpy.keys_[idx];
        if (cpy.is_leaf()) {
            continue;
        }

        BTreeNode cpy_child = *cpy.children_[idx].get();
        cpy_child.parent_ = this;
        this->children_[idx] =
            std::make_unique<BTreeNode>(std::move(cpy_child));
    }
    if (cpy.is_leaf()) {
        return;
    }
    BTreeNode last_cpy_child = *cpy.children_[cpy.n_children_ - 1].get();
    last_cpy_child.parent_ = this;
    this->children_[n_children_ - 1] =
        std::make_unique<BTreeNode>(std::move(last_cpy_child));
}

template <Key K, std::size_t MIN_DEG>
auto BTreeNode<K, MIN_DEG>::operator=(const BTreeNode& cpy) -> BTreeNode& {
    if (&cpy == this) {
        std::swap(this->n_keys_, this->n_keys_);
        return *this;
    }

    this->n_keys_ = cpy.n_keys_;
    this->n_children_ = cpy.n_children_;
    for (std::size_t idx = 0; idx < cpy.n_keys_; ++idx) {
        this->keys_[idx] = cpy.keys_[idx];
        if (cpy.is_leaf()) {
            continue;
        }
        BTreeNode cpy_child = *cpy.children_[idx].get();
        cpy_child.parent_ = this;
        this->children_[idx] =
            std::make_unique<BTreeNode>(std::move(cpy_child));
    }
    if (cpy.is_leaf()) {
        return *this;
    }

    BTreeNode last_cpy_child = *cpy.children_[cpy.n_children_ - 1].get();
    last_cpy_child.parent_ = this;
    this->children_[n_children_ - 1] =
        std::make_unique<BTreeNode>(std::move(last_cpy_child));
}

template <Key K, std::size_t MIN_DEG>
auto BTreeNode<K, MIN_DEG>::inner_key_find_(
    std::conditional_t<CAN_TRIVIAL_COPY_, K, const K&> key) const noexcept
    -> std::pair<bool, long long> {

    long long left = 0;
    long long right = n_keys_ - 1;
    long long mid = (left + right) / 2;
    while (left <= right) {
        if (keys_[mid] == key) {
            return {true, mid};
        }
        if (keys_[mid] < key) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
        mid = (left + right) / 2;
    }
    // At this point, the pointer layout is:
    // ... |        |       |       | ...
    //       right    left
    // Where the element of right pointer is smaller than key
    // and left larger than key.
    return {false, right};
}

template <Key K, std::size_t MIN_DEG>
auto BTreeNode<K, MIN_DEG>::find_(
    std::conditional_t<CAN_TRIVIAL_COPY_, K, const K&> key) const noexcept
    -> std::optional<std::pair<const BTreeNode&, std::size_t>> {
    auto pair_result = inner_key_find_(key);
    // found the key
    if (pair_result.first) {
        return std::make_pair(*this, pair_result.second);
    }
    if (is_leaf()) {
        return std::nullopt;
    }
    return children_[pair_result.second + 1]->find_(key);
}

// TODO: finish inner_split_

template <Key K, std::size_t MIN_DEG>
void BTreeNode<K, MIN_DEG>::inner_split_(BTree<K, MIN_DEG>& curr_bt) {
    assert(need_split_());

    std::unique_ptr<BTreeNode> new_node = std::make_unique<BTreeNode>();
    std::size_t median_idx = (this->keys_.size()) / 2;

    // move keys greater than median
    for (std::size_t pos = median_idx + 1; pos < keys_.size(); ++pos) {
        new_node->keys_[new_node->n_keys_++] = std::move(this->keys_[pos]);
        --this->n_keys_;
    }

    if (!this->is_leaf()) {
        // move children right of median
        for (std::size_t pos = median_idx + 1; pos < children_.size(); ++pos) {
            new_node->children_[new_node->n_children_] =
                std::move(this->children_[pos]);
            new_node->children_[new_node->n_children_]->index_ =
                new_node->n_children_;
            new_node->children_[new_node->n_children_]->parent_ =
                new_node.get();
            ++new_node->n_children_;
            --this->n_children_;
        }
    }

    K median_key = std::move(keys_[median_idx]);
    if (this->is_root()) {
        // create new root
        auto new_root = std::make_unique<BTreeNode>();

        new_root->keys_[0] = std::move(median_key);
        --this->n_keys_;
        // curr_bt->root_ is the owning pointer of this
        new_root->children_[0] = std::move(curr_bt.root_);
        new_root->children_[0]->parent_ = new_root.get();
        new_root->children_[0]->index_ = 0;

        new_root->children_[1] = std::move(new_node);
        new_root->children_[1]->parent_ = new_root.get();
        new_root->children_[1]->index_ = 1;

        new_root->n_keys_ = 1;
        new_root->n_children_ = 2;

        curr_bt.root_ = std::move(new_root);
    } else {
        // make room for median key and new node
        for (std::size_t pos = parent_->n_keys_; pos > this->index_; --pos) {
            parent_->keys_[pos] = std::move(parent_->keys_[pos - 1]);
            ++parent_->children_[pos]->index_;
            parent_->children_[pos + 1] = std::move(parent_->children_[pos]);
        }

        parent_->keys_[this->index_] = std::move(median_key);
        --this->n_keys_;
        ++parent_->n_keys_;
        new_node->index_ = this->index_ + 1;
        new_node->parent_ = parent_;
        parent_->children_[this->index_ + 1] = std::move(new_node);
        ++parent_->n_children_;

        if (parent_->need_split_()) {
            parent_->inner_split_(curr_bt);
        }
    }
}

template <Key K, std::size_t MIN_DEG>
void BTreeNode<K, MIN_DEG>::inner_insert_child_at_(
    std::unique_ptr<BTreeNode>&& child, std::size_t index) {
    assert(n_children_ <= MAX_CHILDREN_);
    assert(child.get() != nullptr);
    assert(index <= MAX_CHILDREN_);

    for (long long idx = n_children_ - 1;
         idx > static_cast<long long>(index) - 1; --idx) {
        this->children_[idx + 1] = std::move(this->children_[idx]);
        this->children_[idx + 1]->index_ = idx + 1;
    }
    this->children_[index] = std::move(child);
    this->children_[index]->set_parent_(this, index);
    ++this->n_children_;
}

template <Key K, std::size_t MIN_DEG>
void BTreeNode<K, MIN_DEG>::inner_insert_key_at_(
    BTree<K, MIN_DEG>& curr_bt,
    std::conditional_t<CAN_TRIVIAL_COPY_, K, K&&> key, std::size_t index) {
    for (std::size_t idx = n_keys_; idx > index; --idx) {
        keys_[idx] = std::move(keys_[idx - 1]);
    }

    keys_[index] = std::move(key);
    ++n_keys_;

    if (need_split_()) {
        inner_split_(curr_bt);
    }
}

// TODO: finish inner_insert_

template <Key K, std::size_t MIN_DEG>
void BTreeNode<K, MIN_DEG>::inner_insert_(
    BTree<K, MIN_DEG>& curr_bt,
    std::conditional_t<CAN_TRIVIAL_COPY_, K, K&&> key) {
    auto insert_idx = static_cast<std::size_t>(inner_key_find_(key).second + 1);

    for (std::size_t idx = n_keys_; idx > insert_idx; --idx) {
        keys_[idx] = std::move(keys_[idx - 1]);
    }
    keys_[insert_idx] = std::move(key);
    ++this->n_keys_;

    if (need_split_()) {
        inner_split_(curr_bt);
    }
}

template <Key K, std::size_t MIN_DEG>
auto BTreeNode<K, MIN_DEG>::leaf_remove_(
    BTree<K, MIN_DEG>& curr_bt,
    std::conditional_t<CAN_TRIVIAL_COPY_, K, const K&> key) noexcept -> bool {
    assert(is_leaf());

    if (!leaf_inner_remove_(key)) {
        return false;
    }

    if (n_keys_ >= MIN_DEG || is_root()) {
        return true;
    }

    leaf_rebalance_(curr_bt);
    return true;
}

template <Key K, std::size_t MIN_DEG>
void BTreeNode<K, MIN_DEG>::leaf_rebalance_(
    BTree<K, MIN_DEG>& curr_bt) noexcept {
    assert(!is_root());
    assert(is_leaf());
    assert(n_keys_ < MIN_DEG);
    if (has_left_() && get_left_().n_keys_ > MIN_DEG) {
        // make room for borrowed key
        for (std::size_t pos = n_keys_; pos > 0; --pos) {
            keys_[pos] = std::move(keys_[pos - 1]);
        }
        keys_[0] = leaf_borrow_left_();
        ++n_keys_;
        return;
    }
    if (has_right_() && get_right_().n_keys_ > MIN_DEG) {
        keys_[n_keys_] = leaf_borrow_right_();
        ++n_keys_;
        return;
    }
    if (has_left_()) {
        get_left_().leaf_merge_right_(curr_bt);
        return;
    }
    this->leaf_merge_right_(curr_bt);
}

template <Key K, std::size_t MIN_DEG>
void BTreeNode<K, MIN_DEG>::nonleaf_rebalance_(
    BTree<K, MIN_DEG>& curr_bt) noexcept {
    assert(!is_leaf());
    assert(!is_root());
    assert(n_keys_ <= MIN_DEG);

    if (has_left_() && get_left_().n_keys_ > MIN_DEG) {
        std::pair<std::conditional_t<CAN_TRIVIAL_COPY_, K, K&&>,
                  std::unique_ptr<BTreeNode>>
            borrow = nonleaf_borrow_left_();
        // make room for new key and child
        children_[n_children_] = std::move(children_[n_children_ - 1]);
        children_[n_children_]->index_ = n_children_;
        for (std::size_t pos = n_keys_; pos > 0; --pos) {
            // key and child smaller than it
            keys_[pos] = std::move(keys_[pos - 1]);
            children_[pos] = std::move(children_[pos - 1]);
            children_[pos]->index_ = pos;
        }

        keys_[0] = borrow.first;
        children_[0] = std::move(borrow.second);
        children_[0]->set_parent_(this, 0);

        ++n_keys_;
        ++n_children_;
        return;
    }
    if (has_right_() && get_right_().n_keys_ > MIN_DEG) {
        std::pair<std::conditional_t<CAN_TRIVIAL_COPY_, K, K&&>,
                  std::unique_ptr<BTreeNode>>
            borrow = nonleaf_borrow_right_();

        keys_[n_keys_] = borrow.first;
        children_[n_children_] = std::move(borrow.second);
        children_[n_children_]->set_parent_(this, n_children_);

        ++n_keys_;
        ++n_children_;
        return;
    }
    if (has_left_()) {
        get_left_().nonleaf_merge_right_(curr_bt);
        return;
    }
    this->nonleaf_merge_right_(curr_bt);
}

template <Key K, std::size_t MIN_DEG>
auto BTreeNode<K, MIN_DEG>::leaf_inner_remove_(
    std::conditional_t<CAN_TRIVIAL_COPY_, K, const K&> key) noexcept -> bool {
    assert(is_leaf());

    std::pair<bool, std::size_t> pair_result = inner_key_find_(key);
    if (!pair_result.first) {
        return false;
    }

    assert(pair_result.second >= 0);
    // overwrite the to-be-removed key
    for (auto pos = static_cast<std::size_t>(pair_result.second);
         pos < n_keys_ - 1; ++pos) {
        keys_[pos] = std::move(keys_[pos + 1]);
    }
    --n_keys_;
    return true;
}

template <Key K, std::size_t MIN_DEG>
auto BTreeNode<K, MIN_DEG>::leaf_inner_remove_at_(std::size_t index) noexcept
    -> std::conditional_t<CAN_TRIVIAL_COPY_, K, K&&> {
    assert(index < n_keys_);

    K ret = std::move(keys_[index]);

    // fill in the gap left by moving keys_[index] away
    for (std::size_t pos = index; pos < n_keys_ - 1; ++pos) {
        keys_[pos] = std::move(keys_[pos + 1]);
    }

    --n_keys_;
    return ret;
}

template <Key K, std::size_t MIN_DEG>
auto BTreeNode<K, MIN_DEG>::nonleaf_remove_at_(BTree<K, MIN_DEG>& curr_bt,
                                               std::size_t index) noexcept
    -> std::conditional_t<CAN_TRIVIAL_COPY_, K, K&&> {
    assert(!is_leaf());
    assert(index < n_keys_);

    // find the smallest element of the right subtree
    BTreeNode* curr_node = children_[index + 1].get();
    while (!curr_node->is_leaf()) {
        curr_node = curr_node->children_[0].get();
    }

    K ret = std::move(this->keys_[index]);
    this->keys_[index] = curr_node->leaf_inner_remove_at_(0);
    if (curr_node->n_keys_ < MIN_DEG) {
        curr_node->leaf_rebalance_(curr_bt);
    }
    return ret;
}

template <Key K, std::size_t MIN_DEG>
[[nodiscard]] auto BTreeNode<K, MIN_DEG>::leaf_borrow_left_() noexcept
    -> std::conditional_t<CAN_TRIVIAL_COPY_, K, K&&> {
    assert(has_left_());

    BTreeNode& left = get_left_();
    K ret = std::move(parent_->keys_[index_ - 1]);
    parent_->keys_[index_ - 1] = left.leaf_inner_remove_at_(left.n_keys_ - 1);

    return ret;
}

template <Key K, std::size_t MIN_DEG>
[[nodiscard]] auto BTreeNode<K, MIN_DEG>::leaf_borrow_right_() noexcept
    -> std::conditional_t<CAN_TRIVIAL_COPY_, K, K&&> {
    assert(has_right_());

    BTreeNode& right = get_right_();
    K ret = std::move(parent_->keys_[index_]);
    parent_->keys_[index_] = right.leaf_inner_remove_at_(0);

    return ret;
}

template <Key K, std::size_t MIN_DEG>
[[nodiscard]] auto BTreeNode<K, MIN_DEG>::nonleaf_borrow_left_() noexcept
    -> std::pair<std::conditional_t<CAN_TRIVIAL_COPY_, K, K&&>,
                 std::unique_ptr<BTreeNode>> {
    assert(!is_leaf());
    assert(has_left_());

    BTreeNode& left = get_left_();
    K ret_key = std::move(parent_->keys_[index_ - 1]);
    parent_->keys_[index_ - 1] = std::move(left.keys_[left.n_keys_ - 1]);
    --left.n_keys_;
    std::unique_ptr<BTreeNode> ret_child =
        std::move(left.children_[left.n_children_ - 1]);
    --left.n_children_;
    return std::make_pair<std::conditional_t<CAN_TRIVIAL_COPY_, K, K&&>,
                          std::unique_ptr<BTreeNode>>(std::move(ret_key),
                                                      std::move(ret_child));
}

template <Key K, std::size_t MIN_DEG>
[[nodiscard]] auto BTreeNode<K, MIN_DEG>::nonleaf_borrow_right_() noexcept
    -> std::pair<std::conditional_t<CAN_TRIVIAL_COPY_, K, K&&>,
                 std::unique_ptr<BTreeNode>> {
    assert(!is_leaf());
    assert(has_right_());

    BTreeNode& right = get_right_();
    K ret_key = std::move(parent_->keys_[index_]);
    parent_->keys_[index_] = std::move(right.keys_[0]);
    std::unique_ptr<BTreeNode> ret_child = std::move(right.children_[0]);
    // rearrange keys and children
    for (std::size_t pos = 1; pos < right.n_keys_; ++pos) {
        // key and the child smaller than it
        right.keys_[pos - 1] = std::move(right.keys_[pos]);
        right.children_[pos - 1] = std::move(right.children_[pos]);
        right.children_[pos - 1]->index_ = pos - 1;
    }
    // the largest child
    right.children_[right.n_children_ - 2] =
        std::move(right.children_[right.n_children_ - 1]);
    right.children_[right.n_children_ - 2]->index_ = right.n_children_ - 2;
    --right.n_keys_;
    --right.n_children_;

    return std::make_pair<std::conditional_t<CAN_TRIVIAL_COPY_, K, K&&>,
                          std::unique_ptr<BTreeNode>>(std::move(ret_key),
                                                      std::move(ret_child));
}

template <Key K, std::size_t MIN_DEG>
void BTreeNode<K, MIN_DEG>::leaf_merge_right_(
    BTree<K, MIN_DEG>& curr_bt) noexcept {
    assert(has_right_() && get_right_().n_keys_ <= MIN_DEG);

    keys_[n_keys_] = std::move(parent_->keys_[this->index_]);
    ++n_keys_;
    std::unique_ptr<BTreeNode> right =
        std::move(parent_->children_[index_ + 1]);
    // rearrange parent's keys (and children)
    std::size_t parent_max_pos = parent_->n_keys_ - 1;
    for (std::size_t pos = this->index_; pos < parent_max_pos; ++pos) {
        parent_->keys_[pos] = std::move(parent_->keys_[pos + 1]);
        parent_->children_[pos + 1] = std::move(parent_->children_[pos + 2]);
        parent_->children_[pos + 1]->index_ = pos + 1;
    }
    --parent_->n_keys_;
    --parent_->n_children_;

    std::size_t max_pos = right->n_keys_ - 1;
    std::size_t curr_pos = this->n_keys_;
    for (std::size_t pos = 0; pos <= max_pos; ++pos) {
        this->keys_[curr_pos] = std::move(right->keys_[pos]);
        ++this->n_keys_;
        ++curr_pos;
    }

    if (parent_->is_root() && parent_->n_keys_ == 0) {
        curr_bt.root_ = std::move(parent_->children_[index_]);
        curr_bt.root_->parent_ = nullptr;
        curr_bt.root_->index_ = 0;
        return;
    }

    if (!parent_->is_root() && parent_->n_keys_ < MIN_DEG) {
        parent_->nonleaf_rebalance_(curr_bt);
    }
}

template <Key K, std::size_t MIN_DEG>
void BTreeNode<K, MIN_DEG>::nonleaf_merge_right_(
    BTree<K, MIN_DEG>& curr_bt) noexcept {
    assert(!is_leaf());
    assert(has_right_());
    assert(get_right_().n_keys_ <= MIN_DEG);
    assert(!is_root());

    keys_[n_keys_] = std::move(parent_->keys_[this->index_]);
    ++n_keys_;
    std::unique_ptr<BTreeNode> right =
        std::move(parent_->children_[index_ + 1]);
    // rearrange parent's keys and children
    std::size_t parent_max_pos = parent_->n_keys_ - 1;
    for (std::size_t pos = this->index_; pos < parent_max_pos; ++pos) {
        parent_->keys_[pos] = std::move(parent_->keys_[pos + 1]);
        parent_->children_[pos + 1] = std::move(parent_->children_[pos + 2]);
        parent_->children_[pos + 1]->index_ = pos + 1;
    }
    --parent_->n_keys_;
    --parent_->n_children_;

    this->children_[this->n_children_] = std::move(right->children_[0]);
    this->children_[this->n_children_]->set_parent_(this, this->n_children_);
    ++this->n_children_;
    std::size_t max_pos = right->n_keys_ - 1;
    std::size_t curr_pos = this->n_keys_;
    for (std::size_t pos = 0; pos <= max_pos; ++pos) {
        this->keys_[curr_pos] = std::move(right->keys_[pos]);
        this->children_[curr_pos + 1] = std::move(right->children_[pos + 1]);
        this->children_[curr_pos + 1]->set_parent_(this, curr_pos + 1);
        ++this->n_keys_;
        ++this->n_children_;
        ++curr_pos;
    }

    if (parent_->is_root() && parent_->n_keys_ == 0) {
        curr_bt.root_ = std::move(curr_bt.root_->children_[index_]);
        curr_bt.root_->parent_ = nullptr;
        curr_bt.root_->index_ = 0;
        return;
    }

    if (!parent_->is_root() && parent_->n_keys_ < MIN_DEG) {
        parent_->nonleaf_rebalance_(curr_bt);
    }
}

template <Key K, std::size_t MIN_DEG>
auto BTree<K, MIN_DEG>::insert(
    std::conditional_t<std::is_trivially_copyable_v<K>, K, K&&> key) -> bool {
    BTreeNode<K, MIN_DEG>* curr_node = root_.get();

    std::pair<bool, std::size_t> pair_result = curr_node->inner_key_find_(key);
    while (!curr_node->is_leaf()) {
        if (pair_result.first) {
            return false;
        }
        curr_node = curr_node->children_[pair_result.second + 1].get();
        pair_result = curr_node->inner_key_find_(key);
    }
    if (pair_result.first) {
        return false;
    }
    auto index = static_cast<std::size_t>(
        static_cast<long long>(pair_result.second) + 1);
    curr_node->inner_insert_key_at_(*this, std::forward<K>(key), index);
    return true;
}

template <Key K, std::size_t MIN_DEG>
auto BTree<K, MIN_DEG>::remove(
    std::conditional_t<std::is_trivially_copyable_v<K>, K, const K&> key)
    -> bool {
    BTreeNode<K, MIN_DEG>* curr_node = root_.get();

    std::pair<bool, std::size_t> pair_result = curr_node->inner_key_find_(key);
    while (!curr_node->is_leaf()) {
        if (pair_result.first) {
            curr_node->nonleaf_remove_at_(*this, pair_result.second);
            return true;
        }
        curr_node = curr_node->children_[pair_result.second + 1].get();
        pair_result = curr_node->inner_key_find_(key);
    }
    if (!pair_result.first) {
        return false;
    }
    curr_node->leaf_remove_(*this, std::forward<K>(key));
    return true;
}
} // namespace my_b_tree

#endif //! B_TREE_PROTO_H
