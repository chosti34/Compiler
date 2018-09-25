#pragma once

#include "IParser.h"
#include "../Lexer/ILexer.h"
#include "../Lexer/TokenType.h"
#include "../grammarlib/Grammar.h"

#include <set>
#include <memory>
#include <vector>
#include <optional>

class LLParser : public IParser
{
public:
	struct State
	{
		std::string name;
		bool shift;
		bool error;
		bool push;
		bool end;
		std::optional<size_t> next;
		std::set<TokenType> beginnings;
	};

public:
	void AddState(std::shared_ptr<State> state);
	std::shared_ptr<State> GetState(size_t index);
	std::shared_ptr<const State> GetState(size_t index)const;
	size_t GetStatesCount()const;

	bool Parse(const std::string& text) override;

private:
	std::vector<std::shared_ptr<State>> m_states;
	std::unique_ptr<ILexer> m_lexer;
};

std::unique_ptr<LLParser> CreateLLParser(const Grammar& grammar);
