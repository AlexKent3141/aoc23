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
  BT,
  BR,
  FF,
  CN
};

struct Node {
  bool populated;
  std::string label;
  NodeType type;
  std::vector<Node*> inputs;
  std::vector<Node*> targets;
  bool ff_is_on;
};

struct Pulse {
  Node* sender;
  Node* target;
  bool is_high;
};

int index_from_label(const std::string& label) {
  return 26 * (label[0] - 'a') + (label[1] - 'a');
}

std::pair<int, int> push_button(const Node& button) {
  int num_low_pulses = 0;
  int num_high_pulses = 0;

  std::deque<Pulse> todo;

  // Send low pulse from button to broadcaster.
  todo.push_back(Pulse{const_cast<Node*>(&button), button.targets.front(), false});

  while (!todo.empty()) {
    const auto p = todo.front();
    todo.pop_front();

    if (p.is_high) ++num_high_pulses;
    else ++num_low_pulses;

    // Execute this pulse updating the state of the target.
    Node* target = p.target;
    switch (p.target->type) {
      case BR: {
        // Forward this pulse on to all of the broadcaster's targets.
        for (Node* next_target : p.target->targets) {
          todo.push_back(Pulse{target, next_target, p.is_high});
        }
        break;
      }
      case FF: {
        if (p.is_high) break;

        // Invert state and fire pulse with the new state.
        p.target->ff_is_on = !p.target->ff_is_on;
        for (Node* next_target : p.target->targets) {
          todo.push_back(Pulse{target, next_target, p.target->ff_is_on});
        }
        break;
      }
      case CN: {
        // Update the conjunction node's latest info for this input.
        break;
      }
      case BT: std::abort();
    }
  }

  return std::make_pair(num_low_pulses, num_high_pulses);
}

int main() {
  std::ifstream fs("input.txt");
  std::string line, token;

  Node button, broadcaster;
  std::array<Node, 26*26> nodes;
  for (auto& n : nodes) n.populated = false;

  button.type = BT;
  button.targets.push_back(&broadcaster);

  while (std::getline(fs, line)) {
    std::stringstream ss(line);
    ss >> token;

    // Which node are we populating?
    Node* n = nullptr;
    if (token[0] == 'b') {
      n = &broadcaster;
      n->label = "broadcaster";
      n->type = BR;
    }
    else {
      n = &nodes[index_from_label(token.substr(1))];
      n->label = token.substr(1);
      n->type = token[0] == '%' ? FF : CN;
      n->ff_is_on = false;
    }

    n->populated = true;

    // Ignore the arrow.
    ss >> token;

    // Populate the targets.
    ss >> token;
    while (ss) {
      Node* target = &nodes[index_from_label(token.substr(0, 2))];
      n->targets.push_back(target);
      target->inputs.push_back(n);
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
