#include "wxlib.h"
#include <wx/dcbuffer.h>
#include <wx/wrapsizer.h>
#include <algorithm>

namespace {
void layoutUnlessFrozen(wxWindow *window)
{
    if (!window) return;
    wxWindow *top = wxGetTopLevelParent(window);
    if (top && top->IsFrozen()) return;
    window->Layout();
}
}

class ContainerPanel : public wxPanel {
public:
    ContainerPanel(wxWindow *parent, bool border, int padding) : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxFULL_REPAINT_ON_RESIZE), padding(padding) { if (border) { SetBackgroundStyle(wxBG_STYLE_PAINT); Bind(wxEVT_PAINT, &ContainerPanel::paintCb, this); Bind(wxEVT_SIZE, [this](wxSizeEvent &event) { event.Skip(); Refresh(false); }); } }
    void setPadding(int value) { padding = value; Refresh(); }
private:
    void paintCb(wxPaintEvent &) { wxAutoBufferedPaintDC dc(this); dc.SetBackground(wxBrush(GetBackgroundColour())); dc.Clear(); wxSize size = GetClientSize(); int inset = std::max(0, padding / 2); dc.SetPen(wxPen(COLOR_BLACK, 2)); dc.SetBrush(*wxTRANSPARENT_BRUSH); dc.DrawRoundedRectangle(inset + 1, inset + 1, std::max(0, size.x - inset * 2 - 3), std::max(0, size.y - inset * 2 - 3), 10); }
    int padding = 0;
};

void Container::create(wxWindow *parent, SizerType type, bool border, wxColour col)
{
    _type = type;
    _pnl = new ContainerPanel(parent, border, _padding);
    if(col.IsOk())_pnl->SetBackgroundColour(col);
    _szr = createContentSizer(type);
    wxSizer *padding = new wxBoxSizer(wxVERTICAL);
    padding->Add(_szr, type == SIZER_ALARMS ? 0 : 1, wxEXPAND | wxALL, DEF_PADDING);
    _pnl->SetSizer(padding);
    bindGrid(type);
    wxSizer *root = new wxBoxSizer(wxVERTICAL);
    root->Add(_pnl, 1, wxEXPAND);
    parent->SetSizer(root, true);
    layoutUnlessFrozen(parent);
}

void Container::create(Container &parent, double prop, SizerType type, bool border, wxColour col)
{
    // parent.setPadding(0);
    _type = type;

    _pnl = new ContainerPanel(parent._pnl, border, _padding);
    if(col.IsOk())_pnl->SetBackgroundColour(col);
    _szr = createContentSizer(type);
    wxSizer *padding = new wxBoxSizer(wxVERTICAL);
    padding->Add(_szr, type == SIZER_ALARMS ? 0 : 1, wxEXPAND | wxALL, _padding);
    _pnl->SetSizer(padding);
    bindGrid(type);
    prop = prop < 0 ? 1 - parent._used : prop;
    parent._szr->Add(_pnl, static_cast<int>(prop * 1000), wxEXPAND);
    parent._used += prop;
    layoutUnlessFrozen(parent._pnl);
}

void Container::add(wxWindow *child, double prop, int flag)
{
    _szr->Add(child, static_cast<int>(prop * 1000), flag);
    layoutUnlessFrozen(_pnl);
}

void Container::addSpacer(int prop)
{
    _szr->AddStretchSpacer(prop);
    layoutUnlessFrozen(_pnl);
}

void Container::addSeparator(wxColour color, int thickness, int margin)
{
    if (!_pnl || !_szr) return;
    thickness = std::max(1, thickness); margin = std::max(0, margin);
    wxPanel *separator = new wxPanel(_pnl, wxID_ANY, wxDefaultPosition, _type == SIZER_VER ? wxSize(-1, thickness) : wxSize(thickness, -1));
    separator->SetBackgroundColour(color);
    _szr->Add(separator, 0, wxEXPAND | (_type == SIZER_VER ? wxTOP | wxBOTTOM : wxLEFT | wxRIGHT), margin);
    layoutUnlessFrozen(_pnl);
}

void Container::setPadding(int padding)
{
    if(padding != _padding)
    {
        _padding = padding;
        if (_pnl) _pnl->GetSizer()->GetItem(_szr)->SetBorder(padding);
        if (_pnl) static_cast<ContainerPanel *>(_pnl)->setPadding(padding);
        layoutUnlessFrozen(_pnl);
    }
}

wxSizer *Container::createContentSizer(SizerType type)
{
    switch (type)
    {
    case SIZER_VER: return new wxBoxSizer(wxVERTICAL);
    case SIZER_HOR: return new wxBoxSizer(wxHORIZONTAL);
    case SIZER_GRID: return new wxWrapSizer(wxVERTICAL, wxWRAPSIZER_DEFAULT_FLAGS);
    case SIZER_WRAP_HOR: return new wxWrapSizer(wxHORIZONTAL, wxWRAPSIZER_DEFAULT_FLAGS);
    case SIZER_ALARMS: return new wxGridSizer(1, 8, 8);
    default: return new wxBoxSizer(wxVERTICAL);
    }
}

void Container::bindGrid(SizerType type)
{
    if (type == SIZER_GRID || type == SIZER_WRAP_HOR) return;
    if (type != SIZER_ALARMS) return;
    wxGridSizer *grid = static_cast<wxGridSizer *>(_szr);
    wxPanel *panel = _pnl;
    panel->Bind(wxEVT_SIZE, [grid, panel](wxSizeEvent &event) {
        event.Skip();
        int count = 0, itemHeight = 1;
        for (size_t i = 0; i < grid->GetItemCount(); ++i) {
            wxSizerItem *item = grid->GetItem(i);
            if (!item->IsShown()) continue;
            ++count;
            itemHeight = std::max(itemHeight, item->CalcMin().y);
        }
        if (!count) return;
        int rows = std::clamp((event.GetSize().y + grid->GetVGap()) / (itemHeight + grid->GetVGap()), 1, count);
        int cols = (count + rows - 1) / rows;
        if (grid->GetCols() != cols) { grid->SetCols(cols); grid->SetRows(0); layoutUnlessFrozen(panel); }
    });
}
