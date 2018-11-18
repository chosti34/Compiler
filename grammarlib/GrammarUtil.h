#ifndef COMPILER_60MIN_GRAMMARUTIL_H
#define COMPILER_60MIN_GRAMMARUTIL_H

#include "GrammarFwd.h"

#include <set>
#include <string>
#include <functional>

// Получить все индексы правил грамматики с заданной левой частью
std::set<int> GatherProductionIndices(const Grammar& grammar, const std::string& nonterminal);
// Получить все индексы правил грамматики, удовлетворяющих заданному предикату
std::set<int> GatherProductionIndices(const Grammar& grammar, std::function<bool(const GrammarProduction&)> && predicate);

// Состоит ли правая часть правила только из нетерминалов
bool ProductionConsistsOfNonterminals(const GrammarProduction& production);
// Существует ли пустое правило с заданной левой частью
bool ExistsEpsilonProduction(const Grammar& grammar, const std::string& nonterminal);
// Может ли быть нетерминал пустым (учитывая транзитивность)
bool NonterminalHasEmptiness(const Grammar& grammar, const std::string& nonterminal);

// Получить грамматические вхождение нетерминала в грамматике
std::set<std::pair<int, int>> GatherNonterminalOccurrences(const Grammar& grammar, const std::string& nonterminal);

// Получить направляющее множество нетерминала
std::set<std::string> GatherBeginningSymbolsOfNonterminal(const Grammar& grammar, const std::string& nonterminal);
// Получить направляющее множество для указанного правила
std::set<std::string> GatherBeginningSymbolsOfProduction(const Grammar& grammar, int productionIndex);
// Получить символы-следователи нетерминала
std::set<std::string> GatherFollowingSymbols(const Grammar& grammar, const std::string& nonterminal);

#endif //COMPILER_60MIN_GRAMMARUTIL_H
