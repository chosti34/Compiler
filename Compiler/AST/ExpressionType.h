#pragma once
#include <string>
#include <boost/optional.hpp>

struct ExpressionType
{
	enum Value
	{
		Int,
		Float,
		Bool,
		String
	};

	Value value;
	unsigned nesting = 0;
};

bool operator ==(const ExpressionType& left, const ExpressionType& right);
bool operator !=(const ExpressionType& left, const ExpressionType& right);
std::string ToString(const ExpressionType& type);

bool Convertible(const ExpressionType& from, const ExpressionType& to);
bool ConvertibleToBool(const ExpressionType& type);

// ���������� ���, � �������� ������ ���� ��������� ����� � ������ �����
//  ��������� ��������� ��� ���������� ���������
boost::optional<ExpressionType> GetPreferredType(const ExpressionType& left, const ExpressionType& right);
