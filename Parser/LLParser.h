#pragma once
#include "IParser.h"
#include <memory>
#include <optional>
#include <vector>
#include <set>

class LLParser : public IParser
{
public:
	struct State
	{
		bool shift;
		bool error;
		bool push;
		bool end;
		std::set<std::string> beginnings;
	};

public:
	void AddState(const State& state);
	const State& GetState(size_t index)const;

	bool Parse(const std::string& text) override;

private:
	std::vector<State> m_states;
};

std::unique_ptr<LLParser> CreateLLParser();
