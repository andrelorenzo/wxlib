#include "wxlib.h"
#include <wx/dcbuffer.h>
#include <algorithm>
#include <cmath>

struct GraphPoint { float x, y; };
static const wxColour GRAPH_COLORS[] = { wxColour(52, 152, 219), wxColour(231, 76, 60), wxColour(46, 204, 113), wxColour(155, 89, 182), wxColour(243, 156, 18), wxColour(26, 188, 156), wxColour(241, 196, 15), wxColour(52, 73, 94) };

class GraphCanvas : public wxControl {
public:
    GraphCanvas(wxWindow *parent, int channels, wxSize size) : wxControl(parent, wxID_ANY, wxDefaultPosition, size, wxBORDER_NONE | wxFULL_REPAINT_ON_RESIZE) { size_t count = static_cast<size_t>(std::max(1, channels)); points.resize(count); names.resize(count); colors.resize(count); for (size_t i = 0; i < count; ++i) { names[i] = wxString::Format("Canal %u", static_cast<unsigned>(i + 1)); colors[i] = GRAPH_COLORS[i % WXSIZEOF(GRAPH_COLORS)]; } SetMinSize(wxSize(size.x > 0 ? size.x : 100, size.y > 0 ? size.y : 100)); SetBackgroundStyle(wxBG_STYLE_PAINT); Bind(wxEVT_PAINT, &GraphCanvas::paintCb, this); Bind(wxEVT_SIZE, [this](wxSizeEvent &event) { event.Skip(); Refresh(false); }); }
    void setLimits(float xmin, float xmax, float ymin, float ymax) { if (xmin > xmax) std::swap(xmin, xmax); if (ymin > ymax) std::swap(ymin, ymax); hmin = initial_hmin = xmin; hmax = initial_hmax = xmax == xmin ? xmin + 1 : xmax; vmin = ymin; vmax = ymax == ymin ? ymin + 1 : ymax; window_width = hmax - hmin; Refresh(); }
    void setChannelName(wxString name, int channel) { if (valid(channel)) { names[channel] = name; Refresh(); } }
    void setXMode(Graph::XMode newMode, float window) { mode = newMode; window_width = window > 0 ? window : initial_hmax - initial_hmin; hmin = initial_hmin; hmax = initial_hmax; Refresh(); }
    void enableLegend(bool enable) { legend = enable; Refresh(); }
    void addValue(float x, float y, int channel) { if (!valid(channel)) return; if (mode == Graph::X_EXPANDING) { hmin = std::min(hmin, x); hmax = std::max(hmax, x); } else if (mode == Graph::X_SLIDING && x > hmax) { hmax = x; hmin = x - window_width; } points[channel].push_back({x, y}); Refresh(false); }
    void showGrid(float dx, float dy, bool enable) { grid_dx = std::max(0.0001f, std::abs(dx)); grid_dy = std::max(0.0001f, std::abs(dy)); grid = enable; Refresh(); }
    void clear(int channel) { if (channel == -1) { for (auto &values : points) values.clear(); hmin = initial_hmin; hmax = initial_hmax; } else if (valid(channel)) points[channel].clear(); Refresh(); }
private:
    bool valid(int channel) const { return channel >= 0 && channel < static_cast<int>(points.size()); }
    int gx(float value, const wxRect &area) const { return area.x + static_cast<int>(std::lround((value - hmin) * area.width / (hmax - hmin))); }
    int gy(float value, const wxRect &area) const { return area.GetBottom() - static_cast<int>(std::lround((value - vmin) * area.height / (vmax - vmin))); }
    void paintCb(wxPaintEvent &)
    {
        wxAutoBufferedPaintDC dc(this); dc.SetBackground(wxBrush(GetParent()->GetBackgroundColour())); dc.Clear(); wxSize size = GetClientSize(); if (size.x < 90 || size.y < 70) return;
        wxRect card(2, 2, size.x - 4, size.y - 4), area(50, 16, size.x - 68, size.y - 52); dc.SetPen(wxPen(wxColour(218, 223, 225))); dc.SetBrush(wxBrush(wxColour(250, 251, 252))); dc.DrawRoundedRectangle(card, 10); dc.SetFont(getFontFromEnum(FONT_TEXT)); dc.SetTextForeground(COLOR_DARK_GRAY);
        if (grid) { dc.SetPen(wxPen(wxColour(225, 230, 234))); int count = 0; for (double value = std::ceil(hmin / grid_dx) * grid_dx; value <= hmax && count++ < 200; value += grid_dx) { int x = gx(static_cast<float>(value), area); dc.DrawLine(x, area.y, x, area.GetBottom()); dc.DrawText(wxString::Format("%g", value), x + 2, area.GetBottom() + 4); } count = 0; for (double value = std::ceil(vmin / grid_dy) * grid_dy; value <= vmax && count++ < 200; value += grid_dy) { int y = gy(static_cast<float>(value), area); dc.DrawLine(area.x, y, area.GetRight(), y); wxString label = wxString::Format("%g", value); wxCoord w, h; dc.GetTextExtent(label, &w, &h); dc.DrawText(label, area.x - w - 5, y - h / 2); } }
        dc.SetPen(wxPen(wxColour(127, 140, 141))); dc.SetBrush(*wxTRANSPARENT_BRUSH); dc.DrawRectangle(area); dc.SetClippingRegion(area);
        for (size_t channel = 0; channel < points.size(); ++channel) { dc.SetPen(wxPen(colors[channel], 2)); for (size_t i = 1; i < points[channel].size(); ++i) dc.DrawLine(gx(points[channel][i - 1].x, area), gy(points[channel][i - 1].y, area), gx(points[channel][i].x, area), gy(points[channel][i].y, area)); dc.SetBrush(wxBrush(colors[channel])); for (const GraphPoint &point : points[channel]) dc.DrawCircle(gx(point.x, area), gy(point.y, area), 2); }
        dc.DestroyClippingRegion(); if (!legend) return; int x = area.GetRight() - 118, y = area.y + 8, height = static_cast<int>(names.size()) * 20 + 8; dc.SetPen(wxPen(wxColour(225, 230, 234))); dc.SetBrush(wxBrush(COLOR_WHITE)); dc.DrawRoundedRectangle(x - 6, y - 4, 122, height, 6); for (size_t i = 0; i < names.size(); ++i) { dc.SetPen(wxPen(colors[i], 3)); dc.DrawLine(x, y + 7, x + 18, y + 7); dc.SetTextForeground(COLOR_BLACK); dc.DrawText(names[i], x + 24, y); y += 20; }
    }
    float hmin = 0, hmax = 100, initial_hmin = 0, initial_hmax = 100, vmin = 0, vmax = 100, window_width = 100, grid_dx = 10, grid_dy = 10;
    Graph::XMode mode = Graph::X_FIXED;
    bool grid = false, legend = false;
    std::vector<std::vector<GraphPoint>> points;
    std::vector<wxString> names;
    std::vector<wxColour> colors;
};

void Graph::create(Container &win, int channels, int height, int width) { if (graph) return; graph = new GraphCanvas(win.window(), channels, wxSize(width > 0 ? width : 320, height > 0 ? height : HEIGHT_GRAPH)); win.add(graph, height == 0 ? 1 : 0, width > 0 ? wxALIGN_LEFT : wxEXPAND); }
void Graph::setLimits(float xmin, float xmax, float ymin, float ymax) { if (graph) graph->setLimits(xmin, xmax, ymin, ymax); }
void Graph::setChannelName(wxString name, int channel) { if (graph) graph->setChannelName(name, channel); }
void Graph::setXMode(XMode mode, float window) { if (graph) graph->setXMode(mode, window); }
void Graph::enableLegend(bool enable) { if (graph) graph->enableLegend(enable); }
void Graph::addValue(float x, float y, int channel) { if (graph) graph->addValue(x, y, channel); }
void Graph::showGrid(float dx, float dy, bool enable) { if (graph) graph->showGrid(dx, dy, enable); }
void Graph::clear(int channel) { if (graph) graph->clear(channel); }
