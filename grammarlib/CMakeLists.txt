cmake_minimum_required(VERSION 3.9)
project(grammarlib)

set(CMAKE_CXX_STANDARD 17)
set(SOURCE_FILES
        Grammar.h
        Grammar.cpp
        GrammarProduction.h
        GrammarProduction.cpp
        GrammarSymbol.h
        GrammarSymbol.cpp
        GrammarFwd.h
        GrammarProductionFactory.h
        GrammarProductionFactory.cpp
        GrammarBuilder.h
        GrammarUtil.h
        GrammarUtil.cpp)

add_library(grammarlib SHARED ${SOURCE_FILES})
target_compile_options(grammarlib PRIVATE -Wall -Wextra -pedantic)

find_package(Boost REQUIRED)
if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIR})
    target_link_libraries(grammarlib ${Boost_LIBRARIES})
endif()
