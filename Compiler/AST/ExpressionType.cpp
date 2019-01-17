#include "stdafx.h"
#include "ExpressionType.h"

namespace
{
const std::unordered_map<ExpressionType::Value, std::unordered_set<ExpressionType::Value>> gcAvailableCasts = {
	{ ExpressionType::Int, { ExpressionType::Float, ExpressionType::Bool } },
	{ ExpressionType::Float, { ExpressionType::Int, ExpressionType::Bool } },
	{ ExpressionType::Bool, { ExpressionType::Int, ExpressionType::Float } }
};

const std::map<std::pair<ExpressionType::Value, ExpressionType::Value>, ExpressionType::Value> gcBinaryCasts = {
	{ { ExpressionType::Int, ExpressionType::Float }, ExpressionType::Float },
	{ { ExpressionType::Int, ExpressionType::Bool }, ExpressionType::Int },
	{ { ExpressionType::Float, ExpressionType::Int }, ExpressionType::Float },
	{ { ExpressionType::Float, ExpressionType::Bool }, ExpressionType::Float },
	{ { ExpressionType::Bool, ExpressionType::Int }, ExpressionType::Int },
	{ { ExpressionType::Bool, ExpressionType::Float }, ExpressionType::Float }
};
}

bool operator ==(const ExpressionType& left, const ExpressionType& right)
{
	return left.value == right.value && left.nesting == right.nesting;
}

bool operator !=(const ExpressionType& left, const ExpressionType& right)
{
	return !(left == right);
}

bool Convertible(const ExpressionType& from, const ExpressionType& to)
{
	if (from.nesting != 0 || to.nesting != 0)
	{
		throw std::runtime_error("array conversion is not supported");
	}

	if (from == to)
	{
		throw std::runtime_error("trying to convert from '" + ToString(from) +  "' to itself");
	}

	auto found = gcAvailableCasts.find(from.value);
	if (found == gcAvailableCasts.end())
	{
		return false;
	}

	const std::unordered_set<ExpressionType::Value>& availableCasts = found->second;
	return availableCasts.find(to.value) != availableCasts.end();
}

bool ConvertibleToBool(const ExpressionType& type)
{
	return Convertible(type, { ExpressionType::Bool, 0 });
}

boost::optional<ExpressionType> GetPreferredType(const ExpressionType& left, const ExpressionType& right)
{
	if (left.nesting != 0 || right.nesting != 0)
	{
		return boost::none;
	}

	if (left == right)
	{
		return left;
	}

	auto found = gcBinaryCasts.find(std::make_pair(left.value, right.value));
	if (found == gcBinaryCasts.end())
	{
		return boost::none;
	}
	return ExpressionType{ found->second, 0 };
}

std::string ToString(const ExpressionType& type)
{
	// Type can represent array
	if (type.nesting != 0)
	{
		const ExpressionType stored = { type.value, type.nesting - 1 };
		return "Array<" + ToString(stored) + ">";
	}

	switch (type.value)
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
