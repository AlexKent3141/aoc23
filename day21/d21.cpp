#include <array>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <queue>
#include <set>
#include <string>
#include <tuple>
#include <vector>

enum Direction : std::uint8_t { N, E, S, W };

const std::array<Direction, 4> inverse_direction{S, W, N, E};

constexpr std::array<std::pair<int, int>, 4> offsets = {
  std::make_pair(0, -1),
  std::make_pair(1, 0),
  std::make_pair(0, 1),
  std::make_pair(-1, 0)
};

class Map {
public:
  Map(const std::vector<std::string>&);
  std::uint32_t count_locations_after_exact_steps(std::uint32_t) const;

private:
  std::uint32_t start_x_, start_y_;
  std::size_t width_, height_;
  std::vector<std::string> grid_;
};

Map::Map(const std::vector<std::string>& grid)
  : width_(grid[0].size()), height_(grid.size()), grid_(grid) {

  // Find the start location and mark as garden plot.
  for (std::size_t y = 0; y < height_; y++) {
    for (std::size_t x = 0; x < height_; x++) {
      if (grid_[y][x] == 'S') {
        grid_[y][x] = '.';

        // Boost the start location to make hashing easier.
        start_x_ = x + 1000 * width_;
        start_y_ = y + 1000 * height_;
        return;
      }
    }
  }
}

std::uint32_t Map::count_locations_after_exact_steps(std::uint32_t num_steps) const {

  struct Node {
    std::uint32_t x;
    std::uint32_t y;
    std::uint32_t steps_taken;

    bool operator<(const Node& other) const {
      return std::tie(x, y) < std::tie(other.x, other.y);
    }
  };

  // Keep track of the locations we've reached after exactly the step limit.
  std::set<Node> reached_locations;

  std::queue<Node> todo;

  // Do a breakdown here to ensure that during the main search there are an
  // even number of steps remaining. This allows us to make an optimisation
  // because even location we visit will end up being reachable.
  if (num_steps & 0x1) {
    // Consider neighbours.
    for (const auto d : {N, E, S, W}) {
      const auto& offset = offsets[d];

      const std::uint32_t next_x = start_x_ + offset.first;
      const std::uint32_t next_y = start_y_ + offset.second;

      // Is this location blocked?
      if (grid_[next_y % height_][next_x % width_] == '#') continue;

      // Have we already reached this location with a lower step count of the
      // same parity?
      Node next{next_x, next_y, 1};
      todo.push(next);
      reached_locations.insert(next);
    }
  }
  else {
    Node start{start_x_, start_y_, 0};
    todo.push(start);
    reached_locations.insert(start);
  }

  while (!todo.empty()) {
    auto current = todo.front();
    todo.pop();

    // Take two steps.
    for (const auto d : {N, E, S, W}) {
      const auto& offset = offsets[d];

      const std::uint32_t next_x = current.x + offset.first;
      const std::uint32_t next_y = current.y + offset.second;

      // Is this location blocked?
      if (grid_[next_y % height_][next_x % width_] == '#') continue;

      // Consider neighbours of this neighbour.
      for (const auto d2 : {N, E, S, W}) {
        if (d2 == inverse_direction[d]) continue;

        const auto& offset2 = offsets[d2];

        const std::uint32_t next_x2 = next_x + offset2.first;
        const std::uint32_t next_y2 = next_y + offset2.second;

        // Is this location blocked?
        if (grid_[next_y2 % height_][next_x2 % width_] == '#') continue;

        Node next2{next_x2, next_y2, current.steps_taken + 2};
        if (reached_locations.find(next2) != reached_locations.end()) continue;

        reached_locations.insert(next2);

        if (next2.steps_taken == num_steps) continue;

        todo.push(next2);
      }
    }
  }

  return reached_locations.size();
}

std::array<long double, 3> second_order_regression(
  const std::vector<int>& xs,
  const std::vector<int>& ys) {

  std::uint64_t sx = 0, sy = 0, sx2 = 0, sx3 = 0, sxy = 0, sx2y = 0, sx4 = 0;
  for (std::size_t i = 0; i < xs.size(); i++) {
    std::uint64_t x = xs[i], y = ys[i];
    sx += x;
    sy += y;
    sx2 += x * x;
    sx3 += x * x * x;
    sxy += x * y;
    sx2y += x * x * y;
    sx4 += x * x * x * x;
  }

  const auto n = static_cast<long double>(xs.size());
  const double Sxx = sx2 - (sx * sx) / n;
  const double Sxy = sxy - (sx * sy) / n;
  const double Sxx2 = sx3 - (sx * sx2) / n;
  const double Sx2y = sx2y - (sx2 * sy) / n;
  const double Sx2x2 = sx4 - (sx2 * sx2) / n;

  const long double a = (Sx2y * Sxx - Sxy * Sxx2) / (Sxx * Sx2x2 - Sxx2 * Sxx2);
  const long double b = (Sxy * Sx2x2 - Sx2y * Sxx2) / (Sxx * Sx2x2 - Sxx2 * Sxx2);
  const long double c = (sy / n) - (b * (sx / n)) - (a * (sx2 / n));

  return { a, b, c };
}

long double eval(const std::array<long double, 3>& coeffs, std::uint64_t x) {
  return coeffs[0] * x * x + coeffs[1] * x + coeffs[2];
}

int main() {
  std::ifstream fs("input.txt");
  std::vector<std::string> grid;
  std::string line;
  while (std::getline(fs, line)) {
    grid.push_back(line);
  }

  Map m(grid);

  const auto p1 = m.count_locations_after_exact_steps(64);
  std::cout << "P1: " << p1 << std::endl;

  // Examining the example data we're given, we can make a guess that we're looking at a
  // quadratic equation in the number of steps.
  // It turns out that this is correct, and the approach I've taken is to use my brute
  // force approach to generate as much data as possible and then fit a second order curve
  // to it which we can use to estimate the result.
  constexpr int target_steps = 26501365;
  const std::size_t s = grid.size();
  const int r = target_steps % s;

  std::vector<int> xs, ys;
  int grid_step = 0;
  while (true) {
    int steps = grid.size() * grid_step + r;
    ++grid_step;

    const auto count = m.count_locations_after_exact_steps(steps);
    std::cout << "Steps: " << steps << " can reach: " << count << "\n";

    xs.push_back(steps);
    ys.push_back(count);

    if (xs.size() >= 3) {
      const auto f = second_order_regression(xs, ys);
      const auto estimate = eval(f, target_steps);
      std::cout << "Best estimate: " << std::setprecision(15) << estimate << "\n";
    }
  }
}
