#ifndef slic3r_FormaWebHost_hpp_
#define slic3r_FormaWebHost_hpp_

#include <wx/panel.h>

namespace Slic3r {
namespace GUI {

class PrinterWebView;

class FormaWebHost final : public wxPanel {
public:
    explicit FormaWebHost(wxWindow* parent);
    ~FormaWebHost() override = default;

private:
    PrinterWebView* m_web_view{nullptr};
};

} // namespace GUI
} // namespace Slic3r

#endif // slic3r_FormaWebHost_hpp_
