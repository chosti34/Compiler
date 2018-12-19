#include "stdafx.h"
#include "ExpressionType.h"

namespace
{
const std::unordered_map<ExpressionType, std::unordered_set<ExpressionType>> gcAvailableCasts = {
	{ ExpressionType::Int, { ExpressionType::Float, ExpressionType::Bool } },
	{ ExpressionType::Float, { ExpressionType::Int, ExpressionType::Bool } },
	{ ExpressionType::Bool, { ExpressionType::Int, ExpressionType::Float } }
};

const std::map<std::pair<ExpressionType, ExpressionType>, ExpressionType> gcBinaryCasts = {
	{ { ExpressionType::Int, ExpressionType::Float }, ExpressionType::Float },
	{ { ExpressionType::Float, ExpressionType::Int }, ExpressionType::Float }
};
}

bool Convertible(ExpressionType from, ExpressionType to)
{
	if (from == to)
	{
		throw std::runtime_error("trying to convert from '" + ToString(from) +  "' to itself");
	}

	auto found = gcAvailableCasts.find(from);
	if (found == gcAvailableCasts.end())
	{
		return false;
	}

	const std::unordered_set<ExpressionType> & availableCasts = found->second;
	return availableCasts.find(to) != availableCasts.end();
}

bool ConvertibleToBool(ExpressionType type)
{
	return Convertible(type, ExpressionType::Bool);
}

boost::optional<ExpressionType> GetPreferredType(
	ExpressionType left, ExpressionType right)
{
	if (left == right)
	{
		return left;
	}

	auto found = gcBinaryCasts.find(std::make_pair(left, right));
	if (found == gcBinaryCasts.end())
	{
		return boost::none;
	}
	return found->second;
}

std::string ToString(ExpressionType type)
{
	switch (type)
	{
	case ExpressionType::Int:
		return "Int";
	case ExpressionType::Float:
		return "Float";
	case ExpressionType::Bool:
		return "Bool";
	case ExpressionType::String:
		return "String";
	default:
		throw std::logic_error("can't get string representation of undefined expression type");
	}
}
