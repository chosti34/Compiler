#pragma once
#include <string>

enum class ExpressionTypeAST
{
	Int,
	Float,
	Bool,
	String
};

bool Convertible(ExpressionTypeAST from, ExpressionTypeAST to);
bool ConvertibleToBool(ExpressionTypeAST type);
std::string ToString(ExpressionTypeAST type);
