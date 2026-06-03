#include "RemoteWebView.hpp"

#include <cstdint>
#include <cstring>
#include <format>
#include <iostream>

#include "utils/samp_utils.hpp"

RemoteWebView::RemoteWebView(BrowserId id, IDirect3DDevice9* device, int x, int y, int width, int height,
                             FrameCallback onCreate)
    : browserId_(id), device_(device), width_(width), height_(height), x_(x), y_(y), onCreate_(onCreate)
{
    CreateTexture(width, height);
}

RemoteWebView::~RemoteWebView()
{
    DeleteTexture();
}

void RemoteWebView::AttachBrowser()
{
    if (sharedMemory_.Open(std::format("LuaCEF_SharedMemory_{}", static_cast<uint32_t>(browserId_))))
    {
        frame_ = static_cast<SharedFrame*>(sharedMemory_.GetBuffer());
        std::println(std::cout, "SharedFrame: {:p} {} {}", (void*)frame_, frame_->header.width, frame_->header.height);
    }
    else
    {
        std::println(std::cout, "Failed to open shared memory: {}", (void*)sharedMemory_.GetBuffer());
    }
}

void RemoteWebView::Close()
{
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
    if (!frame_)
        return;

    if (frame_->header.sequence == lastSequence_)
        return;

    lastSequence_ = frame_->header.sequence;

    UploadTexture();
}

void RemoteWebView::UploadTexture()
{
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
    inputEnabled_ = enabled;
    if (inputEnabled_)
        samp_utils::ToggleCursor(true);
    else
        samp_utils::ToggleCursor(false);
}

void RemoteWebView::CreateTexture(uint32_t width, uint32_t height)
{
    auto hr = device_->CreateTexture(width, height, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &texture_,
                                     nullptr);

    if (FAILED(hr))
    {
        std::println(std::cerr, "CreateTexture failed {:08X}", static_cast<uint32_t>(hr));
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