#include "wxlib.h"
#include <set>

#ifdef _WIN32
#include <iphlpapi.h>
#else
#include <ifaddrs.h>
#include <net/if.h>
#include <netinet/in.h>
#include <wx/dir.h>
#endif

static cb_t callback;
static wxString subnet(const unsigned char *ip, const unsigned char *mask) { wxString value; for (int i = 0; i < 4; ++i) value += (i ? "." : "") + (mask[i] == 255 ? wxString::Format("%d", ip[i] & mask[i]) : "X"); return value; }

void Combo::create(Container &win, int id, int width, int height)
{
    if (cmb) return;
    cmb = new wxComboBox(win.window(), id > 0 ? id : wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(width, height), wxArrayString(), wxCB_READONLY);
    cmb->SetFont(getFontFromEnum(FONT_TEXT));
    cmb->Bind(wxEVT_COMBOBOX, &Combo::changeCb, this);
    cmb->Bind(wxEVT_COMBOBOX_DROPDOWN, [this](wxCommandEvent &event) { opened = true; event.Skip(); });
    cmb->Bind(wxEVT_COMBOBOX_CLOSEUP, [this](wxCommandEvent &event) { opened = false; event.Skip(); });
    updateT.Bind(wxEVT_TIMER, &Combo::updateCb, this);
    win.add(cmb, 0, wxALIGN_LEFT);
}

void Combo::fillCom(void)
{
    fill_type = FILL_COM;
    updateList();
    updateT.Start(1000);
}

void Combo::fillEth(void)
{
    fill_type = FILL_ETH;
    updateList();
    updateT.Start(1000);
}

void Combo::fill(wxVector<wxString> list)
{
    fill_type = FILL_NONE;
    updateT.Stop();
    setItems(list);
}

void Combo::fill(const wxString *list, size_t len)
{
    wxVector<wxString> values;
    for (size_t i = 0; list && i < len; ++i) values.push_back(list[i]);
    fill(values);
}

void Combo::select(wxString value) { if (cmb) cmb->SetSelection(cmb->FindString(value)); }

void Combo::setItems(const wxVector<wxString> &list)
{
    wxString selected = getSelected();
    cmb->Freeze();
    cmb->Clear();
    for (const wxString &item : list) cmb->Append(item);
    int index = cmb->FindString(selected);
    if (index != wxNOT_FOUND) cmb->SetSelection(index);
    else if (!list.empty()) cmb->SetSelection(0);
    cmb->Thaw();
}

void Combo::updateList(void)
{
    wxVector<wxString> list;
    if (fill_type == FILL_COM)
    {
#ifdef _WIN32
        wchar_t target[512];
        for (int i = 1; i <= 256; ++i)
        {
            wxString port = wxString::Format("COM%d", i);
            if (QueryDosDeviceW(port.wc_str(), target, 512)) list.push_back(port);
        }
#else
        wxDir dir("/dev");
        std::set<wxString> ports;
        const wxString patterns[] = { "ttyS*", "ttyUSB*", "ttyACM*" };
        for (const wxString &pattern : patterns)
        {
            wxString name;
            bool found = dir.GetFirst(&name, pattern, wxDIR_FILES);
            while (found) { ports.insert("/dev/" + name); found = dir.GetNext(&name); }
        }
        for (const wxString &port : ports) list.push_back(port);
#endif
    }
    else if (fill_type == FILL_ETH)
    {
#ifdef _WIN32
        ULONG size = 0;
        GetAdaptersAddresses(AF_INET, GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER, nullptr, nullptr, &size);
        std::vector<unsigned char> buffer(size);
        IP_ADAPTER_ADDRESSES *adapters = reinterpret_cast<IP_ADAPTER_ADDRESSES *>(buffer.data());
        std::set<wxString> subnets;
        if (GetAdaptersAddresses(AF_INET, GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER, nullptr, adapters, &size) == NO_ERROR)
            for (IP_ADAPTER_ADDRESSES *item = adapters; item; item = item->Next)
                if ((item->IfType == IF_TYPE_ETHERNET_CSMACD || item->IfType == IF_TYPE_IEEE80211) && item->OperStatus == IfOperStatusUp)
                    for (IP_ADAPTER_UNICAST_ADDRESS *addr = item->FirstUnicastAddress; addr; addr = addr->Next)
                        if (addr->Address.lpSockaddr && addr->Address.lpSockaddr->sa_family == AF_INET)
                        {
                            const unsigned char *ip = reinterpret_cast<const unsigned char *>(&reinterpret_cast<sockaddr_in *>(addr->Address.lpSockaddr)->sin_addr);
                            unsigned char mask[4] = {}; for (unsigned char bit = 0; bit < addr->OnLinkPrefixLength; ++bit) mask[bit / 8] |= 0x80 >> (bit % 8);
                            if (ip[0] != 127 && !(ip[0] == 169 && ip[1] == 254)) subnets.insert(subnet(ip, mask));
                        }
        for (const wxString &subnet : subnets) list.push_back(subnet);
#else
        ifaddrs *interfaces = nullptr;
        std::set<wxString> subnets;
        if (getifaddrs(&interfaces) == 0)
        {
            for (ifaddrs *item = interfaces; item; item = item->ifa_next)
                if (item->ifa_addr && item->ifa_netmask && item->ifa_addr->sa_family == AF_INET && (item->ifa_flags & IFF_UP) && !(item->ifa_flags & IFF_LOOPBACK))
                {
                    const unsigned char *ip = reinterpret_cast<const unsigned char *>(&reinterpret_cast<sockaddr_in *>(item->ifa_addr)->sin_addr);
                    const unsigned char *mask = reinterpret_cast<const unsigned char *>(&reinterpret_cast<sockaddr_in *>(item->ifa_netmask)->sin_addr);
                    if (!(ip[0] == 169 && ip[1] == 254)) subnets.insert(subnet(ip, mask));
                }
            freeifaddrs(interfaces);
        }
        for (const wxString &subnet : subnets) list.push_back(subnet);
#endif
    }
    setItems(list);
}

void Combo::enable(void) { cmb->Enable(); }
void Combo::disable(void) { cmb->Disable(); }
wxString Combo::getSelected(void) { return cmb->GetStringSelection(); }
void Combo::updateCb(wxTimerEvent &event) { (void)event; if (!opened) updateList(); }
void Combo::changeCb(wxCommandEvent &event) { if (callback) callback(event); }
void comboSetCallback(cb_t callb) { callback = callb; }
