#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <boost/optional.hpp>

template <typename Value>
class ScopeChain
{
public:
	void PushScope();
	void PopScope();

	void Define(const std::string& name, const Value& value);
	bool Assign(const std::string& name, const Value& value);
	boost::optional<Value> GetValue(const std::string& name);

private:
	std::vector<std::unordered_map<std::string, Value>> m_scopes;
};

template <typename Value>
void ScopeChain<Value>::PopScope()
{
	if (m_scopes.empty())
	{
		throw std::runtime_error("no scopes to pop");
	}
	m_scopes.pop_back();
}

template <typename Value>
void ScopeChain<Value>::PushScope()
{
	m_scopes.emplace_back();
}

template <typename Value>
void ScopeChain<Value>::Define(const std::string& name, const Value& value)
{
	if (m_scopes.empty())
	{
		throw std::logic_error("you can't define anything without creating a scope");
	}
	m_scopes.back()[name] = value;
}

template <typename Value>
bool ScopeChain<Value>::Assign(const std::string& name, const Value& value)
{
	for (auto it = m_scopes.rbegin(); it != m_scopes.rend(); ++it)
	{
		auto found = it->find(name);
		if (found != it->end())
		{
			found->second = value;
			return true;
		}
	}
	return false;
}

template <typename Value>
boost::optional<Value> ScopeChain<Value>::GetValue(const std::string& name)
{
	for (auto it = m_scopes.crbegin(); it != m_scopes.crend(); ++it)
	{
		auto found = it->find(name);
		if (found != it->end())
		{
			return found->second;
		}
	}
	return boost::none;
}
