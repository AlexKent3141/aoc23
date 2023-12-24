#include <algorithm>
#include <array>
#include <fstream>
#include <iostream>
#include <optional>
#include <ostream>
#include <queue>
#include <sstream>
#include <string>
#include <vector>

struct Point {
  double x;
  double y;
  double z;
};

struct Vector {
  double i;
  double j;
  double k;
};

struct Hail {
  Point pos;
  Vector vel;
};

std::ostream& operator<<(std::ostream& os, const Hail& h) {
  os << h.pos.x << " " << h.pos.y << " " << h.pos.z << ", "
     << h.vel.i << " " << h.vel.j << " " << h.vel.k;
  return os;
}

// Solve two simultaneous equations in two variables to find the path intersection
std::optional<Point> paths_will_cross(const Hail& h1, const Hail& h2) {
  // Solve equations:
  // For h1: y = (j1 / i1) * x + c1
  // For h2: y = (j2 / i2) * x + c2

  // Bit of faff to find the y-intercepts: first find the times of intersection
  // (i.e. when x is zero) and then sub in to find the corresponding y's.
  const double toi1 = h1.pos.x / h1.vel.i;
  const double toi2 = h2.pos.x / h2.vel.i;

  const double c1 = h1.pos.y - toi1 * h1.vel.j;
  const double c2 = h2.pos.y - toi2 * h2.vel.j;

  const double m1 = h1.vel.j / h1.vel.i;
  const double m2 = h2.vel.j / h2.vel.i;

  // Check for parallel lines.
  if (m1 == m2) return std::nullopt;

  const double x = (c2 - c1) / (m1 - m2);
  const double y = m1 * x + c1;

  // Assess whether this intersection point is in the future.
  // Needs to be in the same direction as the velocity.
  if (x > h1.pos.x && h1.vel.i <= 0) return std::nullopt;
  if (x < h1.pos.x && h1.vel.i >= 0) return std::nullopt;
  if (x > h2.pos.x && h2.vel.i <= 0) return std::nullopt;
  if (x < h2.pos.x && h2.vel.i >= 0) return std::nullopt;

  if (y > h1.pos.y && h1.vel.j <= 0) return std::nullopt;
  if (y < h1.pos.y && h1.vel.j >= 0) return std::nullopt;
  if (y > h2.pos.y && h2.vel.j <= 0) return std::nullopt;
  if (y < h2.pos.y && h2.vel.j >= 0) return std::nullopt;

  return Point{x, y, 0};
}

bool crossover_in_range(const Point& p) {
  static constexpr std::array<std::int64_t, 2> r = {200000000000000, 400000000000000};
  return p.x >= r[0] && p.x <= r[1] && p.y >= r[0] && p.y <= r[1];
}

int main() {
  std::ifstream fs("input.txt");
  std::vector<Hail> hailstones;
  std::string line, token;
  while (std::getline(fs, line)) {

    std::replace(line.begin(), line.end(), ',', ' ');
    std::replace(line.begin(), line.end(), '@', ' ');

    Hail h;

    std::stringstream ss(line);
    ss >> token; h.pos.x = std::stod(token);
    ss >> token; h.pos.y = std::stod(token);
    ss >> token; h.pos.z = std::stod(token);

    ss >> token; h.vel.i = std::stod(token);
    ss >> token; h.vel.j = std::stod(token);
    ss >> token; h.vel.k = std::stod(token);

    hailstones.push_back(h);
  }

  // For Part 1 we want to find the intersection in the 2D plane (ignore z).
  int num_intersections = 0;
  for (std::size_t i = 0; i < hailstones.size(); i++) {
    for (std::size_t j = i + 1; j < hailstones.size(); j++) {
      std::cout << "Considering:\n" << hailstones[i] << "\n" << hailstones[j] << "\n";

      const auto res = paths_will_cross(hailstones[i], hailstones[j]);
      if (!res) {
        std::cout << "No future cross-over\n";
        continue;
      }

      const auto p = *res;
      std::cout << "Intersection at: " << p.x << ", " << p.y << ", " << p.z << "\n";

      if (crossover_in_range(p)) ++num_intersections;
    }
  }

  std::cout << "P1: " << num_intersections << "\n";

  // Part 2 is a different kettle of fish.
  // We're effectively in a system of equations which are massively over-constrained.
  // This boiled down to solving a system involving three of the hail stones, which
  // conveniently gives us 9 equations in 9 unknowns.
  // They aren't linear simultaneous equations so not completely obvious how to solve them.
  // Rather than pulling in a dependency to solve them I took a shortcut and used the
  // Python SymPy module which solves them rather handily (see "p2.py").
}
