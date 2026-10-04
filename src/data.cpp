#include "wxlib.h"

void Data::createTelem(Container &win, wxString title, int width, int height, int nchan, int editWidth) { create(win, title, width, height, nchan, false, editWidth); }
void Data::createConfig(Container &win, wxString title, int width, int height, int nchan, int editWidth) { create(win, title, width, height, nchan, true, editWidth); }

void Data::create(Container &win, wxString title, int width, int height, int nchan, bool enabled, int editWidth)
{
    if (panel) return;
    if (nchan < 1) nchan = 1;
    panel = new wxPanel(win.window());
    wxSizer *layout = new wxBoxSizer(wxHORIZONTAL);
    _text.create(panel, title, FONT_TEXT, wxALIGN_LEFT);
    _text.setWidth(width, false);
    layout->Add(_text.window(), 0, wxALIGN_CENTER_VERTICAL);
    _edts = std::vector<Edit>(nchan);
    for (Edit &edit : _edts)
    {
        edit.create(panel, wxEmptyString, editWidth > 0 ? editWidth : width, height);
        if (!enabled) edit.disable();
        edit.window()->SetMinSize(wxSize(editWidth > 0 ? editWidth : 0, height));
        layout->Add(edit.window(), 1, wxEXPAND);
    }
    _bool = new wxComboBox(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(editWidth > 0 ? editWidth : width, height), 0, nullptr, wxCB_READONLY);
    _bool->Append("false"); _bool->Append("true"); _bool->SetFont(getFontFromEnum(FONT_TEXT)); _bool->Hide(); layout->Add(_bool, 1, wxEXPAND);
    panel->SetSizer(layout);
    panel->SetMinSize(wxSize(width + (editWidth > 0 ? editWidth * nchan : 0), height));
    win.add(panel, 0, wxEXPAND);
}

void Data::update(wxString value, uint8_t chan) { if (_boolMode && _bool) { int selection = _bool->FindString(value.Lower()); _bool->SetSelection(selection); } else if (chan < _edts.size()) _edts[chan].setText(value); }
void Data::update(float value, uint8_t chan) { if (chan < _edts.size()) _edts[chan].setText(value); }
void Data::update(int value, uint8_t chan) { if (chan < _edts.size()) _edts[chan].setText(value); }

void Data::update(wxString *value, size_t len) { for (size_t i = 0; i < len && i < _edts.size(); ++i) update(value[i], static_cast<uint8_t>(i)); }
void Data::update(float *value, size_t len) { for (size_t i = 0; i < len && i < _edts.size(); ++i) update(value[i], static_cast<uint8_t>(i)); }
void Data::update(int *value, size_t len) { for (size_t i = 0; i < len && i < _edts.size(); ++i) update(value[i], static_cast<uint8_t>(i)); }

wxString Data::getText(uint8_t chan) { return _boolMode && _bool ? _bool->GetValue() : chan < _edts.size() ? _edts[chan].getText() : wxEmptyString; }
float Data::getNumber(uint8_t chan) { return chan < _edts.size() ? _edts[chan].getNumber() : 0; }
void Data::setColor(wxColour color) { for (Edit &edit : _edts) edit.setColor(color); }
void Data::setBoolMode(bool enabled) { _boolMode = enabled; if (!_bool) return; for (Edit &edit : _edts) edit.window()->Show(!enabled); _bool->Show(enabled); if (panel) panel->Layout(); }
void Data::show(bool show) { if (panel) panel->Show(show); }
