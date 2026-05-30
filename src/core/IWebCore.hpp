#pragma once

#include <d3d9.h>

#include "IWebView.hpp"

class IWebCore
{
  public:
    virtual ~IWebCore() = default;

    virtual void Initialize(IDirect3DDevice9* device) = 0;
    [[nodiscard]] virtual bool IsInitialized() const = 0;

    [[nodiscard]] virtual std::shared_ptr<IWebView> CreateWebView(const std::string& url, int x, int y, int width,
                                                                  int height) = 0;
    [[nodiscard]] virtual std::shared_ptr<IWebView> CreateFullscreenWebView(const std::string& url) = 0;
    virtual void CloseWebView(std::shared_ptr<IWebView> webView) = 0;
};