#include "stdafx.h"
#include "Token.h"
#include <boost/bimap.hpp>

namespace
{
template <typename L, typename R>
boost::bimap<L, R> make_bimap(std::initializer_list<typename boost::bimap<L, R>::value_type> list)
{
	return boost::bimap<L, R>(list.begin(), list.end());
}

const auto TOKEN_STRING_BIMAP = make_bimap<Token::Type, std::string>({
	{ Token::EndOfFile, "EndOfFile" },

	{ Token::Func,   "Func" },
	{ Token::Int,    "Int" },
	{ Token::Float,  "Float" },
	{ Token::Bool,   "Bool" },
	{ Token::Array,  "Array" },
	{ Token::If,     "If" },
	{ Token::Print,  "Print" },
	{ Token::Else,   "Else" },
	{ Token::While,  "While" },
	{ Token::Var,    "Var" },
	{ Token::Return, "Return" },
	{ Token::True,   "True" },
	{ Token::False,  "False" },
	{ Token::String, "String" },

	{ Token::Identifier,      "Identifier" },
	{ Token::IntegerConstant, "IntegerConstant" },
	{ Token::FloatConstant,   "FloatConstant" },
	{ Token::StringConstant,  "StringConstant" },

	{ Token::Assign,             "Assign" },
	{ Token::LeftParenthesis,    "LeftParenthesis" },
	{ Token::RightParenthesis,   "RightParenthesis" },
	{ Token::LeftBracket,        "LeftBracket" },
	{ Token::RightBracket,       "RightBracket" },
	{ Token::LeftCurly,          "LeftCurly" },
	{ Token::RightCurly,         "RightCurly" },
	{ Token::Arrow,              "Arrow" },
	{ Token::Colon,              "Colon" },
	{ Token::Comma,              "Comma" },
	{ Token::Semicolon,          "Semicolon" },
	{ Token::LeftSquareBracket,  "LeftSquareBracket" },
	{ Token::RightSquareBracket, "RightSquareBracket" },

	{ Token::Minus,    "Minus" },
	{ Token::Plus,     "Plus" },
	{ Token::Div,      "Div" },
	{ Token::Mul,      "Mul" },
	{ Token::Mod,      "Mod" },
	{ Token::Or,       "Or" },
	{ Token::And,      "And" },
	{ Token::Equals,   "Equals" },
	{ Token::Negation, "Negation" }
});

const std::unordered_map<Token::Type, std::string> TOKEN_TYPE_TO_STRING_MAP = {
	// Meta
	{ Token::EndOfFile, "EndOfFile" },

	// Keyword
	{ Token::Func,   "Func" },
	{ Token::Int,    "Int" },
	{ Token::Float,  "Float" },
	{ Token::Bool,   "Bool" },
	{ Token::Array,  "Array" },
	{ Token::If,     "If" },
	{ Token::Print,  "Print" },
	{ Token::Else,   "Else" },
	{ Token::While,  "While" },
	{ Token::Var,    "Var" },
	{ Token::Return, "Return" },
	{ Token::True,   "True" },
	{ Token::False,  "False" },
	{ Token::String, "String" },

	// Mutable
	{ Token::Identifier,      "Identifier" },
	{ Token::IntegerConstant, "IntegerConstant" },
	{ Token::FloatConstant,   "FloatConstant" },
	{ Token::StringConstant,  "StringConstant" },

	// Separators
	{ Token::Assign,             "Assign" },
	{ Token::LeftParenthesis,    "LeftParenthesis" },
	{ Token::RightParenthesis,   "RightParenthesis" },
	{ Token::LeftBracket,        "LeftBracket" },
	{ Token::RightBracket,       "RightBracket" },
	{ Token::LeftCurly,          "LeftCurly" },
	{ Token::RightCurly,         "RightCurly" },
	{ Token::Arrow,              "Arrow" },
	{ Token::Colon,              "Colon" },
	{ Token::Comma,              "Comma" },
	{ Token::Semicolon,          "Semicolon" },
	{ Token::LeftSquareBracket,  "LeftSquareBracket" },
	{ Token::RightSquareBracket, "RightSquareBracket" },

	// Operators
	{ Token::Minus,    "Minus" },
	{ Token::Plus,     "Plus" },
	{ Token::Div,      "Div" },
	{ Token::Mul,      "Mul" },
	{ Token::Mod,      "Mod" },
	{ Token::Or,       "Or" },
	{ Token::And,      "And" },
	{ Token::Equals,   "Equals" },
	{ Token::Negation, "Negation" }
};

const std::unordered_map<std::string, Token::Type> STRING_TO_TOKEN_TYPE_MAP = {
	// Meta
	{ "EndOfFile", Token::EndOfFile },

	// Keyword
	{ "Func",   Token::Func },
	{ "Int",    Token::Int },
	{ "Float",  Token::Float },
	{ "Bool",   Token::Bool },
	{ "Array",  Token::Array },
	{ "If",     Token::If },
	{ "Print",  Token::Print },
	{ "Else",   Token::Else },
	{ "While",  Token::While },
	{ "Var",    Token::Var },
	{ "Return", Token::Return },
	{ "True",   Token::True },
	{ "False",  Token::False },
	{ "String", Token::String },

	// Mutable
	{ "Identifier",      Token::Identifier },
	{ "IntegerConstant", Token::IntegerConstant },
	{ "FloatConstant",   Token::FloatConstant },
	{ "StringConstant",  Token::StringConstant },

	// Separators
	{ "Assign",             Token::Assign },
	{ "LeftParenthesis",    Token::LeftParenthesis },
	{ "RightParenthesis",   Token::RightParenthesis },
	{ "LeftBracket",        Token::LeftBracket },
	{ "RightBracket",       Token::RightBracket },
	{ "LeftCurly",          Token::LeftCurly },
	{ "RightCurly",         Token::RightCurly },
	{ "Arrow",              Token::Arrow },
	{ "Colon",              Token::Colon },
	{ "Comma",              Token::Comma },
	{ "Semicolon",          Token::Semicolon },
	{ "LeftSquareBracket",  Token::LeftSquareBracket },
	{ "RightSquareBracket", Token::RightSquareBracket },

	// Operators
	{ "Minus",    Token::Minus },
	{ "Plus",     Token::Plus },
	{ "Div",      Token::Div },
	{ "Mul",      Token::Mul },
	{ "Mod",      Token::Mod },
	{ "Or",       Token::Or },
	{ "And",      Token::And },
	{ "Equals",   Token::Equals },
	{ "Negation", Token::Negation }
};
}

bool TokenTypeExists(const std::string& name)
{
	auto it = STRING_TO_TOKEN_TYPE_MAP.find(name);

	bool hasTokenRepresentation = it != STRING_TO_TOKEN_TYPE_MAP.end();
	if (!hasTokenRepresentation)
	{
		return false;
	}

	return TOKEN_TYPE_TO_STRING_MAP.find(it->second) != TOKEN_TYPE_TO_STRING_MAP.end();
}

std::string TokenTypeToString(Token::Type type)
{
	auto found = TOKEN_TYPE_TO_STRING_MAP.find(type);
	if (found == TOKEN_TYPE_TO_STRING_MAP.end())
	{
		throw std::logic_error("passed token type doesn't have string representation");
	}
	return found->second;
}

Token::Type StringToTokenType(const std::string& str)
{
	auto found = STRING_TO_TOKEN_TYPE_MAP.find(str);
	if (found == STRING_TO_TOKEN_TYPE_MAP.end())
	{
		throw std::logic_error("passed string doesn't have mapped token type");
	}
	return found->second;
}

std::string TokenToString(const Token& token)
{
	auto fmt = boost::format("Token(%1%%2%)")
		% TokenTypeToString(token.type)
		% (token.value ? ", " + *token.value : "");
	return fmt.str();
}
