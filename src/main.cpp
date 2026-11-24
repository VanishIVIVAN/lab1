#include <iostream>
#include <string>

#include "lab1/cli/demo.hpp"
#include "lab1/cli/shell.hpp"
#include "lab1/common/logger.hpp"

namespace {

void printUsage() {
  std::cout << "Usage: lab1 [shell|demo|docdemo] [--seed N] [--verbose]\n"
               "  shell    interactive command line (default)\n"
               "  demo     demonstration of all trees on all value types\n"
               "  docdemo  document model and formatting rules\n";
}

}  // namespace

int main(int argc, char** argv) {
  std::string mode = "shell";
  unsigned seed = 42;
  try {
    for (int i = 1; i < argc; ++i) {
      const std::string arg = argv[i];
      if (arg == "--verbose") {
        lab1::Logger::instance().setLevel(lab1::LogLevel::Debug);
      } else if (arg == "--seed" && i + 1 < argc) {
        seed = static_cast<unsigned>(std::stoul(argv[++i]));
      } else if (arg == "shell" || arg == "demo" || arg == "docdemo") {
        mode = arg;
      } else {
        printUsage();
        return arg == "--help" ? 0 : 1;
      }
    }
    if (mode == "demo") {
      lab1::runStructuresDemo(std::cout, seed);
    } else if (mode == "docdemo") {
      lab1::runDocumentDemo(std::cout);
    } else {
      lab1::Shell(std::cin, std::cout, seed).run();
    }
  } catch (const std::exception& error) {
    std::cerr << "Fatal error: " << error.what() << '\n';
    return 1;
  }
  return 0;
}
