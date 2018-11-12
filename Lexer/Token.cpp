#include "stdafx.h"
#include "Token.h"
#include <boost/format.hpp>

namespace
{
const std::unordered_map<Token::Type, std::string> gcTokenTypes = {
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
}

std::string TokenTypeToString(Token::Type type)
{
	auto found = gcTokenTypes.find(type);
	if (found == gcTokenTypes.end())
	{
		throw std::logic_error("passed token type doesn't have string representation");
	}
	return found->second;
}

Token::Type StringToTokenType(const std::string &str)
{
	return Token::Type();
}

std::string TokenToString(const Token &token)
{
	auto fmt = boost::format("Token(%1%2)")
		% TokenTypeToString(token.type)
		% (token.value ? ", " + *token.value : "");
	return fmt.str();
}
