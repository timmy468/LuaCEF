#pragma once

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

    using FrameCallback = void(__stdcall*)(RemoteWebView*);

    using LoadingCallback = void(__stdcall*)(RemoteWebView*, int);

    using JsonCallback = void(__stdcall*)(RemoteWebView*, const char*, const char**, int);

  public:
    RemoteWebView(std::shared_ptr<ipc::IPCClient> ipcClient, BrowserId id, IDirect3DDevice9* device, int x, int y,
                  int width, int height, FrameCallback onCreate = nullptr);
    ~RemoteWebView();

    // Export
    void Close();
    void AttachBrowser();

    void OnDeviceLost();
    void OnDeviceReset();

    void Update();

    bool InputEnabled() const;
    void SetInputEnabled(bool enabled);

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

    FrameCallback onCreate_{};
    FrameCallback onClose_{};
    LoadingCallback onLoading_{};

    ipc::SharedMemory sharedMemory_;
    SharedFrame* frame_{};
    uint32_t lastSequence_{};

    std::shared_ptr<ipc::IPCClient> ipcClient_;

  private:
    void CreateTexture(uint32_t width, uint32_t height);
    void DeleteTexture();
    void UploadTexture();
};