#include <array>
#include <cassert>
#include <fstream>
#include <iostream>
#include <limits>
#include <queue>
#include <string>
#include <vector>

enum Direction : std::uint8_t { N, E, S, W };

constexpr std::array<Direction, 4> inverse = { S, W, N, E };

constexpr std::array<std::pair<int, int>, 4> offsets = {
  std::make_pair(0, -1),
  std::make_pair(1, 0),
  std::make_pair(0, 1),
  std::make_pair(-1, 0)
};

struct Location {
  std::int16_t x;
  std::int16_t y;
  std::uint16_t cost;
  Direction last_dir;
  std::uint8_t steps_in_dir;
};

// Comparison operator designed such that the top element of the priority queue will have the
// lowest cost (consider cheap locations first).
bool operator<(const Location& loc1, const Location& loc2) {
  return loc1.cost > loc2.cost;
}

class Map {
public:
  Map(const std::vector<std::string>& grid)
    : width_(grid.size()), height_(grid[0].size()), grid_(grid) {}

  template<std::uint8_t direction_min, std::uint8_t direction_max>
  int find_min_path_cost() const;

private:
  std::size_t width_;
  std::size_t height_;
  std::vector<std::string> grid_;
};

template<std::uint8_t direction_min, std::uint8_t direction_max>
int Map::find_min_path_cost() const {
  // Maintain a prioritised queue of locations to consider next.
  std::priority_queue<Location> todo;
  todo.push(Location{0, 0, 0, N, 0});

  // Costs for each location.
  // Need to take into account the incoming directions: it could be that we reach a location
  // faster, but because we were already heading in a specific direction we've clobbered our
  // prospects for the rest of the path.
  constexpr std::size_t required_hash = direction_max * 4;
  std::vector<std::vector<std::array<int, required_hash>>> lowest_costs;
  lowest_costs.reserve(height_);
  for (std::size_t i = 0; i < height_; i++) {
    lowest_costs.push_back(std::vector<std::array<int, required_hash>>());
    lowest_costs.back().reserve(width_);
    for (std::size_t j = 0; j < width_; j++) {
      std::array<int, required_hash> incoming_dir_costs;
      incoming_dir_costs.fill(std::numeric_limits<int>::max());
      lowest_costs[i].push_back(incoming_dir_costs);
    }
  }

  while (!todo.empty()) {
    const auto loc = todo.top();
    todo.pop();

    const bool is_start = loc.x == 0 && loc.y == 0;

    // Generate the neighbours with their costs.
    for (const auto d : {N, E, S, W}) {
      // Will this move run afoul of the N steps in same direction limit?
      if (d == loc.last_dir && loc.steps_in_dir == direction_max) continue;

      // Impose minimum movement limit.
      if (!is_start && d != loc.last_dir && loc.steps_in_dir < direction_min) continue;

      // Prevent going backwards.
      if (!is_start && d == inverse[loc.last_dir]) continue;

      const auto& offset = offsets[d];

      // Will this step be on the grid?
      const std::int16_t next_x = loc.x + offset.first;
      const std::int16_t next_y = loc.y + offset.second;
      if (next_x < 0 ||
          next_x >= static_cast<int>(width_) ||
          next_y < 0 ||
          next_y >= static_cast<int>(height_)) continue;

      std::uint8_t next_step_count = 1;
      if (d == loc.last_dir) next_step_count = loc.steps_in_dir + 1;
      assert(next_step_count <= direction_max);

      // Have we already reached this loc by a shorter path?
      const std::uint16_t next_cost = loc.cost + grid_[next_y][next_x] - '0';
      const int incoming_dir_hash = direction_max * d + next_step_count - 1;
      if (lowest_costs[next_y][next_x][incoming_dir_hash] <= next_cost) continue;

      // Worth considering.
      lowest_costs[next_y][next_x][incoming_dir_hash] = next_cost;
      todo.push(Location{next_x, next_y, next_cost, d, next_step_count});
    }
  }

  // Find the actual lowest.
  int lowest = std::numeric_limits<int>::max();
  for (int val : lowest_costs.back().back()) {
    lowest = std::min(lowest, val);
  }

  return lowest;
}

int main() {
  std::ifstream fs("input.txt");
  std::vector<std::string> grid;
  std::string line;
  while (std::getline(fs, line)) {
    grid.push_back(line);
  }

  Map m(grid);

  const int cost_p1 = m.find_min_path_cost<0, 3>();
  const int cost_p2 = m.find_min_path_cost<4, 10>();
  std::cout << "P1: " << cost_p1 << ", P2: " << cost_p2 << std::endl;

  return 0;
}
