#pragma once

#include <mutex>

#include <d3d9.h>

#include <cef_app.h>

class WebView : public CefClient,
                public CefRenderHandler,
                public CefLifeSpanHandler,
                public CefLoadHandler,
                public CefDisplayHandler
{
  public:
    using FrameCallback = void(__stdcall*)(WebView*);

    using LoadingCallback = void(__stdcall*)(WebView*, int);

    using JsonCallback = void(__stdcall*)(WebView*, const char*, const char**, int);

  public:
    WebView(IDirect3DDevice9* device, int x, int y, int width, int height, FrameCallback onCreate = nullptr);
    ~WebView();

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

    // CefRefPtr<CefDisplayHandler> GetDisplayHandler() override
    // {
    //     return this;
    // }

    // CefRenderHandler
    void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) override;

    void OnPaint(CefRefPtr<CefBrowser>, PaintElementType type, const RectList& dirtyRects, const void* buffer,
                 int width, int height) override;

    // CefLifeSpanHandler
    void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
    void OnBeforeClose(CefRefPtr<CefBrowser>) override;

    // CefLoadHandler
    void OnLoadEnd(CefRefPtr<CefBrowser>, CefRefPtr<CefFrame>, int httpStatusCode) override;

    // Export
    void Close();
    void OnDeviceLost();
    void OnDeviceReset();

    void UpdateTexture();

    bool InputEnabled() const;
    void SetInputEnabled(bool enabled);

    IDirect3DTexture9* GetTexture() const
    {
        return textureData_.texture;
    }

    CefRefPtr<CefBrowser> GetBrowser() const
    {
        return browser_;
    }

    const std::string& GetURL();
    void LoadURL(const std::string& url);
    void ExecuteJS(const std::string& code);

  private:
    struct TextureData
    {
        std::mutex bufferMutex;
        std::mutex textureMutex;

        std::vector<uint8_t> frontBuffer;
        std::vector<uint8_t> backBuffer;

        std::atomic_bool dirty{false};

        int width{};
        int height{};

        IDirect3DTexture9* texture{};
    } textureData_;

    CefRefPtr<CefBrowser> browser_;
    std::string pendingUrl_;
    std::string url_;

    int width_;
    int height_;

    bool inputEnabled_{false};

    IDirect3DDevice9* device_{};

    FrameCallback onCreate_{};
    FrameCallback onClose_{};
    LoadingCallback onLoading_{};

    IMPLEMENT_REFCOUNTING(WebView);
};