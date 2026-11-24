#include "lab1/cli/demo.hpp"

#include <ostream>

#include "lab1/common/text.hpp"
#include "lab1/document/document.hpp"
#include "lab1/document/rules.hpp"
#include "lab1/tree/tree_factory.hpp"

namespace lab1 {

namespace {

/// Client code: works with any tree through the ITree interface.
template <typename T>
void exerciseTree(std::ostream& out, ITree<T>& tree, RandomSource& rng) {
  fillRandom(tree, 7, rng);
  out << tree.toString();
  for (TraversalOrder order : {TraversalOrder::PreOrder, TraversalOrder::PostOrder,
                               TraversalOrder::LevelOrder, TraversalOrder::InOrder}) {
    out << "  " << orderName(order) << "-order: "
        << TextTraits<std::vector<T>>::toText(tree.toVector(order)) << '\n';
  }
  const std::vector<T> values = tree.toVector(TraversalOrder::PreOrder);
  if (values.size() < 2) return;
  const T& target = values.back();
  if (const auto path = tree.pathTo(target)) {
    out << "  path to " << TextTraits<T>::toText(target) << ": " << joinValues(*path, " -> ")
        << " (hasPath: " << (tree.hasPath(*path) ? "yes" : "no") << ")\n";
  }
  const std::size_t before = tree.size();
  tree.removeValue(target);
  out << "  removed " << TextTraits<T>::toText(target) << ": size " << before << " -> "
      << tree.size() << "\n\n";
}

template <typename T>
void demonstrate(std::ostream& out, const std::string& typeName, RandomSource& rng) {
  out << "=================== value type: " << typeName << " ===================\n";
  for (TreeKind kind : {TreeKind::General, TreeKind::GeneralStd, TreeKind::Binary, TreeKind::Bst}) {
    const std::unique_ptr<ITree<T>> tree = makeTree<T>(kind, 2);
    exerciseTree(out, *tree, rng);
  }
}

}  // namespace

void runStructuresDemo(std::ostream& out, unsigned seed) {
  RandomSource rng(seed);
  demonstrate<int>(out, "int", rng);
  demonstrate<double>(out, "double", rng);
  demonstrate<std::string>(out, "std::string", rng);
  demonstrate<std::vector<int>>(out, "std::vector<int>", rng);
  demonstrate<ElementHandle>(out, "document element", rng);
}

void runDocumentDemo(std::ostream& out) {
  const DocumentValidator validator = DocumentValidator::withStandardRules();
  for (bool withMistakes : {false, true}) {
    const Document document = makeSampleDocument(withMistakes);
    out << "========== " << (withMistakes ? "document with mistakes" : "correct document")
        << " ==========\n"
        << document.outline() << "\nTable of contents:\n"
        << document.tableOfContents() << "\nValidation:\n"
        << validator.validate(document).toString() << '\n';
  }
}

}  // namespace lab1
