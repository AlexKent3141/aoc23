cmake_minimum_required(VERSION 3.16)
project(aoc)

add_executable(d5 d5.cpp)

set_target_properties(d5
  PROPERTIES
    LANGUAGE CXX
    CXX_STANDARD 17)

target_compile_options(d5
  PRIVATE
    -O3
    -Wall
    -Wpedantic
    -Werror)
