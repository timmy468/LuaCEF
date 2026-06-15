#pragma once

#include <functional>

#include <cef_client.h>

#include "SharedFrame.hpp"
#include "SharedMemory.hpp"

class BrowserView : public CefClient,
                    public CefRenderHandler,
                    public CefLifeSpanHandler,
                    public CefLoadHandler,
                    public CefDisplayHandler
{
  public:
    using ViewCallback = std::function<void(BrowserView*)>;
    using LoadingCallback = std::function<void(BrowserView*, int)>;
    using AddressChangeCallback = std::function<void(BrowserView*, const CefString&)>;
    using LuaCallback = std::function<void(BrowserView*, const CefString&, const CefString&)>;

  public:
    BrowserView(DWORD parentPid, uint32_t id, int x, int y, int width, int height);

    // CefClient
    CefRefPtr<CefRenderHandler> GetRenderHandler() override
    {
        return this;
    }

    CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override
    {
        return this;
    }

    CefRefPtr<CefLoadHandler> GetLoadHandler() override
    {
        return this;
    }

    CefRefPtr<CefDisplayHandler> GetDisplayHandler() override
    {
        return this;
    }

    virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                          CefProcessId source_process, CefRefPtr<CefProcessMessage> message) override;

    // CefRenderHandler
    void OnPaint(CefRefPtr<CefBrowser>, PaintElementType type, const RectList& dirtyRects, const void* buffer,
                 int width, int height) override;
    void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) override;

    // CefLifeSpanHandler
    bool OnBeforePopup(CefRefPtr<CefBrowser>, CefRefPtr<CefFrame>, const CefString&, const CefString&,
                       WindowOpenDisposition, bool, const CefPopupFeatures&, CefWindowInfo&, CefRefPtr<CefClient>&,
                       CefBrowserSettings&, CefRefPtr<CefDictionaryValue>&, bool*) override
    {
        return true; // блокируем popup
    }
    void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
    void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

    // CefLoadHandler
    void OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode) override;
    void OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, ErrorCode errorCode,
                     const CefString& errorText, const CefString& failedUrl) override;

    // CefDisplayHandler
    void OnAddressChange(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& url) override;
    bool OnConsoleMessage(CefRefPtr<CefBrowser>, cef_log_severity_t, const CefString& message, const CefString& source,
                          int line) override;

    // Export
    CefRefPtr<CefBrowser> GetBrowser() const
    {
        return browser_;
    }

    uint32_t GetWebviewId() const
    {
        return webviewId;
    }

    void AddOnCreateCallback(ViewCallback callback)
    {
        onCreateCallbacks_.push_back(callback);
    }

    void AddOnCloseCallback(ViewCallback callback)
    {
        onCloseCallbacks_.push_back(callback);
    }

    void AddOnLoadingCallback(LoadingCallback callback)
    {
        onLoadingCallbacks_.push_back(callback);
    }

    void AddOnAddressChangeCallback(AddressChangeCallback callback)
    {
        onAddressChangeCallbacks_.push_back(callback);
    }

    void AddOnBindFunctionCallback(LuaCallback callback)
    {
        onBindFunctionCallbacks_.push_back(callback);
    }

    void SetRect(int x, int y, int width, int height)
    {
        x_ = x;
        y_ = y;
        width_ = width;
        height_ = height;
    }

  private:
    CefRefPtr<CefBrowser> browser_;

    uint32_t webviewId;

    int x_;
    int y_;
    int width_;
    int height_;

    SharedFrame* frame_;
    ipc::SharedMemory sharedMemory_;

    std::vector<ViewCallback> onCreateCallbacks_;
    std::vector<ViewCallback> onCloseCallbacks_;
    std::vector<LoadingCallback> onLoadingCallbacks_;
    std::vector<AddressChangeCallback> onAddressChangeCallbacks_;
    std::vector<LuaCallback> onBindFunctionCallbacks_;

    IMPLEMENT_REFCOUNTING(BrowserView);
};