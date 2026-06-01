#pragma once

#include <memory>

#include <plugintmplt/plugintmplt.hpp>

#include "web/WebCore.hpp"

class Plugin final : public plugintmplt::AbstractPlugin<Plugin>
{
  private:
    std::shared_ptr<WebCore> webCore_;

  public:
    void OnAttach(void* handle);
    void OnDetach();

    bool IsInitialized() const;
    WebView* Create(const char* url, int x, int y, int width, int height);
    WebView* CreateFullscreen(const char* url);
    void Close(WebView* view);

  private:
    void InstallGameHooks();

    void OnRenderInit();
    void OnD3DLost();
    void OnD3DReset();
    void OnDrawingEvent();
    void OnWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
};