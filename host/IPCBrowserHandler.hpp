#pragma once

#include <functional>
#include <map>
#include <mutex>
#include <vector>

#include <cef_app.h>

#include "BrowserView.hpp"
#include "IPCServer.hpp"

// Обработчик IPC команд
class IPCBrowserHandler : public ipc::IPCMessageHandler
{
  public:
    explicit IPCBrowserHandler(DWORD parentPid);
    ~IPCBrowserHandler() override;

    void HandleMessage(HANDLE pipe, ipc::CommandType type, const void* payload, uint32_t payloadSize) override;

    void SetIPCServer(std::shared_ptr<ipc::IPCServer> ipcServer)
    {
        ipcServer_ = ipcServer;
    }

    void RegisterBrowser(uint32_t id, CefRefPtr<BrowserView> handler);
    void UnregisterBrowser(uint32_t id);

  private:
    void SendResizeResponse(HANDLE pipe, uint32_t id, int width, int height);

    std::map<uint32_t, CefRefPtr<BrowserView>> browsers_;

    std::shared_ptr<ipc::IPCServer> ipcServer_;

    DWORD parentPid_;
};