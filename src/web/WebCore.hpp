#pragma once

#include <vector>

#include <d3d9.h>

#include "WebView.hpp"

class WebCore
{
  public:
    void Initialize(IDirect3DDevice9* device);

  private:
    std::vector<std::unique_ptr<WebView>> webViews_;
};