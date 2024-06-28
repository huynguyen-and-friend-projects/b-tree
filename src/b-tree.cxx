#include "b-tree.hxx"
#include <optional>
#include <vector>

namespace bt = my_b_tree;

class bt::node final {
  private:
    std::vector<int> keys;
    std::vector<std::unique_ptr<node>> children;
    bt::node* parent;
    friend class bt::b_tree::priv;

  public:
    /**
     * @brief Whether the current node has no children
     */
    auto is_leaf() -> bool { return children.size() == 0; }
    /**
     * @brief Finds the node that contains the specified key.
     *
     * @param key Specified key.
     * @return The node containing the specified key, if found. Otherwise, empty
     * value.
     */
    auto search(int key) -> std::optional<node*> { // NOLINT(misc-no-recursion)
        size_t pos = 0;
        while (pos < this->keys.size() && key > this->keys[pos]) {
            ++pos;
        }
        // key found
        if (pos < this->keys.size() && key == this->keys[pos]) {
            return this;
        }
        if (this->is_leaf()) {
            return std::nullopt;
        }
        return this->children[pos]->search(key);
    }
};

class bt::b_tree::priv final {
  private:
    /**
     * @brief The root of the b_tree. At the start, this is null.
     */
    std::unique_ptr<node> root{nullptr};
    size_t degree;

  public:
    explicit priv(size_t deg) : degree(deg) {}
    friend class bt::b_tree;
    auto find(int val) {}
};

bt::b_tree::b_tree(size_t deg) { this->pimpl_ = std::make_unique<priv>(deg); }

bt::b_tree::b_tree(b_tree&& move_to) noexcept {
    move_to.pimpl_ = std::move(this->pimpl_);
}

bt::b_tree::~b_tree() = default;

auto bt::b_tree::contains(int val) -> bool {
    return this->pimpl_->root != nullptr &&
           this->pimpl_->root->search(val).has_value();
}
