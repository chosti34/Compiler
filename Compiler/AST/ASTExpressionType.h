#pragma once
#include <string>

enum class ASTExpressionType
{
	Int,
	Float,
	Bool,
	String
};

bool Convertible(ASTExpressionType from, ASTExpressionType to);
bool ConvertibleToBool(ASTExpressionType type);
std::string ToString(ASTExpressionType type);
