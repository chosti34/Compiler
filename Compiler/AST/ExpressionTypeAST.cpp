#include "stdafx.h"
#include "ExpressionTypeAST.h"

bool Convertible(ExpressionTypeAST from, ExpressionTypeAST to)
{
	static std::unordered_map<ExpressionTypeAST, std::unordered_set<ExpressionTypeAST>> table = {
		{ ExpressionTypeAST::Int, { ExpressionTypeAST::Float, ExpressionTypeAST::Bool } },
		{ ExpressionTypeAST::Float, { ExpressionTypeAST::Int, ExpressionTypeAST::Bool } }
	};

	auto found = table.find(from);
	if (found == table.end())
	{
		return false;
	}

	const auto& casts = found->second;
	return casts.find(to) != casts.end();
}

bool ConvertibleToBool(ExpressionTypeAST type)
{
	return Convertible(type, ExpressionTypeAST::Bool);
}

std::string ToString(ExpressionTypeAST type)
{
	switch (type)
	{
	case ExpressionTypeAST::Int:
		return "Int";
	case ExpressionTypeAST::Float:
		return "Float";
	case ExpressionTypeAST::Bool:
		return "Bool";
	case ExpressionTypeAST::String:
		return "String";
	default:
		throw std::logic_error("can't get string representation of undefined expression type");
	}
}
