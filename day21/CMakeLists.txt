cmake_minimum_required(VERSION 3.16)
project(aoc)

add_executable(d21 d21.cpp)

set_target_properties(d21
  PROPERTIES
    LANGUAGE CXX
    CXX_STANDARD 17)

target_compile_options(d21
  PRIVATE
    -O3
    -Wall
    -Wpedantic
    -Werror)
