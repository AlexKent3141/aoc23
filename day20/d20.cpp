#include <algorithm>
#include <array>
#include <fstream>
#include <iostream>
#include <optional>
#include <queue>
#include <string>
#include <sstream>
#include <vector>

enum NodeType {
  FF,
  CN
};

struct Node {
  bool populated;
  std::string label;
  NodeType type;
  std::vector<Node*> targets;
};

int index_from_label(const std::string& label) {
  return 26 * (label[0] - 'a') + (label[1] - 'a');
}

int main() {
  std::ifstream fs("input.txt");
  std::string line, token;

  Node broadcaster;
  std::array<Node, 26*26> nodes;
  for (auto& n : nodes) n.populated = false;

  while (std::getline(fs, line)) {
    std::stringstream ss(line);
    ss >> token;

    // Which node are we populating?
    Node* n = nullptr;
    if (token[0] == 'b') {
      n = &broadcaster;
      n->label = "broadcaster";
    }
    else {
      n = &nodes[index_from_label(token.substr(1))];
      n->label = token.substr(1);
      n->type = token[0] == '%' ? FF : CN;
    }

    n->populated = true;

    // Ignore the arrow.
    ss >> token;

    // Populate the targets.
    ss >> token;
    while (ss) {
      n->targets.push_back(&nodes[index_from_label(token.substr(0, 2))]);
      ss >> token;
    }
  }

  for (const Node& n : nodes) {
    if (!n.populated) continue;
    std::cout << n.label << " type: " << n.type;
    for (Node* t : n.targets) {
      std::cout << " " << t->label;
    }
    std::cout << "\n";
  }

  return EXIT_SUCCESS;
}
