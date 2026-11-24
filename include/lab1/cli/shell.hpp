#pragma once

#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

#include "lab1/cli/session.hpp"

namespace lab1 {

/// Interactive command-line shell.
class Shell {
 public:
  Shell(std::istream& in, std::ostream& out, unsigned seed) : in_(in), out_(out), seed_(seed) {}

  /// Reads and executes commands until "quit" or end of input.
  void run();

  /// Executes a single command line; returns false when the shell should stop.
  bool handle(const std::vector<std::string>& args);

 private:
  void printHelp() const;

  std::istream& in_;
  std::ostream& out_;
  unsigned seed_;
  std::unique_ptr<ITreeSession> session_;
};

/// Splits a line into whitespace-separated tokens.
std::vector<std::string> tokenize(const std::string& line);

}  // namespace lab1
