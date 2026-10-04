#include "wxlib.h"


void Text::create(Container &win, wxString text, Fonts font, int align)
{
    create(win.window(), text, font, align);
    win.add(_txt, 0, wxEXPAND);
    win.window()->Bind(wxEVT_SIZE, [this](wxSizeEvent &event) {
        event.Skip();
        scheduleWrap();
    });
    scheduleWrap();
}

void Text::create(wxWindow *parent, wxString text, Fonts font, int align)
{
    this->_text = text;
    int text_align = align;

    if (text_align == wxALIGN_INVALID)
    {
        if (font >= FONT_TITLE) text_align = wxALIGN_CENTER_HORIZONTAL;
        else text_align = wxALIGN_LEFT;
    }

    this->_txt = new wxStaticText(parent, wxID_ANY, text, wxDefaultPosition, wxDefaultSize, text_align | wxST_NO_AUTORESIZE, wxStaticTextNameStr);
    this->_txt->SetFont(getFontFromEnum(font));
    this->_txt->SetMinSize(wxSize(0, -1));
}

void Text::setFont(Fonts font)
{
    this->_txt->SetFont(getFontFromEnum(font));
    wrap();
}

void Text::setWidth(int width, bool wordWrap)
{
    _width = width;
    _wordWrap = wordWrap;
    _lastWrapWidth = -1;
    _txt->SetMinSize(wxSize(width, -1));
    _txt->SetMaxSize(wxSize(width, -1));
    wrap();
}

void Text::setText(wxString text)
{
    if (!text.IsEmpty()) this->_text = text;
    wrap();
}
void Text::setColor(wxColour col)
{
    wxColour c;
    if(col.IsOk() == false) c = this->_col;
    else c = col;

    this->_txt->SetForegroundColour(c);
    this->_txt->Refresh();
}
void Text::setTextAndColor(wxString text, wxColour color)
{
    this->setText(text);
    this->setColor(color);
}

void Text::wrap(void)
{
    if (!_txt) return;
    int width = _width > 0 ? _width : _txt->GetSize().x;
    if (width <= 0) return;
    _txt->SetLabelText(_text);
    if (_wordWrap) _txt->Wrap(width);
    wxClientDC dc(_txt);
    dc.SetFont(_txt->GetFont());
    int height = dc.GetMultiLineTextExtent(_txt->GetLabel()).y;
    wxSize previous = _txt->GetMinSize();
    wxSize required(_width, height);
    _lastWrapWidth = width;
    if (previous == required) return;
    _txt->SetMinSize(required);
    _txt->GetParent()->Layout();
}

void Text::scheduleWrap(void)
{
    if (!_txt || !_wordWrap || _width > 0 || _wrapPending) return;
    int width = _txt->GetSize().x;
    if (width <= 0 || width == _lastWrapWidth) return;
    _wrapPending = true;
    _txt->CallAfter([this] {
        _wrapPending = false;
        if (_txt && _wordWrap) wrap();
    });
}

wxFont getFontFromEnum(Fonts font)
{
    // const char * font_name = "Space Mono";
    const char * font_name = "Calibri";

    switch(font)
    {
    case FONT_TEXT: { return wxFont(wxFontInfo(10).FaceName(font_name));}
    case FONT_TEXTB: { return wxFont(wxFontInfo(10).FaceName(font_name).Bold());}
    case FONT_TITLE: { return wxFont(wxFontInfo(14).FaceName(font_name));}
    case FONT_TITLEB: { return wxFont(wxFontInfo(14).FaceName(font_name).Bold());}
    case FONT_TITLE2: { return wxFont(wxFontInfo(18).FaceName(font_name));}
    case FONT_TITLE2B: { return wxFont(wxFontInfo(18).FaceName(font_name).Bold());}
    default: return  wxFont(wxFontInfo(12).FaceName(font_name));
    }
}
