#pragma once
#include <string>
#include <boost/optional.hpp>

enum class ExpressionType
{
	Int,
	Float,
	Bool,
	String,
	ArrayInt,
	ArrayFloat,
	ArrayBool,
	ArrayString
};

bool Convertible(ExpressionType from, ExpressionType to);
bool ConvertibleToBool(ExpressionType type);

// Возвращает тип, к которому должна быть приведена левая и правая часть
//  бинарного выражения для выполнения оператора
boost::optional<ExpressionType> GetPreferredType(ExpressionType left, ExpressionType right);

std::string ToString(ExpressionType type);
