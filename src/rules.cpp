#include "lab1/document/rules.hpp"

#include <cctype>
#include <map>
#include <set>
#include <sstream>

#include "lab1/common/logger.hpp"

namespace lab1 {

std::string severityName(Severity severity) {
  switch (severity) {
    case Severity::Info: return "info";
    case Severity::Warning: return "warning";
    case Severity::Error: return "error";
  }
  return "?";
}

// ---- Violation / ValidationReport -----------------------------------------

std::string Violation::toString() const {
  return "[" + severityName(severity_) + "] " + rule_ + " @ " + location_ + ": " + message_;
}

std::size_t ValidationReport::count(Severity severity) const {
  std::size_t total = 0;
  for (const Violation& violation : violations_) {
    if (violation.severity() == severity) ++total;
  }
  return total;
}

std::size_t ValidationReport::countForRule(const std::string& rule) const {
  std::size_t total = 0;
  for (const Violation& violation : violations_) {
    if (violation.rule() == rule) ++total;
  }
  return total;
}

std::string ValidationReport::toString() const {
  std::ostringstream out;
  for (const Violation& violation : violations_) out << violation.toString() << '\n';
  out << "Summary: " << count(Severity::Error) << " error(s), " << count(Severity::Warning)
      << " warning(s), " << count(Severity::Info) << " info\n";
  return out.str();
}

// ---- ElementRule family ----------------------------------------------------

void ElementRule::check(const Document& document, ValidationReport& report) const {
  document.structure().traverse(
      TraversalOrder::PreOrder,
      [this, &report](const ElementHandle& element, const VisitContext& context) {
        checkElement(element, context, report);
      });
}

void ElementRule::flag(ValidationReport& report, Severity severity, const ElementHandle& element,
                       const std::string& message) const {
  report.add(Violation(name(), severity, element.toString(), message));
}

void HeadingLevelRule::checkElement(const ElementHandle& element, const VisitContext& context,
                                    ValidationReport& report) const {
  const Section* section = element.as<Section>();
  if (section == nullptr || section->level() == context.depth) return;
  flag(report, Severity::Error, element,
       "level " + std::to_string(section->level()) + " but nesting depth is " +
           std::to_string(context.depth));
}

void EmptySectionRule::checkElement(const ElementHandle& element, const VisitContext& context,
                                    ValidationReport& report) const {
  if (element.as<Section>() != nullptr && context.childCount == 0) {
    flag(report, Severity::Warning, element, "section has no content");
  }
}

void CaptionRule::checkElement(const ElementHandle& element, const VisitContext&,
                               ValidationReport& report) const {
  if (const ImageBlock* image = element.as<ImageBlock>()) {
    if (image->caption().empty()) flag(report, Severity::Error, element, "image has no caption");
  } else if (const TableBlock* table = element.as<TableBlock>()) {
    if (table->caption().empty()) flag(report, Severity::Error, element, "table has no caption");
  }
}

void ParagraphStyleRule::checkElement(const ElementHandle& element, const VisitContext&,
                                      ValidationReport& report) const {
  const Paragraph* paragraph = element.as<Paragraph>();
  if (paragraph == nullptr) return;
  const std::string& text = paragraph->text();
  if (text.empty()) {
    flag(report, Severity::Error, element, "paragraph is empty");
    return;
  }
  if (text.find("  ") != std::string::npos) {
    flag(report, Severity::Warning, element, "contains double spaces");
  }
  const char last = text.back();
  if (last != '.' && last != '!' && last != '?') {
    flag(report, Severity::Warning, element, "does not end with sentence-final punctuation");
  }
  for (const std::string& sentence : paragraph->sentences()) {
    const unsigned char first = static_cast<unsigned char>(sentence.front());
    if (first < 0x80 && std::isalpha(first) && !std::isupper(first)) {
      flag(report, Severity::Error, element, "sentence starts with a lowercase letter: " + sentence);
    }
  }
}

// ---- whole-document rules --------------------------------------------------

void CitationRule::check(const Document& document, ValidationReport& report) const {
  const std::vector<ElementHandle> elements = document.structure().toVector(TraversalOrder::PreOrder);
  std::map<int, int> sourceCounts;
  for (const ElementHandle& element : elements) {
    if (const SourceEntry* source = element.as<SourceEntry>()) ++sourceCounts[source->number()];
  }
  std::set<int> cited;
  for (const ElementHandle& element : elements) {
    const Paragraph* paragraph = element.as<Paragraph>();
    if (paragraph == nullptr) continue;
    for (int number : paragraph->citations()) {
      cited.insert(number);
      if (sourceCounts.count(number) == 0) {
        report.add(Violation(name(), Severity::Error, element.toString(),
                             "citation [" + std::to_string(number) + "] has no source"));
      }
    }
  }
  for (const ElementHandle& element : elements) {
    const SourceEntry* source = element.as<SourceEntry>();
    if (source == nullptr) continue;
    if (sourceCounts[source->number()] > 1) {
      report.add(Violation(name(), Severity::Error, element.toString(), "duplicate source number"));
    }
    if (cited.count(source->number()) == 0) {
      report.add(Violation(name(), Severity::Warning, element.toString(), "source is never cited"));
    }
  }
}

void FormulaNumberingRule::check(const Document& document, ValidationReport& report) const {
  int expected = 1;
  for (const ElementHandle& element : document.structure().toVector(TraversalOrder::PreOrder)) {
    const FormulaBlock* formula = element.as<FormulaBlock>();
    if (formula == nullptr) continue;
    if (formula->number() != expected) {
      report.add(Violation(name(), Severity::Error, element.toString(),
                           "expected number " + std::to_string(expected)));
    }
    ++expected;
  }
}

// ---- DocumentValidator -----------------------------------------------------

DocumentValidator& DocumentValidator::addRule(std::unique_ptr<IFormattingRule> rule) {
  rules_.push_back(std::move(rule));
  return *this;
}

DocumentValidator DocumentValidator::withStandardRules() {
  DocumentValidator validator;
  validator.addRule(std::make_unique<HeadingLevelRule>())
      .addRule(std::make_unique<EmptySectionRule>())
      .addRule(std::make_unique<CaptionRule>())
      .addRule(std::make_unique<ParagraphStyleRule>())
      .addRule(std::make_unique<CitationRule>())
      .addRule(std::make_unique<FormulaNumberingRule>());
  return validator;
}

ValidationReport DocumentValidator::validate(const Document& document) const {
  ValidationReport report;
  for (const auto& rule : rules_) {
    Logger::instance().debug("running " + rule->name());
    rule->check(document, report);
  }
  return report;
}

}  // namespace lab1
