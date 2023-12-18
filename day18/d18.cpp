#include <array>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

struct Instruction {
  char dir;
  int num_steps;
  std::string col;
};

std::int64_t area(const std::vector<Instruction>& instructions) {
  // Using the Trapezoid formula.
  std::int64_t total = 0;
  std::int64_t cur_x = 0, cur_y = 0;
  std::uint64_t num_edges = 0;
  for (const auto& in : instructions) {
    std::int64_t next_x = cur_x;
    std::int64_t next_y = cur_y;

    switch (in.dir) {
      case 'U': next_y -= in.num_steps; break;
      case 'D': next_y += in.num_steps; break;
      case 'L': next_x -= in.num_steps; break;
      case 'R': next_x += in.num_steps; break;
    }

    num_edges += in.num_steps - 1;

    total += (cur_y + next_y) * (cur_x - next_x);

    cur_x = next_x;
    cur_y = next_y;
  }

  total += (cur_y + 0) * (cur_x - 0);
  total >>= 1;

  // The Trapezoid formula doesn't account for the thickness of the edges or corners.
  double boundary_bonus = 0.0;

  // Only half of the edges are accounted for so add the other half.
  boundary_bonus += 0.5 * num_edges;

  // For corners:
  // * Interior corners are only being counted for 3/4 of their real value.
  // * Exterior corners are only being counted for 1/4 of their real value.

  // Due to the shape of the path there will always be 4 more external corners than
  // internal ones.
  const std::size_t num_corners = instructions.size();
  const int num_internal = (num_corners - 4) / 2;
  const int num_external = num_corners - num_internal;

  boundary_bonus += 0.75 * num_external;
  boundary_bonus += 0.25 * num_internal;

  return total + boundary_bonus;
}

int main() {
  std::ifstream fs("input.txt");
  std::string line;
  std::vector<Instruction> instructions;
  while (std::getline(fs, line)) {
    // Create instruction from line.
    Instruction i;
    std::stringstream ss(line);
    std::string token;
    ss >> token; i.dir = token[0];
    ss >> token; i.num_steps = std::stoi(token);
    ss >> token; i.col = token.substr(2, 6);
    instructions.push_back(i);
  }

  const std::int64_t p1 = area(instructions);

  // Correct the instructions.
  for (auto& in : instructions) {
    const int num = std::strtol(in.col.c_str(), NULL, 16);
    in.num_steps = num >> 4;

    // Translate the direction into one of our recognisable chars.
    const int dir = num & 0xF;
    switch (dir) {
      case 0: in.dir = 'R'; break;
      case 1: in.dir = 'D'; break;
      case 2: in.dir = 'L'; break;
      case 3: in.dir = 'U'; break;
    }
  }

  const std::int64_t p2 = area(instructions);
  std::cout << "P1: " << p1 << ", P2: " << p2 << "\n";

  return EXIT_SUCCESS;
}
