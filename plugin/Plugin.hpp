#pragma once

#include <memory>

#include <kthook/kthook.hpp>
#include <plugintmplt/plugintmplt.hpp>

#include "render/IRenderManager.hpp"
#include "web/WebCore.hpp"

class Plugin final : public plugintmplt::AbstractPlugin<Plugin>
{
  private:
    std::shared_ptr<WebCore> webCore_;
    std::shared_ptr<IRenderManager> renderManager_;

    using WndProc_t = LRESULT(CALLBACK*)(HWND, UINT, WPARAM, LPARAM);
    using WndProcHook = kthook::kthook_simple<WndProc_t>;
    std::unique_ptr<WndProcHook> wndProcHook_;

    RemoteWebView* creditsWebView_;

  public:
    void OnAttach(void* handle);
    void OnDetach();

    bool IsInitialized() const;
    bool IsAnyInputActive() const;
    RemoteWebView* Create(const char* url, int x, int y, int width, int height, RemoteWebView::FrameCallback onCreate);
    RemoteWebView* CreateFullscreen(const char* url, RemoteWebView::FrameCallback onCreate);
    void Close(RemoteWebView* view);

    std::shared_ptr<WebCore> GetWebCore() const
    {
        return webCore_;
    }

    void Update();
    void OnD3DLost();
    void OnD3DReset();

  private:
    void InstallGameHooks();
    void InstallWndProcHook(uintptr_t address);
    void UpdateCredits();

    void OnRenderInit();
    void OnGameInit();
    bool OnWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
};