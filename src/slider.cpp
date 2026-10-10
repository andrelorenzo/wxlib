#include "wxlib.h"
#include <wx/dcbuffer.h>
#include <algorithm>
#include <cmath>

static const wxColour TRACK_COLOR(225, 230, 234), VALUE_COLOR(52, 152, 219), AXIS_COLOR(170, 178, 185), STICK_COLOR(41, 128, 185);

class SliderBar : public wxControl {
public:
    SliderBar(wxWindow *parent, wxSize size, bool vertical) : wxControl(parent, wxID_ANY, wxDefaultPosition, size, wxBORDER_NONE | wxFULL_REPAINT_ON_RESIZE | wxWANTS_CHARS), vertical(vertical) { SetBackgroundStyle(wxBG_STYLE_PAINT); SetMinSize(size); Bind(wxEVT_PAINT, &SliderBar::paintCb, this); Bind(wxEVT_SIZE, &SliderBar::sizeCb, this); Bind(wxEVT_LEFT_DOWN, &SliderBar::mouseCb, this); Bind(wxEVT_MOTION, &SliderBar::mouseCb, this); Bind(wxEVT_LEFT_UP, &SliderBar::releaseCb, this); Bind(wxEVT_KEY_DOWN, &SliderBar::keyCb, this); }
    void setLimits(uint16_t minValue, uint16_t maxValue, uint16_t neutralValue) { min = minValue; max = maxValue; neutral = std::clamp(neutralValue, min, max); value = std::clamp(value, min, max); Refresh(); }
    void setValue(uint16_t newValue) { value = std::clamp(newValue, min, max); Refresh(); }
    void setReturnToNeutral(bool enable) { returnToNeutral = enable; }
    void showValue(bool show) { show_value = show; Refresh(); }
    uint16_t getValue(void) const { return value; }
    void setChangeHandler(std::function<void(uint16_t)> handler) { changeHandler = std::move(handler); }
private:
    double proportion(void) const { return max == min ? 0.0 : static_cast<double>(value - min) / (max - min); }
    void update(const wxPoint &position) { wxSize size = GetClientSize(); int length = vertical ? size.y : size.x; if (length <= 0) return; double point = vertical ? 1.0 - static_cast<double>(position.y) / length : static_cast<double>(position.x) / length; const auto next = static_cast<uint16_t>(std::lround(min + std::clamp(point, 0.0, 1.0) * (max - min))); if (next != value) { value = next; if (changeHandler) changeHandler(value); } Refresh(); }
    void paintCb(wxPaintEvent &) { wxAutoBufferedPaintDC dc(this); dc.SetBackground(wxBrush(GetParent()->GetBackgroundColour())); dc.Clear(); wxSize size = GetClientSize(); wxRect track(3, 3, std::max(0, size.x - 6), std::max(0, size.y - 6)), fill = track; dc.SetPen(*wxTRANSPARENT_PEN); dc.SetBrush(wxBrush(TRACK_COLOR)); dc.DrawRoundedRectangle(track, 6); if (vertical) { fill.height = static_cast<int>(std::lround(track.height * proportion())); fill.y = track.GetBottom() - fill.height + 1; } else fill.width = static_cast<int>(std::lround(track.width * proportion())); dc.SetBrush(wxBrush(VALUE_COLOR)); dc.DrawRoundedRectangle(fill, 6); if (!show_value) return; wxString text = wxString::Format("%u", value); dc.SetFont(getFontFromEnum(FONT_TEXTB)); dc.SetTextForeground(COLOR_BLACK); wxCoord w, h; dc.GetTextExtent(text, &w, &h); if (vertical) dc.DrawRotatedText(text, (size.x + h) / 2, (size.y - w) / 2, 90); else dc.DrawText(text, (size.x - w) / 2, (size.y - h) / 2); }
    void sizeCb(wxSizeEvent &event) { event.Skip(); Refresh(false); }
    void mouseCb(wxMouseEvent &event) { if (!IsEnabled() || (!event.LeftDown() && !(event.Dragging() && event.LeftIsDown()))) return; if (event.LeftDown()) SetFocus(); if (!HasCapture()) CaptureMouse(); update(event.GetPosition()); }
    void keyCb(wxKeyEvent &event) { int delta = 0; switch (event.GetKeyCode()) { case WXK_LEFT: case WXK_DOWN: delta = -1; break; case WXK_RIGHT: case WXK_UP: delta = 1; break; default: event.Skip(); return; } if (event.ShiftDown()) delta *= 100; const auto next = static_cast<uint16_t>(std::clamp(static_cast<int>(value) + delta, static_cast<int>(min), static_cast<int>(max))); if (next != value) { value = next; if (changeHandler) changeHandler(value); Refresh(); } }
    void releaseCb(wxMouseEvent &event) { if (HasCapture()) ReleaseMouse(); if (IsEnabled()) update(event.GetPosition()); if (returnToNeutral) setValue(neutral); }
    bool vertical = false, returnToNeutral = false, show_value = false;
    std::function<void(uint16_t)> changeHandler;
    uint16_t min = 0, max = 100, value = 0, neutral = 50;
};

class JoystickPad : public wxControl {
public:
    JoystickPad(wxWindow *parent, wxSize size) : wxControl(parent, wxID_ANY, wxDefaultPosition, size, wxBORDER_NONE | wxFULL_REPAINT_ON_RESIZE) { SetBackgroundStyle(wxBG_STYLE_PAINT); SetMinSize(size); Bind(wxEVT_PAINT, &JoystickPad::paintCb, this); Bind(wxEVT_SIZE, &JoystickPad::sizeCb, this); Bind(wxEVT_LEFT_DOWN, &JoystickPad::mouseCb, this); Bind(wxEVT_MOTION, &JoystickPad::mouseCb, this); Bind(wxEVT_LEFT_UP, &JoystickPad::releaseCb, this); }
    void setLimits(uint16_t minValue, uint16_t maxValue, uint16_t neutralValue) { min = minValue; max = maxValue; neutral = std::clamp(neutralValue, min, max); x = std::clamp(x, min, max); y = std::clamp(y, min, max); Refresh(); }
    void setValue(uint16_t value, int axis) { if (axis == 1) y = std::clamp(value, min, max); else x = std::clamp(value, min, max); Refresh(); }
    void setReturnToNeutral(bool enable, int axis) { if (axis == 1) returnY = enable; else returnX = enable; }
    void showValue(bool show) { show_value = show; Refresh(); }
    uint16_t getValue(int axis) const { return axis == 1 ? y : x; }
private:
    double proportion(uint16_t value) const { return max == min ? 0.5 : static_cast<double>(value - min) / (max - min); }
    void update(const wxPoint &position) { wxSize size = GetClientSize(); double px = std::clamp(static_cast<double>(position.x - 12) / std::max(1, size.x - 24), 0.0, 1.0), py = std::clamp(1.0 - static_cast<double>(position.y - 12) / std::max(1, size.y - 24), 0.0, 1.0); x = static_cast<uint16_t>(std::lround(min + px * (max - min))); y = static_cast<uint16_t>(std::lround(min + py * (max - min))); Refresh(); }
    void paintCb(wxPaintEvent &) { wxAutoBufferedPaintDC dc(this); dc.SetBackground(wxBrush(GetParent()->GetBackgroundColour())); dc.Clear(); wxSize size = GetClientSize(); wxRect area(12, 12, std::max(0, size.x - 24), std::max(0, size.y - 24)); dc.SetPen(wxPen(AXIS_COLOR)); dc.SetBrush(wxBrush(TRACK_COLOR)); dc.DrawRoundedRectangle(area, 12); dc.DrawLine(area.GetLeft(), area.y + area.height / 2, area.GetRight(), area.y + area.height / 2); dc.DrawLine(area.x + area.width / 2, area.GetTop(), area.x + area.width / 2, area.GetBottom()); wxPoint stick(area.x + static_cast<int>(std::lround(area.width * proportion(x))), area.GetBottom() - static_cast<int>(std::lround(area.height * proportion(y)))); dc.SetPen(wxPen(COLOR_WHITE, 2)); dc.SetBrush(wxBrush(STICK_COLOR)); dc.DrawCircle(stick, 9); if (!show_value) return; wxString text = wxString::Format("X: %u  Y: %u", x, y); dc.SetFont(getFontFromEnum(FONT_TEXTB)); dc.SetTextForeground(COLOR_BLACK); wxCoord w, h; dc.GetTextExtent(text, &w, &h); dc.DrawText(text, area.x + (area.width - w) / 2, area.y + 4); }
    void sizeCb(wxSizeEvent &event) { event.Skip(); Refresh(false); }
    void mouseCb(wxMouseEvent &event) { if (!IsEnabled() || (!event.LeftDown() && !(event.Dragging() && event.LeftIsDown()))) return; if (!HasCapture()) CaptureMouse(); update(event.GetPosition()); }
    void releaseCb(wxMouseEvent &event) { if (HasCapture()) ReleaseMouse(); if (IsEnabled()) update(event.GetPosition()); if (returnX) x = neutral; if (returnY) y = neutral; Refresh(); }
    bool returnX = false, returnY = false, show_value = false;
    uint16_t min = 0, max = 100, x = 0, y = 0, neutral = 50;
};

void Slider::create(Container &win, Type type, wxString title, int width, int height)
{
    if (panel) return;
    panel = new wxPanel(win.window()); panel->SetBackgroundColour(win.window()->GetBackgroundColour()); wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL); panel->SetSizer(sizer);
    if (!title.IsEmpty()) { wxStaticText *text = new wxStaticText(panel, wxID_ANY, title); text->SetFont(getFontFromEnum(FONT_TEXTB)); sizer->Add(text, 0, wxBOTTOM | wxALIGN_CENTER_HORIZONTAL, 4); }
    if (type == JOYSTICK) { joystick = new JoystickPad(panel, wxSize(width, width)); sizer->Add(joystick, 0, wxALIGN_CENTER_HORIZONTAL); }
    else { slider = new SliderBar(panel, type == VERTICAL ? wxSize(height, width) : wxSize(width, height), type == VERTICAL); sizer->Add(slider, 0, wxALIGN_CENTER_HORIZONTAL); }
    win.add(panel, 0, wxEXPAND);
}

void Slider::setLimits(uint16_t min, uint16_t max, uint16_t neutral) { if (min > max) std::swap(min, max); neutral = std::clamp(neutral, min, max); if (joystick) joystick->setLimits(min, max, neutral); else if (slider) slider->setLimits(min, max, neutral); }
void Slider::setValue(uint16_t value, int axis) { if (joystick) joystick->setValue(value, axis); else if (slider && axis == 0) slider->setValue(value); }
void Slider::setReturnToNeutral(bool enable, int axis) { if (joystick) joystick->setReturnToNeutral(enable, axis); else if (slider && axis == 0) slider->setReturnToNeutral(enable); }
void Slider::setChangeHandler(std::function<void(uint16_t)> handler) { changeHandler_ = std::move(handler); if (slider) slider->setChangeHandler(changeHandler_); }
void Slider::showValue(bool show) { if (joystick) joystick->showValue(show); else if (slider) slider->showValue(show); }
uint16_t Slider::getValue(int axis) { if (joystick) return joystick->getValue(axis); return slider && axis == 0 ? slider->getValue() : 0; }
void Slider::enable(void) { if (joystick) joystick->Enable(); else if (slider) slider->Enable(); }
void Slider::disable(void) { if (joystick) joystick->Disable(); else if (slider) slider->Disable(); }
