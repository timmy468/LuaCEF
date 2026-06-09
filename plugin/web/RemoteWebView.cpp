#include "RemoteWebView.hpp"

#include <cstdint>
#include <cstring>
#include <format>
#include <iostream>

#include "utils/samp_utils.hpp"

#include "Logger.hpp"

RemoteWebView::RemoteWebView(std::shared_ptr<ipc::IPCClient> ipcClient, BrowserId id, IDirect3DDevice9* device, int x,
                             int y, int width, int height, FrameCallback onCreate)
    : ipcClient_(ipcClient), browserId_(id), device_(device), width_(width), height_(height), x_(x), y_(y)
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

void RemoteWebView::AttachBrowser()
{
    if (closing_)
        return;

    if (sharedMemory_.Open(std::format("LuaCEF_SharedMemory_{}", static_cast<uint32_t>(browserId_))))
    {
        frame_ = static_cast<SharedFrame*>(sharedMemory_.GetBuffer());
        Logger::Instance().Log("SharedFrame: {:p} {} {}", (void*)frame_, frame_->header.width, frame_->header.height);
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
        UploadTexture();
    }
}

void RemoteWebView::Update()
{
    if (closing_)
        return;

    if (!frame_)
        return;

    if (frame_->header.sequence == lastSequence_)
        return;

    lastSequence_ = frame_->header.sequence;

    UploadTexture();
}

void RemoteWebView::UploadTexture()
{
    if (closing_)
        return;

    D3DLOCKED_RECT rect{};

    if (FAILED(texture_->LockRect(0, &rect, nullptr, D3DLOCK_DISCARD)))
    {
        return;
    }

    const uint32_t width = frame_->header.width;
    const uint32_t height = frame_->header.height;

    const uint8_t* src = frame_->pixels;

    uint8_t* dst = static_cast<uint8_t*>(rect.pBits);

    for (uint32_t y = 0; y < height; ++y)
    {
        memcpy(dst + y * rect.Pitch, src + y * width * 4, width * 4);
    }

    texture_->UnlockRect(0);
}

bool RemoteWebView::InputEnabled() const
{
    return inputEnabled_;
}

void RemoteWebView::SetInputEnabled(bool enabled)
{
    if (closing_)
        return;

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