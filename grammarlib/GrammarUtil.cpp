#include "stdafx.h"
#include "GrammarUtil.h"
#include <stack>

std::set<int> GatherProductionIndices(const Grammar& grammar, const std::string& nonterminal)
{
	std::set<int> indices;
	for (size_t i = 0; i < grammar.GetProductionsCount(); ++i)
	{
		const auto production = grammar.GetProduction(i);
		if (production->GetLeftPart() == nonterminal)
		{
			indices.insert(static_cast<int>(i));
		}
	}
	return indices;
}

std::set<int> GatherProductionIndices(const Grammar& grammar, std::function<bool(const GrammarProduction&)> && predicate)
{
	std::set<int> indices;
	for (size_t i = 0; i < grammar.GetProductionsCount(); ++i)
	{
		const auto production = grammar.GetProduction(i);
		if (predicate(*production))
		{
			indices.insert(static_cast<int>(i));
		}
	}
	return indices;
}

bool ProductionConsistsOfNonterminals(const GrammarProduction& production)
{
	assert(production.GetSymbolsCount() != 0);
	for (size_t i = 0; i < production.GetSymbolsCount(); ++i)
	{
		const auto& symbol = production.GetSymbol(i);
		if (symbol.GetType() != GrammarSymbolType::Nonterminal)
		{
			return false;
		}
	}
	return true;
}

bool ExistsEpsilonProduction(const Grammar& grammar, const std::string& nonterminal)
{
	for (const auto index : GatherProductionIndices(grammar, nonterminal))
	{
		const auto production = grammar.GetProduction(index);
		if (production->GetStartSymbol().GetType() == GrammarSymbolType::Epsilon)
		{
			return true;
		}
	}
	return false;
}

bool NonterminalHasEmptiness(const Grammar& grammar, const std::string& nonterminal)
{
	if (ExistsEpsilonProduction(grammar, nonterminal))
	{
		return true;
	}

	const auto indices = GatherProductionIndices(grammar, [&](const GrammarProduction& production) {
		return production.GetLeftPart() == nonterminal && ProductionConsistsOfNonterminals(production);
	});

	// TODO: why compiler says unreachable code?
	for (const auto index : indices)
	{
		const auto production = grammar.GetProduction(index);
		assert(production->GetSymbolsCount() != 0);
		for (size_t i = 0; i < production->GetSymbolsCount(); ++i)
		{
			const auto& symbol = production->GetSymbol(i);
			assert(symbol.GetType() == GrammarSymbolType::Nonterminal);
			if (!NonterminalHasEmptiness(grammar, symbol.GetText()))
			{
				return false;
			}
		}
		return true;
	}

	return false;
}

std::set<std::pair<int, int>> GatherNonterminalOccurrences(const Grammar& grammar, const std::string& nonterminal)
{
	std::set<std::pair<int, int>> indices;
	for (size_t i = 0; i < grammar.GetProductionsCount(); ++i)
	{
		auto production = grammar.GetProduction(i);
		for (size_t j = 0; j < production->GetSymbolsCount(); ++j)
		{
			const auto& symbol = production->GetSymbol(j);
			if (symbol.GetType() == GrammarSymbolType::Nonterminal && symbol.GetText() == nonterminal)
			{
				indices.emplace(static_cast<int>(i), static_cast<int>(j));
			}
		}
	}
	return indices;
}

std::set<std::string> GatherBeginningSymbolsOfNonterminal(const Grammar& grammar, const std::string& nonterminal)
{
	std::stack<std::string> stack;
	std::set<std::string> visited;
	std::set<std::string> symbols;

	stack.push(nonterminal);
	visited.insert(nonterminal);

	while (!stack.empty())
	{
		const auto symbol = std::move(stack.top());
		stack.pop();

		for (const auto productionIndex : GatherProductionIndices(grammar, symbol))
		{
			const auto production = grammar.GetProduction(productionIndex);
			const auto& beginning = production->GetStartSymbol();

			if (beginning.GetType() == GrammarSymbolType::Terminal)
			{
				symbols.insert(beginning.GetText());
			}
			else if (beginning.GetType() == GrammarSymbolType::Nonterminal)
			{
				if (visited.find(beginning.GetText()) == visited.end())
				{
					visited.insert(beginning.GetText());
					stack.push(beginning.GetText());
				}

				bool hasEmptiness = NonterminalHasEmptiness(grammar, beginning.GetText());
				size_t index = 1u;

				while (index < production->GetSymbolsCount() && hasEmptiness)
				{
					const auto& current = production->GetSymbol(index);
					if (current.GetType() == GrammarSymbolType::Terminal)
					{
						symbols.insert(current.GetText());
						break;
					}
					else if (current.GetType() == GrammarSymbolType::Nonterminal)
					{
						if (visited.find(current.GetText()) == visited.end())
						{
							visited.insert(current.GetText());
							stack.push(current.GetText());
						}
						++index;
						hasEmptiness = NonterminalHasEmptiness(grammar, current.GetText());
					}
					else
					{
						throw std::invalid_argument("this symbol can't appear after nonterminal: " + current.GetText());
					}
				}
			}
		}
	}

	return symbols;
}

std::set<std::string> GatherBeginningSymbolsOfProduction(const Grammar& grammar, int productionIndex)
{
	const auto& beginning = grammar.GetProduction(productionIndex)->GetStartSymbol();

	if (beginning.GetType() == GrammarSymbolType::Terminal)
	{
		return { beginning.GetText() };
	}
	else if (beginning.GetType() == GrammarSymbolType::Nonterminal)
	{
		const auto production = grammar.GetProduction(productionIndex);

		std::set<std::string> symbols;
		std::set<std::string> expandables = { beginning.GetText() };

		bool hasEmtiness = NonterminalHasEmptiness(grammar, beginning.GetText());
		size_t index = 1u;

		while (index < production->GetSymbolsCount() && hasEmtiness)
		{
			const auto& current = production->GetSymbol(index);
			if (current.GetType() == GrammarSymbolType::Terminal)
			{
				symbols.insert(current.GetText());
				break;
			}
			else if (current.GetType() == GrammarSymbolType::Nonterminal)
			{
				expandables.insert(current.GetText());
				hasEmtiness = NonterminalHasEmptiness(grammar, current.GetText());
				++index;
			}
			else
			{
				throw std::invalid_argument("this symbol can't appear after nonterminal: " + current.GetText());
			}
		}

		while (!expandables.empty())
		{
			const auto node = *expandables.begin();
			expandables.erase(expandables.begin());

			const auto beginningSymbols = GatherBeginningSymbolsOfNonterminal(grammar, node);
			symbols.insert(beginningSymbols.begin(), beginningSymbols.end());
		}

		return symbols;
	}
	else if (beginning.GetType() == GrammarSymbolType::Epsilon)
	{
		return GatherFollowingSymbols(grammar, grammar.GetProduction(productionIndex)->GetLeftPart());
	}
	throw std::logic_error("unknown beginning symbol type: " + beginning.GetText());
}

std::set<std::string> GatherFollowingSymbols(const Grammar& grammar, const std::string& nonterminal)
{
	std::set<std::string> followings;

	std::stack<std::string> stack;
	std::set<std::string> visited;

	stack.push(nonterminal);
	visited.insert(nonterminal);

	while (!stack.empty())
	{
		const auto node = std::move(stack.top());
		stack.pop();

		for (const auto occurrence : GatherNonterminalOccurrences(grammar, node))
		{
			const auto production = grammar.GetProduction(occurrence.first);
			if (occurrence.second == production->GetSymbolsCount() - 1u)
			{
				if (visited.find(production->GetLeftPart()) == visited.end())
				{
					stack.push(production->GetLeftPart());
					visited.insert(production->GetLeftPart());
				}
				continue;
			}

			size_t index = occurrence.second + 1u;
			const auto& symbol = production->GetSymbol(index++);

			if (symbol.GetType() == GrammarSymbolType::Terminal)
			{
				followings.insert(symbol.GetText());
			}
			else if (symbol.GetType() == GrammarSymbolType::Nonterminal)
			{
				auto beginnings = GatherBeginningSymbolsOfNonterminal(grammar, symbol.GetText());
				followings.insert(beginnings.begin(), beginnings.end());

				// process case when that nonterminal can be empty
				bool hasEmptiness = NonterminalHasEmptiness(grammar, symbol.GetText());
				while (index < production->GetSymbolsCount() && hasEmptiness)
				{
					const auto& current = production->GetSymbol(index++);
					if (current.GetType() == GrammarSymbolType::Terminal)
					{
						followings.insert(current.GetText());
						break;
					}
					else if (current.GetType() == GrammarSymbolType::Nonterminal)
					{
						beginnings = GatherBeginningSymbolsOfNonterminal(grammar, nonterminal);
						followings.insert(beginnings.begin(), beginnings.end());
						hasEmptiness = NonterminalHasEmptiness(grammar, current.GetText());
					}
					else
					{
						throw std::invalid_argument("this symbol can't appear after terminal or nonterminal: " + symbol.GetText());
					}
				}

				const auto& last = production->GetLastSymbol();
				if (index >= production->GetSymbolsCount() && last.GetType() == GrammarSymbolType::Nonterminal &&
					NonterminalHasEmptiness(grammar, last.GetText()) && visited.find(production->GetLeftPart()) == visited.end())
				{
					stack.push(production->GetLeftPart());
					visited.insert(production->GetLeftPart());
				}
			}
			else
			{
				throw std::invalid_argument("this symbol can't appear after terminal or nonterminal: " + symbol.GetText());
			}
		}
	}

	return followings;
}
