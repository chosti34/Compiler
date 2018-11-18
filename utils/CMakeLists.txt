cmake_minimum_required(VERSION 3.9)
project(utils)

set(SOURCE_FILES
        StringUtil.h
        StringUtil.cpp
        FileUtil.h
        FileUtil.cpp
        StreamUtil.h
        FormatUtil.h
        FormatUtil.cpp)

add_library(utils SHARED ${SOURCE_FILES})
target_compile_options(utils PRIVATE -Wall -Wextra -pedantic)
