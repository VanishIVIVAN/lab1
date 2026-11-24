#pragma once

#include <string>

#include "lab1/document/elements.hpp"
#include "lab1/tree/general_tree.hpp"

namespace lab1 {

/**
 * @brief Text document whose structure is a general tree of elements.
 *
 * The root is always a DocumentTitle; sections, paragraphs, tables, images,
 * formulas and sources are nested below it.
 */
class Document {
 public:
  using StructureTree = GeneralTree<ElementHandle, LinkedChildList>;

  /// @throws std::invalid_argument if `title` is not a DocumentTitle
  explicit Document(const ElementHandle& title);

  /**
   * @brief Appends `child` to the first element equal to `parent`.
   * @return false if the parent does not exist or may not contain the child
   */
  bool add(const ElementHandle& parent, const ElementHandle& child);

  const StructureTree& structure() const { return tree_; }
  std::size_t size() const { return tree_.size(); }

  /// Indented outline of all elements.
  std::string outline() const { return tree_.toString(); }

  /// Table of contents with automatic numbering (1., 1.1., ...) built from the sections.
  std::string tableOfContents() const;

 private:
  StructureTree tree_;
};

/// Sample report; with `withMistakes` it deliberately violates formatting rules.
Document makeSampleDocument(bool withMistakes);

}  // namespace lab1
