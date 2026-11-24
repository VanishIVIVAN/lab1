#pragma once

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "lab1/common/text.hpp"
#include "lab1/document/elements.hpp"
#include "lab1/tree/tree_factory.hpp"

namespace lab1 {

/**
 * @brief Static polymorphism: parsing of command-line text into values of type T.
 */
template <typename T>
struct ValueParser;

template <>
struct ValueParser<int> {
  static int parse(const std::string& text) {
    std::size_t consumed = 0;
    int value = 0;
    try {
      value = std::stoi(text, &consumed);
    } catch (const std::exception&) {
      throw std::invalid_argument("not an integer: " + text);
    }
    if (consumed != text.size()) throw std::invalid_argument("not an integer: " + text);
    return value;
  }
};

template <>
struct ValueParser<double> {
  static double parse(const std::string& text) {
    std::size_t consumed = 0;
    double value = 0;
    try {
      value = std::stod(text, &consumed);
    } catch (const std::exception&) {
      throw std::invalid_argument("not a number: " + text);
    }
    if (consumed != text.size()) throw std::invalid_argument("not a number: " + text);
    return value;
  }
};

template <>
struct ValueParser<std::string> {
  static std::string parse(const std::string& text) { return text; }
};

/// Vector of integers written as "1,2,3".
template <>
struct ValueParser<std::vector<int>> {
  static std::vector<int> parse(const std::string& text) {
    std::vector<int> values;
    std::string current;
    for (char c : text + ",") {
      if (c == ',') {
        values.push_back(ValueParser<int>::parse(current));
        current.clear();
      } else {
        current += c;
      }
    }
    return values;
  }
};

template <>
struct ValueParser<ElementHandle> {
  static ElementHandle parse(const std::string& text) { return parseElement(text); }
};

/// Parses a non-negative index.
std::size_t parseIndex(const std::string& text);

/**
 * @brief Type-erased interactive session over one tree (dynamic polymorphism).
 *
 * The shell does not know the element type of the tree it is working with.
 */
class ITreeSession {
 public:
  virtual ~ITreeSession() = default;

  virtual std::string typeName() const = 0;
  /// Executes a tree command (insert, add, remove, ...) and returns its output.
  /// @throws std::invalid_argument for invalid commands or values
  virtual std::string execute(const std::vector<std::string>& args) = 0;
};

/// Session over a tree of values of type T.
template <typename T>
class TreeSession final : public ITreeSession {
 public:
  TreeSession(std::string typeName, std::unique_ptr<ITree<T>> tree, unsigned seed)
      : typeName_(std::move(typeName)), tree_(std::move(tree)), rng_(seed) {}

  std::string typeName() const override { return typeName_; }

  std::string execute(const std::vector<std::string>& args) override {
    const std::string& command = args.at(0);
    if (command == "insert") {
      require(args, 2, "insert <value>");
      return tree_->insert(parse(args[1])) ? "inserted" : "rejected (duplicate value)";
    }
    if (command == "add") {
      require(args, 3, "add <parent> <value>");
      return tree_->addChild(parse(args[1]), parse(args[2]))
                 ? "added"
                 : "failed (parent not found, no free slot, or the tree chooses positions itself)";
    }
    if (command == "remove") {
      require(args, 2, "remove <value>");
      return tree_->removeValue(parse(args[1])) ? "removed" : "not found";
    }
    if (command == "removechild") {
      require(args, 3, "removechild <parent> <index>");
      return tree_->removeChild(parse(args[1]), parseIndex(args[2])) ? "removed"
                                                                      : "no such child";
    }
    if (command == "contains") {
      require(args, 2, "contains <value>");
      return tree_->contains(parse(args[1])) ? "yes" : "no";
    }
    if (command == "path") {
      require(args, 2, "path <value>");
      const auto path = tree_->pathTo(parse(args[1]));
      return path ? joinValues(*path, " -> ") : "not found";
    }
    if (command == "haspath") {
      require(args, 2, "haspath <v1> <v2> ...");
      std::vector<T> path;
      for (std::size_t i = 1; i < args.size(); ++i) path.push_back(parse(args[i]));
      return tree_->hasPath(path) ? "yes" : "no";
    }
    if (command == "traverse") {
      require(args, 2, "traverse <pre|post|level|in>");
      const auto order = parseOrder(args[1]);
      if (!order) throw std::invalid_argument("order must be pre, post, level or in");
      return joinValues(tree_->toVector(*order), ", ");
    }
    if (command == "random") {
      require(args, 2, "random <count> [seed]");
      if (args.size() > 2) rng_ = RandomSource(static_cast<unsigned>(parseIndex(args[2])));
      const std::size_t added = fillRandom(*tree_, parseIndex(args[1]), rng_);
      return "added " + std::to_string(added) + " element(s)\n" + tree_->toString();
    }
    if (command == "print") return tree_->toString();
    if (command == "clear") {
      tree_->clear();
      return "cleared";
    }
    if (command == "info") {
      return tree_->kindName() + " of " + typeName_ + ", size=" + std::to_string(tree_->size()) +
             ", height=" + std::to_string(tree_->height());
    }
    throw std::invalid_argument("unknown command: " + command + " (type 'help')");
  }

 private:
  static T parse(const std::string& text) { return ValueParser<T>::parse(text); }

  static void require(const std::vector<std::string>& args, std::size_t count,
                      const std::string& usage) {
    if (args.size() < count) throw std::invalid_argument("usage: " + usage);
  }

  std::string typeName_;
  std::unique_ptr<ITree<T>> tree_;
  RandomSource rng_;
};

/// Creates a session for the value type (int, double, string, vector, element).
/// @throws std::invalid_argument for an unknown type or tree kind
std::unique_ptr<ITreeSession> makeSession(const std::string& type, const std::string& kind,
                                          std::size_t branchingFactor, unsigned seed);

}  // namespace lab1
