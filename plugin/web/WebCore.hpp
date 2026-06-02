#pragma once

#include <memory>
#include <string>
#include <vector>

#include <d3d9.h>

#include "RemoteWebView.hpp"

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

    void Update();
    void UpdateTextures();
    void Render();
    void OnDeviceLost();
    void OnDeviceReset();

  private:
    IDirect3DDevice9* device_{};
    std::vector<RemoteWebView*> webViews_;
};