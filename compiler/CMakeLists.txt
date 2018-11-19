cmake_minimum_required(VERSION 3.9)
project(compiler)

set(SOURCE_FILES
        main.cpp
        IParser.h
        LLParser.cpp
        LLParser.h
        LLParserTable.cpp
        LLParserTable.h
        AST.h
        VariablesScopeChain.h
        ExpressionType.h
        SemanticsVerifier.h
        ASTVisitor.h
        SemanticsVerifier.cpp
        AST.cpp
        LLVMCodeGenerator.h LLVMCodeGenerator.cpp)

set(LLVM_DIR /usr/local/Cellar/llvm/7.0.0/lib/cmake/llvm/)
find_package(LLVM REQUIRED CONFIG)
if (LLVM_FOUND)
    include_directories(${LLVM_INCLUDE_DIRS})
    add_definitions(${LLVM_DEFINITIONS})
endif()

add_executable(compiler ${SOURCE_FILES})
target_compile_options(compiler PRIVATE -Wall -Wextra -pedantic)

llvm_map_components_to_libnames(llvm_libs support core irreader)
target_link_libraries(compiler utils grammarlib lexer ${Boost_LIBRARIES} ${llvm_libs})
