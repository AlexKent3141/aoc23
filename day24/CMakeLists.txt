cmake_minimum_required(VERSION 3.16)
project(aoc)

add_executable(d24 d24.cpp)

set_target_properties(d24
  PROPERTIES
    LANGUAGE CXX
    CXX_STANDARD 17)

target_compile_options(d24
  PRIVATE
    -O3
    -Wall
    -Wpedantic
    -Werror)
