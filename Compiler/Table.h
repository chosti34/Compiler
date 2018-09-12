#pragma once
#include <string>
#include <vector>

namespace FormatUtils
{
class Table
{
public:
	struct BorderStyle
	{
		char vertical;
		char horizontal;
		char corner;
	};

	enum class Alignment
	{
		Left,
		Right
	};

	struct ColumnProperties
	{
		size_t width;
		unsigned padding;
		Alignment alignment;
	};

public:
	explicit Table(const BorderStyle& borders = { '|', '-', '+' });

	void Append(const std::vector<std::string>& values);
	void SetColumnAlignment(size_t col, Alignment alignment);
	void SetColumnPadding(size_t col, unsigned padding);
	void ShowLineSeparators(bool toggle);
	void Clear();

	const std::string& GetValue(size_t row, size_t col)const;
	const std::vector<std::string>& GetColumns()const;
	std::string CreateLineSeparator()const;

	bool IsEmpty()const;
	size_t GetRowsCount()const;
	size_t GetColumnsCount()const;
	const BorderStyle& GetBorders()const;
	size_t GetColumnWidth(size_t col)const;
	Alignment GetColumnAlignment(size_t col)const;
	unsigned GetColumnPadding(size_t col)const;
	bool ShowLineSeparators()const;

private:
	void VerifyColumnsCount(size_t count)const;
	void SetColumns(const std::vector<std::string>& columns);

private:
	std::vector<std::vector<std::string>> m_table;
	std::vector<ColumnProperties> m_columnProperties;
	BorderStyle m_borders;
	bool m_showLineSeparators;
};
}

std::ostream& operator <<(std::ostream& os, const FormatUtils::Table& table);
