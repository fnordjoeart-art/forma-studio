#ifndef slic3r_FormaWebBridge_hpp_
#define slic3r_FormaWebBridge_hpp_

#include <string>

#include <wx/string.h>
#include <wx/webview.h>

namespace Slic3r {
namespace GUI {

class FormaWebBridge final {
public:
    FormaWebBridge(wxWebView* web_view, const wxString& allowed_document_url);
    ~FormaWebBridge();

private:
    void OnScriptMessage(wxWebViewEvent& event);
    void OnNavigating(wxWebViewEvent& event);
    void OnLoaded(wxWebViewEvent& event);

    void ProcessPayload(const std::string& payload);
    void SendResponse(const std::string& response_json);
    void SendError(const char* code, const char* message);
    void InjectBridge();

    bool IsAllowedDocumentUrl(const wxString& url) const;
    bool IsShowingAllowedDocument() const;
    static wxString NormalizeDocumentUrl(const wxString& url);

private:
    wxWebView* m_web_view{nullptr};
    wxString   m_allowed_document_url;
};

} // namespace GUI
} // namespace Slic3r

#endif // slic3r_FormaWebBridge_hpp_
