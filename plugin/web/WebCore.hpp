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

#include "render/IRenderManager.hpp"

class WebCore
{
  public:
    WebCore() = default;
    ~WebCore();

    void Initialize(IDirect3DDevice9* device, std::shared_ptr<IRenderManager> renderManager);
    void Shutdown();

    bool IsInitialized() const
    {
        return device_ != nullptr;
    }

    RemoteWebView* Create(const std::string& url, int x, int y, int w, int h, RemoteWebView::FrameCallback onCreate);
    void Close(RemoteWebView* view);
    bool IsAnyInputActive() const;

    void LoadURL(RemoteWebView* view, const std::string& url);
    void ReloadPage(RemoteWebView* view, bool ignoreCache);
    void GoBack(RemoteWebView* view);
    void GoForward(RemoteWebView* view);
    void SetFPS(RemoteWebView* view, int fps);

    void ExecuteJS(RemoteWebView* view, const std::string& code);

    void Update();

    bool OnWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    void OnDeviceLost();
    void OnDeviceReset();

  private:
    IDirect3DDevice9* device_{};

    std::vector<RemoteWebView*> webViews_;

    std::shared_ptr<ipc::IPCClient> ipcClient_;
    std::unique_ptr<ProcessManager> processManager_;
    std::shared_ptr<IRenderManager> renderManager_;
    std::atomic_uint32_t nextBrowserId_{1};

    bool messageThreadRunning_{false};
    std::thread messageThread_;

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

    bool focused_ = false;

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