#pragma once

#include <cef_client.h>

#include "SharedFrame.hpp"

class BrowserView : public CefClient, public CefRenderHandler
{
  public:
    BrowserView(SharedFrame* frame, int width, int height);

    // CefClient
    CefRefPtr<CefRenderHandler> GetRenderHandler() override
    {
        return this;
    }

    // CefRenderHandler
    void OnPaint(CefRefPtr<CefBrowser>, PaintElementType type, const RectList& dirtyRects, const void* buffer,
                 int width, int height) override;
    void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) override;

  private:
    SharedFrame* frame_;

    int width_;
    int height_;

    CefRefPtr<CefBrowser> browser_;

    IMPLEMENT_REFCOUNTING(BrowserView);
};