#pragma once

#include <functional>

#include "lab1/common/logger.hpp"
#include "lab1/tree/binary_tree.hpp"

namespace lab1 {

/**
 * @brief Binary search tree (duplicates are rejected).
 *
 * @tparam Compare strict weak ordering "less than" - static polymorphism.
 *
 * removeValue() removes only the matching element (classic BST deletion, an
 * in-order successor replaces a node with two children), whereas removeChild()
 * removes a whole subtree. addChild() is rejected because the BST chooses
 * the position of every element itself.
 */
template <typename T, typename Compare = std::less<T>>
class BinarySearchTree : public BinaryTree<T> {
 public:
  explicit BinarySearchTree(Compare compare = Compare()) : compare_(std::move(compare)) {}

  std::string kindName() const override { return "Binary search tree"; }

  bool insert(const T& value) override {
    std::unique_ptr<Node>* slot = &this->root_;
    while (*slot) {
      if (equivalent(value, (*slot)->value())) return false;
      slot = goesLeft(value, **slot) ? &(*slot)->slot(0) : &(*slot)->slot(1);
    }
    *slot = std::make_unique<Node>(value);
    return true;
  }

  bool addChild(const T&, const T&) override {
    Logger::instance().warning("BST decides positions itself: addChild rejected");
    return false;
  }

  bool removeValue(const T& value) override {
    std::unique_ptr<Node>* slot = &this->root_;
    while (*slot && !equivalent(value, (*slot)->value())) {
      slot = goesLeft(value, **slot) ? &(*slot)->slot(0) : &(*slot)->slot(1);
    }
    if (!*slot) return false;

    Node& node = **slot;
    if (!node.slot(0)) {
      *slot = std::move(node.slot(1));
    } else if (!node.slot(1)) {
      *slot = std::move(node.slot(0));
    } else {
      std::unique_ptr<Node>* successorSlot = &node.slot(1);
      while ((*successorSlot)->slot(0)) successorSlot = &(*successorSlot)->slot(0);
      std::unique_ptr<Node> successor = std::move(*successorSlot);
      *successorSlot = std::move(successor->slot(1));
      successor->slot(0) = std::move(node.slot(0));
      successor->slot(1) = std::move(node.slot(1));
      *slot = std::move(successor);
    }
    return true;
  }

  /// O(height) search that uses the ordering instead of scanning all nodes.
  std::optional<std::vector<T>> pathTo(const T& value) const override {
    std::vector<T> path;
    const Node* current = this->root_.get();
    while (current != nullptr) {
      path.push_back(current->value());
      if (equivalent(value, current->value())) return path;
      current = goesLeft(value, *current) ? current->left() : current->right();
    }
    return std::nullopt;
  }

 private:
  using Node = BinaryNode<T>;

  bool equivalent(const T& a, const T& b) const { return !compare_(a, b) && !compare_(b, a); }
  bool goesLeft(const T& value, const Node& node) const { return compare_(value, node.value()); }

  Compare compare_;
};

}  // namespace lab1
