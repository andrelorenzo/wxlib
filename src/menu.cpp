#include "wxlib.h"

static cb_t callback;

void Menu::create(wxFrame *win)
{
    if (bar || !win) return;
    frame = win;
    bar = new wxMenuBar();
    frame->SetMenuBar(bar);
}

void Menu::create(Container &win) { create(wxDynamicCast(wxGetTopLevelParent(win.window()), wxFrame)); }

int Menu::add(wxString title) { if (!bar) return wxNOT_FOUND; menus.push_back(new wxMenu()); bar->Append(menus.back(), title); return static_cast<int>(menus.size() - 1); }
void Menu::addChild(int parent, wxString title, int id, wxString help) { if (!frame || parent < 0 || parent >= static_cast<int>(menus.size())) return; menus[parent]->Append(id, title, help); frame->Bind(wxEVT_MENU, &Menu::selectCb, this, id); }
void Menu::addSeparator(int parent) { if (parent >= 0 && parent < static_cast<int>(menus.size())) menus[parent]->AppendSeparator(); }
void Menu::enable(int id, bool enabled) { if (bar) bar->Enable(id, enabled); }
void Menu::selectCb(wxCommandEvent &event) { if (callback) callback(event); }
void menuSetCallback(cb_t callb) { callback = callb; }
