#ifndef slic3r_FormaWebHost_hpp_
#define slic3r_FormaWebHost_hpp_

#include <memory>

#include <wx/panel.h>

namespace Slic3r {
namespace GUI {

class PrinterWebView;
class FormaWebBridge;

class FormaWebHost final : public wxPanel {
public:
    explicit FormaWebHost(wxWindow* parent);
    ~FormaWebHost() override;

private:
    PrinterWebView*                  m_web_view{nullptr};
    std::unique_ptr<FormaWebBridge>  m_web_bridge;
};

} // namespace GUI
} // namespace Slic3r

#endif // slic3r_FormaWebHost_hpp_
