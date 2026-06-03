#include "BrowserView.hpp"

#include <format>
#include <iostream>

#include "IPCServer.hpp"

BrowserView::BrowserView(uint32_t id, int32_t width, int32_t height) : width_(width), height_(height), webviewId(id)
{
    sharedMemory_.Create(std::format("LuaCEF_SharedMemory_{}", id));
    memset(sharedMemory_.GetBuffer(), 0, sharedMemory_.GetSize());

    frame_ = static_cast<SharedFrame*>(sharedMemory_.GetBuffer());
}

void BrowserView::OnPaint(CefRefPtr<CefBrowser>, PaintElementType, const RectList&, const void* buffer, int width,
                          int height)
{
    if (!frame_)
        return;

    std::println(std::cout, "BrowserView::OnPaint() {:p} {:d} {:d}", (void*)this, width, height);

    frame_->header.width = width;
    frame_->header.height = height;

    memcpy(frame_->pixels, buffer, width * height * 4);

    frame_->header.sequence.fetch_add(1, std::memory_order_release);
}

void BrowserView::GetViewRect(CefRefPtr<CefBrowser>, CefRect& rect)
{
    rect = CefRect(0, 0, width_, height_);
}

void BrowserView::OnAfterCreated(CefRefPtr<CefBrowser> browser)
{
    std::println(std::cout, "BrowserView::OnAfterCreated() {:p} {:d} {:d}", (void*)this, width_, height_);

    browser_ = browser;

    for (auto& callback : onCreateCallbacks_)
        callback(this);

    // browser_->GetHost()->WasResized();
}

void BrowserView::OnBeforeClose(CefRefPtr<CefBrowser> browser)
{
}