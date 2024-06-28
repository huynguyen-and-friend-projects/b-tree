#ifndef B_TREE_PROTO_H
#define B_TREE_PROTO_H

#include <memory>
namespace my_b_tree {

struct node;

class b_tree final {
  public:
    explicit b_tree(size_t deg);
    /**
     * @brief Moves content of the current b_tree into the specified b_tree.
     *
     * Not recommended to use unless it is certain that the variable holding the
     * current b_tree is no longer being used.
     *
     * @param move_to
     */
    b_tree(b_tree&& move_to) noexcept;
    /**
     * @brief Copies content of the current b_tree into the specified b_tree.
     *
     * This operation is VERY expensive, both memory-wise and processor-wise,
     * especially if the b_tree is large. For most situations, use a reference.
     * Only use this for back-up.
     *
     * @param copy_to
     */
    b_tree(const b_tree& copy_to);
    /**
     * @brief Moves content of the current b_tree into the specified b_tree.
     *
     * Not recommended to use unless it is certain that the variable holding the
     * current b_tree is no longer being used.
     *
     * @param move_to
     */
    auto operator=(b_tree&& move_to) noexcept -> b_tree&;
    /**
     * @brief Copies content of the current b_tree into the specified b_tree.
     *
     * This operation is VERY expensive, both memory-wise and processor-wise,
     * especially if the b_tree is large. For most situations, use a reference.
     * Only use this for back-up.
     *
     * @param copy_to
     */
    auto operator=(const b_tree& copy_to) -> b_tree&;
    ~b_tree();

    /**
     * @brief Whether this b_tree contains the specified value.
     *
     * @param val Specified value
     */
    auto contains(int val) -> bool;

    void insert(int val);
    void remove(int val);

    friend struct node;
  private:
    /**
     * @class priv
     * @brief Private implementation of the b_tree.
     *
     */
    class priv;
    /**
     * @brief Private implementation of the b_tree.
     */
    std::unique_ptr<priv> pimpl_;
};

} // namespace my_b_tree

#endif // !B_TREE_PROTO_H
