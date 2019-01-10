#include "stdafx.h"
#include "TokenType.h"
#include <boost/bimap.hpp>

namespace
{
template <typename L, typename R>
boost::bimap<L, R> make_bimap(std::initializer_list<typename boost::bimap<L, R>::value_type> list)
{
	return boost::bimap<L, R>(list.begin(), list.end());
}

const auto TOKEN_STRING_BIMAP = make_bimap<TokenType, std::string>({
	{ TokenType::EndOfFile, "EndOfFile" },

	{ TokenType::Func,   "Func" },
	{ TokenType::Int,    "Int" },
	{ TokenType::Float,  "Float" },
	{ TokenType::Bool,   "Bool" },
	{ TokenType::Array,  "Array" },
	{ TokenType::If,     "If" },
	{ TokenType::Else,   "Else" },
	{ TokenType::While,  "While" },
	{ TokenType::Var,    "Var" },
	{ TokenType::Return, "Return" },
	{ TokenType::True,   "True" },
	{ TokenType::False,  "False" },
	{ TokenType::String, "String" },
	{ TokenType::Print,  "Print" },
	{ TokenType::Scan,   "Scan" },

	{ TokenType::Identifier,      "Identifier" },
	{ TokenType::IntegerConstant, "IntegerConstant" },
	{ TokenType::FloatConstant,   "FloatConstant" },
	{ TokenType::StringConstant,  "StringConstant" },

	{ TokenType::Assign,             "Assign" },
	{ TokenType::LeftParenthesis,    "LeftParenthesis" },
	{ TokenType::RightParenthesis,   "RightParenthesis" },
	{ TokenType::LeftAngleBracket,   "LeftBracket" },
	{ TokenType::RightAngleBracket,  "RightBracket" },
	{ TokenType::LeftSquareBracket,  "LeftSquareBracket" },
	{ TokenType::RightSquareBracket, "RightSquareBracket" },
	{ TokenType::LeftCurlyBrace,     "LeftCurly" },
	{ TokenType::RightCurlyBrace,    "RightCurly" },
	{ TokenType::Arrow,              "Arrow" },
	{ TokenType::Colon,              "Colon" },
	{ TokenType::Comma,              "Comma" },
	{ TokenType::Semicolon,          "Semicolon" },

	{ TokenType::Minus,    "Minus" },
	{ TokenType::Plus,     "Plus" },
	{ TokenType::Div,      "Div" },
	{ TokenType::Mul,      "Mul" },
	{ TokenType::Mod,      "Mod" },
	{ TokenType::Or,       "Or" },
	{ TokenType::And,      "And" },
	{ TokenType::Equals,   "Equals" },
	{ TokenType::NotEquals, "NotEquals" },
	{ TokenType::LessOrEquals, "LessOrEquals" },
	{ TokenType::MoreOrEquals, "MoreOrEquals" },
	{ TokenType::Negation, "Negation" }
});
}

bool TokenTypeExists(const std::string& name)
{
	auto it = TOKEN_STRING_BIMAP.right.find(name);
	return it != TOKEN_STRING_BIMAP.right.end();
}

std::string TokenTypeToString(TokenType type)
{
	auto it = TOKEN_STRING_BIMAP.left.find(type);
	if (it == TOKEN_STRING_BIMAP.left.end())
	{
		throw std::logic_error("passed token type doesn't have string representation");
	}
	return it->second;
}

TokenType StringToTokenType(const std::string& str)
{
	auto it = TOKEN_STRING_BIMAP.right.find(str);
	if (it == TOKEN_STRING_BIMAP.right.end())
	{
		throw std::logic_error("passed string doesn't have mapped token type");
	}
	return it->second;
}
