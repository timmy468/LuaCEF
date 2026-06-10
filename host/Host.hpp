#pragma once

#include <atomic>
#include <memory>

#include <cef_app.h>

#include "BrowserView.hpp"
#include "IPCBrowserHandler.hpp"

class Host
{
  public:
    bool Initialize(DWORD pid);
    void Run();

  private:
    DWORD parentPid_{};

    std::thread parentMonitorThread_;
    std::atomic_bool running_{true};

    std::shared_ptr<IPCBrowserHandler> ipcHandler_;
    std::shared_ptr<ipc::IPCServer> ipcServer_;

  private:
    void InitCef();

    void StartParentMonitor();
    void ParentMonitorProc();
};