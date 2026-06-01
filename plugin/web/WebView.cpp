#include "WebView.hpp"

#include <cstdint>
#include <cstring>
#include <iostream>

#include "utils/samp_utils.hpp"

WebView::WebView(IDirect3DDevice9* device, int x, int y, int width, int height, FrameCallback onCreate)
    : device_(device), width_(width), height_(height), onCreate_(onCreate)
{
    auto hr = device_->CreateTexture(width, height, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
                                     &textureData_.texture, nullptr);

    if (FAILED(hr))
    {
        std::println(std::cerr, "CreateTexture failed {:08X}", static_cast<uint32_t>(hr));
    }

    CefWindowInfo window_info;
    window_info.SetAsWindowless(nullptr);
    window_info.x = x;
    window_info.y = y;
    window_info.width = width;
    window_info.height = height;

    CefBrowserSettings browser_settings;

    CefBrowserHost::CreateBrowser(window_info, this, "", browser_settings, nullptr, nullptr);
}

WebView::~WebView()
{
    if (textureData_.texture)
    {
        textureData_.texture->Release();
        textureData_.texture = nullptr;
    }

    // if (browser_)
    // {
    //     browser_->GetHost()->CloseBrowser(true);
    //     browser_ = nullptr;
    // }
}

void WebView::GetViewRect(CefRefPtr<CefBrowser>, CefRect& rect)
{
    rect = CefRect(0, 0, width_, height_);
}

void WebView::OnPaint(CefRefPtr<CefBrowser>, PaintElementType, const RectList&, const void* buffer, int width,
                      int height)
{
    auto& data = textureData_;

    const size_t size = static_cast<size_t>(width) * height * 4;

    std::lock_guard lock(data.bufferMutex);

    if (data.backBuffer.size() != size)
    {
        data.backBuffer.resize(size);
    }

    memcpy(data.backBuffer.data(), buffer, size);

    data.width = width;
    data.height = height;

    data.frontBuffer.swap(data.backBuffer);

    data.dirty.store(true, std::memory_order_release);
}

void WebView::OnAfterCreated(CefRefPtr<CefBrowser> browser)
{
    browser_ = browser;

    std::println(std::cout, "Browser created with URL: {}", browser_->GetMainFrame()->GetURL().ToString());

    if (onCreate_)
        onCreate_(this);

    if (!pendingUrl_.empty())
        browser_->GetMainFrame()->LoadURL(pendingUrl_);
}

void WebView::OnBeforeClose(CefRefPtr<CefBrowser>)
{
    browser_ = nullptr;

    if (onClose_)
        onClose_(this);
}

void WebView::OnLoadEnd(CefRefPtr<CefBrowser>, CefRefPtr<CefFrame>, int httpStatusCode)
{
    loaded_ = true;

    if (onLoading_)
        onLoading_(this, httpStatusCode);
}

void WebView::Close()
{
    if (!browser_)
        return;

    browser_->GetHost()->CloseBrowser(true);
}

void WebView::OnDeviceLost()
{
    auto& data = textureData_;

    std::lock_guard lock(data.textureMutex);

    if (data.texture)
    {
        data.texture->Release();
        data.texture = nullptr;
    }
}

void WebView::OnDeviceReset()
{
    auto& data = textureData_;

    std::lock_guard lock(data.textureMutex);

    if (!data.texture)
    {
        device_->CreateTexture(width_, height_, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &data.texture,
                               nullptr);
    }

    data.dirty.store(true, std::memory_order_release);
}

void WebView::UpdateTexture()
{
    auto& data = textureData_;

    if (!data.dirty.exchange(false, std::memory_order_acq_rel))
    {
        return;
    }

    std::lock_guard textureLock(data.textureMutex);

    auto* texture = data.texture;

    if (!texture)
        return;

    std::lock_guard bufferLock(data.bufferMutex);

    if (data.frontBuffer.empty())
        return;

    D3DLOCKED_RECT rect{};

    if (FAILED(texture->LockRect(0, &rect, nullptr, D3DLOCK_DISCARD)))
    {
        data.dirty.store(true);
        return;
    }

    const auto* src = data.frontBuffer.data();

    auto* dst = static_cast<uint8_t*>(rect.pBits);

    const uint32_t srcPitch = data.width * 4;

    if (rect.Pitch == srcPitch)
    {
        memcpy(rect.pBits, src, srcPitch * data.height);
    }
    else
    {
        auto* dst = static_cast<uint8_t*>(rect.pBits);

        for (int y = 0; y < data.height; ++y)
        {
            memcpy(dst + y * rect.Pitch, src + y * srcPitch, srcPitch);
        }
    }

    data.texture->UnlockRect(0);
}

bool WebView::InputEnabled() const
{
    return inputEnabled_;
}

void WebView::SetInputEnabled(bool enabled)
{
    inputEnabled_ = enabled;
    if (inputEnabled_)
        samp_utils::ToggleCursor(true);
    else
        samp_utils::ToggleCursor(false);
}

const std::string& WebView::GetURL()
{
    if (!browser_)
        return url_;

    url_ = browser_->GetMainFrame()->GetURL();

    return url_;
}

void WebView::LoadURL(const std::string& url)
{
    if (!browser_)
    {
        pendingUrl_ = url;
        return;
    }

    browser_->GetMainFrame()->LoadURL(url);
}

void WebView::ExecuteJS(const std::string& code)
{
    if (!browser_)
        return;

    browser_->GetMainFrame()->ExecuteJavaScript(code, "", 0);
}