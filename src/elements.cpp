#include "lab1/document/elements.hpp"

#include <algorithm>
#include <cctype>

namespace lab1 {

namespace {

std::string trim(const std::string& text) {
  std::size_t begin = 0;
  std::size_t end = text.size();
  while (begin < end && std::isspace(static_cast<unsigned char>(text[begin]))) ++begin;
  while (end > begin && std::isspace(static_cast<unsigned char>(text[end - 1]))) --end;
  return text.substr(begin, end - begin);
}

std::vector<std::string> split(const std::string& text, char separator) {
  std::vector<std::string> parts;
  std::string current;
  for (char c : text) {
    if (c == separator) {
      parts.push_back(current);
      current.clear();
    } else {
      current += c;
    }
  }
  parts.push_back(current);
  return parts;
}

std::string join(const std::vector<std::string>& parts, std::size_t from, char separator) {
  std::string result;
  for (std::size_t i = from; i < parts.size(); ++i) {
    if (i > from) result += separator;
    result += parts[i];
  }
  return result;
}

std::string unscore(std::string text) {
  std::replace(text.begin(), text.end(), '_', ' ');
  return text;
}

int parseNumber(const std::string& text, const std::string& what) {
  std::size_t consumed = 0;
  int value = 0;
  try {
    value = std::stoi(text, &consumed);
  } catch (const std::exception&) {
    throw std::invalid_argument(what + " must be an integer, got: " + text);
  }
  if (consumed != text.size() || value < 0) {
    throw std::invalid_argument(what + " must be a non-negative integer, got: " + text);
  }
  return value;
}

}  // namespace

std::string kindName(ElementKind kind) {
  switch (kind) {
    case ElementKind::Title: return "title";
    case ElementKind::Section: return "section";
    case ElementKind::Paragraph: return "paragraph";
    case ElementKind::List: return "list";
    case ElementKind::Table: return "table";
    case ElementKind::Image: return "image";
    case ElementKind::Formula: return "formula";
    case ElementKind::Source: return "source";
  }
  return "?";
}

// ---- DocumentElement -------------------------------------------------------

bool DocumentElement::canContain(const DocumentElement&) const { return false; }

std::string DocumentElement::shorten(const std::string& text, std::size_t maxLength) {
  if (text.size() <= maxLength) return text;
  return text.substr(0, maxLength) + "...";
}

// ---- DocumentTitle / Section ----------------------------------------------

std::string DocumentTitle::describe() const {
  std::string result = "Title \"" + title_ + "\"";
  if (!author_.empty()) result += " by " + author_;
  return result;
}

bool DocumentTitle::canContain(const DocumentElement& child) const {
  return child.kind() != ElementKind::Title;
}

std::string Section::describe() const {
  return "Section L" + std::to_string(level_) + ": " + heading_;
}

bool Section::canContain(const DocumentElement& child) const {
  return child.kind() != ElementKind::Title;
}

// ---- Paragraph -------------------------------------------------------------

std::vector<std::string> Paragraph::sentences() const {
  std::vector<std::string> result;
  std::string current;
  for (std::size_t i = 0; i < text_.size(); ++i) {
    const char c = text_[i];
    current += c;
    const bool terminator = c == '.' || c == '!' || c == '?';
    const bool boundary = terminator && (i + 1 == text_.size() || text_[i + 1] == ' ');
    if (boundary) {
      const std::string sentence = trim(current);
      if (!sentence.empty()) result.push_back(sentence);
      current.clear();
    }
  }
  const std::string rest = trim(current);
  if (!rest.empty()) result.push_back(rest);
  return result;
}

std::vector<int> Paragraph::citations() const {
  std::vector<int> result;
  for (std::size_t i = 0; i < text_.size(); ++i) {
    if (text_[i] != '[') continue;
    std::size_t j = i + 1;
    while (j < text_.size() && std::isdigit(static_cast<unsigned char>(text_[j]))) ++j;
    const std::size_t digits = j - i - 1;
    if (digits > 0 && digits <= 6 && j < text_.size() && text_[j] == ']') {
      result.push_back(std::stoi(text_.substr(i + 1, digits)));
      i = j;
    }
  }
  return result;
}

std::string Paragraph::describe() const { return "Paragraph: \"" + shorten(text_, 40) + "\""; }

std::string Paragraph::key() const { return "Paragraph: \"" + text_ + "\""; }

// ---- other elements --------------------------------------------------------

std::string ListBlock::describe() const {
  return std::string("List (") + (ordered_ ? "ordered" : "unordered") + ", " +
         std::to_string(items_.size()) + " items)";
}

std::string ListBlock::key() const {
  std::string result = describe();
  for (const std::string& item : items_) result += "|" + item;
  return result;
}

std::string TableBlock::describe() const {
  return "Table \"" + caption_ + "\" " + std::to_string(rows_) + "x" + std::to_string(columns_);
}

std::string ImageBlock::describe() const { return "Image " + path_ + " \"" + caption_ + "\""; }

std::string FormulaBlock::describe() const {
  return "Formula (" + std::to_string(number_) + "): " + expression_;
}

std::string SourceEntry::describe() const {
  return "Source [" + std::to_string(number_) + "]: " + description_;
}

// ---- ElementHandle ---------------------------------------------------------

ElementHandle::ElementHandle(std::unique_ptr<DocumentElement> element)
    : element_(std::move(element)) {
  if (!element_) throw std::invalid_argument("ElementHandle requires an element");
}

ElementHandle::ElementHandle(const ElementHandle& other)
    : element_(other.element_ ? other.element_->clone() : nullptr) {}

ElementHandle& ElementHandle::operator=(const ElementHandle& other) {
  if (this != &other) element_ = other.element_ ? other.element_->clone() : nullptr;
  return *this;
}

// ---- random generation and parsing ----------------------------------------

ElementHandle RandomValue<ElementHandle, void>::generate(RandomSource& rng) {
  switch (rng.intInRange(0, 6)) {
    case 0:
      return ElementHandle::make<Section>(rng.capitalizedWord(4, 9),
                                          static_cast<std::size_t>(rng.intInRange(1, 3)));
    case 1:
      return ElementHandle::make<Paragraph>(rng.capitalizedWord(3, 8) + " " + rng.word(3, 8) +
                                            " " + rng.word(3, 8) + ".");
    case 2:
      return ElementHandle::make<ListBlock>(
          std::vector<std::string>{rng.word(3, 6), rng.word(3, 6), rng.word(3, 6)},
          rng.intInRange(0, 1) == 1);
    case 3:
      return ElementHandle::make<TableBlock>(rng.capitalizedWord(4, 8),
                                             static_cast<std::size_t>(rng.intInRange(1, 5)),
                                             static_cast<std::size_t>(rng.intInRange(1, 5)));
    case 4:
      return ElementHandle::make<ImageBlock>(rng.word(3, 6) + ".png", rng.capitalizedWord(4, 8));
    case 5:
      return ElementHandle::make<FormulaBlock>("x = " + std::to_string(rng.intInRange(1, 99)),
                                               rng.intInRange(1, 9));
    default:
      return ElementHandle::make<SourceEntry>(rng.intInRange(1, 9), rng.capitalizedWord(5, 10));
  }
}

ElementHandle parseElement(const std::string& spec) {
  const std::vector<std::string> parts = split(spec, ':');
  const std::string& kind = parts[0];
  auto require = [&](std::size_t count) {
    if (parts.size() < count) {
      throw std::invalid_argument("element '" + kind + "' needs more fields (see help)");
    }
  };

  if (kind == "title") {
    require(2);
    return ElementHandle::make<DocumentTitle>(unscore(parts[1]),
                                              parts.size() > 2 ? unscore(parts[2]) : "");
  }
  if (kind == "section") {
    require(3);
    const int level = parseNumber(parts[1], "section level");
    if (level < 1) throw std::invalid_argument("section level must be at least 1");
    return ElementHandle::make<Section>(unscore(join(parts, 2, ':')),
                                        static_cast<std::size_t>(level));
  }
  if (kind == "paragraph") {
    require(2);
    return ElementHandle::make<Paragraph>(unscore(join(parts, 1, ':')));
  }
  if (kind == "list") {
    require(2);
    std::vector<std::string> items = split(parts[1], ';');
    for (std::string& item : items) item = unscore(item);
    return ElementHandle::make<ListBlock>(std::move(items), false);
  }
  if (kind == "table") {
    require(2);
    const int rows = parts.size() > 2 ? parseNumber(parts[2], "table rows") : 1;
    const int columns = parts.size() > 3 ? parseNumber(parts[3], "table columns") : 1;
    return ElementHandle::make<TableBlock>(unscore(parts[1]), static_cast<std::size_t>(rows),
                                           static_cast<std::size_t>(columns));
  }
  if (kind == "image") {
    require(3);
    return ElementHandle::make<ImageBlock>(parts[1], unscore(join(parts, 2, ':')));
  }
  if (kind == "formula") {
    require(3);
    return ElementHandle::make<FormulaBlock>(unscore(join(parts, 2, ':')),
                                             parseNumber(parts[1], "formula number"));
  }
  if (kind == "source") {
    require(3);
    return ElementHandle::make<SourceEntry>(parseNumber(parts[1], "source number"),
                                            unscore(join(parts, 2, ':')));
  }
  throw std::invalid_argument("unknown element kind: " + kind);
}

}  // namespace lab1
