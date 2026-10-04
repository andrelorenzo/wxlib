#include "wxlib.h"

Project::~Project()
{
    if (_timer.IsRunning()) _timer.Stop();
}

void Project::create(Container &parent, int period)
{
    if (_built) return;
    _built = true;
    _root = true;
    cb_t route = [this](wxCommandEvent &event) { callback(event); };
    buttonsSetCallback(route);
    comboSetCallback(route);
    menuSetCallback(route);
    tabSetCallback(route);
    windowSetCallback(route);
    if (!_timerBound) {
        _timer.Bind(wxEVT_TIMER, &Project::timerCb, this);
        _timerBound = true;
    }
    build(parent);
    if (period > 0) _timer.Start(period);
}

void Project::mount(Project &project, Container &parent)
{
    if (!_built || project._built) return;
    project._built = true;
    _children.push_back(&project);
    project.build(parent);
}

void Project::destroy(void)
{
    if (!_built) return;
    if (_timer.IsRunning()) _timer.Stop();
    for (auto project = _children.rbegin(); project != _children.rend(); ++project) (*project)->destroy();
    _children.clear();
    close();
    if (_root) {
        buttonsSetCallback({});
        comboSetCallback({});
        menuSetCallback({});
        tabSetCallback({});
        windowSetCallback({});
    }
    _root = false;
    _built = false;
}

void Project::timerCb(wxTimerEvent &event)
{
    (void)event;
    timer();
}
