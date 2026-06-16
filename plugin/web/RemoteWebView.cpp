#include "RemoteWebView.hpp"

#include <cstdint>
#include <cstring>
#include <format>
#include <iostream>

#include "utils/samp_utils.hpp"

#include "Logger.hpp"

RemoteWebView::RemoteWebView(std::shared_ptr<ipc::IPCClient> ipcClient, std::shared_ptr<IRenderManager> renderManager,
                             BrowserId id, IDirect3DDevice9* device, int x, int y, int width, int height,
                             FrameCallback onCreate)
    : ipcClient_(ipcClient), renderManager_(renderManager), browserId_(id), device_(device), width_(width),
      height_(height), x_(x), y_(y)
{
    onAfterCreateCallbacks_.push_back(onCreate);

    CreateTexture(width, height);
}

RemoteWebView::~RemoteWebView()
{
    sharedMemory_.Close();
    frame_ = nullptr;

    DeleteTexture();
}

void RemoteWebView::AttachBrowser(DWORD parentPid)
{
    if (closing_)
        return;

    if (sharedMemory_.Open(std::format("LuaCEF_SharedMemory_{}_{}", parentPid, static_cast<uint32_t>(browserId_)),
                           width_, height_))
    {
        frame_ = static_cast<SharedFrame*>(sharedMemory_.GetBuffer());
        Logger::Instance().Log("SharedFrame: {:p} {} {}", (void*)frame_, frame_->header.width.load(),
                               frame_->header.height.load());
    }
    else
    {
        Logger::Instance().Log("Failed to open shared memory: {}", (void*)sharedMemory_.GetBuffer());
    }
}

void RemoteWebView::OnAfterCreate()
{
    if (closing_)
        return;

    for (auto& callback : onAfterCreateCallbacks_)
        callback(this);
}

void RemoteWebView::OnLoading(int httpStatusCode)
{
    if (closing_)
        return;

    for (auto& callback : onLoadingCallbacks_)
        callback(this, httpStatusCode);
}

void RemoteWebView::OnClose()
{
    for (auto& callback : onCloseCallbacks_)
        callback(this);
}

void RemoteWebView::OnURLChanged(const std::string& url)
{
    url_ = url;
}

void RemoteWebView::OnBindFunctionCall(const std::string& function, const std::string& args)
{
    if (closing_)
        return;

    Logger::Instance().Log("RemoteWebView::OnBindFunctionCall() {} {}", function, args);

    auto it = bindFunctions_.find(function);
    if (it != bindFunctions_.end())
    {
        it->second(this, args);
    }
}

void RemoteWebView::Close()
{
    closing_ = true;

    SetInputEnabled(false);
    SetOffscreenRenderingEnabled(false);
    SetRenderEnabled(false);
    SetFocused(false);
}

void RemoteWebView::OnDeviceLost()
{
    DeleteTexture();
}

void RemoteWebView::OnDeviceReset()
{
    if (!texture_)
    {
        CreateTexture(width_, height_);
        RestoreTextureFromSharedMemory();
    }
}

void RemoteWebView::Update()
{
    if (closing_ || !frame_)
        return;

    const uint32_t beginSeq = frame_->header.sequence.load(std::memory_order_acquire);

    if (beginSeq & 1)
        return;

    if (beginSeq == lastSequence_)
        return;

    if (!UploadTexture(beginSeq))
        return;

    const uint32_t endSeq = frame_->header.sequence.load(std::memory_order_acquire);

    if (beginSeq != endSeq)
        return;

    hasFrame_ = true;
    lastSequence_ = beginSeq;
}

void RemoteWebView::Render()
{
    if (!hasFrame_)
        return;

    if (closing_)
        return;

    if (offscreenRenderingEnabled_ || !renderEnabled_)
        return;

    if (texture_)
    {
        renderManager_->RenderTexture(texture_, x_, y_, width_, height_);
    }
}

bool RemoteWebView::UploadTexture(uint32_t expectedSeq)
{
    if (closing_ || !texture_ || !frame_)
        return false;

    const uint32_t width = frame_->header.width.load(std::memory_order_relaxed);

    const uint32_t height = frame_->header.height.load(std::memory_order_relaxed);

    if (width == 0 || height == 0 || width > width_ || height > height_)
    {
        return false;
    }

    if (width != static_cast<uint32_t>(width_) || height != static_cast<uint32_t>(height_))
    {
        DeleteTexture();

        width_ = width;
        height_ = height;

        CreateTexture(width_, height_);

        if (!texture_)
            return false;
    }

    const uint32_t dirtyCount =
        std::min(frame_->header.dirtyRectCount.load(std::memory_order_relaxed), MAX_DIRTY_RECTS);

    if (dirtyCount == 0)
        return true;

    const uint8_t* src = frame_->GetPixelBuffer();

    for (uint32_t i = 0; i < dirtyCount; ++i)
    {
        if (frame_->header.sequence.load(std::memory_order_acquire) != expectedSeq)
            return false;

        const auto& dirty = frame_->dirtyRects[i];

        if (dirty.x < 0 || dirty.y < 0 || dirty.width <= 0 || dirty.height <= 0)
        {
            continue;
        }

        if (dirty.x >= static_cast<int>(width) || dirty.y >= static_cast<int>(height))
        {
            continue;
        }

        if (dirty.width > static_cast<int>(width) - dirty.x || dirty.height > static_cast<int>(height) - dirty.y)
        {
            continue;
        }

        RECT rect{dirty.x, dirty.y, dirty.x + dirty.width, dirty.y + dirty.height};

        D3DLOCKED_RECT lockedRect{};

        if (FAILED(texture_->LockRect(0, &lockedRect, &rect, 0)))
            continue;

        auto* dst = static_cast<uint8_t*>(lockedRect.pBits);

        for (int y = 0; y < dirty.height; ++y)
        {
            memcpy(dst + y * lockedRect.Pitch, src + ((dirty.y + y) * width + dirty.x) * 4, dirty.width * 4);
        }

        texture_->UnlockRect(0);
    }

    return true;
}

bool RemoteWebView::RestoreTextureFromSharedMemory()
{
    if (!texture_ || !frame_)
        return false;

    uint32_t beginSeq;
    uint32_t endSeq;

    do
    {
        beginSeq = frame_->header.sequence.load(std::memory_order_acquire);

        if (beginSeq & 1)
            return false;

        const uint32_t width = frame_->header.width.load(std::memory_order_relaxed);

        const uint32_t height = frame_->header.height.load(std::memory_order_relaxed);

        if (width == 0 || height == 0 || width > frame_->header.width || height > frame_->header.height)
        {
            return false;
        }

        D3DLOCKED_RECT rect{};

        if (FAILED(texture_->LockRect(0, &rect, nullptr, 0)))
            return false;

        const uint8_t* src = frame_->GetPixelBuffer();
        auto* dst = static_cast<uint8_t*>(rect.pBits);

        for (uint32_t y = 0; y < height; ++y)
        {
            memcpy(dst + y * rect.Pitch, src + y * width * 4, width * 4);
        }

        texture_->UnlockRect(0);

        endSeq = frame_->header.sequence.load(std::memory_order_acquire);

    } while (beginSeq != endSeq);

    return true;
}

bool RemoteWebView::InputEnabled() const
{
    return inputEnabled_;
}

void RemoteWebView::SetInputEnabled(bool enabled)
{
    inputEnabled_ = enabled;
    if (!inputEnabled_)
        samp_utils::ToggleCursor(false);
}

void RemoteWebView::CreateTexture(uint32_t width, uint32_t height)
{
    auto hr = device_->CreateTexture(width, height, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &texture_,
                                     nullptr);

    if (FAILED(hr))
    {
        Logger::Instance().Log("CreateTexture failed {:08X}", static_cast<uint32_t>(hr));
    }
}

void RemoteWebView::DeleteTexture()
{
    if (texture_)
    {
        texture_->Release();
        texture_ = nullptr;
    }
}