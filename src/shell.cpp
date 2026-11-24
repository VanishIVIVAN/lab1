#include "lab1/cli/shell.hpp"

#include <iostream>
#include <sstream>

#include "lab1/cli/demo.hpp"

namespace lab1 {

std::vector<std::string> tokenize(const std::string& line) {
  std::istringstream stream(line);
  std::vector<std::string> tokens;
  std::string token;
  while (stream >> token) tokens.push_back(token);
  return tokens;
}

void Shell::run() {
  out_ << "Tree laboratory shell. Type 'help' for commands.\n";
  std::string line;
  while (true) {
    out_ << "lab1> " << std::flush;
    if (!std::getline(in_, line)) break;
    const std::vector<std::string> args = tokenize(line);
    if (args.empty()) continue;
    try {
      if (!handle(args)) break;
    } catch (const std::exception& error) {
      out_ << "Error: " << error.what() << '\n';
    }
  }
}

bool Shell::handle(const std::vector<std::string>& args) {
  const std::string& command = args.front();
  if (command == "quit" || command == "exit") return false;
  if (command == "help") {
    printHelp();
  } else if (command == "new") {
    if (args.size() < 3) throw std::invalid_argument("usage: new <kind> <type> [branching]");
    const std::size_t branching = args.size() > 3 ? parseIndex(args[3]) : 3;
    session_ = makeSession(args[2], args[1], branching, seed_);
    out_ << "created " << args[1] << " tree of " << args[2] << '\n';
  } else if (command == "demo") {
    runStructuresDemo(out_, seed_);
  } else if (command == "docdemo") {
    runDocumentDemo(out_);
  } else if (!session_) {
    throw std::invalid_argument("create a tree first: new <kind> <type>");
  } else {
    out_ << session_->execute(args) << '\n';
  }
  return true;
}

void Shell::printHelp() const {
  out_ << "Commands:\n"
          "  new <kind> <type> [branching]  kind: general | general-std | binary | bst\n"
          "                                 type: int | double | string | vector | element\n"
          "  insert <v>                     add where the tree decides\n"
          "  add <parent> <v>               add as a child of <parent>\n"
          "  remove <v>                     remove the first node equal to <v>\n"
          "  removechild <parent> <i>       remove the i-th child (0-based) with its subtree\n"
          "  contains <v> | path <v>        search by value / path from the root\n"
          "  haspath <v1> <v2> ...          is it a downward path starting in the root?\n"
          "  traverse <pre|post|level|in>   list the values in the given order\n"
          "  random <count> [seed]          fill with random data\n"
          "  print | info | clear\n"
          "  demo                           demonstration on all types and tree kinds\n"
          "  docdemo                        document model and formatting rules\n"
          "  quit\n"
          "Values: int 5; double 2.5; string word; vector 1,2,3;\n"
          "element title:Name | section:1:Heading | paragraph:Some_text. | list:a;b |\n"
          "        table:Caption:2:3 | image:file.png:Caption | formula:1:E=mc2 | source:1:Book\n"
          "        ('_' stands for a space)\n";
}

}  // namespace lab1
