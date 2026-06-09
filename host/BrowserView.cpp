#include "BrowserView.hpp"

#include <format>
#include <iostream>

#include "Logger.hpp"

BrowserView::BrowserView(uint32_t id, int32_t x, int32_t y, int32_t width, int32_t height)
    : x_(x), y_(y), width_(width), height_(height), webviewId(id)
{
    sharedMemory_.Create(std::format("LuaCEF_SharedMemory_{}", id));
    memset(sharedMemory_.GetBuffer(), 0, sharedMemory_.GetSize());

    frame_ = static_cast<SharedFrame*>(sharedMemory_.GetBuffer());

    Logger::Instance().Log("BrowserView::BrowserView() {:p} {:d} {:d}", (void*)this, width, height);
}

bool BrowserView::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                           CefProcessId source_process, CefRefPtr<CefProcessMessage> message)
{
    if (message->GetName() == "BindFunctionCall")
    {
        auto args = message->GetArgumentList();

        const CefString function = args->GetString(0);
        const CefString jsonArgs = args->GetString(1);

        Logger::Instance().Log("BindFunctionCall {} {}", function.ToString(), jsonArgs.ToString());

        for (auto& callback : onBindFunctionCallbacks_)
        {
            callback(this, function, jsonArgs);
        }

        return true;
    }

    return false;
}

void BrowserView::OnPaint(CefRefPtr<CefBrowser>, PaintElementType, const RectList&, const void* buffer, int width,
                          int height)
{
    if (!frame_)
        return;

    Logger::Instance().Log("BrowserView::OnPaint() {:p} {:d} {:d}", (void*)this, width, height);

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
    Logger::Instance().Log("BrowserView::OnAfterCreated() {:p} {:d} {:d}", (void*)this, width_, height_);

    browser_ = browser;

    for (auto& callback : onCreateCallbacks_)
        callback(this);
}

void BrowserView::OnBeforeClose(CefRefPtr<CefBrowser> browser)
{
    for (auto& callback : onCloseCallbacks_)
        callback(this);
}

void BrowserView::OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode)
{
    for (auto& callback : onLoadingCallbacks_)
        callback(this, httpStatusCode);
}

void BrowserView::OnAddressChange(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& url)
{
    for (auto& callback : onAddressChangeCallbacks_)
        callback(this, url);
}