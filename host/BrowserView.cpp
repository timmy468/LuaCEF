#include "BrowserView.hpp"

BrowserView::BrowserView(SharedFrame* frame, int width, int height) : frame_(frame), width_(width), height_(height)
{
}

void BrowserView::OnPaint(CefRefPtr<CefBrowser>, PaintElementType, const RectList&, const void* buffer, int width,
                          int height)
{
    frame_->header.width = width;
    frame_->header.height = height;

    memcpy(frame_->pixels, buffer, width * height * 4);

    frame_->header.sequence.fetch_add(1, std::memory_order_release);
}

void BrowserView::GetViewRect(CefRefPtr<CefBrowser>, CefRect& rect)
{
    rect = CefRect(0, 0, width_, height_);
}