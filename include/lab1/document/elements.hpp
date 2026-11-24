#pragma once

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "lab1/common/random.hpp"
#include "lab1/common/text.hpp"

namespace lab1 {

/// Kinds of document elements.
enum class ElementKind { Title, Section, Paragraph, List, Table, Image, Formula, Source };

/// Lower-case name of the kind ("section", "paragraph", ...).
std::string kindName(ElementKind kind);

/**
 * @brief Abstract element of a text document (dynamic polymorphism).
 *
 * The document is a tree of such elements.
 */
class DocumentElement {
 public:
  virtual ~DocumentElement() = default;

  virtual ElementKind kind() const = 0;
  /// Short text shown in outlines and reports.
  virtual std::string describe() const = 0;
  /// Full identity string; two elements are equal when their keys are equal.
  virtual std::string key() const { return describe(); }
  virtual std::unique_ptr<DocumentElement> clone() const = 0;
  /// Whether `child` may be placed under this element (leaf elements: no).
  virtual bool canContain(const DocumentElement& child) const;

 protected:
  DocumentElement() = default;
  DocumentElement(const DocumentElement&) = default;
  DocumentElement& operator=(const DocumentElement&) = default;

  /// Cuts text to `maxLength` characters and appends "..." when shortened.
  static std::string shorten(const std::string& text, std::size_t maxLength);
};

/// Title page: document title and author. Root of every document.
class DocumentTitle final : public DocumentElement {
 public:
  explicit DocumentTitle(std::string title, std::string author = "")
      : title_(std::move(title)), author_(std::move(author)) {}

  const std::string& title() const { return title_; }
  const std::string& author() const { return author_; }

  ElementKind kind() const override { return ElementKind::Title; }
  std::string describe() const override;
  std::unique_ptr<DocumentElement> clone() const override {
    return std::make_unique<DocumentTitle>(*this);
  }
  bool canContain(const DocumentElement& child) const override;

 private:
  std::string title_;
  std::string author_;
};

/// Numbered section; `level` is the heading level (1 = chapter, 2 = subsection, ...).
class Section final : public DocumentElement {
 public:
  Section(std::string heading, std::size_t level)
      : heading_(std::move(heading)), level_(level) {}

  const std::string& heading() const { return heading_; }
  std::size_t level() const { return level_; }

  ElementKind kind() const override { return ElementKind::Section; }
  std::string describe() const override;
  std::unique_ptr<DocumentElement> clone() const override { return std::make_unique<Section>(*this); }
  bool canContain(const DocumentElement& child) const override;

 private:
  std::string heading_;
  std::size_t level_;
};

/// Paragraph of text that consists of sentences and may cite sources as [n].
class Paragraph final : public DocumentElement {
 public:
  explicit Paragraph(std::string text) : text_(std::move(text)) {}

  const std::string& text() const { return text_; }
  /// Splits the text into sentences (terminators: . ! ? followed by a space or the end).
  std::vector<std::string> sentences() const;
  /// Numbers n of all citations written as [n].
  std::vector<int> citations() const;

  ElementKind kind() const override { return ElementKind::Paragraph; }
  std::string describe() const override;
  std::string key() const override;
  std::unique_ptr<DocumentElement> clone() const override {
    return std::make_unique<Paragraph>(*this);
  }

 private:
  std::string text_;
};

/// Bulleted or numbered list.
class ListBlock final : public DocumentElement {
 public:
  ListBlock(std::vector<std::string> items, bool ordered)
      : items_(std::move(items)), ordered_(ordered) {}

  const std::vector<std::string>& items() const { return items_; }
  bool ordered() const { return ordered_; }

  ElementKind kind() const override { return ElementKind::List; }
  std::string describe() const override;
  std::string key() const override;
  std::unique_ptr<DocumentElement> clone() const override { return std::make_unique<ListBlock>(*this); }

 private:
  std::vector<std::string> items_;
  bool ordered_;
};

/// Table with a caption.
class TableBlock final : public DocumentElement {
 public:
  TableBlock(std::string caption, std::size_t rows, std::size_t columns)
      : caption_(std::move(caption)), rows_(rows), columns_(columns) {}

  const std::string& caption() const { return caption_; }
  std::size_t rows() const { return rows_; }
  std::size_t columns() const { return columns_; }

  ElementKind kind() const override { return ElementKind::Table; }
  std::string describe() const override;
  std::unique_ptr<DocumentElement> clone() const override { return std::make_unique<TableBlock>(*this); }

 private:
  std::string caption_;
  std::size_t rows_;
  std::size_t columns_;
};

/// Image with a caption.
class ImageBlock final : public DocumentElement {
 public:
  ImageBlock(std::string path, std::string caption)
      : path_(std::move(path)), caption_(std::move(caption)) {}

  const std::string& path() const { return path_; }
  const std::string& caption() const { return caption_; }

  ElementKind kind() const override { return ElementKind::Image; }
  std::string describe() const override;
  std::unique_ptr<DocumentElement> clone() const override { return std::make_unique<ImageBlock>(*this); }

 private:
  std::string path_;
  std::string caption_;
};

/// Numbered formula.
class FormulaBlock final : public DocumentElement {
 public:
  FormulaBlock(std::string expression, int number)
      : expression_(std::move(expression)), number_(number) {}

  const std::string& expression() const { return expression_; }
  int number() const { return number_; }

  ElementKind kind() const override { return ElementKind::Formula; }
  std::string describe() const override;
  std::unique_ptr<DocumentElement> clone() const override {
    return std::make_unique<FormulaBlock>(*this);
  }

 private:
  std::string expression_;
  int number_;
};

/// Entry of the list of references.
class SourceEntry final : public DocumentElement {
 public:
  SourceEntry(int number, std::string description)
      : number_(number), description_(std::move(description)) {}

  int number() const { return number_; }
  const std::string& description() const { return description_; }

  ElementKind kind() const override { return ElementKind::Source; }
  std::string describe() const override;
  std::unique_ptr<DocumentElement> clone() const override { return std::make_unique<SourceEntry>(*this); }

 private:
  int number_;
  std::string description_;
};

/**
 * @brief Copyable value wrapper around a polymorphic DocumentElement.
 *
 * Lets the element hierarchy be stored in the generic trees, which require
 * value semantics, ==, < and a text representation.
 */
class ElementHandle {
 public:
  explicit ElementHandle(std::unique_ptr<DocumentElement> element);
  ElementHandle(const ElementHandle& other);
  ElementHandle(ElementHandle&&) noexcept = default;
  ElementHandle& operator=(const ElementHandle& other);
  ElementHandle& operator=(ElementHandle&&) noexcept = default;
  ~ElementHandle() = default;

  /// Creates a handle that owns a new element of type E.
  template <typename E, typename... Args>
  static ElementHandle make(Args&&... args) {
    return ElementHandle(std::make_unique<E>(std::forward<Args>(args)...));
  }

  const DocumentElement& element() const { return *element_; }

  /// The element as concrete type E or nullptr if it has another type.
  template <typename E>
  const E* as() const {
    return dynamic_cast<const E*>(element_.get());
  }

  std::string toString() const { return element_->describe(); }

  friend bool operator==(const ElementHandle& a, const ElementHandle& b) {
    return a.element_->key() == b.element_->key();
  }
  friend bool operator!=(const ElementHandle& a, const ElementHandle& b) { return !(a == b); }
  friend bool operator<(const ElementHandle& a, const ElementHandle& b) {
    return a.element_->key() < b.element_->key();
  }

 private:
  std::unique_ptr<DocumentElement> element_;
};

/// Random document element (static polymorphism: specialization for own class).
template <>
struct RandomValue<ElementHandle, void> {
  static ElementHandle generate(RandomSource& rng);
};

/**
 * @brief Parses a textual element description; '_' stands for a space.
 *
 * Formats: title:Name[:Author], section:Level:Heading, paragraph:Text,
 * list:a;b;c, table:Caption[:Rows:Cols], image:Path:Caption,
 * formula:Number:Expression, source:Number:Description.
 *
 * @throws std::invalid_argument for malformed input
 */
ElementHandle parseElement(const std::string& spec);

}  // namespace lab1
