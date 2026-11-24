#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include "lab1/tree/traversal.hpp"

namespace lab1 {

/**
 * @brief Common interface of all trees (dynamic polymorphism).
 *
 * Client code (CLI, demo, random generator) works only with this interface.
 * Values are compared with operator==; a "parent" or "target" node is the first
 * node with the given value in pre-order.
 */
template <typename T>
class ITree {
 public:
  using Visitor = typename ITraversal<T>::Visitor;

  virtual ~ITree() = default;

  virtual std::string kindName() const = 0;
  virtual bool empty() const = 0;
  virtual std::size_t size() const = 0;
  /// Number of levels (0 for an empty tree).
  virtual std::size_t height() const = 0;
  virtual void clear() = 0;

  /// Adds a value at the place chosen by the tree itself.
  virtual bool insert(const T& value) = 0;
  /// Adds a value as a child of the first node equal to `parentValue`.
  virtual bool addChild(const T& parentValue, const T& value) = 0;
  /// Removes the first node equal to `value`.
  virtual bool removeValue(const T& value) = 0;
  /// Removes the `index`-th child (with its subtree) of the first node equal to `parentValue`.
  virtual bool removeChild(const T& parentValue, std::size_t index) = 0;

  virtual bool contains(const T& value) const = 0;
  /// Values on the way from the root to the first node equal to `value`.
  virtual std::optional<std::vector<T>> pathTo(const T& value) const = 0;
  /// True if the sequence of values is a downward path that starts in the root.
  virtual bool hasPath(const std::vector<T>& path) const = 0;

  virtual void traverse(TraversalOrder order, const Visitor& visitor) const = 0;
  virtual std::vector<T> toVector(TraversalOrder order) const = 0;
  virtual std::string toString() const = 0;
};

}  // namespace lab1
