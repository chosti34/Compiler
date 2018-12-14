#include "stdafx.h"
#include "ASTExpressionType.h"

bool Convertible(ASTExpressionType from, ASTExpressionType to)
{
	static std::unordered_map<ASTExpressionType, std::unordered_set<ASTExpressionType>> table = {
		{ ASTExpressionType::Int, { ASTExpressionType::Float, ASTExpressionType::Bool } },
		{ ASTExpressionType::Float, { ASTExpressionType::Int, ASTExpressionType::Bool } }
	};

	auto found = table.find(from);
	if (found == table.end())
	{
		return false;
	}

	const auto& casts = found->second;
	return casts.find(to) != casts.end();
}

bool ConvertibleToBool(ASTExpressionType type)
{
	return Convertible(type, ASTExpressionType::Bool);
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
