#include "format_utils.h"
#include <cassert>
#include <iomanip>

using namespace format_utils;

namespace
{
const unsigned MIN_COLUMNS_COUNT = 1u;
const unsigned DEFAULT_PADDING = 1u;
const Table::Alignment DEFAULT_ALIGNMENT = Table::Alignment::Right;

void DisplayWithoutLineSeparators(const Table& table, std::ostream& os)
{
    const std::string lineSeparator = table.CreateLineSeparator();
    const Table::BorderStyle& borders = table.GetBorderStyle();

    if (!table.IsEmpty())
    {
        os << lineSeparator << "\n";
    }

    for (size_t row = 0; row < table.GetRowsCount(); ++row)
    {
        os << borders.vertical;
        for (size_t col = 0; col < table.GetColumnsCount(); ++col)
        {
            const auto& alignment = (table.GetColumnAlignment(col) == Table::Alignment::Left) ? std::left : std::right;
            os << std::string(table.GetColumnPadding(col), ' ') << alignment << std::setw(static_cast<int>(table.GetColumnWidth(col)))
               << table.GetValue(row, col) << std::string(table.GetColumnPadding(col), ' ') << borders.vertical;
        }

        os << "\n";
        if (row == table.GetRowsCount() - 1)
        {
            os << lineSeparator;
        }
    }
}

void DisplayWithLineSeparators(const Table& table, std::ostream& os)
{
    const std::string lineSeparator = table.CreateLineSeparator();
    const Table::BorderStyle& borders = table.GetBorderStyle();

    for (size_t row = 0; row < table.GetRowsCount(); ++row)
    {
        os << lineSeparator << "\n" << borders.vertical;

        for (size_t col = 0; col < table.GetColumnsCount(); ++col)
        {
            const auto& alignment = (table.GetColumnAlignment(col) == Table::Alignment::Left) ? std::left : std::right;
            os << std::string(table.GetColumnPadding(col), ' ') << alignment << std::setw(static_cast<int>(table.GetColumnWidth(col)))
               << table.GetValue(row, col) << std::string(table.GetColumnPadding(col), ' ') << borders.vertical;
        }

        os << "\n";
        if (row == table.GetRowsCount() - 1)
        {
            os << lineSeparator;
        }
    }
}

void DisplayColumnsLineSeparated(const Table& table, std::ostream& os)
{
    const std::string lineSeparator = table.CreateLineSeparator();
    const Table::BorderStyle& borders = table.GetBorderStyle();

    if (!table.IsEmpty())
    {
        os << lineSeparator << "\n";
    }

    for (size_t row = 0; row < table.GetRowsCount(); ++row)
    {
        os << borders.vertical;

        for (size_t col = 0; col < table.GetColumnsCount(); ++col)
        {
            const auto& alignment = (table.GetColumnAlignment(col) == Table::Alignment::Left) ? std::left : std::right;
            os << std::string(table.GetColumnPadding(col), ' ') << alignment << std::setw(static_cast<int>(table.GetColumnWidth(col)))
               << table.GetValue(row, col) << std::string(table.GetColumnPadding(col), ' ') << borders.vertical;
        }

        os << "\n";
        if (row == 0)
        {
            os << lineSeparator << "\n";
        }
        if (row == table.GetRowsCount() - 1)
        {
            os << lineSeparator;
        }
    }
}
}

Table::Table(const BorderStyle& borders)
        : m_borderStyle(borders)
        , m_displayMethod(DisplayMethod::WithLineSeparators)
{
}

void Table::Append(const std::vector<std::string>& values)
{
    if (IsEmpty())
    {
        SetColumns(values);
        return;
    }

    const auto& columns = GetColumns();
    if (columns.size() != values.size())
    {
        throw std::invalid_argument("row's values count must match columns count");
    }

    m_table.emplace_back(values);
    for (size_t i = 0; i < values.size(); ++i)
    {
        if (values[i].length() > m_columnProperties[i].width)
        {
            m_columnProperties[i].width = values[i].length();
        }
    }
}

void Table::SetColumnAlignment(size_t col, Alignment alignment)
{
    if (IsEmpty())
    {
        throw std::logic_error("set columns before changing alignment");
    }

    const auto& columns = GetColumns();
    if (col >= columns.size())
    {
        throw std::out_of_range("column's index must be less than columns count");
    }
    m_columnProperties[col].alignment = alignment;
}

void Table::SetColumnPadding(size_t col, unsigned padding)
{
    if (IsEmpty())
    {
        throw std::logic_error("set columns before changing padding");
    }

    const auto& columns = GetColumns();
    if (col >= columns.size())
    {
        throw std::out_of_range("column's index must be less than columns count");
    }
    m_columnProperties[col].padding = padding;
}

void format_utils::Table::SetDisplayMethod(DisplayMethod method)
{
    m_displayMethod = method;
}

void Table::Clear()
{
    m_table.clear();
    m_columnProperties.clear();
}

Table::DisplayMethod Table::GetDisplayMethod()const
{
    return m_displayMethod;
}

const std::string& Table::GetValue(size_t row, size_t col)const
{
    if (row >= m_table.size())
    {
        throw std::out_of_range("row index must be less than rows count");
    }

    const auto& values = m_table[row];
    if (col >= values.size())
    {
        throw std::out_of_range("column index must be less than columns count");
    }

    return values[col];
}

const std::vector<std::string>& Table::GetColumns()const
{
    if (IsEmpty())
    {
        throw std::runtime_error("columns aren't set yet");
    }
    return m_table.front();
}

std::string Table::CreateLineSeparator()const
{
    if (IsEmpty())
    {
        return "";
    }

    std::string separator(1, m_borderStyle.corner);

    const auto& columns = GetColumns();
    for (size_t i = 0; i < columns.size(); ++i)
    {
        separator += std::string(m_columnProperties[i].width + 2 * m_columnProperties[i].padding,
                                 m_borderStyle.horizontal) + m_borderStyle.corner;
    }

    return separator;
}

bool Table::IsEmpty()const
{
    return m_table.empty();
}

size_t Table::GetRowsCount()const
{
    return m_table.size();
}

size_t Table::GetColumnsCount()const
{
    if (!m_table.empty())
    {
        assert(m_table.front().size() >= MIN_COLUMNS_COUNT);
        return m_table.front().size();
    }
    return 0;
}

const Table::BorderStyle& Table::GetBorderStyle()const
{
    return m_borderStyle;
}

void Table::VerifyColumnsCount(size_t count)const
{
    if (count < MIN_COLUMNS_COUNT)
    {
        throw std::runtime_error("columns count must be at least " + std::to_string(MIN_COLUMNS_COUNT));
    }
}

void Table::SetColumns(const std::vector<std::string>& columns)
{
    VerifyColumnsCount(columns.size());
    Clear();

    m_table.emplace_back(columns);
    for (const std::string& column : columns)
    {
        m_columnProperties.push_back({ column.length(), DEFAULT_PADDING, DEFAULT_ALIGNMENT });
    }
}

size_t Table::GetColumnWidth(std::size_t col)const
{
    const auto& columns = GetColumns();
    if (col >= columns.size())
    {
        throw std::out_of_range("column's index must be less than columns count");
    }
    return m_columnProperties[col].width;
}

Table::Alignment Table::GetColumnAlignment(size_t col)const
{
    const auto& columns = GetColumns();
    if (col >= columns.size())
    {
        throw std::out_of_range("column's index must be less than columns count");
    }
    return m_columnProperties[col].alignment;
}

unsigned Table::GetColumnPadding(size_t col)const
{
    const auto& columns = GetColumns();
    if (col >= columns.size())
    {
        throw std::out_of_range("column's index must be less than columns count");
    }
    return m_columnProperties[col].padding;
}

std::ostream& operator <<(std::ostream& os, const Table& table)
{
    switch (table.GetDisplayMethod())
    {
    case Table::DisplayMethod::WithLineSeparators:
        DisplayWithLineSeparators(table, os);
        break;
    case Table::DisplayMethod::WithoutLineSeparators:
        DisplayWithoutLineSeparators(table, os);
        break;
    case Table::DisplayMethod::ColumnsLineSeparated:
        DisplayColumnsLineSeparated(table, os);
        break;
    default:
        assert(false);
        break;
    }
    return os;
}
