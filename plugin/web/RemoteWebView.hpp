#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <vector>

#include <d3d9.h>

#include "SharedFrame.hpp"
#include "process/ProcessManager.hpp"

#include "IPCClient.hpp"
#include "SharedMemory.hpp"

class RemoteWebView
{
  public:
    using BrowserId = uint32_t;

    using FrameCallback = std::function<void(RemoteWebView*)>;
    using LoadingCallback = std::function<void(RemoteWebView*, int)>;
    using BindFunctionCallback = std::function<void(RemoteWebView*, const std::string&)>;

  public:
    RemoteWebView(std::shared_ptr<ipc::IPCClient> ipcClient, BrowserId id, IDirect3DDevice9* device, int x, int y,
                  int width, int height, FrameCallback onCreate = nullptr);
    ~RemoteWebView();

    // Export
    void OnAfterCreate();
    void OnLoading(int httpStatusCode);
    void OnClose();
    void OnURLChanged(const std::string& url);
    void OnBindFunctionCall(const std::string& function, const std::string& args);

    void AddOnAfterCreateCallback(FrameCallback callback)
    {
        onAfterCreateCallbacks_.push_back(callback);
    }

    void AddOnLoadingCallback(LoadingCallback callback)
    {
        onLoadingCallbacks_.push_back(callback);
    }

    void AddOnCloseCallback(FrameCallback callback)
    {
        onCloseCallbacks_.push_back(callback);
    }

    void AddBindFunction(const std::string& name, BindFunctionCallback callback)
    {
        bindFunctions_[name] = callback;
    }

    void Close();
    void AttachBrowser();

    void OnDeviceLost();
    void OnDeviceReset();

    void Update();

    bool InputEnabled() const;
    void SetInputEnabled(bool enabled);

    void SetFocused(bool focused)
    {
        focused_ = focused;
    }

    bool IsFocused() const
    {
        return focused_;
    }

    bool OffscreenRenderingEnabled() const
    {
        return offscreenRenderingEnabled_;
    }

    void SetOffscreenRenderingEnabled(bool enabled)
    {
        offscreenRenderingEnabled_ = enabled;
    }

    bool RenderEnabled() const
    {
        return renderEnabled_;
    }

    void SetRenderEnabled(bool enabled)
    {
        renderEnabled_ = enabled;
    }

    IDirect3DTexture9* GetTexture() const
    {
        return texture_;
    }

    int GetX() const
    {
        return x_;
    }
    int GetY() const
    {
        return y_;
    }

    int GetWidth() const
    {
        return width_;
    }

    int GetHeight() const
    {
        return height_;
    }

    BrowserId GetBrowserId() const
    {
        return browserId_;
    }

    void SetRect(int x, int y, int width, int height)
    {
        x_ = x;
        y_ = y;
        width_ = width;
        height_ = height;
    }

    void SetURL(const std::string& url)
    {
        url_ = url;
    }

    const std::string& GetURL() const
    {
        return url_;
    }

    bool IsClosing() const
    {
        return closing_;
    }

    void MarkAsClosing()
    {
        closing_ = true;
    }

  private:
    BrowserId browserId_;

    IDirect3DTexture9* texture_{};

    std::string url_;

    int x_;
    int y_;
    int width_;
    int height_;

    bool inputEnabled_{false};

    bool offscreenRenderingEnabled_{false};
    bool renderEnabled_{true};

    IDirect3DDevice9* device_{};

    std::vector<FrameCallback> onAfterCreateCallbacks_;
    std::vector<LoadingCallback> onLoadingCallbacks_;
    std::vector<FrameCallback> onCloseCallbacks_;

    std::unordered_map<std::string, BindFunctionCallback> bindFunctions_;

    ipc::SharedMemory sharedMemory_;
    SharedFrame* frame_{};
    uint32_t lastSequence_{};

    std::shared_ptr<ipc::IPCClient> ipcClient_;

    bool closing_{};
    bool focused_{};

  private:
    void CreateTexture(uint32_t width, uint32_t height);
    void DeleteTexture();
    void UploadTexture();
};