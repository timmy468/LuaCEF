#pragma once

#include <atomic>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
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

    RemoteWebView* Create(const std::string& url, int x, int y, int w, int h, RemoteWebView::FrameCallback onCreate);
    void Close(RemoteWebView* view);
    bool IsAnyInputActive() const;

    void Update();

    bool OnWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    void OnDeviceLost();
    void OnDeviceReset();

  private:
    IDirect3DDevice9* device_{};

    std::vector<RemoteWebView*> webViews_;

    std::map<RemoteWebView::BrowserId, RemoteWebView*> webviewIdMap_; // Маппинг ID из subprocess к локальным объектам

    std::shared_ptr<ipc::IPCClient> ipcClient_;
    std::unique_ptr<ProcessManager> processManager_;
    std::atomic_uint32_t nextBrowserId_{1};

    bool messageThreadRunning_{false};
    struct
    {
        std::mutex mutex;
        std::vector<std::function<void()>> tasks;
    } gameThreadTasks_;

    struct
    {
        std::mutex mutex;
        std::vector<std::function<void()>> tasks;
    } messageThreadTasks_;

  private:
    void ProcessInput();

    void UpdateTextures();
    void Render();

    void ProcessGameThreadTasks();
    void AddTaskToGameThread(std::function<void()> task);

    void ProcessMessageThreadTasks();
    void AddTaskToMessageThread(std::function<void()> task);

    void StartMessageReceiveThread();
    void MessageReceiveThreadProc();
};