#pragma once

#include <iosfwd>

namespace lab1 {

/// Demonstrates all tree kinds on int, double, string, vector<int> and document elements.
void runStructuresDemo(std::ostream& out, unsigned seed);

/// Builds sample documents and prints outline, table of contents and validation reports.
void runDocumentDemo(std::ostream& out);

}  // namespace lab1
