#include "stdafx.h"
#include "TokenType.h"

std::string TokenTypeToString(TokenType type)
{
	static const std::unordered_map<TokenType, std::string> mapping = {
		{ TokenType::FunKeyword, "fun" },
		{ TokenType::IntKeyword, "int" },
		{ TokenType::FloatKeyword, "float" },
		{ TokenType::BoolKeyword, "bool" },
		{ TokenType::ArrayKeyword, "array" },
		{ TokenType::IfKeyword, "if" },
		{ TokenType::ElseKeyword, "else" },
		{ TokenType::WhileKeyword, "while" },
		{ TokenType::VarKeyword, "var" },
		{ TokenType::ReturnKeyword, "return" },
		{ TokenType::TrueKeyword, "true" },
		{ TokenType::FalseKeyword, "false" },
		{ TokenType::Identifier, "identifier" },
		{ TokenType::IntegerLiteral, "integralnum" },
		{ TokenType::FloatLiteral, "floatnum" },
		{ TokenType::LeftParenthesis, "lparen" },
		{ TokenType::RightParenthesis, "rparen" },
		{ TokenType::LeftAngleBracket, "langlebracket" },
		{ TokenType::RightAngleBracket, "rightanglebracket" },
		{ TokenType::LeftCurly, "lcurly" },
		{ TokenType::RightCurly, "rcurly" },
		{ TokenType::ArrowSeparator, "arrow" },
		{ TokenType::ColonSeparator, "colon" },
		{ TokenType::CommaSeparator, "comma" },
		{ TokenType::SemicolonSeparator, "semicolon" },
		{ TokenType::AssignOperator, "assign" },
		{ TokenType::End, "end" }
	};

	auto found = mapping.find(type);
	if (found == mapping.end())
	{
		assert(false);
		throw std::logic_error("can't convert token type to string");
	}
	return found->second;
}

TokenType StringToTokenType(const std::string& str)
{
	static const std::unordered_map<std::string, TokenType> mapping = {
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

	auto found = mapping.find(str);
	if (found == mapping.end())
	{
		assert(false);
		throw std::logic_error("can't convert string to token type");
	}
	return found->second;
}
