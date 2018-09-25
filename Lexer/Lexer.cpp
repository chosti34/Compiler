#include "stdafx.h"
#include "Lexer.h"

const std::vector<std::pair<std::string, TokenType>> Lexer::TOKENS = {
		{ "fun", TokenType::FunKeyword },
		{ "int", TokenType::IntKeyword },
		{ "float", TokenType::FloatKeyword },
		{ "bool", TokenType::BoolKeyword },
		{ "array", TokenType::ArrayKeyword },
		{ "if", TokenType::IfKeyword },
		{ "else", TokenType::ElseKeyword },
		{ "while", TokenType::WhileKeyword },
		{ "var", TokenType::VarKeyword },
		{ "return", TokenType::ReturnKeyword },
		{ "true", TokenType::TrueKeyword },
		{ "false", TokenType::FalseKeyword },
		{ "identifier", TokenType::Identifier },
		{ "integralnum", TokenType::IntegerLiteral },
		{ "floatnum", TokenType::FloatLiteral },
		{ "lparen", TokenType::LeftParenthesis },
		{ "rparen", TokenType::RightParenthesis },
		{ "langlebracket", TokenType::LeftAngleBracket },
		{ "rightanglebracket", TokenType::RightAngleBracket },
		{ "lcurly", TokenType::LeftCurly },
		{ "rcurly", TokenType::RightCurly },
		{ "arrow", TokenType::ArrowSeparator },
		{ "colon", TokenType::ColonSeparator },
		{ "comma", TokenType::CommaSeparator },
		{ "semicolon", TokenType::SemicolonSeparator },
		{ "assign", TokenType::AssignOperator },
		{ "end", TokenType::End }
};
