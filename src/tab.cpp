#include "wxlib.h"

static cb_t callback;

void Tab::create(Container &win)
{
    if (book) return;
    book = new wxNotebook(win.window(), wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNB_TOP | wxBORDER_NONE);
    book->SetFont(getFontFromEnum(FONT_TEXTB));
    book->Bind(wxEVT_NOTEBOOK_PAGE_CHANGED, &Tab::changeCb, this);
    win.add(book, 1, wxEXPAND);
}

void Tab::add(Container &page, wxString title, int id, Container::SizerType type, wxColour color)
{
    if (!book) return;
    wxPanel *host = new wxPanel(book);
    page.create(host, type, false, color);
    ids.push_back(id);
    book->AddPage(host, title, book->GetPageCount() == 0);
}

void Tab::select(int page) { if (book && page >= 0 && page < static_cast<int>(book->GetPageCount())) book->SetSelection(page); }
int Tab::getSelected(void) { return book ? book->GetSelection() : wxNOT_FOUND; }
void Tab::enable(void) { if (book) book->Enable(); }
void Tab::disable(void) { if (book) book->Disable(); }
void Tab::changeCb(wxBookCtrlEvent &event)
{
    int page = event.GetSelection();
    event.SetInt(page);
    if (page >= 0 && page < static_cast<int>(ids.size())) event.SetId(ids[page]);
    if (callback) callback(event);
    event.Skip();
}
void tabSetCallback(cb_t callb) { callback = callb; }
