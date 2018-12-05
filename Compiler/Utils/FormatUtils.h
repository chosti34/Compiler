#pragma once
#include <string>
#include <vector>

namespace format_utils
{
class Table
{
public:
	enum class DisplayMethod
	{
		WithLineSeparators,
		WithoutLineSeparators,
		ColumnsLineSeparated
	};

	enum class Alignment
	{
		Left,
		Right
	};

	struct BorderStyle
	{
		char vertical;
		char horizontal;
		char corner;
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
	void SetDisplayMethod(DisplayMethod method);
	void Clear();

	DisplayMethod GetDisplayMethod()const;
	const std::string& GetValue(size_t row, size_t col)const;
	const std::vector<std::string>& GetColumns()const;
	std::string CreateLineSeparator()const;

	bool IsEmpty()const;
	size_t GetRowsCount()const;
	size_t GetColumnsCount()const;

	const BorderStyle& GetBorderStyle()const;
	size_t GetColumnWidth(size_t col)const;
	Alignment GetColumnAlignment(size_t col)const;
	unsigned GetColumnPadding(size_t col)const;

private:
	void VerifyColumnsCount(size_t count)const;
	void SetColumns(const std::vector<std::string>& columns);

private:
	std::vector<std::vector<std::string>> m_table;
	std::vector<ColumnProperties> m_columnProperties;
	BorderStyle m_borderStyle;
	DisplayMethod m_displayMethod;
};
}

std::ostream& operator <<(std::ostream& os, const format_utils::Table& table);
