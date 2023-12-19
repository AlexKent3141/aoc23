#include <algorithm>
#include <array>
#include <fstream>
#include <iostream>
#include <optional>
#include <queue>
#include <string>
#include <sstream>
#include <vector>

struct Part {
  std::array<int, 4> categories;
};

// An interval over all categories.
struct Interval {
  std::string target_label;
  std::array<std::pair<int, int>, 4> category_intervals;

  std::uint64_t num_elements() const {
    std::uint64_t prod = 1;
    for (const auto& i : category_intervals) {
      prod *= (i.second - i.first + 1);
    }
    return prod;
  }
};

struct Step {
  int category;
  char op;
  int val;
  std::string output_label;

  bool is_fallthrough() const {
    return op == ' ';
  }

  bool matches_part(const Part& p) const {
    if (is_fallthrough()) return true; // An unavoidable step.

    const int part_val = p.categories[category];
    if (op == '>') return part_val > val;
    else return part_val < val;
  }

  // First interval is the matching one, second is the non-matching one.
  std::pair<std::optional<Interval>, std::optional<Interval>> match_interval(const Interval& i) const {
    if (is_fallthrough()) {
      Interval fallthrough_interval(i);
      fallthrough_interval.target_label = output_label;
      return std::make_pair(fallthrough_interval, std::nullopt);
    }

    const auto& category_interval = i.category_intervals[category];
    if (op == '<') {
      if (category_interval.first >= val) {
        // No matches.
        return std::make_pair(std::nullopt, i);
      }

      if (category_interval.second < val) {
        // Whole thing matches.
        return std::make_pair(i, std::nullopt);
      }

      // Need to split.
      Interval matching_interval(i);
      matching_interval.target_label = output_label;
      matching_interval.category_intervals[category].second = val - 1;
      Interval fallthrough_interval(i);
      fallthrough_interval.category_intervals[category].first = val;
      return std::make_pair(matching_interval, fallthrough_interval);
    }
    else {
      if (category_interval.second <= val) {
        // No matches.
        return std::make_pair(std::nullopt, i);
      }

      if (category_interval.first > val) {
        // Whole thing matches.
        return std::make_pair(i, std::nullopt);
      }

      // Need to split.
      Interval matching_interval(i);
      matching_interval.target_label = output_label;
      matching_interval.category_intervals[category].first = val + 1;
      Interval fallthrough_interval(i);
      fallthrough_interval.category_intervals[category].second = val;
      return std::make_pair(matching_interval, fallthrough_interval);
    }
  }
};

struct Workflow {
  std::string label;
  std::vector<Step> steps;

  std::string execute(const Part& p) const {
    for (const auto& step : steps) {
      if (step.matches_part(p)) {
        return step.output_label;
      }
    }

    std::abort();
  }
};

const Workflow& find_workflow(const std::string& label, const std::vector<Workflow>& workflows) {
  for (const auto& w : workflows) {
    if (w.label == label) return w;
  }

  std::abort();
}

bool is_accepted(const Part& p, const std::vector<Workflow>& workflows) {
  // Locate the "in" workflow.
  std::string label = "in";
  while (label != "A" && label != "R") {
    const Workflow& w = find_workflow(label, workflows);
    label = w.execute(p);
  }

  return label == "A";
}

std::uint64_t count_all_possible_parts(const std::vector<Workflow>& workflows) {
  // Do interval calculation to find all possible parts which would be accepted.
  std::uint64_t total = 0;

  Interval start {
    "in",
    std::make_pair(1, 4000),
    std::make_pair(1, 4000),
    std::make_pair(1, 4000),
    std::make_pair(1, 4000)
  };

  std::queue<Interval> todo;
  todo.push(start);
  while (!todo.empty()) {
    auto current = todo.front();
    todo.pop();

    // Feed this interval through its target workflow and create new intervals.
    // We can resolve intervals when end up with the "A" or "R" target.
    const auto& w = find_workflow(current.target_label, workflows);
    for (const auto& s : w.steps) {
      // Feed the current interval through this step.
      // In the common case some portion on the interval will match and some will not causing it to fall through to
      // the next step.
      auto [matching, passthrough] = s.match_interval(current);

      if (matching) {
        if (matching->target_label == "A") {
          total += matching->num_elements();
        }
        else if (matching->target_label != "R") {
          // Enqueue this interval.
          todo.push(*matching);
        }
      }

      if (passthrough) {
        // Try to match this interval on the next step.
        current = *passthrough;
      }
    }
  }

  return total;
}

int main() {
  std::ifstream fs("input.txt");
  std::string line, token;
  std::vector<Workflow> workflows;
  std::vector<Part> parts;
  bool in_parts = false;
  while (std::getline(fs, line)) {
    if (line.empty()) {
      in_parts = true;
      continue;
    }

    // We're going to need to split by each of "{}:,".
    std::replace(line.begin(), line.end(), '{', ' ');
    std::replace(line.begin(), line.end(), '}', ' ');
    std::replace(line.begin(), line.end(), ':', ' ');
    std::replace(line.begin(), line.end(), ',', ' ');

    std::stringstream ss(line);

    if (in_parts) {
      Part p;
      ss >> token; p.categories[0] = std::stoi(token.substr(2));
      ss >> token; p.categories[1] = std::stoi(token.substr(2));
      ss >> token; p.categories[2] = std::stoi(token.substr(2));
      ss >> token; p.categories[3] = std::stoi(token.substr(2));
      parts.push_back(p);
    }
    else {
      Workflow w;
      ss >> token; w.label = token;
      while (ss) {
        Step s;
        ss >> token;

        // Is this a comparison or a label?
        const bool is_comparison = token.size() > 1 && (token[1] == '<' || token[1] == '>');
        if (is_comparison) {
          // Parse the operation.
          switch (token[0]) {
            case 'x': s.category = 0; break;
            case 'm': s.category = 1; break;
            case 'a': s.category = 2; break;
            case 's': s.category = 3; break;
          }
          s.op = token[1];
          s.val = std::stoi(token.substr(2));
          ss >> s.output_label;
        }
        else {
          // This is the fallthrough label.
          s.category = ' ';
          s.val = 0;
          s.op = ' ';
          s.output_label = token;
          ss >> token;
        }

        w.steps.push_back(s);
      }

      workflows.push_back(w);
    }
  }

  int total1 = 0;
  for (const auto& p : parts) {
    if (is_accepted(p, workflows)) {
      total1 += p.categories[0] + p.categories[1] + p.categories[2] + p.categories[3];
    }
  }

  const std::uint64_t total2 = count_all_possible_parts(workflows);

  std::cout << "P1: " << total1 << ", P2: " << total2 << "\n";

  return EXIT_SUCCESS;
}
