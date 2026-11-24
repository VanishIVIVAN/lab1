#pragma once

#include <memory>
#include <string>
#include <vector>

#include "lab1/document/document.hpp"

namespace lab1 {

enum class Severity { Info, Warning, Error };

std::string severityName(Severity severity);

/// One found violation of a formatting rule.
class Violation {
 public:
  Violation(std::string rule, Severity severity, std::string location, std::string message)
      : rule_(std::move(rule)),
        severity_(severity),
        location_(std::move(location)),
        message_(std::move(message)) {}

  const std::string& rule() const { return rule_; }
  Severity severity() const { return severity_; }
  const std::string& location() const { return location_; }
  const std::string& message() const { return message_; }

  std::string toString() const;

 private:
  std::string rule_;
  Severity severity_;
  std::string location_;
  std::string message_;
};

/// Result of validation: all violations that were found.
class ValidationReport {
 public:
  void add(Violation violation) { violations_.push_back(std::move(violation)); }

  const std::vector<Violation>& violations() const { return violations_; }
  bool empty() const { return violations_.empty(); }
  std::size_t count(Severity severity) const;
  std::size_t countForRule(const std::string& rule) const;
  bool hasErrors() const { return count(Severity::Error) > 0; }

  /// Human readable report with a summary line.
  std::string toString() const;

 private:
  std::vector<Violation> violations_;
};

/// Interface of a formatting rule (dynamic polymorphism).
class IFormattingRule {
 public:
  virtual ~IFormattingRule() = default;

  virtual std::string name() const = 0;
  virtual std::string description() const = 0;
  virtual void check(const Document& document, ValidationReport& report) const = 0;
};

/**
 * @brief Rule that is evaluated for every element independently (Template Method).
 *
 * check() walks the document in pre-order and calls checkElement().
 */
class ElementRule : public IFormattingRule {
 public:
  void check(const Document& document, ValidationReport& report) const final;

 protected:
  virtual void checkElement(const ElementHandle& element, const VisitContext& context,
                            ValidationReport& report) const = 0;

  void flag(ValidationReport& report, Severity severity, const ElementHandle& element,
            const std::string& message) const;
};

/// A section's level must equal its nesting depth (no skipped levels).
class HeadingLevelRule final : public ElementRule {
 public:
  std::string name() const override { return "HeadingLevelRule"; }
  std::string description() const override { return "Section level must match nesting depth"; }

 protected:
  void checkElement(const ElementHandle& element, const VisitContext& context,
                    ValidationReport& report) const override;
};

/// A section must contain at least one element.
class EmptySectionRule final : public ElementRule {
 public:
  std::string name() const override { return "EmptySectionRule"; }
  std::string description() const override { return "Section must not be empty"; }

 protected:
  void checkElement(const ElementHandle& element, const VisitContext& context,
                    ValidationReport& report) const override;
};

/// Images and tables must have a caption.
class CaptionRule final : public ElementRule {
 public:
  std::string name() const override { return "CaptionRule"; }
  std::string description() const override { return "Images and tables need captions"; }

 protected:
  void checkElement(const ElementHandle& element, const VisitContext& context,
                    ValidationReport& report) const override;
};

/// Paragraph style: sentence capitalisation, final punctuation, no double spaces.
class ParagraphStyleRule final : public ElementRule {
 public:
  std::string name() const override { return "ParagraphStyleRule"; }
  std::string description() const override {
    return "Sentences start with a capital letter, paragraphs end with punctuation, no double spaces";
  }

 protected:
  void checkElement(const ElementHandle& element, const VisitContext& context,
                    ValidationReport& report) const override;
};

/// Every [n] citation needs a source n; unused and duplicated sources are reported.
class CitationRule final : public IFormattingRule {
 public:
  std::string name() const override { return "CitationRule"; }
  std::string description() const override { return "Citations and sources must correspond"; }
  void check(const Document& document, ValidationReport& report) const override;
};

/// Formulas must be numbered 1, 2, 3, ... in document order.
class FormulaNumberingRule final : public IFormattingRule {
 public:
  std::string name() const override { return "FormulaNumberingRule"; }
  std::string description() const override { return "Formulas are numbered consecutively"; }
  void check(const Document& document, ValidationReport& report) const override;
};

/// Runs a configurable set of rules over a document.
class DocumentValidator {
 public:
  DocumentValidator& addRule(std::unique_ptr<IFormattingRule> rule);
  std::size_t ruleCount() const { return rules_.size(); }

  /// Validator with all six rules of this module.
  static DocumentValidator withStandardRules();

  ValidationReport validate(const Document& document) const;

 private:
  std::vector<std::unique_ptr<IFormattingRule>> rules_;
};

}  // namespace lab1
