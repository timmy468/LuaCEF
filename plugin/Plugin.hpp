#pragma once

#include <memory>

#include <plugintmplt/plugintmplt.hpp>

#include "render/IRenderManager.hpp"
#include "web/WebCore.hpp"

class Plugin final : public plugintmplt::AbstractPlugin<Plugin>
{
  private:
    std::shared_ptr<WebCore> webCore_;
    std::shared_ptr<IRenderManager> renderManager_;

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

  private:
    void InstallGameHooks();

    void OnRenderInit();
    void OnD3DLost();
    void OnD3DReset();
    void OnWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
};