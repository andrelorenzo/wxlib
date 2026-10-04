#include "wxlib.h"
#include <wx/filename.h>
#include <wx/stdpaths.h>
static cb_t callback;

void Button::create(Container &win, wxString text, int id, int width, int height, wxColour col)
{
    _btn = new wxButton(win.window(), id > 0 ? id : wxID_ANY, text, wxDefaultPosition, wxSize(width, height));
    _btn->SetBackgroundColour(col);

    _btn->SetFont(getFontFromEnum(FONT_TEXTB));

    _btn->Bind(wxEVT_BUTTON, &Button::PressedCb, this);
    blinkT.Bind(wxEVT_TIMER, &Button::blinkCb, this);

    win.add(_btn, 0.0, wxALIGN_CENTER);

    _text = text;
    _col = col;
}

void Button::setImage(wxString path, wxDirection align)
{
    if (_btn == nullptr || path.IsEmpty())return;
    if (!wxFileExists(path)) path = wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetPathWithSep() + "assets" + wxFILE_SEP_PATH + path;
    if (!wxFileExists(path)) return;
    wxBitmap image(path, wxBITMAP_TYPE_PNG);
    if (!image.IsOk()) return;

    _btn->SetBitmap( wxBitmapBundle::FromBitmap(image), align);

    _btn->Refresh();
}

void Button::setText(wxString text)
{
    wxString t;
    if(text.IsEmpty()) t = this->_text;
    else t = text;

    _btn->SetLabelText(t);
}


void Button::setColor(wxColour col)
{
    wxColour c;
    if(col.IsOk() == false) c = this->_col;
    else c = col;

    _btn->SetBackgroundColour(c);
    _btn->Refresh();
}

void Button::setTextAndColor(wxString text, wxColour color)
{
    this->setText(text);
    this->setColor(color);
}


void Button::startBlink(wxColour color1, wxColour color2, uint32_t period)
{
    if(_btn == nullptr)return;
    if (period == 0U)
    {
        if (this->blinkT.IsRunning())
        {
            this->blinkT.Stop();
        }

        this->setColor(this->_col);
        return;
    }

    this->col1 = color1;
    this->col2 = color2;
    this->is_blinking = false;

    this->setColor(color1);
    this->blinkT.Start(period);
}
void Button::stopBlink(void)
{
    this->blinkT.Stop();
}

void Button::enable() { _btn->Enable(); }
void Button::disable() { _btn->Enable(false); }
void Button::setFont(Fonts font) { _btn->SetFont(getFontFromEnum(font)); }

/////// CALLBACK /////// 

void Button::PressedCb(wxCommandEvent & event)
{
    pressed = !pressed;

    event.SetInt(static_cast<int>(this->pressed));

    if(callback)callback(event);

}
void Button::blinkCb(wxTimerEvent& event)
{
    (void)(event);
    setColor(is_blinking ? col1 : col2);
    is_blinking = !is_blinking;
}



void buttonsSetCallback(cb_t callb)
{
    callback = callb;
}
