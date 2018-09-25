#include "stdafx.h"
#include "TokenType.h"

static const GrammarTokens GRAMMAR_TOKENS = {
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
	{ "ranglebracket", TokenType::RightAngleBracket },
	{ "lcurly", TokenType::LeftCurly },
	{ "rcurly", TokenType::RightCurly },
	{ "arrow", TokenType::ArrowSeparator },
	{ "colon", TokenType::ColonSeparator },
	{ "comma", TokenType::CommaSeparator },
	{ "semicolon", TokenType::SemicolonSeparator },
	{ "assign", TokenType::AssignOperator },
	{ "end", TokenType::End }
};

const GrammarTokens& GetGrammarTokens()
{
	return GRAMMAR_TOKENS;
}

std::string TokenTypeToString(TokenType type)
{
	auto found = std::find_if(GRAMMAR_TOKENS.begin(), GRAMMAR_TOKENS.end(), [&type](const auto& pair) {
		return type == pair.second;
	});
	if (found != GRAMMAR_TOKENS.end())
	{
		return found->first;
	}
	throw std::logic_error("no string mapping for token type: " + std::to_string(static_cast<int>(type)));
}

TokenType StringToTokenType(const std::string& str)
{
	auto found = std::find_if(GRAMMAR_TOKENS.begin(), GRAMMAR_TOKENS.end(), [&str](const auto& pair) {
		return str == pair.first;
	});
	if (found != GRAMMAR_TOKENS.end())
	{
		return found->second;
	}
	throw std::logic_error("no token type mapping for string: " + str);
}
