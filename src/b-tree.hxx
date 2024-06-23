#ifndef B_TREE_PROTO_H
#define B_TREE_PROTO_H

#include <cstddef>
#include <memory>

class b_tree {
  public:
    // use 5 for simplicity
    static constexpr size_t MAX_KEYS = 5;

    b_tree();

    b_tree(b_tree&&) noexcept;
    auto operator=(b_tree&&) noexcept -> b_tree&;

    auto operator=(const b_tree&) -> b_tree&;
    b_tree(const b_tree&);

    ~b_tree();

  private:
    class node {
      public:
        node();

        auto operator=(node&&) noexcept -> node&;
        node(node&&) noexcept;

        auto operator=(const node&) -> node&;
        node(const node&);

        ~node();

      private:
        /**
         * @brief The children of this node
         */
        std::array<std::unique_ptr<node>, MAX_KEYS> children;
        // use int instead of template for simplicity
        int data;
    };

    node root;
    /**
     * @brief How many nodes are inside the tree
     */
    size_t count;
};

#endif // !B_TREE_PROTO_H
