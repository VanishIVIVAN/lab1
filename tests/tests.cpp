// Minimal self-contained unit tests (no external framework needed).
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "lab1/cli/shell.hpp"
#include "lab1/document/rules.hpp"
#include "lab1/tree/tree_factory.hpp"

using namespace lab1;

namespace {

int g_checks = 0;
int g_failures = 0;

#define CHECK(condition)                                                              \
  do {                                                                                \
    ++g_checks;                                                                       \
    if (!(condition)) {                                                               \
      ++g_failures;                                                                   \
      std::cerr << __FILE__ << ':' << __LINE__ << "  CHECK failed: " #condition "\n"; \
    }                                                                                 \
  } while (0)

using Ints = std::vector<int>;
using TestFunction = void (*)();
std::vector<std::pair<std::string, TestFunction>>& registry() {
  static std::vector<std::pair<std::string, TestFunction>> tests;
  return tests;
}
struct Registrar {
  Registrar(const char* name, TestFunction function) { registry().emplace_back(name, function); }
};
#define TEST(name)                         \
  static void name();                      \
  static Registrar registrar_##name(#name, name); \
  static void name()

template <typename Tree>
void fillOneToSeven(Tree& tree) {
  for (int v = 1; v <= 7; ++v) CHECK(tree.insert(v));
}

// ---- general trees: both child storages (static polymorphism) ---------------

template <template <typename> class Storage>
void checkGeneralTree() {
  GeneralTree<int, Storage> tree(2);
  CHECK(tree.empty());
  CHECK(tree.height() == 0);
  fillOneToSeven(tree);
  CHECK(tree.size() == 7);
  CHECK(tree.height() == 3);
  CHECK(tree.toVector(TraversalOrder::PreOrder) == (Ints{1, 2, 4, 5, 3, 6, 7}));
  CHECK(tree.toVector(TraversalOrder::PostOrder) == (Ints{4, 5, 2, 6, 7, 3, 1}));
  CHECK(tree.toVector(TraversalOrder::LevelOrder) == (Ints{1, 2, 3, 4, 5, 6, 7}));
  CHECK(tree.toVector(TraversalOrder::InOrder) == (Ints{4, 2, 5, 1, 6, 3, 7}));

  CHECK(tree.contains(6));
  CHECK(!tree.contains(42));
  CHECK((tree.pathTo(5) == std::optional<Ints>(Ints{1, 2, 5})));
  CHECK(!tree.pathTo(42).has_value());
  CHECK(tree.hasPath({1, 2, 5}));
  CHECK(!tree.hasPath({1, 3, 5}));
  CHECK(!tree.hasPath({2, 5}));  // must start in the root
  CHECK(!tree.hasPath({}));

  CHECK(tree.addChild(7, 8));
  CHECK(tree.addChild(7, 9));
  CHECK(tree.addChild(7, 10));  // addChild ignores the branching factor
  CHECK(!tree.addChild(99, 1));
  CHECK(tree.size() == 10);
  CHECK((tree.pathTo(10) == std::optional<Ints>(Ints{1, 3, 7, 10})));

  CHECK(tree.removeChild(3, 1));  // removes 7 with its three children
  CHECK(tree.size() == 6);
  CHECK(!tree.contains(7));
  CHECK(!tree.removeChild(3, 5));
  CHECK(!tree.removeChild(99, 0));
  CHECK(tree.removeValue(2));  // removes 2, 4 and 5
  CHECK(tree.toVector(TraversalOrder::PreOrder) == (Ints{1, 3, 6}));
  CHECK(!tree.removeValue(2));
  CHECK(tree.removeValue(1));  // removing the root empties the tree
  CHECK(tree.empty());
  CHECK(tree.toString().find("empty") != std::string::npos);
}

TEST(general_tree_linked_list) { checkGeneralTree<LinkedChildList>(); }
TEST(general_tree_std_list) { checkGeneralTree<StdChildList>(); }

TEST(general_tree_rejects_zero_branching) {
  bool thrown = false;
  try {
    GeneralTree<int> tree(0);
  } catch (const std::invalid_argument&) {
    thrown = true;
  }
  CHECK(thrown);
}

TEST(linked_child_list_operations) {
  LinkedChildList<int> list;
  for (int v : {10, 20, 30}) list.append(std::make_unique<int>(v));
  CHECK(list.size() == 3);
  CHECK(*list.at(1) == 20);
  CHECK(list.at(3) == nullptr);
  CHECK(*list.removeAt(2) == 30);  // tail removal
  list.append(std::make_unique<int>(40));
  CHECK(*list.at(2) == 40);
  CHECK(*list.removeAt(0) == 10);  // head removal
  CHECK(*list.at(0) == 20);
  CHECK(list.removeAt(7) == nullptr);
}

// ---- binary tree -----------------------------------------------------------

TEST(binary_tree_complete_fill_and_edit) {
  BinaryTree<int> tree;
  fillOneToSeven(tree);
  CHECK(tree.toVector(TraversalOrder::PreOrder) == (Ints{1, 2, 4, 5, 3, 6, 7}));
  CHECK(tree.toVector(TraversalOrder::InOrder) == (Ints{4, 2, 5, 1, 6, 3, 7}));
  CHECK(!tree.addChild(2, 9));  // both slots of 2 are taken
  CHECK(tree.removeChild(2, 0));  // removes 4
  CHECK(tree.addChild(2, 9));     // goes to the free left slot
  CHECK(tree.toVector(TraversalOrder::InOrder) == (Ints{9, 2, 5, 1, 6, 3, 7}));
  CHECK(!tree.removeChild(2, 2));
  CHECK(tree.toString().find("L: ") != std::string::npos);
}

TEST(in_order_handles_missing_left_child) {
  BinaryTree<int> tree;
  tree.insert(1);
  tree.insert(2);
  tree.insert(3);
  CHECK(tree.removeChild(1, 0));  // only the right child 3 remains
  CHECK(tree.toVector(TraversalOrder::InOrder) == (Ints{1, 3}));
  CHECK(tree.toVector(TraversalOrder::PreOrder) == (Ints{1, 3}));
}

// ---- binary search tree ------------------------------------------------------

BinarySearchTree<int> makeBst() {
  BinarySearchTree<int> tree;
  for (int v : {5, 3, 8, 1, 4, 7, 9}) tree.insert(v);
  return tree;
}

TEST(bst_orders_and_search) {
  BinarySearchTree<int> tree = makeBst();
  CHECK(tree.toVector(TraversalOrder::InOrder) == (Ints{1, 3, 4, 5, 7, 8, 9}));
  CHECK(tree.toVector(TraversalOrder::PreOrder) == (Ints{5, 3, 1, 4, 8, 7, 9}));
  CHECK(tree.toVector(TraversalOrder::PostOrder) == (Ints{1, 4, 3, 7, 9, 8, 5}));
  CHECK(tree.toVector(TraversalOrder::LevelOrder) == (Ints{5, 3, 8, 1, 4, 7, 9}));
  CHECK(!tree.insert(4));  // duplicates are rejected
  CHECK((tree.pathTo(7) == std::optional<Ints>(Ints{5, 8, 7})));
  CHECK(!tree.pathTo(6).has_value());
  CHECK(tree.contains(9));
  CHECK(!tree.addChild(5, 6));  // BST chooses positions itself
}

TEST(bst_removal_cases) {
  BinarySearchTree<int> leaf = makeBst();
  CHECK(leaf.removeValue(1));
  CHECK(leaf.toVector(TraversalOrder::InOrder) == (Ints{3, 4, 5, 7, 8, 9}));

  BinarySearchTree<int> oneChild = makeBst();
  oneChild.removeValue(7);
  CHECK(oneChild.removeValue(8));  // 8 has only the right child 9 now
  CHECK(oneChild.toVector(TraversalOrder::InOrder) == (Ints{1, 3, 4, 5, 9}));

  BinarySearchTree<int> twoChildren = makeBst();
  CHECK(twoChildren.removeValue(5));  // root with two children
  CHECK(twoChildren.toVector(TraversalOrder::PreOrder) == (Ints{7, 3, 1, 4, 8, 9}));
  CHECK(twoChildren.toVector(TraversalOrder::InOrder) == (Ints{1, 3, 4, 7, 8, 9}));
  CHECK(!twoChildren.removeValue(100));

  BinarySearchTree<int> single;
  single.insert(1);
  CHECK(single.removeValue(1));
  CHECK(single.empty());
}

TEST(bst_works_with_custom_comparator) {
  BinarySearchTree<int, std::greater<int>> tree;
  for (int v : {2, 9, 4}) tree.insert(v);
  CHECK(tree.toVector(TraversalOrder::InOrder) == (Ints{9, 4, 2}));
}

// ---- other value types ------------------------------------------------------------

TEST(trees_of_strings_doubles_vectors) {
  BinarySearchTree<std::string> words;
  for (const char* w : {"pear", "apple", "zebra"}) words.insert(w);
  CHECK(words.toVector(TraversalOrder::InOrder) ==
        (std::vector<std::string>{"apple", "pear", "zebra"}));
  CHECK(words.toString().find("\"pear\"") != std::string::npos);

  GeneralTree<double> numbers;
  numbers.insert(1.5);
  numbers.addChild(1.5, 2.25);
  CHECK(numbers.contains(2.25));

  BinarySearchTree<std::vector<int>> vectors;
  vectors.insert({1, 2});
  vectors.insert({1, 1, 5});
  CHECK(vectors.toVector(TraversalOrder::InOrder).front() == (Ints{1, 1, 5}));
  CHECK(vectors.toString().find("[1, 2]") != std::string::npos);
}

// ---- random generation --------------------------------------------------------------

template <typename T>
void checkRandomFill() {
  for (TreeKind kind : {TreeKind::General, TreeKind::GeneralStd, TreeKind::Binary, TreeKind::Bst}) {
    RandomSource rng(7);
    auto tree = makeTree<T>(kind, 2);
    fillRandom(*tree, 12, rng);
    CHECK(tree->size() == 12);
    CHECK(tree->height() >= 1);
  }
}

TEST(random_fill_all_kinds_and_types) {
  checkRandomFill<int>();
  checkRandomFill<double>();
  checkRandomFill<std::string>();
  checkRandomFill<std::vector<int>>();
  checkRandomFill<ElementHandle>();
}

TEST(random_fill_is_reproducible) {
  RandomSource a(5), b(5);
  auto first = makeTree<int>(TreeKind::General, 3);
  auto second = makeTree<int>(TreeKind::General, 3);
  fillRandom(*first, 10, a);
  fillRandom(*second, 10, b);
  CHECK(first->toString() == second->toString());
}

// ---- document model and rules ----------------------------------------------------------

TEST(element_parsing) {
  CHECK(parseElement("section:2:Hello_world").as<Section>()->heading() == "Hello world");
  CHECK(parseElement("section:2:Hello_world").as<Section>()->level() == 2);
  CHECK(parseElement("paragraph:Time_is_12:30.").as<Paragraph>()->text() == "Time is 12:30.");
  CHECK(parseElement("list:a;b;c").as<ListBlock>()->items().size() == 3);
  CHECK(parseElement("table:T:2:3").as<TableBlock>()->columns() == 3);
  for (const char* bad : {"nonsense:x", "section:x:y", "section:0:y", "section:1", "formula:-1:x"}) {
    bool thrown = false;
    try {
      parseElement(bad);
    } catch (const std::invalid_argument&) {
      thrown = true;
    }
    CHECK(thrown);
  }
}

TEST(paragraph_sentences_and_citations) {
  const Paragraph p("First one. Second [1] and [22]! Third 3.14 here?");
  CHECK(p.sentences().size() == 3);
  CHECK(p.citations() == (Ints{1, 22}));
  CHECK(Paragraph("[x] [] [12").citations().empty());
}

TEST(handle_has_value_semantics) {
  const ElementHandle a = ElementHandle::make<Section>("A", 1);
  ElementHandle copy = a;
  CHECK(copy == a);
  copy = ElementHandle::make<Section>("B", 1);
  CHECK(copy != a);
  CHECK(a < copy);
}

TEST(document_structure_rules) {
  const ElementHandle title = ElementHandle::make<DocumentTitle>("T");
  Document doc(title);
  const ElementHandle paragraph = ElementHandle::make<Paragraph>("Text.");
  CHECK(doc.add(title, paragraph));
  CHECK(!doc.add(paragraph, ElementHandle::make<Paragraph>("Nested.")));  // leaf
  CHECK(!doc.add(title, title));                                          // second title
  CHECK(!doc.add(ElementHandle::make<Section>("Missing", 1), paragraph));  // unknown parent
  bool thrown = false;
  try {
    Document bad(paragraph);
  } catch (const std::invalid_argument&) {
    thrown = true;
  }
  CHECK(thrown);
}

TEST(correct_document_has_no_violations) {
  const Document doc = makeSampleDocument(false);
  const ValidationReport report = DocumentValidator::withStandardRules().validate(doc);
  CHECK(report.empty());
  CHECK(doc.tableOfContents() == "1. Introduction\n  1.1. Goals\n2. Results\n3. References\n");
}

TEST(document_with_mistakes_is_reported) {
  const Document doc = makeSampleDocument(true);
  const DocumentValidator validator = DocumentValidator::withStandardRules();
  CHECK(validator.ruleCount() == 6);
  const ValidationReport report = validator.validate(doc);
  CHECK(report.hasErrors());
  CHECK(report.countForRule("HeadingLevelRule") == 1);
  CHECK(report.countForRule("EmptySectionRule") == 1);
  CHECK(report.countForRule("CaptionRule") == 1);
  CHECK(report.countForRule("FormulaNumberingRule") == 1);
  CHECK(report.countForRule("ParagraphStyleRule") >= 3);
  CHECK(report.countForRule("CitationRule") == 2);  // [2] without source, source 3 never cited
  CHECK(report.toString().find("Summary:") != std::string::npos);
}

// ---- command line --------------------------------------------------------------------------

TEST(shell_commands) {
  std::istringstream in;
  std::ostringstream out;
  Shell shell(in, out, 1);
  CHECK(shell.handle(tokenize("new bst int")));
  CHECK(shell.handle(tokenize("insert 5")));
  CHECK(shell.handle(tokenize("insert 3")));
  CHECK(shell.handle(tokenize("traverse in")));
  CHECK(out.str().find("3, 5") != std::string::npos);
  CHECK(!shell.handle(tokenize("quit")));
  bool thrown = false;
  try {
    shell.handle(tokenize("insert abc"));
  } catch (const std::invalid_argument&) {
    thrown = true;
  }
  CHECK(thrown);
  thrown = false;
  try {
    Shell fresh(in, out, 1);
    fresh.handle(tokenize("insert 1"));  // no tree yet
  } catch (const std::invalid_argument&) {
    thrown = true;
  }
  CHECK(thrown);
}

}  // namespace

int main() {
  for (const auto& [name, function] : registry()) {
    const int failuresBefore = g_failures;
    function();
    std::cout << (g_failures == failuresBefore ? "[ OK ] " : "[FAIL] ") << name << '\n';
  }
  std::cout << g_checks << " checks, " << g_failures << " failure(s)\n";
  return g_failures == 0 ? 0 : 1;
}
