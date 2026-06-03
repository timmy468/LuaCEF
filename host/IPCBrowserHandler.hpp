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
    explicit IPCBrowserHandler();
    ~IPCBrowserHandler() override;

    void HandleMessage(HANDLE pipe, ipc::CommandType type, const void* payload, uint32_t payloadSize) override;

    void RegisterBrowser(uint32_t id, CefRefPtr<BrowserView> handler);
    void UnregisterBrowser(uint32_t id);

    void AddTask(std::function<void()> task);

  private:
    std::map<uint32_t, CefRefPtr<BrowserView>> browsers_;

    std::vector<std::function<void()>> tasks_;
    std::mutex tasks_mutex_;
};