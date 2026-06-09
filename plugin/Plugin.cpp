#include "Plugin.hpp"

#include <iostream>

#include <kthook/kthook.hpp>
#include <plugin.h>

#include "Logger.hpp"

using WndProc_t = LRESULT(CALLBACK*)(HWND, UINT, WPARAM, LPARAM);

void Plugin::OnAttach(void* handle)
{
    Logger::Instance().SetLogFilePath(std::filesystem::current_path() / "luacef" / "plugin.log");

    Logger::Instance().Log("Plugin attached with handle: {}", handle);

    webCore_ = std::make_shared<WebCore>();

    plugin::Events::initRwEvent += [this]() { OnRenderInit(); };
}

void Plugin::OnDetach()
{
    Logger::Instance().Log("Plugin detached.");

    webCore_->Shutdown();
}

void Plugin::InstallGameHooks()
{
    static kthook::kthook_simple<WndProc_t> wndProcHook(0x747EB0,
                                                        [this](auto& hook, auto&&... args)
                                                        {
                                                            OnWndProc(std::forward<decltype(args)>(args)...);
                                                            return hook.call_trampoline(
                                                                std::forward<decltype(args)>(args)...);
                                                        });

    const auto d3d9_device_vtable = *(void***)(GetD3DDevice());
    static kthook::kthook_naked presentHook(d3d9_device_vtable[17], [this](const auto& hook) { OnDrawingEvent(); });
}

bool Plugin::IsInitialized() const
{
    return webCore_ != nullptr && webCore_->IsInitialized();
}

// bool Plugin::IsAnyInputActive() const
// {
//     if (!IsInitialized())
//         return false;

//     return webCore_->IsAnyInputActive();
// }

RemoteWebView* Plugin::Create(const char* url, int x, int y, int width, int height,
                              RemoteWebView::FrameCallback onCreate)
{
    if (!IsInitialized())
        return nullptr;

    Logger::Instance().Log("Creating web view with URL: {}, x: {}, y: {}, width: {}, height: {}", url, x, y, width,
                           height);

    auto webView = webCore_->Create(url, x, y, width, height, onCreate);
    return webView;
}

RemoteWebView* Plugin::CreateFullscreen(const char* url, RemoteWebView::FrameCallback onCreate)
{
    if (!IsInitialized())
        return nullptr;

    Logger::Instance().Log("Creating fullscreen web view with URL: {}", url);

    auto webView = webCore_->Create(url, 0, 0, RsGlobal.maximumWidth, RsGlobal.maximumHeight, onCreate);
    return webView;
}

void Plugin::Close(RemoteWebView* view)
{
    if (!IsInitialized())
        return;

    Logger::Instance().Log("Closing web view.");
    webCore_->Close(view);
}

void Plugin::OnRenderInit()
{
    Logger::Instance().Log("Initializing render...");
    webCore_->Initialize(GetD3DDevice());
    Logger::Instance().Log("Render initialized.");

    Logger::Instance().Log("Installing game hooks...");
    InstallGameHooks();
    Logger::Instance().Log("Game hooks installed.");
}

void Plugin::OnD3DLost()
{
    Logger::Instance().Log("D3D device lost.");

    webCore_->OnDeviceLost();
}

void Plugin::OnD3DReset()
{
    Logger::Instance().Log("D3D device reset.");

    webCore_->OnDeviceReset();
}

void Plugin::OnDrawingEvent()
{
    webCore_->Update();
}

void Plugin::OnWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    webCore_->OnWndProc(hwnd, msg, wParam, lParam);
}