#pragma once

#include <mutex>
#include <vector>

#include <d3d9.h>

#include "SharedFrame.hpp"
#include "process/ProcessManager.hpp"

class RemoteWebView
{
  public:
    using FrameCallback = void(__stdcall*)(RemoteWebView*);

    using LoadingCallback = void(__stdcall*)(RemoteWebView*, int);

    using JsonCallback = void(__stdcall*)(RemoteWebView*, const char*, const char**, int);

  public:
    RemoteWebView(IDirect3DDevice9* device, int x, int y, int width, int height, FrameCallback onCreate = nullptr);
    ~RemoteWebView();

    // Export
    void Close();
    void OnDeviceLost();
    void OnDeviceReset();

    void Update();
    void UploadTexture();

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

    bool IsLoaded() const
    {
        return loaded_;
    }

  private:
    IDirect3DTexture9* texture_{};

    std::string pendingUrl_;
    std::string url_;

    int x_;
    int y_;
    int width_;
    int height_;

    bool inputEnabled_{false};
    bool offscreenRenderingEnabled_{false};
    std::atomic<bool> loaded_{false};

    IDirect3DDevice9* device_{};

    FrameCallback onCreate_{};
    FrameCallback onClose_{};
    LoadingCallback onLoading_{};

    SharedFrame* frame_{};
    HANDLE mapping_{};
    uint32_t lastSequence_{};

    std::shared_ptr<ProcessManager> processManager_;
};