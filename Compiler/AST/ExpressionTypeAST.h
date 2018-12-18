#pragma once
#include <string>
#include <boost/optional.hpp>

enum class ExpressionTypeAST
{
	Int,
	Float,
	Bool,
	String
};

bool Convertible(ExpressionTypeAST from, ExpressionTypeAST to);
bool ConvertibleToBool(ExpressionTypeAST type);

// Возвращает тип, к которому должна быть приведена левая и правая часть
//  бинарного выражения для выполнения оператора
boost::optional<ExpressionTypeAST> GetPreferredTypeFromBinaryExpression(
	ExpressionTypeAST left, ExpressionTypeAST right);

std::string ToString(ExpressionTypeAST type);
