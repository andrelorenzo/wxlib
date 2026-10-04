#include "wxlib.h"
#include <algorithm>
#include <wx/display.h>
#include <wx/filename.h>
#include <wx/icon.h>
#include <wx/statline.h>
#include <wx/stdpaths.h>
#include <wx/toplevel.h>
#include <wx/utils.h>

static cb_t windowCallback;
static std::vector<wxWindow *> independentWindows;

wxIcon appIcon()
{
#if defined(__WXMSW__)
    wxIcon icon("APP_ICON", wxBITMAP_TYPE_ICO_RESOURCE);
    if (icon.IsOk()) return icon;
#endif
    wxFileName path(wxStandardPaths::Get().GetExecutablePath());
    path.SetFullName("app.ico");
    path.AppendDir("assets");
    return wxIcon(path.GetFullPath(), wxBITMAP_TYPE_ICO);
}

static int displayFor(wxWindow *reference)
{
    int displayIndex = reference ? wxDisplay::GetFromWindow(reference) : wxNOT_FOUND;
    if (displayIndex == wxNOT_FOUND) displayIndex = wxDisplay::GetFromPoint(wxGetMousePosition());
    if (displayIndex == wxNOT_FOUND || displayIndex >= static_cast<int>(wxDisplay::GetCount())) return 0;
    return displayIndex;
}

static void positionOnDisplay(wxWindow *window, int displayIndex)
{
    if (!window) return;
    wxDisplay display(static_cast<unsigned>(displayIndex));
    if (!display.IsOk()) return;
    wxRect area = display.GetClientArea();
    wxSize size = window->GetSize();
    int x = area.x + std::max(0, (area.width - size.x) / 2);
    int y = area.y + std::max(0, (area.height - size.y) / 2);
    window->SetPosition(wxPoint(x, y));
}

void Window::create(Container &parent, wxString title, int id, bool controls, bool dialog, wxSize size)
{
    wxWindow *reference = wxGetTopLevelParent(parent.window());
    int displayIndex = displayFor(reference);
    if (_top) { _top->Layout(); if (!_top->IsShown()) positionOnDisplay(_top, displayIndex); _top->Show(); _top->Raise(); return; }
    _id = id;
    _top = dialog ? static_cast<wxWindow *>(new wxDialog(nullptr, wxID_ANY, title, wxDefaultPosition, size, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxDIALOG_NO_PARENT)) : static_cast<wxWindow *>(new wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, size, wxDEFAULT_FRAME_STYLE));
    wxIcon icon = appIcon();
    if (icon.IsOk()) static_cast<wxTopLevelWindow *>(_top)->SetIcon(icon);
    independentWindows.push_back(_top);
    _top->Freeze();
    wxBoxSizer *layout = new wxBoxSizer(wxVERTICAL);
    wxPanel *content = new wxPanel(_top);
    _win.create(content);
    layout->Add(content, 1, wxEXPAND);
    if (controls)
    {
        wxPanel *footer = new wxPanel(_top); footer->SetMinSize(wxSize(-1, 56));
        wxBoxSizer *footerLayout = new wxBoxSizer(wxVERTICAL), *buttons = new wxBoxSizer(wxHORIZONTAL);
        wxButton *cancel = new wxButton(footer, wxID_CANCEL, "Cancel"), *apply = new wxButton(footer, wxID_APPLY, "Apply"), *accept = new wxButton(footer, wxID_OK, "Accept");
        cancel->SetFont(getFontFromEnum(FONT_TEXTB)); apply->SetFont(getFontFromEnum(FONT_TEXTB)); accept->SetFont(getFontFromEnum(FONT_TEXTB)); accept->SetDefault();
        buttons->AddStretchSpacer(); buttons->Add(cancel, 0, wxRIGHT, 8); buttons->Add(apply, 0, wxRIGHT, 8); buttons->Add(accept);
        footerLayout->Add(new wxStaticLine(footer), 0, wxEXPAND); footerLayout->Add(buttons, 1, wxEXPAND | wxALL, 10); footer->SetSizer(footerLayout); layout->Add(footer, 0, wxEXPAND);
        cancel->Bind(wxEVT_BUTTON, [this](wxCommandEvent &event) { _top->Hide(); event.StopPropagation(); });
        apply->Bind(wxEVT_BUTTON, &Window::applyCb, this);
        accept->Bind(wxEVT_BUTTON, [this](wxCommandEvent &event) { applyCb(event); _top->Hide(); event.StopPropagation(); });
    }
    _top->Bind(wxEVT_CLOSE_WINDOW, [this](wxCloseEvent &event) { if (event.CanVeto()) { _top->Hide(); event.Veto(); event.StopPropagation(); } else event.Skip(); });
    _top->SetSizer(layout); _top->SetMinSize(wxSize(300, 200));
    _top->CallAfter([this, displayIndex]() { if (!_top) return; _top->Layout(); positionOnDisplay(_top, displayIndex); _top->Thaw(); _top->Show(); _top->Raise(); });
}

void Window::applyCb(wxCommandEvent &event) { if (_id > 0) event.SetId(_id); if (_callback) _callback(event); else if (windowCallback) windowCallback(event); }
void Window::setApplyCallback(cb_t callback) { _callback = callback; }
void windowSetCallback(cb_t callback) { windowCallback = callback; }
void windowCloseAll(void) { std::vector<wxWindow *> windows = independentWindows; independentWindows.clear(); for (wxWindow *window : windows) if (window && !window->IsBeingDeleted()) window->Destroy(); }
