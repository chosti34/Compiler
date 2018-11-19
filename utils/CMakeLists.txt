cmake_minimum_required(VERSION 3.9)
project(utils)

set(SOURCE_FILES
        string_utils.h
        string_utils.cpp
        file_utils.h
        file_utils.cpp
        stream_utils.h
        format_utils.h
        format_utils.cpp)

add_library(utils SHARED ${SOURCE_FILES})
target_compile_options(utils PRIVATE -Wall -Wextra -pedantic)
