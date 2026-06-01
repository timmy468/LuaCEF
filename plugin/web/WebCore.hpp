#pragma once

#include <memory>
#include <string>
#include <vector>

#include <d3d9.h>

#include "WebView.hpp"

class WebCore
{
  public:
    WebCore() = default;
    ~WebCore();
    void Initialize(IDirect3DDevice9* device);
    void Shutdown();
    bool IsInitialized() const
    {
        return device_ != nullptr;
    }

    WebView* Create(const std::string& url, int x, int y, int w, int h);
    void Close(WebView* view);
    void UpdateTextures();

    bool OnWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    void OnDeviceLost();
    void OnDeviceReset();

  private:
    IDirect3DDevice9* device_{};
    std::vector<WebView*> webViews_;
};