cmake_minimum_required(VERSION 3.16)
project(aoc)

add_executable(d20 d20.cpp)

set_target_properties(d20
  PROPERTIES
    LANGUAGE CXX
    CXX_STANDARD 17)

target_compile_options(d20
  PRIVATE
    -O3
    -Wall
    -Wpedantic
    -Werror)
