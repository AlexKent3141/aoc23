cmake_minimum_required(VERSION 3.16)
project(aoc)

add_executable(d1 d1.c)

set_target_properties(d1
  PROPERTIES
    LANGUAGE C)

target_compile_options(d1
  PRIVATE
    -Wall
    -Wpedantic
    -Werror)
