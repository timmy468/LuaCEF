#include "Plugin.hpp"

#include <iostream>

#include <kthook/kthook.hpp>
#include <plugin.h>

using WndProc_t = LRESULT(CALLBACK*)(HWND, UINT, WPARAM, LPARAM);

void Plugin::OnAttach(void* handle)
{
    std::println(std::cout, "Plugin attached with handle: {}", handle);

    webCore_ = std::make_shared<WebCore>();

    plugin::Events::initRwEvent += [this]() { OnRenderInit(); };
    plugin::Events::d3dLostEvent += [this]() { OnD3DLost(); };
    plugin::Events::d3dResetEvent += [this]() { OnD3DReset(); };
}

void Plugin::OnDetach()
{
    std::println(std::cout, "Plugin detached.");

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

bool Plugin::IsAnyInputActive() const
{
    if (!IsInitialized())
        return false;

    return webCore_->IsAnyInputActive();
}

WebView* Plugin::Create(const char* url, int x, int y, int width, int height, WebView::FrameCallback onCreate)
{
    if (!IsInitialized())
        return nullptr;

    std::println(std::cout, "Creating web view with URL: {}, x: {}, y: {}, width: {}, height: {}", url, x, y, width,
                 height);

    auto webView = webCore_->Create(url, x, y, width, height, onCreate);
    return webView;
}

WebView* Plugin::CreateFullscreen(const char* url, WebView::FrameCallback onCreate)
{
    if (!IsInitialized())
        return nullptr;

    std::println(std::cout, "Creating fullscreen web view with URL: {}", url);

    auto webView = webCore_->Create(url, 0, 0, RsGlobal.maximumWidth, RsGlobal.maximumHeight, onCreate);
    return webView;
}

void Plugin::Close(WebView* view)
{
    if (!IsInitialized())
        return;

    std::println(std::cout, "Closing web view.");
    webCore_->Close(view);
}

void Plugin::OnRenderInit()
{
    std::println(std::cout, "Initializing render...");
    webCore_->Initialize(GetD3DDevice());
    std::println(std::cout, "Render initialized.");

    std::println(std::cout, "Installing game hooks...");
    InstallGameHooks();
    std::println(std::cout, "Game hooks installed.");
}

void Plugin::OnD3DLost()
{
    std::println(std::cout, "D3D device lost.");

    webCore_->OnDeviceLost();
}

void Plugin::OnD3DReset()
{
    std::println(std::cout, "D3D device reset.");

    webCore_->OnDeviceReset();
}

void Plugin::OnDrawingEvent()
{
    webCore_->Update();
}

void Plugin::OnWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // std::println(std::cout, "WndProc called with hwnd: {}, msg: {}, wParam: {}, lParam: {}", hwnd, msg, wParam,
    // lParam);

    webCore_->OnWndProc(hwnd, msg, wParam, lParam);
}