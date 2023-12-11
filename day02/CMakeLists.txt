cmake_minimum_required(VERSION 3.16)
project(aoc)

add_executable(d2 d2.c)

set_target_properties(d2
  PROPERTIES
    LANGUAGE C)

target_compile_options(d2
  PRIVATE
    -Wall
    -Wpedantic
    -Werror)
