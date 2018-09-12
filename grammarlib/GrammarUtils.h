#pragma once
#include <set>
#include <string>
#include <functional>
#include "Grammar.h"

// �������� ��� ������� ������ ���������� � �������� ����� ������
std::set<int> GatherProductionIndices(const Grammar& grammar, const std::string& nonterminal);
// �������� ��� ������� ������ ����������, ��������������� ��������� ���������
std::set<int> GatherProductionIndices(const Grammar& grammar, std::function<bool(const GrammarProduction&)> && predicate);

// ������� �� ������ ����� ������� ������ �� ������������
bool ProductionConsistsOfNonterminals(const GrammarProduction& production);
// ���������� �� ������ ������� � �������� ����� ������
bool ExistsEpsilonProduction(const Grammar& grammar, const std::string& nonterminal);
// ����� �� ���� ���������� ������ (�������� ��������������)
bool NonterminalHasEmptiness(const Grammar& grammar, const std::string& nonterminal);

// �������� �������������� ��������� ����������� � ����������
std::set<std::pair<int, int>> GatherNonterminalOccurrences(const Grammar& grammar, const std::string& nonterminal);

// �������� ������������ ��������� �����������
std::set<std::string> GatherBeginningSymbolsOfNonterminal(const Grammar& grammar, const std::string& nonterminal);
// �������� ������������ ��������� ��� ���������� �������
std::set<std::string> GatherBeginningSymbolsOfProduction(const Grammar& grammar, int productionIndex);
// �������� �������-����������� �����������
std::set<std::string> GatherFollowingSymbols(const Grammar& grammar, const std::string& nonterminal);
