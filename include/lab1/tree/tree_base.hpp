#pragma once

#include <algorithm>
#include <sstream>

#include "lab1/common/text.hpp"
#include "lab1/tree/itree.hpp"

namespace lab1 {

/**
 * @brief Algorithms shared by all trees; they rely only on INodeView.
 *
 * Derived classes provide the root view and the editing operations.
 */
template <typename T>
class TreeBase : public ITree<T> {
 public:
  using Visitor = typename ITree<T>::Visitor;

  bool empty() const override { return rootView() == nullptr; }

  std::size_t size() const override {
    std::size_t count = 0;
    traverse(TraversalOrder::PreOrder, [&count](const T&, const VisitContext&) { ++count; });
    return count;
  }

  std::size_t height() const override {
    std::size_t levels = 0;
    traverse(TraversalOrder::PreOrder, [&levels](const T&, const VisitContext& context) {
      levels = std::max(levels, context.depth + 1);
    });
    return levels;
  }

  bool contains(const T& value) const override { return pathTo(value).has_value(); }

  std::optional<std::vector<T>> pathTo(const T& value) const override {
    const INodeView<T>* root = rootView();
    if (root == nullptr) return std::nullopt;
    std::vector<T> path;
    if (!findPath(*root, value, path)) return std::nullopt;
    return path;
  }

  bool hasPath(const std::vector<T>& path) const override {
    const INodeView<T>* root = rootView();
    if (root == nullptr || path.empty()) return false;
    return matchPath(*root, path, 0);
  }

  void traverse(TraversalOrder order, const Visitor& visitor) const override {
    makeTraversal<T>(order)->traverse(rootView(), visitor);
  }

  std::vector<T> toVector(TraversalOrder order) const override {
    std::vector<T> values;
    traverse(order, [&values](const T& value, const VisitContext&) { values.push_back(value); });
    return values;
  }

  /// Indented outline, one node per line.
  std::string toString() const override {
    std::ostringstream out;
    out << header() << '\n';
    traverse(TraversalOrder::PreOrder, [&out](const T& value, const VisitContext& context) {
      out << std::string(context.depth * 2, ' ') << "- " << TextTraits<T>::toText(value) << '\n';
    });
    return out.str();
  }

 protected:
  virtual const INodeView<T>* rootView() const = 0;

  std::string header() const {
    std::ostringstream out;
    out << this->kindName();
    if (empty()) {
      out << " (empty)";
    } else {
      out << " | size=" << size() << ", height=" << height();
    }
    return out.str();
  }

 private:
  static bool findPath(const INodeView<T>& node, const T& target, std::vector<T>& path) {
    path.push_back(node.value());
    if (node.value() == target) return true;
    for (const INodeView<T>* child : node.childSlots()) {
      if (child != nullptr && findPath(*child, target, path)) return true;
    }
    path.pop_back();
    return false;
  }

  static bool matchPath(const INodeView<T>& node, const std::vector<T>& path, std::size_t index) {
    if (!(node.value() == path[index])) return false;
    if (index + 1 == path.size()) return true;
    for (const INodeView<T>* child : node.childSlots()) {
      if (child != nullptr && matchPath(*child, path, index + 1)) return true;
    }
    return false;
  }
};

}  // namespace lab1
