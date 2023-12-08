#include <fstream>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

std::uint64_t gcd(std::uint64_t a, std::uint64_t b)
{
  if (a > b) return gcd(b, a);
  int tmp;
  while (a)
  {
    tmp = a;
    a = b % a;
    b = tmp;
  }

  return b;
}

struct Node
{
  std::string label;
  std::string left_label;
  std::string right_label;
  const Node* left;
  const Node* right;
};

const Node* find_node(const std::string& label, const std::vector<Node>& nodes)
{
  for (const auto& n : nodes)
  {
    if (n.label == label) return &n;
  }

  return nullptr;
}

int steps_to_reach_target(
  const Node& start,
  const std::string& directions,
  std::function<bool(const Node*)> check)
{
  int num_steps = 0;
  int index = -1;
  const Node* current = &start;
  while (!check(current))
  {
    ++num_steps;
    ++index;
    index %= directions.size();
    if (directions[index] == 'L') current = current->left;
    else current = current->right;
  }

  return num_steps;
}

int main()
{
  std::ifstream fs("input.txt");
  std::string directions; // The first line
  std::vector<Node> nodes;

  std::string line;
  while (std::getline(fs, line))
  {
    if (line.empty()) continue;

    if (directions.empty())
    {
      directions = line;
      continue;
    }

    nodes.push_back(Node
    {
      line.substr(0, 3),
      line.substr(7, 3),
      line.substr(12, 3),
      nullptr,
      nullptr
    });
  }

  // Iterate again and link nodes up.
  const Node* start = nullptr, *end = nullptr;
  for (Node& n : nodes)
  {
    const Node* l = find_node(n.left_label, nodes);
    const Node* r = find_node(n.right_label, nodes);

    n.left = l;
    n.right = r;

    if (n.label == "AAA") start = &n;
    if (n.label == "ZZZ") end = &n;
  }

  // P1
  int p1_steps = steps_to_reach_target(
    *start,
    directions,
    [end] (const Node* n) { return n == end; });

  // P2
  std::uint64_t lcm = 1;
  for (const Node& n : nodes)
  {
    if (n.label[2] != 'A') continue;
    int num_steps = steps_to_reach_target(
      n,
      directions,
      [] (const Node* n) { return n->label[2] == 'Z'; });

    // Update the LCM.
    auto d = gcd(lcm, num_steps);
    lcm *= num_steps;
    lcm /= d;
  }

  std::cout << "P1: " << p1_steps << ", P2: " << lcm << std::endl;

  return 0;
}
