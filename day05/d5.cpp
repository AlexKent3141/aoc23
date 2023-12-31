#include <fstream>
#include <iostream>
#include <limits>
#include <string>
#include <sstream>
#include <vector>

struct Range
{
  std::int64_t dest_start;
  std::int64_t source_start;
  std::int64_t length;
};

std::int64_t stage_input_for_output(
  const std::vector<Range>& stage,
  std::int64_t output)
{
  for (const Range& r : stage)
  {
    std::int64_t potential_in = r.source_start + output - r.dest_start;
    if (potential_in >= r.source_start && potential_in < r.source_start + r.length)
    {
      return potential_in;
    }
  }

  return -1;
}

bool does_seed_exist(std::int64_t seed, const std::vector<std::int64_t>& seeds)
{
  for (std::size_t i = 0; i < seeds.size() / 2; i++)
  {
    std::int64_t upper = seeds[2*i] + seeds[2*i + 1];
    if (seed < upper && seed >= seeds[2*i])
    {
      return true;
    }
  }

  return false;
}

int main()
{
  std::ifstream fs("input.txt");
  std::string line;
  std::vector<std::int64_t> seeds;
  std::vector<std::vector<Range>> almanac;
  std::vector<Range> current;
  while (std::getline(fs, line))
  {
    std::cout << line << std::endl;
    if (line.substr(0, 5) == "seeds")
    {
      // Parse seeds array.
      std::stringstream ss(line);
      std::string token;
      ss >> token;
      while (ss >> token)
      {
        seeds.push_back(std::stoull(token));
      }

      // Skip empty line.
      std::getline(fs, line);
    }
    else
    {
      if (std::isalpha(line[0]))
      {
        if (!current.empty()) almanac.push_back(current);
        current.clear();
      }
      else
      {
        Range r;
        std::sscanf(line.c_str(), "%ld %ld %ld", &r.dest_start, &r.source_start, &r.length);
        current.push_back(r);
      }
    }
  }

  if (!current.empty()) almanac.push_back(current);

  auto lowest = std::numeric_limits<std::int64_t>::max();
  for (auto seed : seeds)
  {
    for (const auto& stage : almanac)
    {
      // Find the range which applies to this input.
      for (const Range& r : stage)
      {
        if (seed >= r.source_start && seed < r.source_start + r.length)
        {
          seed += (r.dest_start - r.source_start);
          break;
        }
      }
    }

    lowest = std::min(lowest, seed);
  }

  // P2.
  // Backtrack and see what the lowest possible location we can reach is.
  // To make this more efficient:
  // 1) Step up from 0 in increments of 1000 until we find a valid seed.
  // 2) Now step downwards one at a time until we are no longer a valid seed.
  // This only works because the minimum seed is in a "range" of at least 1000 seeds.
  // Seems like quite a safe bet given the large seed ranges we're dealing with.
  std::int64_t test = -1;
  std::int64_t step = 1000;
  bool solution_found = false;
  while (!solution_found)
  {
next_try:
    test += step;

    auto current = test;
    for (int i = almanac.size() - 1; i >= 0; i--)
    {
      current = stage_input_for_output(almanac[i], current);
      if (current < 0) goto next_try;
    }

    // Is this in a seed range?
    if (does_seed_exist(current, seeds))
    {
      if (step == 1000)
      {
        step = -1;
      }
    }
    else if (step == -1)
    {
      test -= step;
      solution_found = true;
    }
  }

  std::cout << "P1: " << lowest << ", P2: " << test << std::endl;

  return 0;
}
