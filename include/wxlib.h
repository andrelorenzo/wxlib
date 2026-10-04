#pragma once
#include "wx/colour.h"
#include <functional>
#include <vector>
#include <wx/aboutdlg.h>
#include <wx/button.h>
#include <wx/combobox.h>
#include <wx/gauge.h>
#include <wx/grid.h>
#include <wx/notebook.h>
#include <wx/panel.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/wx.h>

// #include "stdbool.h"

#define DEF_HEIGHT 24
#define LENGTH_BUTTON 120
#define LENGTH_EDIT 120
#define LENGTH_COMBO 120
#define LENGTH_BAR 120
#define LENGTH_SLIDER 180
#define LENGTH_DATA 240
#define HEIGHT_GRAPH 240
#define HEIGHT_GRID 300
#define DEF_PADDING 30

#define COLOR_RED wxColour(236, 112, 99)
#define COLOR_GREEN wxColour(88, 214, 141)
#define COLOR_ORANGE wxColour(235, 152, 78)
#define COLOR_YELLOW wxColour(244, 208, 63)
#define COLOR_PURPLE wxColour(175, 122, 197)
#define COLOR_GRAY wxColour(202, 207, 210)
#define COLOR_DARK_GRAY wxColour(98, 101, 103)
#define COLOR_BLACK wxColour(0, 0, 0)
#define COLOR_WHITE wxColour(255, 255, 255)

#define COLOR_NOT_PROVIDED wxColour(wxS("null"))

enum Fonts {
  FONT_TEXT = 0U,
  FONT_TEXTB,
  FONT_TITLE,
  FONT_TITLEB,
  FONT_TITLE2,
  FONT_TITLE2B
};

using cb_t = std::function<void(wxCommandEvent &)>;
void buttonsSetCallback(cb_t callback);
void comboSetCallback(cb_t callback);
void menuSetCallback(cb_t callback);
void tabSetCallback(cb_t callback);
void windowSetCallback(cb_t callback);
void windowCloseAll(void);
wxIcon appIcon();

wxFont getFontFromEnum(Fonts font);

class Container {
public:
  enum SizerType {
    SIZER_VER = 0U,
    SIZER_HOR,
    SIZER_GRID,
    SIZER_WRAP_HOR,
    SIZER_ALARMS
  };
  Container() = default;
  void create(wxWindow *parent, SizerType type = SIZER_VER, bool border = false, wxColour col = COLOR_NOT_PROVIDED);
  void create(Container &parent, double prop = -1, SizerType type = SIZER_VER, bool border = false, wxColour col = COLOR_NOT_PROVIDED);
  void setPadding(int padding);
  wxWindow *window(void) const { return _pnl; }
  void add(wxWindow *child, double prop, int flag = wxEXPAND);
  void addSpacer(int prop = 1);
  void addSeparator(wxColour color = COLOR_DARK_GRAY, int thickness = 1, int margin = 5);

private:
  wxSizer *createContentSizer(SizerType type);
  void bindGrid(SizerType type);

private:
  wxPanel *_pnl = nullptr;
  wxSizer *_szr = nullptr;
  double _used = 0;
  int _padding = DEF_PADDING;
  SizerType _type = SIZER_VER;
};

class Project {
public:
  Project() = default;
  virtual ~Project();
  void create(Container &parent, int period = 100);
  void destroy(void);
protected:
  virtual void build(Container &parent) = 0;
  virtual void close(void) {}
  virtual void callback(wxCommandEvent &event) { (void)event; }
  virtual void timer(void) {}
  void mount(Project &project, Container &parent);
private:
  void timerCb(wxTimerEvent &event);
  wxTimer _timer;
  std::vector<Project *> _children;
  bool _built = false;
  bool _root = false;
  bool _timerBound = false;
};

class Button {
public:
  Button() = default;
  void create(Container &win, wxString text, int id = 0,int width = LENGTH_BUTTON, int height = DEF_HEIGHT,wxColour col = COLOR_GRAY);
  void setText(wxString text = wxS(""));
  void setColor(wxColour col = COLOR_NOT_PROVIDED);
  void setTextAndColor(wxString text = wxS(""),wxColour color = COLOR_NOT_PROVIDED);
  void setImage(wxString path, wxDirection align = wxLEFT);
  void setFont(Fonts font);

  void enable(void);
  void disable(void);

  void startBlink(wxColour color1, wxColour color2, uint32_t period = 400);
  void stopBlink(void);

private:
  void PressedCb(wxCommandEvent &event);
  void blinkCb(wxTimerEvent &event);

private:
  wxButton *_btn = nullptr;
  bool pressed = false;

  wxString _text = wxS("");
  wxColour _col = COLOR_NOT_PROVIDED;

  bool is_blinking = false;
  wxTimer blinkT;
  wxColour col1 = COLOR_NOT_PROVIDED;
  wxColour col2 = COLOR_NOT_PROVIDED;
};

class Text {
public:
  Text() = default;
  void create(Container &win, wxString text = wxS(""), Fonts font = FONT_TEXT,int align = wxALIGN_INVALID);
  void create(wxWindow *parent, wxString text = wxS(""), Fonts font = FONT_TEXT,int align = wxALIGN_INVALID);
  wxStaticText *window(void) const { return _txt; }
  void setFont(Fonts font);
  void setWidth(int width, bool wordWrap = true);
  void setText(wxString text = wxS(""));
  void setColor(wxColour col = COLOR_NOT_PROVIDED);
  void setTextAndColor(wxString text = wxS(""),wxColour color = COLOR_NOT_PROVIDED);

private:
  void wrap(void);
  void scheduleWrap(void);
  wxStaticText *_txt = nullptr;
  wxString _text = wxS("");
  wxColour _col = COLOR_NOT_PROVIDED;
  int _width = 0;
  bool _wordWrap = true;
  int _lastWrapWidth = -1;
  bool _wrapPending = false;
};

class Edit {
public:
  Edit() = default;
  void create(Container &win, wxString text, int width = LENGTH_EDIT,int height = DEF_HEIGHT, wxColour col = COLOR_GRAY);
  void create(wxWindow *parent, wxString text, int width = LENGTH_EDIT, int height = DEF_HEIGHT, wxColour col = COLOR_GRAY);
  wxTextCtrl *window(void) const { return edt; }
  void setScrollDown();
  void setText(wxString text = wxS(""));
  void setText(int value);
  void setText(float value);
  void setColor(wxColour col = COLOR_NOT_PROVIDED);
  void setTextAndColor(wxString text = wxS(""), wxColour color = COLOR_NOT_PROVIDED);
  void setFont(Fonts font);

  wxString getText(void);
  float getNumber(void);

  void enable(void);
  void disable(void);

  void startBlink(wxColour color1, wxColour color2, uint32_t period = 400);
  void stopBlink(void);

  void addTextLine(wxString text);
  void clear(void);
  void updateLast(wxString text);

private:
  void blinkCb(wxTimerEvent &event);

private:
  wxTextCtrl *edt = nullptr;
  bool scroll = false;

  wxString _text = wxS("");
  wxColour _col = COLOR_NOT_PROVIDED;

  bool is_blinking = false;
  wxTimer blinkT;
  wxColour col1 = COLOR_NOT_PROVIDED;
  wxColour col2 = COLOR_NOT_PROVIDED;
};

class Data {
public:
  Data() = default;
  void createTelem(Container &win, wxString title, int width = LENGTH_DATA, int height = DEF_HEIGHT, int nchan = 1, int editWidth = -1);
  void createConfig(Container &win, wxString title, int width = LENGTH_DATA, int height = DEF_HEIGHT, int nchan = 1, int editWidth = -1);

  void update(wxString value, uint8_t chan = 0);
  void update(float value, uint8_t chan = 0);
  void update(int value, uint8_t chan = 0);

  void update(wxString *value, size_t len);
  void update(float *value, size_t len);
  void update(int *value, size_t len);

  wxString getText(uint8_t chan = 0);
  float getNumber(uint8_t chan = 0);
  void setColor(wxColour color = COLOR_NOT_PROVIDED);
  void setBoolMode(bool enabled = true);
  void show(bool show = true);
  Edit *edit(int chan = 0) { return &_edts[chan]; }
  Text *text() { return &_text; }

private:
  void create(Container &win, wxString title, int width, int height, int nchan, bool enabled, int editWidth);

private:
  wxPanel *panel = nullptr;
  std::vector<Edit> _edts;
  Text _text;
  wxComboBox *_bool = nullptr;
  bool _boolMode = false;
};

class Combo {
public:
  Combo() = default;
  void create(Container &win, int id = 0, int width = LENGTH_COMBO, int height = DEF_HEIGHT);
  void fillCom(void);
  void fillEth(void);
  void fill(wxVector<wxString> list = wxVector<wxString>());
  void fill(const wxString *list, size_t len);
  void select(wxString value);

  void enable(void);
  void disable(void);

  wxString getSelected(void);

private:
  void updateList(void);
  void setItems(const wxVector<wxString> &list);
  void updateCb(wxTimerEvent &event);
  void changeCb(wxCommandEvent &event);

private:
  enum FillType { FILL_NONE = 0U, FILL_COM, FILL_ETH };
  FillType fill_type = FILL_NONE;
  bool opened = false;

  wxComboBox *cmb = nullptr;
  wxTimer updateT;
};

class Bar {
public:
  Bar() = default;
  void create(Container &win, int height = DEF_HEIGHT, int width = 0, uint32_t steps = 100);
  void set(float percentage);
  void advance(uint16_t steps = 1);
  void showValue(bool show = true);
  void enable(void);
  void disable(void);
  void clear(void);

private:
  void updateValue(void);
  wxPanel *panel = nullptr;
  wxGauge *bar = nullptr;
  wxStaticText *valueText = nullptr;
};

class SliderBar;
class JoystickPad;

class Slider {
public:
  enum Type { HORIZONTAL = 0U, VERTICAL, JOYSTICK };
  Slider() = default;
  void create(Container &win, Type type = HORIZONTAL, wxString title = wxS(""), int width = LENGTH_SLIDER, int height = DEF_HEIGHT);
  void setLimits(uint16_t min, uint16_t max, uint16_t neutral = 50);
  void setValue(uint16_t value, int axis = 0);
  void setReturnToNeutral(bool enable, int axis = 0);
  void showValue(bool show = true);
  uint16_t getValue(int axis = 0);
  void enable(void);
  void disable(void);

private:
  wxPanel *panel = nullptr;
  SliderBar *slider = nullptr;
  JoystickPad *joystick = nullptr;
};

class Tab {
public:
  Tab() = default;
  void create(Container &win);
  void add(Container &page, wxString title, int id = 0, Container::SizerType type = Container::SIZER_VER, wxColour color = COLOR_NOT_PROVIDED);
  void select(int page);
  int getSelected(void);
  void enable(void);
  void disable(void);
private:
  void changeCb(wxBookCtrlEvent &event);
  wxNotebook *book = nullptr;
  std::vector<int> ids;
};

class Menu {
public:
  Menu() = default;
  void create(wxFrame *win);
  void create(Container &win);
  int add(wxString title);
  void addChild(int parent, wxString title, int id, wxString help = wxS(""));
  void addSeparator(int parent);
  void enable(int id, bool enabled = true);
private:
  void selectCb(wxCommandEvent &event);
  wxFrame *frame = nullptr;
  wxMenuBar *bar = nullptr;
  std::vector<wxMenu *> menus;
};

class GraphCanvas;

class Graph {
public:
  enum XMode { X_FIXED = 0U, X_EXPANDING, X_SLIDING };
  Graph() = default;
  void create(Container &win, int channels = 1, int height = HEIGHT_GRAPH, int width = 0);
  void setLimits(float xmin, float xmax, float ymin, float ymax);
  void setChannelName(wxString name, int channel = 0);
  void setXMode(XMode mode, float window = 0);
  void enableLegend(bool enable = true);
  void addValue(float x, float y, int channel = 0);
  void showGrid(float dx = 10, float dy = 10, bool enable = true);
  void clear(int channel = -1);
private:
  GraphCanvas *graph = nullptr;
};


class Window {
public:
    Window() = default;
    void create(Container &parent, wxString title, int id = 0, bool controls = false, bool dialog = false, wxSize size = wxSize(600, 400));
    void setApplyCallback(cb_t callback);

    Container * window(void){return &_win;}
private:
    void applyCb(wxCommandEvent &event);
    Container _win;
    cb_t _callback;
    wxWindow *_top = nullptr;
    int _id = 0;
    
};

class Grid {
public:
    Grid() = default;
    void create(Container &win, const wxString *columns, size_t len, int rows = 0, int height = HEIGHT_GRID, int width = 0);
    void addRow(const wxString *values = nullptr, size_t len = 0);
    void setRow(int row, const wxString *values, size_t len);
    void setText(int row, int column, wxString text);
    wxString getText(int row, int column);
    void setEditable(int column, bool editable = true);
    void setColumnWidth(int column, int width = 0);
    void setColor(int row, int column, wxColour color);
    void deleteRow(int row);
    int getSelectedRow(void);
    void clear(void);
    int getRowsCount(void);
    wxGrid *window(void) const { return grid; }
private:
    void fitColumns(void);
    void styleRow(int row);
    wxGrid *grid = nullptr;
    std::vector<int> widths;
    std::vector<bool> editable;
};

void customConfig(Container &win, Data *configs, size_t len, const wxString *names = nullptr, int channels = 1, int width = LENGTH_EDIT);
void customTelem(Container &win, Data *telemetry, size_t len, const wxString *names = nullptr, int channels = 1, int width = LENGTH_EDIT);
void customButtons(Container &win, Button *alarms, size_t len, const wxString *names = nullptr);
