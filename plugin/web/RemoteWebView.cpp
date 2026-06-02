#include "RemoteWebView.hpp"

#include <cstdint>
#include <cstring>
#include <format>
#include <iostream>

#include "utils/samp_utils.hpp"

RemoteWebView::RemoteWebView(IDirect3DDevice9* device, int x, int y, int width, int height, FrameCallback onCreate)
    : device_(device), width_(width), height_(height), x_(x), y_(y), onCreate_(onCreate)
{
    auto hr = device_->CreateTexture(width, height, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &texture_,
                                     nullptr);

    if (FAILED(hr))
    {
        std::println(std::cerr, "CreateTexture failed {:08X}", static_cast<uint32_t>(hr));
    }
    else
    {
        std::println(std::cout, "RemoteWebView::RemoteWebView() {:p} {:d} {:d} {:d} {:d} created texture {:p}",
                     (void*)device, x, y, width, height, (void*)texture_);
    }

    constexpr int id = 0; // TODO: generate unique ID for each view
    std::string name = std::format("LuaCEF_Frame_{}", id);

    processManager_ = std::make_shared<ProcessManager>();
    if (!processManager_->Start())
    {
        std::cerr << "Failed to start CEF subprocess" << std::endl;
        device_ = nullptr;
        return;
    }

    // wait startup
    for (int i = 0; i < 100; ++i)
    {
        mapping_ = OpenFileMappingA(FILE_MAP_READ, FALSE, name.c_str());

        if (mapping_)
            break;

        Sleep(50);
    }

    mapping_ = OpenFileMappingA(FILE_MAP_READ, FALSE, name.c_str());
    if (!mapping_)
    {
        std::cerr << "Failed to open CEF frame mapping" << std::endl;
        device_ = nullptr;
        return;
    }

    frame_ = static_cast<SharedFrame*>(MapViewOfFile(mapping_, FILE_MAP_READ, 0, 0, 0));
}

RemoteWebView::~RemoteWebView()
{
    if (texture_)
    {
        texture_->Release();
        texture_ = nullptr;
    }

    if (processManager_)
    {
        processManager_->Stop();
        processManager_ = nullptr;
    }

    // if (browser_)
    // {
    //     browser_->GetHost()->CloseBrowser(true);
    //     browser_ = nullptr;
    // }
}

void RemoteWebView::Close()
{
}

void RemoteWebView::OnDeviceLost()
{
    if (texture_)
    {
        texture_->Release();
        texture_ = nullptr;
    }
}

void RemoteWebView::OnDeviceReset()
{

    if (!texture_)
    {
        device_->CreateTexture(width_, height_, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &texture_,
                               nullptr);
    }
}

void RemoteWebView::Update()
{
    if (frame_->header.sequence == lastSequence_)
        return;

    lastSequence_ = frame_->header.sequence;

    UploadTexture();
}

void RemoteWebView::UploadTexture()
{
    if (!frame_)
        return;

    std::println(std::cout, "RemoteWebView::UploadTexture() {:p} {} {}", (void*)texture_, frame_->header.width,
                 frame_->header.height);

    D3DLOCKED_RECT rect;

    texture_->LockRect(0, &rect, nullptr, D3DLOCK_DISCARD);

    memcpy(rect.pBits, frame_->pixels, frame_->header.width * frame_->header.height * 4);

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