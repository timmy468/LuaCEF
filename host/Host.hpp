#pragma once

#include <atomic>
#include <memory>

#include <cef_app.h>

#include "BrowserView.hpp"
#include "IPCBrowserHandler.hpp"

class Host
{
  public:
    Host() = default;
    ~Host();

    bool Initialize(DWORD pid);
    void Run();

  private:
    DWORD parentPid_{};

    struct
    {
        std::thread thread;
        std::atomic_bool running{true};
    } parentMonitorThread_;

    struct
    {
        std::thread thread;
        std::atomic_bool running{true};
    } serverThread_;

    std::shared_ptr<IPCBrowserHandler> ipcHandler_;
    std::shared_ptr<ipc::IPCServer> ipcServer_;

  private:
    void InitCef();

    void StartParentMonitor();
    void ParentMonitorProc();
};