cmake_minimum_required(VERSION 3.16)
project(aoc)

add_executable(d6 d6.cpp)

set_target_properties(d6
  PROPERTIES
    LANGUAGE CXX
    CXX_STANDARD 17)

target_compile_options(d6
  PRIVATE
    -O3
    -Wall
    -Wpedantic
    -Werror)
