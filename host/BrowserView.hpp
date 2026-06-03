#pragma once

#include <functional>

#include <cef_client.h>

#include "SharedFrame.hpp"
#include "SharedMemory.hpp"

class BrowserView : public CefClient, public CefRenderHandler, public CefLifeSpanHandler
{
  public:
    using ViewCallback = std::function<void(BrowserView*)>;

  public:
    BrowserView(uint32_t id, int32_t width, int32_t height);

    // CefClient
    CefRefPtr<CefRenderHandler> GetRenderHandler() override
    {
        return this;
    }

    CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override
    {
        return this;
    }

    // CefRenderHandler
    void OnPaint(CefRefPtr<CefBrowser>, PaintElementType type, const RectList& dirtyRects, const void* buffer,
                 int width, int height) override;
    void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) override;

    // CefLifeSpanHandler
    void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
    void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

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

  private:
    CefRefPtr<CefBrowser> browser_;

    uint32_t webviewId;

    int width_;
    int height_;

    SharedFrame* frame_;
    ipc::SharedMemory sharedMemory_;

    std::vector<ViewCallback> onCreateCallbacks_;

    IMPLEMENT_REFCOUNTING(BrowserView);
};