#pragma once

#include <memory>

#include <cef_app.h>

#include "BrowserView.hpp"
#include "IPCBrowserHandler.hpp"

class Host
{
  public:
    bool Initialize();
    void Run();

  private:
    std::shared_ptr<IPCBrowserHandler> ipcHandler_;
    std::shared_ptr<ipc::IPCServer> ipcServer_;

  private:
    void InitCef();
};