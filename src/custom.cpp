#include "wxlib.h"
#include <algorithm>


static void customData(Container &win, Data *data, size_t len, const wxString *names, int channels, int width, bool config)
{
    if (!data || !len) return;
    wxClientDC dc(win.window()); dc.SetFont(getFontFromEnum(FONT_TEXT)); int textWidth = 0;
    for (size_t i = 0; i < len; ++i) { wxString name = names ? names[i] : wxString::Format(config ? "Config %u" : "Telemetry %u", static_cast<unsigned>(i + 1)); textWidth = std::max(textWidth, dc.GetTextExtent(name).x + 4); }
    Container grid; grid.create(win, -1, Container::SIZER_GRID); grid.setPadding(0);
    for (size_t i = 0; i < len; ++i) { wxString name = names ? names[i] : wxString::Format(config ? "Config %u" : "Telemetry %u", static_cast<unsigned>(i + 1)); if (config) data[i].createConfig(grid, name, textWidth, DEF_HEIGHT, channels, width); else data[i].createTelem(grid, name, textWidth, DEF_HEIGHT, channels, width); }
    grid.window()->SendSizeEvent();
}
void customConfig(Container &win, Data *configs, size_t len, const wxString *names, int channels, int width) 
{ 
    customData(win, configs, len, names, channels, std::max(1, width), true); 
}
void customTelem(Container &win, Data *telemetry, size_t len, const wxString *names, int channels, int width) 
{ 
    customData(win, telemetry, len, names, channels, std::max(1, width), false); 
}
void customButtons(Container &win, Button *alarms, size_t len, const wxString *names) { 
    if (!alarms || !len) return; 
    Container grid; 
    grid.create(win, -1, Container::SIZER_ALARMS); 
    grid.setPadding(0); 
    for (size_t i = 0; i < len; ++i) 
    { 
        alarms[i].create(grid, names ? names[i] : wxString::Format("Alarm %u", static_cast<unsigned>(i + 1)), 0, LENGTH_BUTTON, DEF_HEIGHT, COLOR_GRAY); 
        alarms[i].disable(); 
    } 
    grid.window()->SendSizeEvent(); 
}

// imu calib, rtsp viewer, 
