#include <algorithm>
#include <array>
#include <fstream>
#include <iostream>
#include <queue>
#include <string>
#include <vector>

enum Direction : std::uint8_t { N, E, S, W };
const std::array<char, 4> direction_slopes{'^', '>', 'v', '<'};

const std::array<Direction, 4> inverse_direction{S, W, N, E};

constexpr std::array<std::pair<int, int>, 4> offsets = {
  std::make_pair(0, -1),
  std::make_pair(1, 0),
  std::make_pair(0, 1),
  std::make_pair(-1, 0)
};

struct Vertex {
  int x;
  int y;
  std::vector<int> edges;
  bool visited;
};

struct Edge {
  Vertex* start;
  Vertex* end;
  int weight;
};

class Graph {
public:
  Graph(const std::vector<std::string>&, bool directed);

  // Recursive method which finds the longest path from start to end in the graph.
  int find_longest_path(Vertex* current = nullptr);

private:
  Vertex* start_, *end_;
  int width_, height_;
  std::vector<Vertex> vertices_;
  std::vector<Edge> edges_;
  std::vector<std::string> grid_;
  bool directed_;

  void calculate_edge_weights(Vertex*, Direction);
};

Graph::Graph(const std::vector<std::string>& grid, bool directed)
  : grid_(grid), directed_(directed) {

  // First create a vector of all vertices.
  width_ = static_cast<int>(grid[0].size());
  height_ = static_cast<int>(grid.size());

  // Find the start and end vertices.
  for (int x = 0; x < width_; x++) {
    if (grid[0][x] == '.') vertices_.push_back(Vertex{x, 0});
    if (grid.back()[x] == '.') vertices_.push_back(Vertex{x, height_ - 1});
  }

  // If this is a directed graph then create all other vertices based on the slopes.
  // If not then clear the slopes and instead create vertices at junctions.
  const auto is_junction = [this] (int x, int y) {
    // This location is a junction if it has three or more empty neighbours.
    int num_empty = 0;
    for (Direction d : {N, E, S, W}) {
      const auto& offset = offsets[d];
      int next_x = x + offset.first;
      int next_y = y + offset.second;
      if (next_x < 0 || next_x >= width_ || next_y < 0 || next_y >= height_) continue;
      num_empty += grid_[next_y][next_x] != '#';
    }
    return num_empty >= 3;
  };

  for (int y = 1; y < height_; y++) {
    for (int x = 0; x < width_; x++) {
      char c = grid[y][x];
      if (c != '.' && c != '#') {
        if (directed) {
          vertices_.push_back(Vertex{x, y});
        }
        else {
          grid_[y][x] = '.';
        }
      }

      if (!directed && c == '.') {
        // Check whether this is a junction.
        if (is_junction(x, y)) {
          vertices_.push_back(Vertex{x, y});
          grid_[y][x] = 'o';
        }
      }
    }
  }

  // Now that there's no chance of pointers to vector elements being invalidated store
  // the start and end.
  start_ = &vertices_[0];
  end_ = &vertices_[1];

  // Now we need to work out the edge weights i.e. the distances between vertices.
  calculate_edge_weights(start_, S);
}

void Graph::calculate_edge_weights(Vertex* v, Direction d) {

  struct StepNode {
    int x;
    int y;
    int steps_taken;
    Direction prev_dir;
  };

  auto offset = offsets[d];
  int next_x = v->x + offset.first;
  int next_y = v->y + offset.second;

  // If we're off the grid then stop here (probably the end vertex?).
  if (next_x < 0 || next_x >= width_ || next_y < 0 || next_y >= height_) return;

  if (grid_[next_y][next_x] == '#') return;

  std::queue<StepNode> todo;
  todo.push(StepNode{next_x, next_y, 1, d});

  // Keep stepping until we have reached all child vertices.
  while (!todo.empty()) {
    const StepNode current = todo.front();
    todo.pop();

    // If we've reached another vertex then add the complete edge and kick off an edge
    // weight calc for that one.
    char c = grid_[current.y][current.x];
    if (c != '.' && c != '#') {
      const auto it = std::find_if(vertices_.begin(), vertices_.end(), [current] (const auto& v) {
        return v.x == current.x && v.y == current.y;
      });

      Vertex* end = &vertices_[it - vertices_.begin()];

      // Add an edge.
      edges_.push_back(Edge{v, end, current.steps_taken});
      v->edges.push_back(edges_.size() - 1);

      // Is this vertex already expanded?
      if (end->edges.empty()) {
        // Calc edges for this end.
        if (directed_) {
          calculate_edge_weights(end, current.prev_dir);
        }
        else {
          for (const auto d : {N, E, S, W}) {
            if (d == inverse_direction[current.prev_dir]) continue;
            calculate_edge_weights(end, d);
          }

          // Add the inverse edge.
          Edge inverse{end, v, current.steps_taken};
          edges_.push_back(inverse);
          end->edges.push_back(edges_.size() - 1);
        }
      }

      continue;
    }

    // If we've reached the end then stop.
    if (current.x == end_->x && current.y == end_->y) {
      // Add an edge.
      edges_.push_back(Edge{v, end_, current.steps_taken});
      v->edges.push_back(edges_.size() - 1);
      continue;
    }

    // Generate neighbour nodes.
    for (const auto d : {N, E, S, W}) {
      if (d == inverse_direction[current.prev_dir]) continue;
      
      offset = offsets[d];
      next_x = current.x + offset.first;
      next_y = current.y + offset.second;
      if (next_x < 0 || next_x >= width_ || next_y < 0 || next_y >= height_) continue;

      c = grid_[next_y][next_x];
      if (c == '#') continue;

      // If it's a vertex but the slope is pointing the wrong way then skip.
      if (c == direction_slopes[inverse_direction[d]]) continue;

      todo.push(StepNode{next_x, next_y, current.steps_taken + 1, d});
    }
  }
}

int Graph::find_longest_path(Vertex* current) {
  if (!current) current = start_;

  if (current == end_) return 0;

  int best_length = 0;

  // Consider all edges which lead to available vertices.
  static constexpr int NO_PATH = -1000000;
  bool has_edges = false;
  int length = 0;
  for (int edge_index : current->edges) {
    auto& edge = edges_[edge_index];
    if (edge.end->visited) continue;

    edge.end->visited = true;
    length += edge.weight;

    const int remaining_len = find_longest_path(edge.end);

    length += remaining_len;

    best_length = std::max(best_length, length);

    // Reset things back how they were.
    edge.end->visited = false;
    length -= edge.weight;
    length -= remaining_len;

    has_edges |= remaining_len != NO_PATH;
  }

  // If we have no edges this means that we've painted outselves into a corner and should
  // return a large negative value indicating "impossible".
  if (!has_edges) return NO_PATH;

  return best_length;
}

// Plan: turn the map into a tree structure where the vertices are the downward slopes we
// can traverse and the edges are the distance between each vertex.
int main() {
  std::ifstream fs("input.txt");
  std::vector<std::string> grid;
  std::string line;
  while (std::getline(fs, line)) {
    grid.push_back(line);
  }

  Graph directed_graph(grid, true);

  // The problem now becomes finding the longest path from the start node to the end.
  const int p1 = directed_graph.find_longest_path();

  // I think part 2 can be framed as a modification to part 1:
  // * The slopes are no longer vertices, instead every "junction" where two or more paths
  //   meet is now a vertex.
  // * Edges can only be traversed once but can go in either direction.
  Graph undirected_graph(grid, false);
  const int p2 = undirected_graph.find_longest_path();
  std::cout << "P1: " << p1 << ", P2: " << p2 << "\n";
}
