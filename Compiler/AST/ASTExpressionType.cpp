#include "stdafx.h"
#include "ASTExpressionType.h"

bool ConvertibleToBool(ASTExpressionType type)
{
	switch (type)
	{
	case ASTExpressionType::Int:
	case ASTExpressionType::Bool:
	case ASTExpressionType::Float:
		return true;
	case ASTExpressionType::String:
		return false;
	default:
		throw std::logic_error("undefined ast expression type is not convertible to bool");
	}
}

std::string ToString(ASTExpressionType type)
{
	switch (type)
	{
	case ASTExpressionType::Int:
		return "Int";
	case ASTExpressionType::Float:
		return "Float";
	case ASTExpressionType::Bool:
		return "Bool";
	case ASTExpressionType::String:
		return "String";
	default:
		throw std::logic_error("can't get string representation of undefined expression type");
	}
}
