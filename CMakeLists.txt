cmake_minimum_required(VERSION 3.9)
project(compiler_60min)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(utils)
add_subdirectory(grammarlib)
add_subdirectory(lexer)
add_subdirectory(compiler)
