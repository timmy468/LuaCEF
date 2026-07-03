#include "Plugin.hpp"

#include <chrono>
#include <iostream>

#include <kthook/kthook.hpp>
#include <plugin.h>

#include "Logger.hpp"
#include "render/RenderManager.hpp"

#include "utils/samp_utils.hpp"

void Plugin::OnAttach(void* handle)
{
    Logger::Instance().SetLogFilePath(std::filesystem::current_path() / "luacef" / "plugin.log");

    Logger::Instance().Log("Plugin attached with handle: {}", handle);

    webCore_ = std::make_shared<WebCore>();
    renderManager_ = std::make_shared<RenderManager>();

    plugin::Events::initRwEvent += [this]() { OnRenderInit(); };
}

void Plugin::OnDetach()
{
    webCore_->Shutdown();
    renderManager_->Shutdown();

    Logger::Instance().Log("Plugin detached.");
}

void Plugin::InstallWndProcHook(uintptr_t address)
{
    wndProcHook_ = std::make_unique<WndProcHook>(address,
                                                 [this](auto& hook, auto&&... args) -> LRESULT
                                                 {
                                                     if (OnWndProc(std::forward<decltype(args)>(args)...))
                                                         return 1;

                                                     return hook.call_trampoline(std::forward<decltype(args)>(args)...);
                                                 });

    Logger::Instance().Log("WndProc changed");
}

bool Plugin::IsInitialized() const
{
    return webCore_ != nullptr && webCore_->IsInitialized();
}

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

    Logger::Instance().Log("Closing web view with id {}", view->GetBrowserId());
    webCore_->Close(view);
}

void Plugin::OnRenderInit()
{
    Logger::Instance().Log("Initializing RenderManager...");
    renderManager_->Initialize();
    Logger::Instance().Log("RenderManager initialized.");

    Logger::Instance().Log("Initializing WebCore...");
    webCore_->Initialize(GetD3DDevice(), renderManager_);
    Logger::Instance().Log("WebCore initialized.");
}

void Plugin::OnD3DLost()
{
    if (!IsInitialized())
        return;

    Logger::Instance().Log("D3D device lost.");

    webCore_->OnDeviceLost();
    renderManager_->OnDeviceLost();
}

void Plugin::OnD3DReset()
{
    if (!IsInitialized())
        return;

    Logger::Instance().Log("D3D device reset.");

    webCore_->OnDeviceReset();
    renderManager_->OnDeviceReset();
}

void Plugin::Update()
{
    if (!IsInitialized())
        return;

    webCore_->Update();

    static std::chrono::steady_clock::time_point lastTime;
    const auto currentTime = std::chrono::steady_clock::now();
    if (currentTime - lastTime < std::chrono::seconds(1))
        return;

    Logger::Instance().Log("Update with time");

    lastTime = currentTime;

    static auto lastWndProcAddr = 0;
    if (lastWndProcAddr != GetWindowLongPtr(RsGlobal.ps->window, GWLP_WNDPROC))
    {
        lastWndProcAddr = GetWindowLongPtrW(RsGlobal.ps->window, GWLP_WNDPROC);
        InstallWndProcHook(lastWndProcAddr);
    }
}

bool Plugin::OnWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (!IsInitialized())
        return false;

    return webCore_->OnWndProc(hwnd, msg, wParam, lParam);
}