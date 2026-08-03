#include "FormaWebBridge.hpp"

#include "slic3r/GUI/Widgets/WebView.hpp"

#include <cstdint>
#include <string>

#include <nlohmann/json.hpp>
#include <wx/uri.h>

namespace Slic3r {
namespace GUI {

namespace {

using json = nlohmann::json;

constexpr const char* PROTOCOL_VERSION = "1.0";
constexpr std::size_t MAX_PAYLOAD_SIZE = 64 * 1024;
constexpr std::size_t MAX_ENCODED_PAYLOAD_SIZE = MAX_PAYLOAD_SIZE * 3;
constexpr std::uint64_t MAX_JAVASCRIPT_SAFE_INTEGER = 9007199254740991ULL;
const wxString FALLBACK_PREFIX = "app://forma-bridge/";

bool is_safe_request_id(const json& value)
{
    if (value.is_number_unsigned()) {
        const auto id = value.get<std::uint64_t>();
        return id > 0 && id <= MAX_JAVASCRIPT_SAFE_INTEGER;
    }

    if (value.is_number_integer()) {
        const auto id = value.get<std::int64_t>();
        return id > 0 && static_cast<std::uint64_t>(id) <= MAX_JAVASCRIPT_SAFE_INTEGER;
    }

    return false;
}

json make_error_response(const json& id, const char* code, const char* message)
{
    return {
        {"version", PROTOCOL_VERSION},
        {"id", id},
        {"ok", false},
        {"error", {
            {"code", code},
            {"message", message}
        }}
    };
}

} // namespace

FormaWebBridge::FormaWebBridge(wxWebView* web_view, const wxString& allowed_document_url)
    : m_web_view(web_view)
    , m_allowed_document_url(NormalizeDocumentUrl(allowed_document_url))
{
    if (!m_web_view)
        return;

    m_web_view->Bind(wxEVT_WEBVIEW_SCRIPT_MESSAGE_RECEIVED, &FormaWebBridge::OnScriptMessage, this);
    m_web_view->Bind(wxEVT_WEBVIEW_NAVIGATING, &FormaWebBridge::OnNavigating, this);
    m_web_view->Bind(wxEVT_WEBVIEW_LOADED, &FormaWebBridge::OnLoaded, this);
}

FormaWebBridge::~FormaWebBridge()
{
    if (!m_web_view)
        return;

    m_web_view->Unbind(wxEVT_WEBVIEW_SCRIPT_MESSAGE_RECEIVED, &FormaWebBridge::OnScriptMessage, this);
    m_web_view->Unbind(wxEVT_WEBVIEW_NAVIGATING, &FormaWebBridge::OnNavigating, this);
    m_web_view->Unbind(wxEVT_WEBVIEW_LOADED, &FormaWebBridge::OnLoaded, this);
}

void FormaWebBridge::OnScriptMessage(wxWebViewEvent& event)
{
    if (!IsShowingAllowedDocument() || event.GetMessageHandler() != "wx")
        return;

    const wxScopedCharBuffer buffer = event.GetString().ToUTF8();
    const std::string payload(buffer.data(), buffer.length());
    if (payload.size() > MAX_PAYLOAD_SIZE) {
        SendError("INVALID_REQUEST", "Payload superiore al limite di 64 KiB");
        return;
    }

    ProcessPayload(payload);
}

void FormaWebBridge::OnNavigating(wxWebViewEvent& event)
{
    const wxString url = event.GetURL();

    if (url.StartsWith(FALLBACK_PREFIX)) {
        event.Veto();

        if (!IsShowingAllowedDocument())
            return;

        const wxString encoded_payload = url.Mid(FALLBACK_PREFIX.length());
        if (encoded_payload.length() > MAX_ENCODED_PAYLOAD_SIZE) {
            SendError("INVALID_REQUEST", "Payload superiore al limite di 64 KiB");
            return;
        }

        const wxScopedCharBuffer buffer = wxURI::Unescape(encoded_payload).ToUTF8();
        const std::string payload(buffer.data(), buffer.length());
        if (payload.size() > MAX_PAYLOAD_SIZE) {
            SendError("INVALID_REQUEST", "Payload superiore al limite di 64 KiB");
            return;
        }

        ProcessPayload(payload);
        return;
    }

    if (!IsAllowedDocumentUrl(url)) {
        event.Veto();
        return;
    }

    event.Skip();
}

void FormaWebBridge::OnLoaded(wxWebViewEvent& event)
{
    if (IsAllowedDocumentUrl(event.GetURL()))
        InjectBridge();

    event.Skip();
}

void FormaWebBridge::ProcessPayload(const std::string& payload)
{
    json response_id = nullptr;

    try {
        const json request = json::parse(payload, nullptr, false);
        if (request.is_discarded()) {
            SendResponse(make_error_response(nullptr, "INVALID_JSON", "JSON non valido").dump());
            return;
        }

        if (request.is_object() && request.contains("id") && is_safe_request_id(request["id"]))
            response_id = request["id"];

        if (!request.is_object() ||
            !request.contains("version") || !request["version"].is_string() ||
            !request.contains("id") || !is_safe_request_id(request["id"]) ||
            !request.contains("method") || !request["method"].is_string() || request["method"].get_ref<const std::string&>().empty() ||
            !request.contains("params") || !request["params"].is_object()) {
            SendResponse(make_error_response(response_id, "INVALID_REQUEST", "Richiesta non valida").dump());
            return;
        }

        if (request["version"].get_ref<const std::string&>() != PROTOCOL_VERSION) {
            SendResponse(make_error_response(response_id, "UNSUPPORTED_VERSION", "Versione non supportata").dump());
            return;
        }

        if (request["method"].get_ref<const std::string&>() != "get_engine_status") {
            SendResponse(make_error_response(response_id, "METHOD_NOT_FOUND", "Metodo non supportato").dump());
            return;
        }

        const json response = {
            {"version", PROTOCOL_VERSION},
            {"id", response_id},
            {"ok", true},
            {"result", {
                {"ready", true},
                {"mode", "native"},
                {"label", "Bridge FORMA collegato"}
            }}
        };
        SendResponse(response.dump());
    } catch (...) {
        SendResponse(make_error_response(response_id, "INTERNAL_ERROR", "Errore interno del bridge").dump());
    }
}

void FormaWebBridge::SendResponse(const std::string& response_json)
{
    if (!IsShowingAllowedDocument())
        return;

    const std::string response_literal = json(response_json).dump(-1, ' ', true);
    const std::string script =
        "(function(){"
        "const response=JSON.parse(" + response_literal + ");"
        "document.dispatchEvent(new CustomEvent(\"forma:native-response\",{detail:response}));"
        "window.dispatchEvent(new CustomEvent(\"forma:native-response\",{detail:response}));"
        "})();";

    WebView::RunScript(m_web_view, wxString::FromUTF8(script));
}

void FormaWebBridge::SendError(const char* code, const char* message)
{
    SendResponse(make_error_response(nullptr, code, message).dump());
}

void FormaWebBridge::InjectBridge()
{
    if (!IsShowingAllowedDocument())
        return;

    static const wxString script = R"JS(
        window.__FORMA_NATIVE__ = Object.freeze({ version: "1.0" });
        document.dispatchEvent(
            new CustomEvent("forma:native-ready", {
                detail: window.__FORMA_NATIVE__
            })
        );
        window.dispatchEvent(
            new CustomEvent("forma:native-ready", {
                detail: window.__FORMA_NATIVE__
            })
        );
    )JS";

    WebView::RunScript(m_web_view, script);
}

bool FormaWebBridge::IsAllowedDocumentUrl(const wxString& url) const
{
    return NormalizeDocumentUrl(url) == m_allowed_document_url;
}

bool FormaWebBridge::IsShowingAllowedDocument() const
{
    return m_web_view && IsAllowedDocumentUrl(m_web_view->GetCurrentURL());
}

wxString FormaWebBridge::NormalizeDocumentUrl(const wxString& url)
{
    wxString normalized = url.BeforeFirst('#');
    normalized = wxURI::Unescape(normalized);
    normalized.Replace("\\", "/");

    if (normalized.Left(7).CmpNoCase("file://") == 0)
        normalized = "file://" + normalized.Mid(7);

#ifdef __WXMSW__
    normalized.MakeLower();
#endif

    return normalized;
}

} // namespace GUI
} // namespace Slic3r
