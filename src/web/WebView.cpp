#include "WebView.hpp"

#include <cstdint>
#include <cstring>

#include <cstring>

WebView::WebView(IDirect3DDevice9* device, int width, int height) : device_(device), width_(width), height_(height)
{
    device_->CreateTexture(width, height, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &texture_, nullptr);
}

bool WebView::GetViewRect(CefRefPtr<CefBrowser>, CefRect& rect)
{
    rect = CefRect(0, 0, width_, height_);
    return true;
}

void WebView::OnPaint(CefRefPtr<CefBrowser>, PaintElementType, const RectList&, const void* buffer, int width,
                      int height)
{
    if (!texture_)
        return;

    D3DLOCKED_RECT rect{};

    if (SUCCEEDED(texture_->LockRect(0, &rect, nullptr, D3DLOCK_DISCARD)))
    {
        const auto* src = static_cast<const uint8_t*>(buffer);

        auto* dst = static_cast<uint8_t*>(rect.pBits);

        for (int y = 0; y < height; y++)
        {
            memcpy(dst + y * rect.Pitch, src + y * width * 4, width * 4);
        }

        texture_->UnlockRect(0);
    }
}