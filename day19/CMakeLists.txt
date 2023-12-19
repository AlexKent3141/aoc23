cmake_minimum_required(VERSION 3.16)
project(aoc)

add_executable(d19 d19.cpp)

set_target_properties(d19
  PROPERTIES
    LANGUAGE CXX
    CXX_STANDARD 17)

target_compile_options(d19
  PRIVATE
    -O3
    -Wall
    -Wpedantic
    -Werror)
