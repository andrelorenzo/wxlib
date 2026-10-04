#include "wxlib.h"

void Edit::create(Container &win, wxString text, int width, int height, wxColour col)
{
    if (edt) return;
    _text = text;
    _col = col;
    edt = new wxTextCtrl(win.window(), wxID_ANY, text, wxDefaultPosition, wxSize(width > 0 ? width : wxDefaultCoord, height), height > DEF_HEIGHT ? wxTE_MULTILINE : 0);
    edt->SetBackgroundColour(col);
    edt->SetFont(getFontFromEnum(FONT_TEXTB));
    blinkT.Bind(wxEVT_TIMER, &Edit::blinkCb, this);
    win.add(edt, width > 0 ? 0 : 1, width > 0 ? wxALIGN_LEFT : wxEXPAND);
}

void Edit::create(wxWindow *parent, wxString text, int width, int height, wxColour col)
{
    if (edt || !parent) return;
    _text = text; _col = col;
    edt = new wxTextCtrl(parent, wxID_ANY, text, wxDefaultPosition, wxSize(width, height), height > DEF_HEIGHT ? wxTE_MULTILINE : 0);
    edt->SetBackgroundColour(col); edt->SetFont(getFontFromEnum(FONT_TEXTB)); blinkT.Bind(wxEVT_TIMER, &Edit::blinkCb, this);
}


void Edit::setScrollDown()
{
    scroll = true;
    if (edt) 
    { 
        edt->SetEditable(false);
        edt->SetInsertionPointEnd(); 
        edt->ShowPosition(edt->GetLastPosition()); 
    }
}

void Edit::setText(wxString text)
{
    edt->ChangeValue(text.IsEmpty() ? _text : text);
    if (scroll) { edt->SetInsertionPointEnd(); edt->ShowPosition(edt->GetLastPosition()); }
}

void Edit::setText(int value) { setText(wxString::Format("%d", value)); }
void Edit::setText(float value) { setText(wxString::Format("%g", static_cast<double>(value))); }

void Edit::setColor(wxColour col)
{
    edt->SetBackgroundColour(col.IsOk() ? col : _col);
    edt->Refresh();
}

void Edit::setTextAndColor(wxString text, wxColour color)
{
    setText(text);
    setColor(color);
}

void Edit::setFont(Fonts font) { edt->SetFont(getFontFromEnum(font)); }
wxString Edit::getText(void) { return edt->GetValue(); }

float Edit::getNumber(void)
{
    double value = 0;
    edt->GetValue().ToDouble(&value);
    return static_cast<float>(value);
}

void Edit::enable(void) { edt->Enable(); }
void Edit::disable(void) { edt->Disable(); }

void Edit::startBlink(wxColour color1, wxColour color2, uint32_t period)
{
    if (!edt) return;
    if (period == 0)
    {
        stopBlink();
        return;
    }
    col1 = color1;
    col2 = color2;
    is_blinking = false;
    setColor(color1);
    blinkT.Start(period);
}

void Edit::stopBlink(void)
{
    blinkT.Stop();
    setColor(_col);
}

void Edit::addTextLine(wxString text)
{
    if (!edt->IsEmpty() && !edt->GetValue().EndsWith("\n")) edt->AppendText("\n");
    edt->AppendText(text);
    if (scroll) { edt->SetInsertionPointEnd(); edt->ShowPosition(edt->GetLastPosition()); }
}

void Edit::clear(void) { edt->ChangeValue(wxEmptyString); }

void Edit::updateLast(wxString text)
{
    if (edt->IsEmpty())
    {
        addTextLine(text);
        return;
    }
    long line = edt->GetNumberOfLines() - 1;
    if (line > 0 && edt->GetLineText(line).IsEmpty()) --line;
    long start = edt->XYToPosition(0, line);
    edt->Replace(start, start + edt->GetLineLength(line), text);
    if (scroll) { edt->SetInsertionPointEnd(); edt->ShowPosition(edt->GetLastPosition()); }
}

void Edit::blinkCb(wxTimerEvent &event)
{
    (void)event;
    setColor(is_blinking ? col1 : col2);
    is_blinking = !is_blinking;
}
