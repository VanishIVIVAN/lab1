#include "lab1/cli/session.hpp"

namespace lab1 {

std::size_t parseIndex(const std::string& text) {
  std::size_t consumed = 0;
  unsigned long value = 0;
  try {
    value = std::stoul(text, &consumed);
  } catch (const std::exception&) {
    throw std::invalid_argument("not a non-negative integer: " + text);
  }
  if (consumed != text.size() || text.front() == '-') {
    throw std::invalid_argument("not a non-negative integer: " + text);
  }
  return static_cast<std::size_t>(value);
}

std::unique_ptr<ITreeSession> makeSession(const std::string& type, const std::string& kindName,
                                          std::size_t branchingFactor, unsigned seed) {
  const auto kind = parseTreeKind(kindName);
  if (!kind) {
    throw std::invalid_argument("unknown tree kind: " + kindName +
                                " (general, general-std, binary, bst)");
  }
  if (type == "int") {
    return std::make_unique<TreeSession<int>>(type, makeTree<int>(*kind, branchingFactor), seed);
  }
  if (type == "double") {
    return std::make_unique<TreeSession<double>>(type, makeTree<double>(*kind, branchingFactor), seed);
  }
  if (type == "string") {
    return std::make_unique<TreeSession<std::string>>(type, makeTree<std::string>(*kind, branchingFactor), seed);
  }
  if (type == "vector") {
    return std::make_unique<TreeSession<std::vector<int>>>(type, makeTree<std::vector<int>>(*kind, branchingFactor), seed);
  }
  if (type == "element") {
    return std::make_unique<TreeSession<ElementHandle>>(type, makeTree<ElementHandle>(*kind, branchingFactor), seed);
  }
  throw std::invalid_argument("unknown value type: " + type +
                              " (int, double, string, vector, element)");
}

}  // namespace lab1
