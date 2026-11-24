#pragma once

#include <memory>
#include <optional>
#include <string>

#include "lab1/common/random.hpp"
#include "lab1/tree/binary_search_tree.hpp"
#include "lab1/tree/binary_tree.hpp"
#include "lab1/tree/general_tree.hpp"

namespace lab1 {

/// Kinds of trees that can be created by name.
enum class TreeKind { General, GeneralStd, Binary, Bst };

inline std::optional<TreeKind> parseTreeKind(const std::string& text) {
  if (text == "general") return TreeKind::General;
  if (text == "general-std") return TreeKind::GeneralStd;
  if (text == "binary") return TreeKind::Binary;
  if (text == "bst") return TreeKind::Bst;
  return std::nullopt;
}

/// Creates a tree of the given kind behind the common interface.
template <typename T>
std::unique_ptr<ITree<T>> makeTree(TreeKind kind, std::size_t branchingFactor = 3) {
  switch (kind) {
    case TreeKind::General: return std::make_unique<GeneralTree<T, LinkedChildList>>(branchingFactor);
    case TreeKind::GeneralStd: return std::make_unique<GeneralTree<T, StdChildList>>(branchingFactor);
    case TreeKind::Binary: return std::make_unique<BinaryTree<T>>();
    case TreeKind::Bst: return std::make_unique<BinarySearchTree<T>>();
  }
  throw std::invalid_argument("unknown tree kind");
}

/**
 * @brief Fills a tree with random data until it has `targetSize` elements.
 *
 * Works through the ITree interface for any tree kind and any value type that
 * has a RandomValue specialization. Every new value is attached to a random
 * existing node; trees that refuse explicit placement (BST) or have no room
 * (binary) fall back to insert().
 *
 * @return number of elements that were actually added
 */
template <typename T>
std::size_t fillRandom(ITree<T>& tree, std::size_t targetSize, RandomSource& rng) {
  const std::size_t before = tree.size();
  const std::size_t maxAttempts = targetSize * 50 + 100;
  for (std::size_t attempt = 0; attempt < maxAttempts && tree.size() < targetSize; ++attempt) {
    T value = RandomValue<T>::generate(rng);
    if (tree.empty()) {
      tree.insert(value);
      continue;
    }
    const std::vector<T> existing = tree.toVector(TraversalOrder::PreOrder);
    const T& parent = existing[rng.index(existing.size())];
    if (!tree.addChild(parent, value)) tree.insert(value);
  }
  return tree.size() - before;
}

}  // namespace lab1
