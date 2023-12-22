cmake_minimum_required(VERSION 3.16)
project(aoc)

add_executable(d22 d22.cpp)

set_target_properties(d22
  PROPERTIES
    LANGUAGE CXX
    CXX_STANDARD 17)

target_compile_options(d22
  PRIVATE
    -O3
    -Wall
    -Wpedantic
    -Werror)
