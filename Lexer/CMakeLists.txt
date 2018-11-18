cmake_minimum_required(VERSION 3.9)
project(lexer)

set(CMAKE_CXX_STANDARD 17)
set(SOURCE_FILES
        Lexer.h
        ILexer.h
        Token.h
        Token.cpp Lexer.cpp)

add_library(lexer SHARED ${SOURCE_FILES})
target_compile_options(lexer PRIVATE -Wall -Wextra -pedantic)

find_package(Boost REQUIRED)
if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIR})
    target_link_libraries(lexer ${Boost_LIBRARIES})
endif()
