cmake_minimum_required(VERSION 3.9)
project(compiler)

set(SOURCE_FILES
        main.cpp IParser.h LLParser.cpp LLParser.h LLParserTable.cpp LLParserTable.h AST.h VariablesScopeChain.h ExpressionType.h SemanticsVerifier.h ASTVisitor.h SemanticsVerifier.cpp AST.cpp)

add_executable(compiler ${SOURCE_FILES})
target_compile_options(compiler PRIVATE -Wall -Wextra -pedantic)

target_link_libraries(compiler utils grammarlib lexer ${Boost_LIBRARIES})
