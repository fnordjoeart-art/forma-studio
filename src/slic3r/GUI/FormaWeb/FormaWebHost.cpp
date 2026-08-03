#include "FormaWebHost.hpp"

#include "slic3r/GUI/GUI.hpp"
#include "slic3r/GUI/PrinterWebView.hpp"
#include "libslic3r/Utils.hpp"

#include <wx/sizer.h>

namespace Slic3r {
namespace GUI {

FormaWebHost::FormaWebHost(wxWindow* parent)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize)
{
    SetMinSize(wxSize(FromDIP(320), FromDIP(260)));

    m_web_view = new PrinterWebView(this);
    auto* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(m_web_view, 1, wxEXPAND);
    SetSizer(sizer);

    const wxString url = wxString::Format("file://%s/web/forma/dist/index.html", from_u8(resources_dir()));
    m_web_view->load_url(url);
}

} // namespace GUI
} // namespace Slic3r
