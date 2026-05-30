#pragma once

#include <d3d9.h>

class WebView : public CefClient, public CefRenderHandler
{
  public:
    WebView(IDirect3DDevice9* device, int width, int height);

    // CefClient
    CefRefPtr<CefRenderHandler> GetRenderHandler() override
    {
        return this;
    }

    // CefRenderHandler
    bool GetViewRect(CefRefPtr<CefBrowser>, CefRect& rect) override;

    void OnPaint(CefRefPtr<CefBrowser>, PaintElementType type, const RectList& dirtyRects, const void* buffer,
                 int width, int height) override;

    IDirect3DTexture9* GetTexture() const
    {
        return texture_;
    }

  private:
    int width_;
    int height_;

    IDirect3DDevice9* device_{};
    IDirect3DTexture9* texture_{};

    IMPLEMENT_REFCOUNTING(Webview);
};