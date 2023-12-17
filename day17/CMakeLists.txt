cmake_minimum_required(VERSION 3.16)
project(aoc)

add_executable(d17 d17.cpp)

set_target_properties(d17
  PROPERTIES
    LANGUAGE CXX
    CXX_STANDARD 17)

target_compile_options(d17
  PRIVATE
    -O3
    -Wall
    -Wpedantic
    -Werror)
