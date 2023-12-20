#include <algorithm>
#include <array>
#include <fstream>
#include <functional>
#include <iostream>
#include <optional>
#include <queue>
#include <string>
#include <sstream>
#include <vector>

enum NodeType {
  UNKNOWN,
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
  std::vector<bool> latest_state_sent_to_target;

  std::optional<std::function<void()>> all_inputs_on_callback;

  void update_state_for_target(const Node* target, bool is_high) {
    for (std::size_t i = 0; i < targets.size(); i++) {
      if (targets[i] == target) {
        latest_state_sent_to_target[i] = is_high;
        return;
      }
    }

    std::abort();
  }

  bool state_for_target(const Node* target) const {
    for (std::size_t i = 0; i < targets.size(); i++) {
      if (targets[i] == target) {
        return latest_state_sent_to_target[i];
      }
    }

    std::abort();
  }

  bool all_inputs_high() const {
    bool is_high = true;
    for (std::size_t i = 0; i < inputs.size() && is_high; i++) {
      is_high &= inputs[i]->state_for_target(this);
    }
    return is_high;
  }
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

    // Update the latest state sent to this target.
    p.sender->update_state_for_target(target, p.is_high);

    if (target->type == UNKNOWN) continue;

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
        // Check whether all inputs are high.
        bool pulse_state = !target->all_inputs_high();

        if (!pulse_state && target->all_inputs_on_callback) {
          (*target->all_inputs_on_callback)();
        }

        for (Node* next_target : p.target->targets) {
          todo.push_back(Pulse{target, next_target, pulse_state});
        }
        break;
      }
      case UNKNOWN:
      case BT: std::abort();
    }
  }

  return std::make_pair(num_low_pulses, num_high_pulses);
}

void reset(std::array<Node, 26*26>& nodes) {
  for (Node& n : nodes) {
    if (!n.populated) continue;
    n.ff_is_on = false;
    for (std::size_t i = 0; i < n.latest_state_sent_to_target.size(); i++) {
      n.latest_state_sent_to_target[i] = false;
    }
  }
}

int main() {
  std::ifstream fs("input.txt");
  std::string line, token;

  Node button, broadcaster;
  std::array<Node, 26*26> nodes;
  for (auto& n : nodes) n.populated = false;

  button.type = BT;
  button.targets.push_back(&broadcaster);
  button.latest_state_sent_to_target.push_back(false);

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
      n->latest_state_sent_to_target.push_back(false);
      target->inputs.push_back(n);
      ss >> token;
    }
  }

  int low_total = 0, high_total = 0;
  for (int i = 0; i < 1000; i++) {
    auto [low, high] = push_button(button);
    low_total += low;
    high_total += high;
  }

  const int p1 = low_total * high_total;

  // For part 2 I've made some deductions based on the actual shape of my particular
  // network (presumably other inputs have similar patterns).
  // Working backwards from the rx node, we have the following graph of conjunctions:
  /*
      [zq]  [kx]   [zd]  [mt]
    L  |     |       |     |
       v     v       v     v
      [qz]  [cq]   [jx]  [tt]
        \     \     /     /
    H    \     \   /     /
          \     v v     /
           --->[qn] <---
                |
    L           v
               [rx]
  */
  // In order for the rx to receive a low pulse the outputs from the preceding nodes will
  // need to be as described on the left of the diagram, which means that the inputs to the
  // nodes zq, kx, zd and mt must all be high.
  // I assume that there is some periodic behaviour at work. Let's work out the periods!

  std::uint64_t prod = 1;
  bool all_inputs_on;
  const auto all_inputs_on_detected = [&all_inputs_on] () { all_inputs_on = true; };
  for (const auto& label : {"zq", "kx", "zd", "mt"}) {
    reset(nodes);

    Node* n = &nodes[index_from_label(label)];
    n->all_inputs_on_callback = all_inputs_on_detected;

    // Push the button until the inputs first come on.
    all_inputs_on = false;
    while (!all_inputs_on) {
      push_button(button);
    }

    // We're in a "clean" state now so go around again and see what the period is.
    int num_presses = 0;
    all_inputs_on = false;
    while (!all_inputs_on) {
      push_button(button);
      ++num_presses;
    }

    n->all_inputs_on_callback = std::nullopt;

    std::cout << "All high for label: " << label << " at: " << num_presses << std::endl;

    // Note: in my case all periods were prime. If this was not true then need to LCM.
    prod *= num_presses;
  }

  std::cout << "P1: " << p1 << ", P2: " << prod << "\n";

  return EXIT_SUCCESS;
}
