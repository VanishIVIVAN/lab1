#pragma once

#include <cstddef>
#include <deque>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace lab1 {

/// Supported tree traversal orders.
enum class TraversalOrder { PreOrder, PostOrder, LevelOrder, InOrder };

/// Human readable name of an order ("pre", "post", "level", "in").
inline std::string orderName(TraversalOrder order) {
  switch (order) {
    case TraversalOrder::PreOrder: return "pre";
    case TraversalOrder::PostOrder: return "post";
    case TraversalOrder::LevelOrder: return "level";
    case TraversalOrder::InOrder: return "in";
  }
  return "?";
}

/// Parses "pre", "post", "level" or "in".
inline std::optional<TraversalOrder> parseOrder(const std::string& text) {
  if (text == "pre") return TraversalOrder::PreOrder;
  if (text == "post") return TraversalOrder::PostOrder;
  if (text == "level") return TraversalOrder::LevelOrder;
  if (text == "in") return TraversalOrder::InOrder;
  return std::nullopt;
}

/// Information about a node that is passed to a visitor.
struct VisitContext {
  std::size_t depth = 0;       ///< Root has depth 0.
  std::size_t childCount = 0;  ///< Number of existing children.
};

/**
 * @brief Read-only view of a tree node that is independent of the tree kind.
 *
 * Traversal strategies work only with this interface, so a single implementation
 * of every traversal serves general, binary and search trees.
 */
template <typename T>
class INodeView {
 public:
  virtual ~INodeView() = default;
  virtual const T& value() const = 0;
  /// Child slots in order. nullptr marks an empty slot (e.g. missing left child).
  virtual std::vector<const INodeView<T>*> childSlots() const = 0;
};

namespace detail {
template <typename T>
std::size_t countPresent(const std::vector<const INodeView<T>*>& slots) {
  std::size_t count = 0;
  for (const INodeView<T>* slot : slots) {
    if (slot != nullptr) ++count;
  }
  return count;
}
}  // namespace detail

/**
 * @brief Strategy interface of a traversal (dynamic polymorphism).
 */
template <typename T>
class ITraversal {
 public:
  using Visitor = std::function<void(const T&, const VisitContext&)>;

  virtual ~ITraversal() = default;
  virtual void traverse(const INodeView<T>* root, const Visitor& visitor) const = 0;
  virtual std::string name() const = 0;
};

/// Node, then children from left to right.
template <typename T>
class PreOrderTraversal final : public ITraversal<T> {
 public:
  using Visitor = typename ITraversal<T>::Visitor;

  void traverse(const INodeView<T>* root, const Visitor& visitor) const override {
    if (root != nullptr) visit(*root, 0, visitor);
  }
  std::string name() const override { return "pre-order"; }

 private:
  static void visit(const INodeView<T>& node, std::size_t depth, const Visitor& visitor) {
    const auto slots = node.childSlots();
    visitor(node.value(), VisitContext{depth, detail::countPresent<T>(slots)});
    for (const INodeView<T>* child : slots) {
      if (child != nullptr) visit(*child, depth + 1, visitor);
    }
  }
};

/// Children from left to right, then the node.
template <typename T>
class PostOrderTraversal final : public ITraversal<T> {
 public:
  using Visitor = typename ITraversal<T>::Visitor;

  void traverse(const INodeView<T>* root, const Visitor& visitor) const override {
    if (root != nullptr) visit(*root, 0, visitor);
  }
  std::string name() const override { return "post-order"; }

 private:
  static void visit(const INodeView<T>& node, std::size_t depth, const Visitor& visitor) {
    const auto slots = node.childSlots();
    for (const INodeView<T>* child : slots) {
      if (child != nullptr) visit(*child, depth + 1, visitor);
    }
    visitor(node.value(), VisitContext{depth, detail::countPresent<T>(slots)});
  }
};

/// Breadth-first: level by level.
template <typename T>
class LevelOrderTraversal final : public ITraversal<T> {
 public:
  using Visitor = typename ITraversal<T>::Visitor;

  void traverse(const INodeView<T>* root, const Visitor& visitor) const override {
    if (root == nullptr) return;
    std::deque<std::pair<const INodeView<T>*, std::size_t>> queue;
    queue.emplace_back(root, 0);
    while (!queue.empty()) {
      const auto [node, depth] = queue.front();
      queue.pop_front();
      const auto slots = node->childSlots();
      visitor(node->value(), VisitContext{depth, detail::countPresent<T>(slots)});
      for (const INodeView<T>* child : slots) {
        if (child != nullptr) queue.emplace_back(child, depth + 1);
      }
    }
  }
  std::string name() const override { return "level-order"; }
};

/**
 * @brief In-order: first slot's subtree, node, remaining slots' subtrees.
 *
 * For binary trees this is the classic left-node-right order; for trees with
 * many children it is the usual generalisation (first child, node, the rest).
 */
template <typename T>
class InOrderTraversal final : public ITraversal<T> {
 public:
  using Visitor = typename ITraversal<T>::Visitor;

  void traverse(const INodeView<T>* root, const Visitor& visitor) const override {
    if (root != nullptr) visit(*root, 0, visitor);
  }
  std::string name() const override { return "in-order"; }

 private:
  static void visit(const INodeView<T>& node, std::size_t depth, const Visitor& visitor) {
    const auto slots = node.childSlots();
    if (!slots.empty() && slots.front() != nullptr) visit(*slots.front(), depth + 1, visitor);
    visitor(node.value(), VisitContext{depth, detail::countPresent<T>(slots)});
    for (std::size_t i = 1; i < slots.size(); ++i) {
      if (slots[i] != nullptr) visit(*slots[i], depth + 1, visitor);
    }
  }
};

/// Factory of traversal strategies.
template <typename T>
std::unique_ptr<ITraversal<T>> makeTraversal(TraversalOrder order) {
  switch (order) {
    case TraversalOrder::PreOrder: return std::make_unique<PreOrderTraversal<T>>();
    case TraversalOrder::PostOrder: return std::make_unique<PostOrderTraversal<T>>();
    case TraversalOrder::LevelOrder: return std::make_unique<LevelOrderTraversal<T>>();
    case TraversalOrder::InOrder: return std::make_unique<InOrderTraversal<T>>();
  }
  throw std::invalid_argument("unknown traversal order");
}

}  // namespace lab1
