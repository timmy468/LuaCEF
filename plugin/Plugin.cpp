#include "Plugin.hpp"

#include <iostream>

#include <kthook/kthook.hpp>
#include <plugin.h>

#include "Logger.hpp"
#include "render/RenderManager.hpp"

#include "utils/samp_utils.hpp"

using WndProc_t = LRESULT(CALLBACK*)(HWND, UINT, WPARAM, LPARAM);

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

void Plugin::InstallGameHooks()
{
    static kthook::kthook_simple<WndProc_t> wndProcHook(samp_utils::GetWndProcHookAddress(),
                                                        [this](auto& hook, auto&&... args) -> LRESULT
                                                        {
                                                            if (OnWndProc(std::forward<decltype(args)>(args)...))
                                                                return 1;

                                                            return hook.call_trampoline(
                                                                std::forward<decltype(args)>(args)...);
                                                        });
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

    Logger::Instance().Log("Closing web view with id {}", view->GetBrowserId());
    webCore_->Close(view);
}

void Plugin::OnRenderInit()
{
    Logger::Instance().Log("Initializing WebCore...");
    webCore_->Initialize(GetD3DDevice());
    Logger::Instance().Log("WebCore initialized.");

    Logger::Instance().Log("Initializing RenderManager...");
    renderManager_->Initialize();
    Logger::Instance().Log("RenderManager initialized.");

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

void Plugin::Update()
{
    webCore_->Update(renderManager_);
}

bool Plugin::OnWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    return webCore_->OnWndProc(hwnd, msg, wParam, lParam);
}