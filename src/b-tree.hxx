/**
 * @file b-tree.hxx
 */

#ifndef B_TREE_PROTO_H
#define B_TREE_PROTO_H

#include <array>
#include <cassert>
#include <climits>
#include <concepts>
#include <cstddef>
#include <memory>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace my_b_tree {

/**
 * @brief Key concept.
 *
 * Requires to be equality-comparable and movable.
 */
template <typename T>
concept Key = std::equality_comparable<T> && std::movable<T>;

/**
 * @brief A BTree.
 */
template <Key K, std::size_t MIN_DEG> class BTree;

/**
 * @brief The node of the BTree
 */
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
     * 2 * MIN_DEG
     */
    static constexpr std::size_t MAX_KEYS_ = 2 * MIN_DEG;

    /**
     * @brief Same as BTree<K, MIN_DEG>::MAX_CHILDREN
     * (2 * MIN_DEG) + 1
     */
    static constexpr std::size_t MAX_CHILDREN_ = MAX_KEYS_ + 1;

    /**
     * @brief Keys array.
     */
    std::array<K, MAX_KEYS_> keys_{};

    /**
     * @brief Number of keys.
     */
    std::size_t n_keys_{0};

    /**
     * @brief Children array.
     */
    std::array<std::unique_ptr<BTreeNode>, MAX_CHILDREN_> children_{};

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
     */
    std::size_t index_{0};

    friend class BTree<K, MIN_DEG>;

    /**
     * @brief Gets the minimum degree of this node.
     *
     * If the node is a root node, minimum degree is 1, otherwise, MIN_DEG.
     *
     * NOTE: use this when the difference between root's minimum degree
     * and other nodes' minimum degree matters (eg, removing keys). Otherwise,
     * simply use MIN_DEG for better performance.
     *
     * @return The minimum degree of this node.
     */
    auto minimum_degree_() -> std::size_t { return is_root() ? 1 : MIN_DEG; }

    /**
     * @brief Simply a more explicit version of n_keys_ > minimum_degree_
     *
     * @return Whether one key from this node can be removed without rebalancing
     * the tree.
     */
    auto no_rebalance_removable_() -> bool {
        return n_keys_ > minimum_degree_();
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
    void set_parent_(BTreeNode* parent, std::size_t index) {
        // in case you don't listen
        assert(parent != this);
        assert(index < MAX_CHILDREN_);

        index_ = index;
        parent_ = parent;
    }

    /**
     * Sets index with bound-checking.
     *
     * index must be between 0 and MAX_CHILDREN_
     */
    void set_index_(std::size_t index) {
        // in case you don't listen
        assert(index > 0 && index < MAX_CHILDREN_);
        index_ = index;
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
     *      - So, -1 if the key is smaller than every element and max_keys()
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
     *   - First value is the pointer to the node containing the specified key.
     *   - Second value is the index of that key.
     */
    [[nodiscard]] auto
    find_(std::conditional_t<CAN_TRIVIAL_COPY_, K, const K&> key) const noexcept
        -> std::optional<std::pair<const BTreeNode*, std::size_t>>;

    /**
     * @brief Splits the current node into 2, and insert the current median into
     * the parent. inner_split_ calls this function when the current node is
     * leaf.
     *
     * Only used inside inner_split_. DO NOT USE ELSEWHERE.
     *
     * In case a new root node is made, curr_bt->root_ is updated.
     *
     * @param curr_bt
     * @param new_node The new node created inside the function inner_split_
     */
    void inner_split_leaf_(BTree<K, MIN_DEG>* curr_bt,
                           BTreeNode* new_node) noexcept;

    /**
     * @brief Splits the current node into 2, and insert the current median into
     * the parent. inner_split_ calls this function when the current node is NOT
     * leaf.
     *
     * Only used inside inner_split_. DO NOT USE ELSEWHERE.
     *
     * In case a new root node is made, curr_bt->root_ is updated.
     *
     * @param curr_bt
     * @param new_node The new node created inside the function inner_split_
     */
    void inner_split_nonleaf_(BTree<K, MIN_DEG>* curr_bt,
                              BTreeNode* new_node) noexcept;

    /**
     * @brief (Simply) split the current node into 2, and insert the current
     * median into the parent.
     *
     * Should only be called when this node is full.
     *
     * In case a new root node is made, curr_bt->root_ is updated.
     *
     * @param curr_bt
     */
    void inner_split_(BTree<K, MIN_DEG>* curr_bt) noexcept;
    /**
     * @brief Inserts the specified child at the specified index.
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
                                std::size_t index) noexcept;
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
    void inner_insert_key_at_(BTree<K, MIN_DEG>* curr_bt,
                              std::conditional_t<CAN_TRIVIAL_COPY_, K, K&&> key,
                              std::size_t index) noexcept;
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
    void
    inner_insert_(BTree<K, MIN_DEG>* curr_bt,
                  std::conditional_t<CAN_TRIVIAL_COPY_, K, K&&> key) noexcept;

    /**
     * @brief Takes the separator between this node and its left neighbour, and
     * take the left child's largest key to be the new separator.
     *
     * Must only be called when:
     * 1. There exists a left neighbour.
     * 2. The left neighbour's number of keys is at least one more than the
     * minimum degree.
     * 3. This node (and consequently its left neighbour) is not root (aka, has
     * parent).
     *
     * If a node cannot borrow from both its left and right neighbour, a merge
     * is triggered.
     *
     * @return The old separator.
     */
    auto borrow_left_()
        -> std::conditional_t<std::is_trivially_copyable_v<K>, K, K&&>;
    /**
     * @brief Takes the separator between this node and its right neighbour, and
     * take the right child's smallest key to be the new separator.
     *
     * Must only be called when:
     * 1. There exists a right neighbour.
     * 2. The right neigbbour's number of keys is at least one more than the
     * minimum degree.
     * 3. This node (and consequently its right neighbour) is not root (aka, has
     * parent).
     *
     * If a node cannot borrow from both its left and right neighbour, a merge
     * is triggered.
     *
     * @return The old separator.
     */
    auto borrow_right_()
        -> std::conditional_t<std::is_trivially_copyable_v<K>, K, K&&>;

    /**
     * @brief Merge with the node right of this node.
     *
     * The new node's key layout is the following:
     * [keys of this] [separator] [keys of right]
     *
     * Must only be called when:
     * 1. This node (and its right neighbour consequently) are leaves.
     * 2. This node has a right neighbour (aka, this->index_ <
     * parent->n_children_ - 1)
     * 3. Both this node and its right neighbour are at or below minimum degree.
     *
     * The current BTree is passed in, in the case this causes a merge happening
     * when the root only has 1 key left, so a new root is updated.
     */
    void leaf_merge_right_(BTree<K, MIN_DEG>* curr_bt);

    /**
     * @brief Merge with the node right of this node.
     *
     * The new node's key layout is the following:
     * [keys of this] [separator] [keys of right]
     *
     * The new node's children layout is the following:
     * [children of this] [children of right]
     *
     * Must only be called when:
     * 1. This node (and its right neighbour consequently) are NOT leaves.
     * 2. This node has a right neighbour (aka, this->index_ <
     * parent->n_children_ - 1)
     * 3. Both this node and its right neighbour are at or below minimum degree.
     *
     * If this and the right neighbour's parent is the tree root, curr_bt->root_
     * is updated to the new node.
     *
     * The current BTree is passed in, in the case this causes a merge happening
     * when the root only has 1 key left, so a new root is updated.
     *
     * @param curr_bt
     */
    void nonleaf_merge_right_(BTree<K, MIN_DEG>* curr_bt);

    /**
     * @brief
     * @return
     */
    auto get_left_separator_()
        -> std::conditional_t<std::is_trivially_copyable_v<K>, K, K&&>;

    /**
     * @brief
     * @return
     */
    auto get_right_separator_()
        -> std::conditional_t<std::is_trivially_copyable_v<K>, K, K&&>;

    /**
     * @brief
     * @return
     */
    auto get_right_neighbour_and_sep_()
        -> std::pair<
            std::conditional_t<std::is_trivially_copyable_v<K>, K, K&&>,
            std::unique_ptr<BTreeNode<K, MIN_DEG>>>;

    /**
     * @brief Removes the key at the specified index.
     *
     * Will trigger borrowing if removal causes the number of keys to fall below
     * its minimum degree.
     *
     * If a node cannot borrow from both its left and right neighbour, a merge
     * is triggered.
     *
     * Must only be called when:
     * 1. index is in bound 0 to this->n_keys_ - 1
     * 2. This node is leaf.
     *
     * @param index The specified index.
     * @return The removed key.
     */
    auto leaf_remove_at_(std::size_t index)
        -> std::conditional_t<std::is_trivially_copyable_v<K>, K, K&&>;

    /**
     * @brief Removes the key with the specified value
     *
     * Will trigger borrowing if removal causes the number of keys to fall below
     * its minimum degree.
     *
     * If a node cannot borrow from both its left and right neighbour, a merge
     * is triggered.
     *
     * Must only be called when:
     * 1. This node is leaf.
     *
     * @param key The specified key.
     * @return The removed key, if it exists, or nullopt otherwise.
     */
    auto leaf_remove_(
        std::conditional_t<std::is_trivially_copyable_v<K>, K, const K&> key)
        -> std::optional<
            std::conditional_t<std::is_trivially_copyable_v<K>, K, K&&>>;

    /**
     * @brief Removes the key at the specified index.
     *
     * Will trigger borrowing if removal causes the number of keys to fall below
     * its minimum degree.
     *
     * If a node cannot borrow from both its left and right neighbour, a merge
     * is triggered.
     *
     * Must only be called when:
     * 1. index is in bound 0 to this->n_keys_ - 1
     * 2. This node is NOT leaf.
     *
     * @param index The specified index.
     * @return The removed key.
     */
    auto nonleaf_remove_at_(std::size_t index)
        -> std::conditional_t<std::is_trivially_copyable_v<K>, K, K&&>;

    /**
     * @brief Removes the key with the specified value
     *
     * Will trigger borrowing if removal causes the number of keys to fall below
     * its minimum degree.
     *
     * If a node cannot borrow from both its left and right neighbour, a merge
     * is triggered.
     *
     * Must only be called when:
     * 1. This node is NOT leaf.
     *
     * @param key The specified key.
     * @return The removed key, if it exists, or nullopt otherwise.
     */
    auto nonleaf_remove_(
        std::conditional_t<std::is_trivially_copyable_v<K>, K, K&&> key)
        -> std::optional<
            std::conditional_t<std::is_trivially_copyable_v<K>, K, K&&>>;

  public:
    BTreeNode() noexcept = default;

    explicit BTreeNode(BTreeNode* parent) : parent_(parent) {
        if (parent == this) {
            throw std::invalid_argument(
                "Expected parent pointer to NOT be the same as this");
        }
    }

    BTreeNode(BTreeNode&&) noexcept = default;

    /**
     * WARNING: VERY EXPENSIVE OPERATION.
     */
    BTreeNode(const BTreeNode& cpy);

    auto operator=(BTreeNode&&) noexcept -> BTreeNode& = default;

    /**
     * WARNING: VERY EXPENSIVE OPERATION.
     */
    auto operator=(const BTreeNode& cpy) noexcept -> BTreeNode&;

    ~BTreeNode() noexcept = default;

    [[nodiscard]] auto keys_count() const noexcept -> std::size_t {
        return n_keys_;
    }

    [[nodiscard]] auto children_count() const noexcept -> std::size_t {
        return n_children_;
    }

    /**
     * @return Whether this node has no child.
     */
    [[nodiscard]] auto is_leaf() const noexcept -> bool {
        return n_children_ == 0;
    }

    /**
     * This is just an alternative to BTree::MAX_KEYS
     */
    [[nodiscard]] auto max_keys() const noexcept -> std::size_t {
        return MAX_KEYS_;
    }

    /**
     * This is just an alternative to BTree::MAX_CHILDREN
     */
    [[nodiscard]] auto max_children() const noexcept -> std::size_t {
        return MAX_CHILDREN_;
    }

    /**
     * @return Whether this node's key array is fully occupied.
     */
    [[nodiscard]] auto is_full() const noexcept -> bool {
        return n_keys_ == keys_.size();
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
    auto operator=(const BTree& cpy) noexcept -> BTree& {
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
    [[nodiscard]] auto find(std::conditional_t<std::is_trivially_copyable_v<K>,
                                               K, const K&> key) const noexcept
        -> std::optional<std::pair<const BTreeNode<K, MIN_DEG>*, std::size_t>> {
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
    auto insert(std::conditional_t<std::is_trivially_copyable_v<K>, K, K&&>
                    key) noexcept -> bool;
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
        -> std::enable_if_t<std::is_copy_constructible_v<K>, bool> {
        K pass_in = key;
        return insert(std::move(pass_in));
    }
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
auto BTreeNode<K, MIN_DEG>::operator=(const BTreeNode& cpy) noexcept
    -> BTreeNode& {
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
    -> std::optional<std::pair<const BTreeNode*, std::size_t>> {
    auto pair_result = inner_key_find_(key);
    // found the key
    if (pair_result.first) {
        return std::make_pair(this, pair_result.second);
    }
    if (is_leaf()) {
        return std::nullopt;
    }
    return children_[pair_result.second + 1]->find_(key);
}

template <Key K, std::size_t MIN_DEG>
void BTreeNode<K, MIN_DEG>::inner_split_leaf_(BTree<K, MIN_DEG>* curr_bt,
                                              BTreeNode* new_node) noexcept {
    assert(is_leaf());

    std::size_t median_idx = (n_keys_ - 1) / 2;
    std::size_t max_idx = n_keys_ - 1;
    std::size_t new_node_idx = 0;

    // move keys larger than the median to the new node.
    for (std::size_t this_idx = median_idx + 1; this_idx <= max_idx;
         ++this_idx) {
        new_node->inner_insert_key_at_(
            curr_bt, std::forward<K>(this->keys_[this_idx]), new_node_idx);
        --this->n_keys_;
        ++new_node_idx;
    }
}

template <Key K, std::size_t MIN_DEG>
void BTreeNode<K, MIN_DEG>::inner_split_nonleaf_(BTree<K, MIN_DEG>* curr_bt,
                                                 BTreeNode* new_node) noexcept {
    assert(!is_leaf());

    std::size_t median_idx = (n_keys_ - 1) / 2;
    std::size_t max_idx = n_keys_ - 1;
    std::size_t new_node_idx = 0;

    // move the child just larger than the median (if any) to the first
    // child pointer of the new node
    new_node->inner_insert_child_at_(std::move(this->children_[median_idx + 1]),
                                     0);
    --this->n_children_;

    // move keys larger than the median and the children just larger than each
    // of those key to the new node.
    for (std::size_t this_idx = median_idx + 1; this_idx <= max_idx;
         ++this_idx) {
        new_node->inner_insert_key_at_(
            curr_bt, std::forward<K>(this->keys_[this_idx]), new_node_idx);
        new_node->inner_insert_child_at_(
            std::move(this->children_[this_idx + 1]), new_node_idx + 1);
        --this->n_keys_;
        --this->n_children_;
        ++new_node_idx;
    }
}

template <Key K, std::size_t MIN_DEG>
void BTreeNode<K, MIN_DEG>::inner_split_(BTree<K, MIN_DEG>* curr_bt) noexcept {
    // in case you don't listen.
    assert(is_full());

    std::size_t median_idx = (n_keys_ - 1) / 2;
    auto new_node = std::make_unique<BTreeNode>(this->parent_);

    if (is_leaf()) {
        inner_split_leaf_(curr_bt, new_node.get());
    } else {
        inner_split_nonleaf_(curr_bt, new_node.get());
    }

    if (this->parent_ != nullptr) {
        // this is not root.
        this->parent_->inner_insert_key_at_(
            curr_bt, std::move(this->keys_[median_idx]), this->index_);
        --this->n_keys_;
        this->parent_->inner_insert_child_at_(std::move(new_node),
                                              this->index_ + 1);
    } else {
        // this is root
        auto new_root = std::make_unique<BTreeNode>();
        new_root->inner_insert_key_at_(curr_bt,
                                       std::move(this->keys_[median_idx]), 0);
        --this->n_keys_;
        // curr_bt->root_ is essentially an unique_ptr to this
        new_root->inner_insert_child_at_(std::move(curr_bt->root_), 0);
        new_root->inner_insert_child_at_(std::move(new_node), 1);
        curr_bt->root_ = std::move(new_root);
    }
}

template <Key K, std::size_t MIN_DEG>
void BTreeNode<K, MIN_DEG>::inner_insert_child_at_(
    std::unique_ptr<BTreeNode>&& child, std::size_t index) noexcept {
    assert(n_children_ < MAX_CHILDREN_);
    assert(child.get() != nullptr);
    assert(index < MAX_CHILDREN_);

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
    BTree<K, MIN_DEG>* curr_bt,
    std::conditional_t<CAN_TRIVIAL_COPY_, K, K&&> key,
    std::size_t index) noexcept {
    assert(is_full() ? index <= MAX_KEYS_ : index < MAX_KEYS_);

    bool is_split = false;
    if (is_full()) {
        this->inner_split_(curr_bt);
        is_split = true;
    }

    // after splitting, parent is not nullptr
    // parent_->keys_[this->index_] points to the key just larger than this
    // node.
    // Also, in no situation shall key == [any key in parent's key array]
    if (is_split) {
        if (key > this->parent_->keys_[this->index_]) {
            // insert into the new node instead.
            // HACK: new node is assumed to be this node's right neighbour
            this->parent_->children_[this->index_ + 1]->inner_insert_(
                curr_bt, std::forward<K>(key));
            return;
        }
        this->inner_insert_(curr_bt, std::forward<K>(key));
        return;
    }

    for (long long idx = n_keys_ - 1; idx > static_cast<long long>(index - 1);
         --idx) {
        this->keys_[idx + 1] = std::move(this->keys_[idx]);
    }
    this->keys_[index] = std::forward<K>(key);
    ++this->n_keys_;
}

template <Key K, std::size_t MIN_DEG>
void BTreeNode<K, MIN_DEG>::inner_insert_(
    BTree<K, MIN_DEG>* curr_bt,
    std::conditional_t<CAN_TRIVIAL_COPY_, K, K&&> key) noexcept {
    bool is_split = false;
    if (is_full()) {
        this->inner_split_(curr_bt);
        is_split = true;
    }

    // after splitting, parent is not nullptr
    // parent_->keys_[this->index_] points to the key just larger than this
    // node.
    // Also, in no situation shall key == [any key in parent's key array]
    if (is_split) {
        if (key > this->parent_->keys_[this->index_]) {
            // insert into the new node instead.
            // HACK: new node is assumed to be this node's right neighbour
            this->parent_->children_[this->index_ + 1]->inner_insert_(
                curr_bt, std::move(key));
            return;
        }
        this->inner_insert_(curr_bt, std::forward<K>(key));
        return;
    }
    long long insert_idx = inner_key_find_(key).second + 1;

    for (long long idx = n_keys_ - 1; idx > insert_idx - 1; --idx) {
        keys_[idx + 1] = std::move(keys_[idx]);
    }
    keys_[insert_idx] = std::move(key);
    ++this->n_keys_;
}

template <Key K, std::size_t MIN_DEG>
auto BTree<K, MIN_DEG>::insert(
    std::conditional_t<std::is_trivially_copyable_v<K>, K, K&&> key) noexcept
    -> bool {
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
    long long index = static_cast<long long>(pair_result.second) + 1;
    curr_node->inner_insert_key_at_(this, std::forward<K>(key), index);
    return true;
}

} // namespace my_b_tree

#endif //! B_TREE_PROTO_H
