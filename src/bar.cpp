#include "wxlib.h"
#include <algorithm>
#include <cmath>

void Bar::create(Container &win, int height, int width, uint32_t steps)
{
    if (panel) return;
    panel = new wxPanel(win.window());
    panel->SetBackgroundColour(win.window()->GetBackgroundColour());
    wxBoxSizer *layout = new wxBoxSizer(wxHORIZONTAL);
    bar = new wxGauge(panel, wxID_ANY, std::max(1, static_cast<int>(steps)));
    valueText = new wxStaticText(panel, wxID_ANY, "100%", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL | wxST_NO_AUTORESIZE);
    valueText->SetFont(getFontFromEnum(FONT_TEXTB));
    valueText->SetMinSize(valueText->GetBestSize());
    valueText->SetLabel("0%");
    valueText->Hide();
    layout->Add(bar, 1, wxEXPAND);
    layout->Add(valueText, 0, wxLEFT | wxALIGN_CENTER_VERTICAL, 6);
    panel->SetSizer(layout);
    panel->SetMinSize(wxSize(width > 0 ? width : 0, height));
    win.add(panel, 0, width > 0 ? wxALIGN_LEFT : wxEXPAND);
}

void Bar::updateValue(void) { if (bar && valueText) valueText->SetLabel(wxString::Format("%d%%", static_cast<int>(std::round(100.0 * bar->GetValue() / bar->GetRange())))); }
void Bar::set(float percentage) { if (bar) { bar->SetValue(static_cast<int>(std::round(bar->GetRange() * std::clamp(percentage, 0.0f, 100.0f) / 100.0f))); updateValue(); } }
void Bar::advance(uint16_t steps) { if (bar) { bar->SetValue(std::min(bar->GetRange(), bar->GetValue() + static_cast<int>(steps))); updateValue(); } }
void Bar::showValue(bool show) { if (valueText) { valueText->Show(show); panel->Layout(); } }
void Bar::enable(void) { if (panel) panel->Enable(); }
void Bar::disable(void) { if (panel) panel->Disable(); }
void Bar::clear(void) { set(0); }
