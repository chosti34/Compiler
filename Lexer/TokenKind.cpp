#include "stdafx.h"
#include "TokenKind.h"
#include <stdexcept>
#include <unordered_map>

namespace
{
const std::unordered_map<TokenKind, std::string> TOKENS = {
	// Metas
	{ TokenKind::EndOfFile, "EndOfFile"},

	// Keywords
	{ TokenKind::Func, "Func" },
	{ TokenKind::Int, "Int" },
	{ TokenKind::Float, "Float" },
	{ TokenKind::Bool, "Bool" },
	{ TokenKind::Array, "Array" },
	{ TokenKind::If, "If" },
	{ TokenKind::Else, "Else" },
	{ TokenKind::While, "While" },
	{ TokenKind::Var, "Var" },
	{ TokenKind::Return, "Return" },
	{ TokenKind::True, "True" },
	{ TokenKind::False, "False" },

	// Mutables
	{ TokenKind::Identifier, "Identifier" },
	{ TokenKind::IntegerConstant, "IntegerConstant" },
	{ TokenKind::FloatConstant, "FloatConstant" },

	// Separators
	{ TokenKind::LeftParenthesis, "LeftParenthesis" },
	{ TokenKind::RightParenthesis, "RightParenthesis" },
	{ TokenKind::LeftBracket, "LeftBracket" },
	{ TokenKind::RightBracket, "RightBracket" },
	{ TokenKind::LeftCurly, "LeftCurly" },
	{ TokenKind::RightCurly, "RightCurly" },
	{ TokenKind::Arrow, "Arrow" },
	{ TokenKind::Colon, "Colon" },
	{ TokenKind::Comma, "Comma" },
	{ TokenKind::Semicolon, "Semicolon" },

	// Operators
	{ TokenKind::Assign, "Assign" },
	{ TokenKind::Minus, "Minus" },
	{ TokenKind::Plus, "Plus" },
	{ TokenKind::Div, "Div" },
	{ TokenKind::Mul, "Mul" }
};
}

std::string ToString(TokenKind kind)
{
	auto it = TOKENS.find(kind);
	if (it == TOKENS.end())
	{
		throw std::logic_error("string for passed token kind is undefined");
	}
	return it->second;
}
