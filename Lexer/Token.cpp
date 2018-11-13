#include "stdafx.h"
#include "Token.h"
#include <boost/format.hpp>

namespace
{
const std::unordered_map<Token::Type, std::string> TOKEN_TYPE_TO_STRING = {
	// Meta
	{ Token::EndOfFile, "EndOfFile" },
	// Keyword
	{ Token::Func, "Func" },
	{ Token::Int, "Int" },
	{ Token::Float, "Float" },
	{ Token::Bool, "Bool" },
	{ Token::Array, "Array" },
	{ Token::If, "If" },
	{ Token::Else, "Else" },
	{ Token::While, "While" },
	{ Token::Var, "Var" },
	{ Token::Return, "Return" },
	{ Token::True, "True" },
	{ Token::False, "False" },
	// Mutable
	{ Token::Identifier, "Identifier" },
	{ Token::IntegerConstant, "IntegerConstant" },
	{ Token::FloatConstant, "FloatConstant" },
	// Separators
	{ Token::LeftParenthesis, "LeftParenthesis" },
	{ Token::RightParenthesis, "RightParenthesis" },
	{ Token::LeftBracket, "LeftBracket" },
	{ Token::RightBracket, "RightBracket" },
	{ Token::LeftCurly, "LeftCurly" },
	{ Token::RightCurly, "RightCurly" },
	{ Token::Arrow, "Arrow" },
	{ Token::Colon, "Colon" },
	{ Token::Comma, "Comma" },
	{ Token::Semicolon, "Semicolon" },
	// Operators
	{ Token::Assign, "Assign" },
	{ Token::Minus, "Minus" },
	{ Token::Plus, "Plus" },
	{ Token::Div, "Div" },
	{ Token::Mul, "Mul" }
};

const std::unordered_map<std::string, Token::Type> STRING_TO_TOKEN_TYPE = {
	// Meta
	{ "EndOfFile", Token::EndOfFile },
	// Keyword
	{ "Func", Token::Func },
	{ "Int", Token::Int },
	{ "Float", Token::Float },
	{ "Bool", Token::Bool },
	{ "Array", Token::Array },
	{ "If", Token::If },
	{ "Else", Token::Else },
	{ "While", Token::While },
	{ "Var", Token::Var },
	{ "Return", Token::Return },
	{ "True", Token::True },
	{ "False", Token::False },
	// Mutable
	{ "Identifier", Token::Identifier },
	{ "IntegerConstant", Token::IntegerConstant },
	{ "FloatConstant", Token::FloatConstant },
	// Separators
	{ "LeftParenthesis", Token::LeftParenthesis },
	{ "RightParenthesis", Token::RightParenthesis },
	{ "LeftBracket", Token::LeftBracket },
	{ "RightBracket", Token::RightBracket },
	{ "LeftCurly", Token::LeftCurly },
	{ "RightCurly", Token::RightCurly },
	{ "Arrow", Token::Arrow },
	{ "Colon", Token::Colon },
	{ "Comma", Token::Comma },
	{ "Semicolon", Token::Semicolon },
	// Operators
	{ "Assign", Token::Assign },
	{ "Minus", Token::Minus },
	{ "Plus", Token::Plus },
	{ "Div", Token::Div },
	{ "Mul", Token::Mul }
};
}

bool TokenExists(const std::string &token)
{
	auto found = STRING_TO_TOKEN_TYPE.find(token);
	return found != STRING_TO_TOKEN_TYPE.end();
}

std::string TokenTypeToString(Token::Type type)
{
	auto found = TOKEN_TYPE_TO_STRING.find(type);
	if (found == TOKEN_TYPE_TO_STRING.end())
	{
		throw std::logic_error("passed token type doesn't have string representation");
	}
	return found->second;
}

Token::Type StringToTokenType(const std::string &str)
{
	auto found = STRING_TO_TOKEN_TYPE.find(str);
	if (found == STRING_TO_TOKEN_TYPE.end())
	{
		throw std::logic_error("passed string doesn't have mapped token type");
	}
	return found->second;
}

std::string TokenToString(const Token &token)
{
	auto fmt = boost::format("Token(%1%2)")
		% TokenTypeToString(token.type)
		% (token.value ? ", " + *token.value : "");
	return fmt.str();
}
