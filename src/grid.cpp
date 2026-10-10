#include "wxlib.h"
#include <algorithm>

void Grid::create(Container &win, const wxString *columns, size_t len, int rows, int height, int width)
{
    if (grid || !columns || !len) return;
    grid = new wxGrid(win.window(), wxID_ANY, wxDefaultPosition, wxSize(width > 0 ? width : wxDefaultCoord, height > 0 ? height : wxDefaultCoord));
    grid->CreateGrid(std::max(0, rows), static_cast<int>(len + 1));
    widths.assign(len + 1, 0);
    editable.assign(len + 1, false);
    widths[0] = 45;
    grid->HideRowLabels();
    grid->SetDefaultRowSize(DEF_HEIGHT);
    grid->SetColLabelSize(DEF_HEIGHT + 6);
    grid->SetDefaultCellFont(getFontFromEnum(FONT_TEXT));
    grid->SetLabelFont(getFontFromEnum(FONT_TEXTB));
    grid->SetLabelBackgroundColour(COLOR_DARK_GRAY);
    grid->SetLabelTextColour(COLOR_WHITE);
    grid->SetGridLineColour(wxColour(218, 223, 225));
    grid->EnableDragRowSize(false);
    grid->SetMargins(0, 0);
    grid->SetColLabelValue(0, "#");
    for (size_t column = 0; column < len; ++column) grid->SetColLabelValue(static_cast<int>(column + 1), columns[column]);
    for (int row = 0; row < rows; ++row) styleRow(row);
    grid->SetMinSize(wxSize(width > 0 ? width : 100, height > 0 ? height : 100));
    grid->Bind(wxEVT_SIZE, [this](wxSizeEvent &event) {
        event.Skip();
        grid->CallAfter([this] { fitColumns(); });
    });
    win.add(grid, height == 0 ? 1 : 0, width > 0 ? wxALIGN_LEFT : wxEXPAND);
    grid->CallAfter([this] { fitColumns(); });
}

void Grid::fitColumns(void)
{
    if (!grid || widths.empty()) return;
    int available = grid->GetGridWindow()->GetClientSize().x;
    int fixed = 0;
    int flexible = 0;
    for (size_t column = 0; column < widths.size(); ++column) {
        if (column == 0 && !numberColumnVisible) continue;
        widths[column] > 0 ? fixed += widths[column] : ++flexible;
    }
    int remaining = std::max(60 * flexible, available - fixed);
    int pending = flexible;
    for (size_t column = 0; column < widths.size(); ++column) {
        if (column == 0 && !numberColumnVisible) {
            grid->HideCol(0);
            continue;
        }
        if (widths[column] > 0) grid->SetColSize(static_cast<int>(column), widths[column]);
        else {
            int width = pending == 1 ? remaining : remaining / pending;
            grid->SetColSize(static_cast<int>(column), width);
            remaining -= width;
            --pending;
        }
    }
}

void Grid::styleRow(int row)
{
    if (!grid || row < 0 || row >= grid->GetNumberRows()) return;
    for (int column = 0; column < grid->GetNumberCols(); ++column) {
        grid->SetCellBackgroundColour(row, column, row % 2 ? wxColour(245, 247, 249) : COLOR_WHITE);
        grid->SetReadOnly(row, column, !editable[column]);
    }
    grid->SetCellValue(row, 0, wxString::Format("%d", row + 1));
    grid->SetCellAlignment(row, 0, wxALIGN_CENTER, wxALIGN_CENTER);
}

void Grid::addRow(const wxString *values, size_t len)
{
    if (!grid) return;
    int row = grid->GetNumberRows();
    grid->AppendRows(1);
    styleRow(row);
    if (values) setRow(row, values, len);
}

void Grid::setRow(int row, const wxString *values, size_t len)
{
    if (!grid || !values || row < 0 || row >= grid->GetNumberRows()) return;
    for (size_t column = 0; column < len && column + 1 < static_cast<size_t>(grid->GetNumberCols()); ++column) setText(row, static_cast<int>(column), values[column]);
}

void Grid::setText(int row, int column, wxString text)
{
    if (grid && row >= 0 && row < grid->GetNumberRows() && column >= 0 && column + 1 < grid->GetNumberCols()) grid->SetCellValue(row, column + 1, text);
}

wxString Grid::getText(int row, int column)
{
    return grid && row >= 0 && row < grid->GetNumberRows() && column >= 0 && column + 1 < grid->GetNumberCols() ? grid->GetCellValue(row, column + 1) : wxEmptyString;
}

void Grid::setEditable(int column, bool enabled)
{
    if (!grid || column < 0 || column + 1 >= grid->GetNumberCols()) return;
    int gridColumn = column + 1;
    editable[gridColumn] = enabled;
    for (int row = 0; row < grid->GetNumberRows(); ++row) grid->SetReadOnly(row, gridColumn, !enabled);
}

void Grid::setColumnWidth(int column, int width)
{
    if (!grid || column < 0 || column + 1 >= grid->GetNumberCols()) return;
    widths[column + 1] = std::max(0, width);
    fitColumns();
}

void Grid::setNumberColumnVisible(bool visible)
{
    if (!grid || numberColumnVisible == visible) return;
    numberColumnVisible = visible;
    fitColumns();
}

void Grid::setColor(int row, int column, wxColour color)
{
    if (grid && row >= 0 && row < grid->GetNumberRows() && column >= 0 && column + 1 < grid->GetNumberCols()) {
        grid->SetCellBackgroundColour(row, column + 1, color);
        // Refresh is queued and coalesced by wxWidgets. ForceRefresh here made
        // bulk table population repaint the complete grid once per cell.
        grid->Refresh(false);
    }
}

void Grid::deleteRow(int row)
{
    if (!grid || row < 0 || row >= grid->GetNumberRows()) return;
    grid->DeleteRows(row);
    for (int current = row; current < grid->GetNumberRows(); ++current) grid->SetCellValue(current, 0, wxString::Format("%d", current + 1));
}

int Grid::getSelectedRow(void)
{
    return grid && grid->GetNumberRows() ? grid->GetGridCursorRow() : wxNOT_FOUND;
}

void Grid::clear(void)
{
    if (grid && grid->GetNumberRows()) grid->DeleteRows(0, grid->GetNumberRows());
}


int Grid::getRowsCount(void)
{   
    return grid->GetNumberRows();
}   
