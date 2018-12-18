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

// ���������� ���, � �������� ������ ���� ��������� ����� � ������ �����
//  ��������� ��������� ��� ���������� ���������
boost::optional<ExpressionTypeAST> GetPreferredTypeFromBinaryExpression(
	ExpressionTypeAST left, ExpressionTypeAST right);

std::string ToString(ExpressionTypeAST type);
