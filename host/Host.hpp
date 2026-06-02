#pragma once

#include <cef_app.h>

#include "BrowserView.hpp"

class Host
{
  public:
    bool Initialize();

    void Run();

  private:
    HANDLE mapping_{};

    SharedFrame* frame_{};

    CefRefPtr<BrowserView> browserView_;
};