cmake_minimum_required(VERSION 3.16)
project(aoc)

add_executable(d25 d25.cpp)

set_target_properties(d25
  PROPERTIES
    LANGUAGE CXX
    CXX_STANDARD 17)

target_compile_options(d25
  PRIVATE
    -O3
    -Wall
    -Wpedantic
    -Werror)
