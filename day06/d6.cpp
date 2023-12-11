#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

std::vector<int> parse_array(const std::string& line, std::int64_t& combined)
{
  std::vector<int> data;
  std::stringstream ss(line);
  std::string token, concat;
  while (ss >> token)
  {
    data.push_back(std::stoi(token));
    concat += token;
  }

  combined = std::stoull(concat);
  return data;
}

std::int64_t count_winning_times(std::int64_t time, std::int64_t record)
{
  // Need double precision for P2.
  double td = time;
  double rd = record;

  double upper = 0.5 * (td + std::sqrt(td * td - 4 * rd));
  double lower = 0.5 * (td - std::sqrt(td * td - 4 * rd));

  // Annoying special case when the bounds are integers.
  if (upper == std::round(upper) && lower == std::round(lower))
  {
    return upper - lower - 1;
  }

  return std::floor(upper) - std::ceil(lower) + 1;
}

int main()
{
  std::ifstream fs("input.txt");
  std::string line;
  std::vector<int> times, distances;
  std::int64_t times2 = 0, distances2 = 0;
  while (std::getline(fs, line))
  {
    line = line.substr(12);
    if (times.empty())
    {
      times = parse_array(line, times2);
    }
    else
    {
      distances = parse_array(line, distances2);
    }
  }

  int prod = 1;
  for (std::size_t i = 0; i < times.size(); i++)
  {
    std::cout << times[i] << " " << distances[i] << std::endl;

    int count = count_winning_times(times[i], distances[i]);
    std::cout << "Count: " << count << std::endl;
    prod *= count;
  }

  std::cout << "P2 input: " << times2 << " " << distances2 << std::endl;
  int count = count_winning_times(times2, distances2);

  std::cout << "P1: " << prod << ", P2: " << count << std::endl;

  return 0;
}
