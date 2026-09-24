#include "BrowserView.hpp"

#include <format>
#include <iostream>

#include <Windows.h>
#include <commdlg.h>

#include "Logger.hpp"

BrowserView::BrowserView(DWORD parentPid, uint32_t id, int32_t x, int32_t y, int32_t width, int32_t height,
                         uintptr_t parentWindow)
    : parentPid_(parentPid), webviewId(id), x_(x), y_(y), width_(width), height_(height), parentWindow_(parentWindow)
{
    sharedMemory_.Create(ipc::SharedMemory::MakeName(parentPid, id), width, height);
    memset(sharedMemory_.GetBuffer(), 0, sharedMemory_.GetSize());

    frame_ = static_cast<SharedFrame*>(sharedMemory_.GetBuffer());

    Logger::Instance().Log("BrowserView::BrowserView() {:p} {:d} {:d} {:p}", (void*)this, width, height,
                           (void*)parentWindow);
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

void BrowserView::OnPaint(CefRefPtr<CefBrowser>, PaintElementType, const RectList& dirtyRects, const void* buffer,
                          int width, int height)
{
#ifndef NDEBUG
    Logger::Instance().Log("BrowserView::OnPaint() {:p} {:d} {:d}", (void*)this, width, height);
#endif

    if (!frame_)
        return;

    if (width > width_ || height > height_)
    {
        Logger::Instance().Log("Browser too large {}x{}", width, height);
        return;
    }

    uint32_t seq = frame_->header.sequence.fetch_add(1,
                                                     std::memory_order_acq_rel); // становится odd

    frame_->header.width.store(width, std::memory_order_relaxed);
    frame_->header.height.store(height, std::memory_order_relaxed);

    const auto* srcPixels = static_cast<const uint8_t*>(buffer);

    uint32_t index = 0;

    for (const auto& rect : dirtyRects)
    {
        if (index >= MAX_DIRTY_RECTS)
            break;

        if (rect.x < 0 || rect.y < 0 || rect.width <= 0 || rect.height <= 0)
        {
            continue;
        }

        if (rect.x >= width || rect.y >= height)
        {
            continue;
        }

        if (rect.width > width - rect.x || rect.height > height - rect.y)
        {
            continue;
        }

        frame_->dirtyRects[index] = {rect.x, rect.y, rect.width, rect.height};

        for (int y = rect.y; y < rect.y + rect.height; ++y)
        {
            memcpy(frame_->GetPixelBuffer() + (y * width + rect.x) * 4,

                   srcPixels + (y * width + rect.x) * 4,

                   rect.width * 4);
        }

        ++index;
    }

    frame_->header.dirtyRectCount.store(index, std::memory_order_relaxed);

    frame_->header.sequence.store(seq + 2, std::memory_order_release);
}

void BrowserView::GetViewRect(CefRefPtr<CefBrowser>, CefRect& rect)
{
    rect = CefRect(0, 0, width_, height_);
}

bool BrowserView::GetScreenInfo(CefRefPtr<CefBrowser>, CefScreenInfo& screen_info)
{
    const CefRect view_rect(0, 0, width_, height_);

    screen_info.device_scale_factor = 1.0f;
    screen_info.rect = view_rect;
    screen_info.available_rect = view_rect;

    return true;
}

void BrowserView::SetRect(int x, int y, int width, int height)
{
    if (width <= 0 || height <= 0)
        return;

    const bool sizeChanged = (width != width_ || height != height_);

    x_ = x;
    y_ = y;
    width_ = width;
    height_ = height;

    if (sizeChanged)
    {
        // Пересоздаём shared memory под новый размер. Plugin перед отправкой
        // команды закрывает своё отображение, поэтому Create с тем же именем
        // создаёт свежий объект нужного размера.
        sharedMemory_.Close();
        frame_ = nullptr;

        const std::string name = ipc::SharedMemory::MakeName(parentPid_, webviewId);

        if (sharedMemory_.Create(name, width, height))
        {
            memset(sharedMemory_.GetBuffer(), 0, sharedMemory_.GetSize());
            frame_ = static_cast<SharedFrame*>(sharedMemory_.GetBuffer());
        }
        else
        {
            Logger::Instance().Log("Failed to resize shared memory for view {}", webviewId);
        }
    }

    if (browser_ && browser_->GetHost())
    {
        browser_->GetHost()->WasResized();
        browser_->GetHost()->NotifyScreenInfoChanged();
    }

    Logger::Instance().Log("BrowserView::SetRect() id={} {}x{} at {},{}", webviewId, width, height, x, y);
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

void BrowserView::OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, ErrorCode errorCode,
                              const CefString& errorText, const CefString& failedUrl)
{
    Logger::Instance().Log("OnLoadError: code={} text={} url={}", static_cast<int>(errorCode), errorText.ToString(),
                           failedUrl.ToString());
}

void BrowserView::OnAddressChange(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& url)
{
    for (auto& callback : onAddressChangeCallbacks_)
        callback(this, url);
}

bool BrowserView::OnConsoleMessage(CefRefPtr<CefBrowser>, cef_log_severity_t, const CefString& message,
                                   const CefString& source, int line)
{
    Logger::Instance().Log("JS: {} ({}:{})", message.ToString().c_str(), source.ToString().c_str(), line);

    return false;
}

bool BrowserView::OnFileDialog(CefRefPtr<CefBrowser> browser, FileDialogMode mode, const CefString& title,
                               const CefString& default_file_path, const std::vector<CefString>& accept_filters,
                               const std::vector<CefString>& accept_extensions,
                               const std::vector<CefString>& accept_descriptions,
                               CefRefPtr<CefFileDialogCallback> callback)
{

    return false;
}