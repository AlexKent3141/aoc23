#include <algorithm>
#include <array>
#include <cassert>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <vector>

static constexpr std::size_t base_size = 10;
static constexpr std::size_t max_height = 1000;

struct Point {
  int x, y, z;
};

struct Block {
  int id;
  std::vector<Point> points;

  Block(const Point&, const Point&);

  // Shift the whole block up or down.
  void lower() { for (auto& p : points) --p.z; }
  void raise() { for (auto& p : points) ++p.z; }
};

Block::Block(const Point& end1, const Point& end2) {
  if (end1.x != end2.x) {
    // Oriented along the x axis.
    assert(end1.y == end2.y && end1.z == end2.z);
    for (int x = end1.x; x < end2.x + 1; x++) {
      points.push_back({x, end1.y, end1.z});
    }
  }
  else if (end1.y != end2.y) {
    // Oriented along the y axis.
    assert(end1.x == end2.x && end1.z == end2.z);
    for (int y = end1.y; y < end2.y + 1; y++) {
      points.push_back({end1.x, y, end1.z});
    }
  }
  else {
    // Oriented along the z axis.
    assert(end1.x == end2.x && end1.y == end2.y);
    for (int z = end1.z; z < end2.z + 1; z++) {
      points.push_back({end1.x, end1.y, z});
    }
  }
}

class Tetris {
public:
  Tetris(const std::vector<Block>&);

  void gravity();
  int count_safe_disintegrations() const;

private:
  std::vector<Block> blocks_;

  // 2D grid of stacks.
  // Each locations is either zero (unoccupied) or has a block id.
  std::array<std::array<std::array<int, max_height>, base_size>, base_size> occupied_stacks_;
};

Tetris::Tetris(const std::vector<Block>& blocks)
  : blocks_(blocks) {

  std::sort(
    blocks_.begin(),
    blocks_.end(),
    [] (const auto& b1, const auto& b2) {
      return b1.points[0].z < b2.points[0].z;
    });

  // Allocate each block an ID based on its index.
  for (std::size_t id = 0; id < blocks_.size(); id++) {
    blocks_[id].id = id;
  }

  // Fill the occupied stacks with zeros initially.
  for (std::size_t x = 0; x < base_size; x++) {
    for (std::size_t y = 0; y < base_size; y++) {
      occupied_stacks_[x][y].fill(-1);
    }
  }
}

void Tetris::gravity() {
  // Work through the blocks starting from the lowest first.
  // For each block, try to lower its position as much as possible without
  // overlapping with an already placed block.
  const auto block_intersects = [this] (const Block& b) {
    for (const auto& p : b.points) {
      if (p.z < 1) return true; // Cannot go through the floor.
      if (occupied_stacks_[p.x][p.y][p.z] != -1) return true;
    }
    return false;
  };

  for (auto& b : blocks_) {
    bool hit = false;
    while (!hit) {
      b.lower();
      hit = block_intersects(b);
    }

    b.raise();

    // Flag these locations as occupied.
    for (const auto& p : b.points) {
      occupied_stacks_[p.x][p.y][p.z] = b.id;
    }
  }
}

int Tetris::count_safe_disintegrations() const {
  // A block can be safely disintegrated if its removal would not cause
  // any blocks immediately above to fall (must have at least one other support).

  // Plan: for each block, look at the blocks directly above it.
  // If all blocks directly above have multiple blocks supporting then this block
  // can be safely removed.
  int count = 0;
  for (const auto& block : blocks_) {
    Block b(block);
    b.raise();

    std::set<int> ids_above;
    for (const auto& p : b.points) {
      int id = occupied_stacks_[p.x][p.y][p.z];
      if (id == -1 || id == b.id) continue;
      ids_above.insert(id);
    }

    bool single_support = false;
    for (auto id : ids_above) {
      // Count the supports for this block.
      Block b_above(blocks_[id]);
      b_above.lower();
      std::set<int> ids_below;
      for (const auto& p : b_above.points) {
        int id_below = occupied_stacks_[p.x][p.y][p.z];
        if (id_below == -1 || id_below == b_above.id) continue;
        ids_below.insert(id_below);
      }

      if (ids_below.size() == 1) {
        single_support = true;
        break;
      }
    }

    count += !single_support;
  }

  return count;
}

int main() {
  std::ifstream fs("input.txt");
  std::vector<Block> blocks;
  std::string line, token;
  while (std::getline(fs, line)) {
    std::replace(line.begin(), line.end(), ',', ' ');
    std::replace(line.begin(), line.end(), '~', ' ');

    std::stringstream ss(line);

    Point end1, end2;
    ss >> token; end1.x = std::stoi(token);
    ss >> token; end1.y = std::stoi(token);
    ss >> token; end1.z = std::stoi(token);
    ss >> token; end2.x = std::stoi(token);
    ss >> token; end2.y = std::stoi(token);
    ss >> token; end2.z = std::stoi(token);

    Block b(end1, end2);
    blocks.push_back(b);
  }

  Tetris t(blocks);
  t.gravity();

//for (const auto& b : blocks) {
//  std::cout << b.points[0].x << ", "<< b.points[0].y << ", " << b.points[0].z << " ~ "
//            << b.points.back().x << ", "<< b.points.back().y << ", " << b.points.back().z << "\n";
//}

  int p1 = t.count_safe_disintegrations();
  std::cout << "P1: " << p1 << "\n";
}
