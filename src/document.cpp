#include "lab1/document/document.hpp"

#include <sstream>

#include "lab1/common/logger.hpp"

namespace lab1 {

Document::Document(const ElementHandle& title) {
  if (title.as<DocumentTitle>() == nullptr) {
    throw std::invalid_argument("the root of a document must be a DocumentTitle");
  }
  tree_.insert(title);
}

bool Document::add(const ElementHandle& parent, const ElementHandle& child) {
  if (!parent.element().canContain(child.element())) {
    Logger::instance().warning("structure rejected: " + parent.toString() + " cannot contain " +
                               child.toString());
    return false;
  }
  return tree_.addChild(parent, child);
}

std::string Document::tableOfContents() const {
  std::ostringstream out;
  std::vector<std::size_t> counters;
  tree_.traverse(TraversalOrder::PreOrder, [&](const ElementHandle& handle, const VisitContext& context) {
    const Section* section = handle.as<Section>();
    if (section == nullptr || context.depth == 0) return;
    counters.resize(context.depth, 0);
    ++counters[context.depth - 1];
    std::string number;
    for (std::size_t counter : counters) number += std::to_string(counter) + ".";
    out << std::string((context.depth - 1) * 2, ' ') << number << ' ' << section->heading() << '\n';
  });
  return out.str();
}

Document makeSampleDocument(bool withMistakes) {
  using H = ElementHandle;
  const H title = H::make<DocumentTitle>("Lab report on trees", "A. Student");
  Document doc(title);

  const H intro = H::make<Section>("Introduction", 1);
  const H goals = H::make<Section>("Goals", withMistakes ? 3 : 2);
  const H results = H::make<Section>("Results", 1);
  const H references = H::make<Section>("References", 1);
  doc.add(title, intro);
  doc.add(intro, H::make<Paragraph>(withMistakes
                                        ? "trees are everywhere  and described in [2]"
                                        : "Trees are everywhere. They are described in [1]."));
  doc.add(intro, goals);
  doc.add(goals, H::make<Paragraph>("We implement several kinds of trees."));
  doc.add(title, results);
  doc.add(results, H::make<ImageBlock>("tree.png", withMistakes ? "" : "Example of a tree"));
  doc.add(results, H::make<TableBlock>("Timings", 2, 3));
  doc.add(results, H::make<FormulaBlock>("T(n) = O(n)", 1));
  doc.add(results, H::make<FormulaBlock>("H(n) = O(log n)", withMistakes ? 3 : 2));
  doc.add(title, references);
  doc.add(references, H::make<SourceEntry>(withMistakes ? 3 : 1, "Cormen et al. Introduction to Algorithms"));
  if (withMistakes) doc.add(title, H::make<Section>("Appendix", 1));  // empty section
  return doc;
}

}  // namespace lab1
