cmake_minimum_required(VERSION 3.16)
project(aoc)

add_executable(d8 d8.cpp)

set_target_properties(d8
  PROPERTIES
    LANGUAGE CXX
    CXX_STANDARD 17)

target_compile_options(d8
  PRIVATE
    -O3
    -Wall
    -Wpedantic
    -Werror)
