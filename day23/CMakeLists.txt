cmake_minimum_required(VERSION 3.16)
project(aoc)

add_executable(d23 d23.cpp)

set_target_properties(d23
  PROPERTIES
    LANGUAGE CXX
    CXX_STANDARD 17)

target_compile_options(d23
  PRIVATE
    -O3
    -Wall
    -Wpedantic
    -Werror)
