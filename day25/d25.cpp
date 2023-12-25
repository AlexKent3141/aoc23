#include <algorithm>
#include <array>
#include <cassert>
#include <fstream>
#include <iostream>
#include <ostream>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <vector>

struct Vertex {
  bool initialised;
  std::string label;
  std::vector<int> edge_indices;
  Vertex* parent;
};

struct Edge {
  Vertex* end1;
  Vertex* end2;
  bool removed;
  int hits;

  Vertex* other_end(const Vertex* v) const {
    return v == end1 ? end2 : end1;
  }
};

std::ostream& operator<<(std::ostream& os, const Edge& e) {
  os << e.end1->label << " --> " << e.end2->label;
  return os;
}

int label_to_index(const std::string& label) {
  assert(label.size() == 3);
  return (label[0] - 'a') * 26 * 26 + (label[1] - 'a') * 26 + (label[2] - 'a');
}

void shortest_path_with_stats(Vertex* v1, Vertex* v2, std::vector<Edge>& edges) {
  // The graph isn't weighted so when we first encounter a node we have the shortest path to it
  // (assuming a BFS).
  std::queue<Vertex*> todo;
  todo.push(v1);
  while (!todo.empty()) {
    Vertex* current = todo.front();
    todo.pop();
    for (int edge_index : current->edge_indices) {
      const Edge& e = edges[edge_index];

      Vertex* target = e.other_end(current);

      // Check whether we already have the shortest path to this vertex.
      if (target->parent != nullptr) continue;

      target->parent = current;
      if (target == v2) goto path_found;

      todo.push(target);
    }
  }

path_found:
  // Backtrack and increment edge counters.
  Vertex* prev, *next = v2;
  do {
    prev = next->parent;

    // Find the edge between prev and next.
    for (int edge_index : prev->edge_indices) {
      auto& edge = edges[edge_index];
      if ((edge.end1 == prev && edge.end2 == next) ||
          (edge.end2 == prev && edge.end1 == next)) {
        ++edge.hits;
        break;
      }
    }

    next = prev;

  } while (next != v1);
}

int component_size(const Vertex* v, const std::vector<Edge>& edges) {
  std::set<const Vertex*> considered;
  std::queue<const Vertex*> todo;
  todo.push(v);
  considered.insert(v);
  while (!todo.empty()) {
    const Vertex* current = todo.front();
    considered.insert(current);
    todo.pop();
    for (int edge_index : current->edge_indices) {
      const Edge& e = edges[edge_index];
      if (e.removed) continue;

      Vertex* target = e.other_end(current);
      if (considered.find(target) != considered.end()) continue;

      todo.push(target);
      considered.insert(target);
    }
  }

  return considered.size();
}

void reset_vertices(std::array<Vertex, 26*26*26>& vertices) {
  for (auto& v : vertices) v.parent = nullptr;
}

int main() {
  srand(time(NULL));

  std::ifstream fs("input.txt");
  std::array<Vertex, 26*26*26> vertices;
  vertices.fill(Vertex{false});
  std::vector<Edge> edges;

  std::string line, token;
  while (std::getline(fs, line)) {

    std::replace(line.begin(), line.end(), ':', ' ');

    std::stringstream ss(line);
    ss >> token;

    Vertex* end1 = &vertices[label_to_index(token)];
    end1->initialised = true;
    end1->label = token;

    ss >> token;
    while (ss) {
      Vertex* end2 = &vertices[label_to_index(token)];
      end2->initialised = true;
      end2->label = token;

      // Add bidirectional edge.
      edges.push_back(Edge{end1, end2});

      end1->edge_indices.push_back(edges.size() - 1);
      end2->edge_indices.push_back(edges.size() - 1);
      ss >> token;
    }
  }

  // New plan:
  // Pick pairs of vertices at random and find the shortest path between them, keeping track
  // of how many "hits" each edge gets over time.
  // The idea is that there is that some of the randomly selected vertex pairs will be in
  // different connected components, so the edges which connect the components will rack up
  // more hits over time.
  // This will be more effective if the two components are roughly the same size.
  std::vector<Vertex*> vs;
  for (std::size_t i = 0; i < vertices.size(); i++) {
    if (!vertices[i].initialised) continue;
    vs.push_back(&vertices[i]);
  }

  int i = 0;
  Vertex* v1, *v2;
  while (i < 1000) {
    // Select two vertices at random.
    v1 = vs[rand() % vs.size()];

    // Ensure we end up with a different vertex for v2.
    v2 = v1;
    while (v2 == v1) v2 = vs[rand() % vs.size()];

    reset_vertices(vertices);
    shortest_path_with_stats(v1, v2, edges);

    ++i;
  }

  // Find the edges which are highest priority.
  std::vector<Edge*> es;
  for (auto& e : edges) es.push_back(&e);

  std::sort(
    es.begin(),
    es.end(),
    [] (const auto& e1, const auto& e2) { return e1->hits < e2->hits; });

  // Remove the most "important" edges.
  std::vector<Edge*> top3(es.end() - 3, es.end());
  for (auto* e : top3) {
    std::cout << *e << " hits: " << e->hits << "\n";
    e->removed = true;
  }

  // Calc size of each component.
  int size1 = component_size(top3[0]->end1, edges);
  int size2 = component_size(top3[0]->end2, edges);

  std::cout << "Components: " << size1 << " " << size2 << "\n";
  std::cout << "P1: " << size1 * size2 << "\n";
}
