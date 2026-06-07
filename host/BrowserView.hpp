#pragma once

#include <functional>

#include <cef_client.h>

#include "SharedFrame.hpp"
#include "SharedMemory.hpp"

class BrowserView : public CefClient, public CefRenderHandler, public CefLifeSpanHandler, public CefLoadHandler
{
  public:
    using ViewCallback = std::function<void(BrowserView*)>;

  public:
    BrowserView(uint32_t id, int x, int y, int width, int height);

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

    // CefRenderHandler
    void OnPaint(CefRefPtr<CefBrowser>, PaintElementType type, const RectList& dirtyRects, const void* buffer,
                 int width, int height) override;
    void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) override;

    // CefLifeSpanHandler
    void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
    void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

    // CefLoadHandler
    void OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode) override;

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

    void AddOnLoadingCallback(ViewCallback callback)
    {
        onLoadingCallbacks_.push_back(callback);
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
    std::vector<ViewCallback> onLoadingCallbacks_;

    IMPLEMENT_REFCOUNTING(BrowserView);
};