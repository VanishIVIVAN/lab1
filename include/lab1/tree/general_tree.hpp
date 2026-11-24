#pragma once

#include <deque>
#include <memory>
#include <stdexcept>

#include "lab1/tree/child_storage.hpp"
#include "lab1/tree/tree_base.hpp"

namespace lab1 {

/**
 * @brief Node of a tree with an arbitrary number of children.
 *
 * @tparam ChildStorage policy template that stores the children
 *         (LinkedChildList or StdChildList) - static polymorphism.
 */
template <typename T, template <typename> class ChildStorage>
class GeneralNode final : public INodeView<T> {
 public:
  explicit GeneralNode(T value) : value_(std::move(value)) {}

  const T& value() const override { return value_; }

  std::vector<const INodeView<T>*> childSlots() const override {
    std::vector<const INodeView<T>*> slots;
    slots.reserve(children_.size());
    children_.forEach([&slots](GeneralNode& child) { slots.push_back(&child); });
    return slots;
  }

  ChildStorage<GeneralNode>& children() { return children_; }
  const ChildStorage<GeneralNode>& children() const { return children_; }

 private:
  T value_;
  ChildStorage<GeneralNode> children_;
};

/**
 * @brief Tree with an arbitrary number of children per node.
 *
 * insert() fills the tree level by level, giving every node at most
 * `branchingFactor` children; addChild() ignores that limit and can attach any
 * number of children to the chosen parent.
 */
template <typename T, template <typename> class ChildStorage = LinkedChildList>
class GeneralTree : public TreeBase<T> {
 public:
  explicit GeneralTree(std::size_t branchingFactor = 3) : branchingFactor_(branchingFactor) {
    if (branchingFactor_ == 0) throw std::invalid_argument("branching factor must be positive");
  }

  std::string kindName() const override {
    return "General tree [" + ChildStorage<Node>::storageName() + "]";
  }

  void clear() override { root_.reset(); }

  bool insert(const T& value) override {
    if (!root_) {
      root_ = std::make_unique<Node>(value);
      return true;
    }
    Node* host = findFirstWithRoom();
    host->children().append(std::make_unique<Node>(value));
    return true;
  }

  bool addChild(const T& parentValue, const T& value) override {
    Node* parent = findByValue(root_.get(), parentValue);
    if (parent == nullptr) return false;
    parent->children().append(std::make_unique<Node>(value));
    return true;
  }

  bool removeValue(const T& value) override {
    if (!root_) return false;
    if (root_->value() == value) {
      root_.reset();
      return true;
    }
    return removeFrom(*root_, value);
  }

  bool removeChild(const T& parentValue, std::size_t index) override {
    Node* parent = findByValue(root_.get(), parentValue);
    if (parent == nullptr) return false;
    return parent->children().removeAt(index) != nullptr;
  }

 protected:
  const INodeView<T>* rootView() const override { return root_.get(); }

 private:
  using Node = GeneralNode<T, ChildStorage>;

  /// First node (level-order) that may still receive a child through insert().
  Node* findFirstWithRoom() {
    std::deque<Node*> queue{root_.get()};
    while (!queue.empty()) {
      Node* current = queue.front();
      queue.pop_front();
      if (current->children().size() < branchingFactor_) return current;
      current->children().forEach([&queue](Node& child) { queue.push_back(&child); });
    }
    return nullptr;  // unreachable: some leaf always has room
  }

  /// First node equal to `value` in pre-order.
  static Node* findByValue(Node* node, const T& value) {
    if (node == nullptr) return nullptr;
    if (node->value() == value) return node;
    Node* found = nullptr;
    node->children().forEach([&](Node& child) {
      if (found == nullptr) found = findByValue(&child, value);
    });
    return found;
  }

  /// Removes the first descendant equal to `value` (pre-order) with its subtree.
  static bool removeFrom(Node& node, const T& value) {
    for (std::size_t i = 0; i < node.children().size(); ++i) {
      Node* child = node.children().at(i);
      if (child->value() == value) {
        node.children().removeAt(i);
        return true;
      }
      if (removeFrom(*child, value)) return true;
    }
    return false;
  }

  std::size_t branchingFactor_;
  std::unique_ptr<Node> root_;
};

template <typename T>
using LinkedGeneralTree = GeneralTree<T, LinkedChildList>;
template <typename T>
using StdListGeneralTree = GeneralTree<T, StdChildList>;

}  // namespace lab1
