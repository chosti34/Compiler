#pragma once

#include <vector>
#include <memory>
#include <string>
#include <unordered_map>

#include <boost/optional.hpp>
#include <boost/variant.hpp>
#include <boost/range/adaptor/reversed.hpp>

#include "ExpressionType.h"

class Value
{
public:
	using ValueType = boost::variant<
		int,
		float,
		bool,
		std::string
	>;

	Value(const ValueType& value)
		: m_value(value)
	{
		SetExpressionType();
	}

	Value(ExpressionType type)
		: m_type(type)
	{
	}

	Value(const Value& other)
		: m_value(other.m_value)
		, m_type(other.m_type)
	{
	}

	int AsInt()const
	{
		if (const int* integer = boost::get<int>(&m_value))
		{
			return *integer;
		}
		throw std::runtime_error("can't cast value to int");
	}

	float AsFloat()const
	{
		if (const float* floating = boost::get<float>(&m_value))
		{
			return *floating;
		}
		throw std::runtime_error("can't cast value to float");
	}

	bool AsBool()const
	{
		if (const bool* boolean = boost::get<bool>(&m_value))
		{
			return *boolean;
		}
		throw std::runtime_error("can't cast value to bool");
	}

	std::string AsString()const
	{
		if (const std::string* str = boost::get<std::string>(&m_value))
		{
			return *str;
		}
		throw std::runtime_error("can't cast value to string");
	}

	Value& operator =(const Value& other)
	{
		m_value = other.m_value;
		SetExpressionType();
	}

	ExpressionType GetExpressionType()const
	{
		return m_type;
	}

private:
	ExpressionType SetExpressionType()
	{
		if (m_value.type() == typeid(int))
		{
			m_type = ExpressionType::Int;
		}
		else if (m_value.type() == typeid(float))
		{
			m_type = ExpressionType::Float;
		}
		else if (m_value.type() == typeid(bool))
		{
			m_type = ExpressionType::Bool;
		}
		else if (m_value.type() == typeid(std::string))
		{
			m_type = ExpressionType::String;
		}
		throw std::logic_error("can't cast value variant to expression type");
	}

	void SetDefaultValue()
	{
		switch (m_type)
		{
		case ExpressionType::Int:
			m_value = 0;
			break;
		case ExpressionType::Float:
			m_value = 0.f;
			break;
		case ExpressionType::Bool:
			m_value = false;
			break;
		case ExpressionType::String:
			m_value = "";
			break;
		default:
			throw std::logic_error("can't set default value of undefined type");
		}
	}

private:
	ValueType m_value;
	ExpressionType m_type;
};

class VariablesScope
{
public:
	void Define(const std::string& name, const Value& value)
	{
		m_scope.emplace(name, value);
	}

	Value* GetValue(const std::string& name)
	{
		auto it = m_scope.find(name);
		if (it != m_scope.end())
		{
			return &it->second;
		}
		return nullptr;
	}

	const Value* GetValue(const std::string& name)const
	{
		auto it = m_scope.find(name);
		if (it != m_scope.end())
		{
			return &it->second;
		}
		return nullptr;
	}

private:
	std::unordered_map<std::string, Value> m_scope;
};

class ScopeChain
{
public:
	void PushScope()
	{
		m_chain.emplace_back();
	}

	void PopScope()
	{
		m_chain.pop_back();
	}

	void Define(const std::string& name, const Value& value)
	{
		m_chain.back().Define(name, value);
	}

	Value* GetValue(const std::string& name)
	{
		for (VariablesScope& scope : boost::adaptors::reverse(m_chain))
		{
			if (Value* value = scope.GetValue(name))
			{
				return value;
			}
		}
		return nullptr;
	}

	const Value* GetValue(const std::string& name)const
	{
		for (const VariablesScope& scope : boost::adaptors::reverse(m_chain))
		{
			if (const Value* value = scope.GetValue(name))
			{
				return value;
			}
		}
		return nullptr;
	}

private:
	std::vector<VariablesScope> m_chain;
};
