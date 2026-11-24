#pragma once

#include <deque>
#include <memory>

#include "lab1/tree/tree_base.hpp"

namespace lab1 {

/// Node of a binary tree: slot 0 is the left child, slot 1 the right one.
template <typename T>
class BinaryNode final : public INodeView<T> {
 public:
  explicit BinaryNode(T value) : value_(std::move(value)) {}

  const T& value() const override { return value_; }

  std::vector<const INodeView<T>*> childSlots() const override {
    return {left_.get(), right_.get()};
  }

  std::unique_ptr<BinaryNode>& slot(std::size_t index) { return index == 0 ? left_ : right_; }
  const std::unique_ptr<BinaryNode>& slot(std::size_t index) const {
    return index == 0 ? left_ : right_;
  }
  const BinaryNode* left() const { return left_.get(); }
  const BinaryNode* right() const { return right_.get(); }

 private:
  T value_;
  std::unique_ptr<BinaryNode> left_;
  std::unique_ptr<BinaryNode> right_;
};

/**
 * @brief Binary tree without ordering rules.
 *
 * insert() builds a complete tree (first free slot in level order); child
 * index 0 is the left child and 1 is the right one.
 */
template <typename T>
class BinaryTree : public TreeBase<T> {
 public:
  std::string kindName() const override { return "Binary tree"; }

  void clear() override { root_.reset(); }

  bool insert(const T& value) override {
    if (!root_) {
      root_ = std::make_unique<Node>(value);
      return true;
    }
    std::deque<Node*> queue{root_.get()};
    while (!queue.empty()) {
      Node* current = queue.front();
      queue.pop_front();
      for (std::size_t slot = 0; slot < 2; ++slot) {
        if (!current->slot(slot)) {
          current->slot(slot) = std::make_unique<Node>(value);
          return true;
        }
        queue.push_back(current->slot(slot).get());
      }
    }
    return false;  // unreachable
  }

  /// Puts the value into the first free slot (left, then right) of the parent.
  bool addChild(const T& parentValue, const T& value) override {
    Node* parent = findByValue(root_.get(), parentValue);
    if (parent == nullptr) return false;
    for (std::size_t slot = 0; slot < 2; ++slot) {
      if (!parent->slot(slot)) {
        parent->slot(slot) = std::make_unique<Node>(value);
        return true;
      }
    }
    return false;
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
    if (index > 1) return false;
    Node* parent = findByValue(root_.get(), parentValue);
    if (parent == nullptr || !parent->slot(index)) return false;
    parent->slot(index).reset();
    return true;
  }

  /// Outline that marks left (L) and right (R) children.
  std::string toString() const override {
    std::ostringstream out;
    out << this->header() << '\n';
    if (root_) print(*root_, 0, "", out);
    return out.str();
  }

 protected:
  using Node = BinaryNode<T>;

  const INodeView<T>* rootView() const override { return root_.get(); }

  std::unique_ptr<Node> root_;

 private:
  static Node* findByValue(Node* node, const T& value) {
    if (node == nullptr) return nullptr;
    if (node->value() == value) return node;
    if (Node* found = findByValue(node->slot(0).get(), value)) return found;
    return findByValue(node->slot(1).get(), value);
  }

  static bool removeFrom(Node& node, const T& value) {
    for (std::size_t slot = 0; slot < 2; ++slot) {
      std::unique_ptr<Node>& child = node.slot(slot);
      if (!child) continue;
      if (child->value() == value) {
        child.reset();
        return true;
      }
      if (removeFrom(*child, value)) return true;
    }
    return false;
  }

  static void print(const Node& node, std::size_t depth, const std::string& label,
                    std::ostringstream& out) {
    out << std::string(depth * 2, ' ') << label << TextTraits<T>::toText(node.value()) << '\n';
    if (node.left() != nullptr) print(*node.left(), depth + 1, "L: ", out);
    if (node.right() != nullptr) print(*node.right(), depth + 1, "R: ", out);
  }
};

}  // namespace lab1
