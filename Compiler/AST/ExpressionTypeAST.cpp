#include "stdafx.h"
#include "ExpressionTypeAST.h"

namespace
{
const std::unordered_map<ExpressionTypeAST, std::unordered_set<ExpressionTypeAST>> gcCasts = {
	{ ExpressionTypeAST::Int, { ExpressionTypeAST::Float, ExpressionTypeAST::Bool } },
	{ ExpressionTypeAST::Float, { ExpressionTypeAST::Int, ExpressionTypeAST::Bool } },
	{ ExpressionTypeAST::Bool, { ExpressionTypeAST::Int, ExpressionTypeAST::Float } }
};
}

bool Convertible(ExpressionTypeAST from, ExpressionTypeAST to)
{
	if (from == to)
	{
		throw std::runtime_error("trying to convert from '" + ToString(from) +  "' to itself");
	}

	auto found = gcCasts.find(from);
	if (found == gcCasts.end())
	{
		return false;
	}

	const std::unordered_set<ExpressionTypeAST> & availableCasts = found->second;
	return availableCasts.find(to) != availableCasts.end();
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
