#include "stdafx.h"
#include "LLParser.h"

void LLParser::AddState(const State& state)
{
	m_states.push_back(state);
}

const LLParser::State& LLParser::GetState(size_t index)const
{
	if (index >= m_states.size())
	{
		throw std::out_of_range("index must be less than states count");
	}
	return m_states[index];
}

bool LLParser::Parse(const std::string& text)
{
	(void)text;
	return false;
}

std::unique_ptr<LLParser> CreateLLParser()
{
	return nullptr;
}
