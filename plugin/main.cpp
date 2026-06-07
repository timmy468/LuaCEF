#include "Plugin.hpp"

PLUGIN_ENTRY_POINT(Plugin);

#define API_EXPORT extern "C" __declspec(dllexport)

extern "C"
{
    API_EXPORT const char* CEF_GetVersion()
    {
        return "1.0.0";
    }

    API_EXPORT bool LuaCEF_IsInitialized()
    {
        return Plugin::GetInstance().IsInitialized();
    }

    API_EXPORT bool LuaCEF_IsAnyInputActive()
    {
        return Plugin::GetInstance().GetWebCore()->IsAnyInputActive();
    }

    API_EXPORT RemoteWebView* LuaCEF_Create(const char* url, int x, int y, int width, int height,
                                            RemoteWebView::FrameCallback onCreate)
    {
        if (!Plugin::GetInstance().IsInitialized())
            return nullptr;

        return Plugin::GetInstance().Create(url, x, y, width, height, onCreate);
    }

    API_EXPORT RemoteWebView* LuaCEF_CreateFullscreen(const char* url, RemoteWebView::FrameCallback onCreate)
    {
        if (!Plugin::GetInstance().IsInitialized())
            return nullptr;

        return Plugin::GetInstance().CreateFullscreen(url, onCreate);
    }

    API_EXPORT void LuaCEF_Close(RemoteWebView* view)
    {
        if (!Plugin::GetInstance().IsInitialized())
            return;

        Plugin::GetInstance().Close(view);
    }

    API_EXPORT IDirect3DTexture9* LuaCEF_GetTexture(RemoteWebView* view)
    {
        if (!Plugin::GetInstance().IsInitialized())
            return nullptr;

        if (!view)
            return nullptr;

        return view->GetTexture();
    }

    API_EXPORT void LuaCEF_Update()
    {
        if (!Plugin::GetInstance().IsInitialized())
            return;

        Plugin::GetInstance().GetWebCore()->Update();
    }

    API_EXPORT void LuaCEF_OnD3DLost()
    {
        if (!Plugin::GetInstance().IsInitialized())
            return;

        Plugin::GetInstance().GetWebCore()->OnDeviceLost();
    }

    API_EXPORT void LuaCEF_OnD3DReset()
    {
        if (!Plugin::GetInstance().IsInitialized())
            return;

        Plugin::GetInstance().GetWebCore()->OnDeviceReset();
    }

    API_EXPORT bool LuaCEF_IsInputActive(RemoteWebView* view)
    {
        if (!Plugin::GetInstance().IsInitialized())
            return false;

        if (!view)
            return false;

        return view->InputEnabled();
    }

    API_EXPORT void LuaCEF_SetInputActive(RemoteWebView* view, bool active)
    {
        if (!Plugin::GetInstance().IsInitialized())
            return;

        if (!view)
            return;

        view->SetInputEnabled(active);
    }

    API_EXPORT void LuaCEF_SetRenderEnabled(RemoteWebView* view, bool enable)
    {
        if (!Plugin::GetInstance().IsInitialized())
            return;

        if (!view)
            return;

        view->SetRenderEnabled(enable);
    }

    API_EXPORT bool LuaCEF_IsRenderEnabled(RemoteWebView* view)
    {
        if (!Plugin::GetInstance().IsInitialized())
            return false;

        if (!view)
            return false;

        return view->RenderEnabled();
    }

    API_EXPORT void LuaCEF_SetOffscreenRenderingEnabled(RemoteWebView* view, bool enable)
    {
        if (!Plugin::GetInstance().IsInitialized())
            return;

        if (!view)
            return;

        view->SetOffscreenRenderingEnabled(enable);
    }

    API_EXPORT bool LuaCEF_IsOffscreenRenderingEnabled(RemoteWebView* view)
    {
        if (!Plugin::GetInstance().IsInitialized())
            return false;

        if (!view)
            return false;

        return view->OffscreenRenderingEnabled();
    }

    API_EXPORT void LuaCEF_SetRect(RemoteWebView* view, int x, int y, int width, int height)
    {
        if (!Plugin::GetInstance().IsInitialized())
            return;

        if (!view)
            return;

        view->SetRect(x, y, width, height);
    }

    API_EXPORT const char* LuaCEF_GetURL(RemoteWebView* view)
    {
        if (!Plugin::GetInstance().IsInitialized())
            return nullptr;

        if (!view)
            return nullptr;

        auto& url_ = view->GetURL();
        return url_.c_str();
    }

    API_EXPORT void LuaCEF_LoadURL(RemoteWebView* view, const char* url)
    {
        if (!Plugin::GetInstance().IsInitialized())
            return;

        if (!view)
            return;

        Plugin::GetInstance().GetWebCore()->LoadURL(view, url);
    }

    API_EXPORT void LuaCEF_ReloadPage(RemoteWebView* view, bool ignoreCache)
    {
        if (!Plugin::GetInstance().IsInitialized())
            return;

        if (!view)
            return;

        Plugin::GetInstance().GetWebCore()->ReloadPage(view, ignoreCache);
    }

    API_EXPORT void GoBack(RemoteWebView* view)
    {
        if (!Plugin::GetInstance().IsInitialized())
            return;

        if (!view)
            return;

        Plugin::GetInstance().GetWebCore()->GoBack(view);
    }

    API_EXPORT void GoForward(RemoteWebView* view)
    {
        if (!Plugin::GetInstance().IsInitialized())
            return;

        if (!view)
            return;

        Plugin::GetInstance().GetWebCore()->GoForward(view);
    }

    API_EXPORT void LuaCEF_ExecuteJS(RemoteWebView* view, const char* code)
    {
        if (!Plugin::GetInstance().IsInitialized())
            return;

        if (!view)
            return;

        Plugin::GetInstance().GetWebCore()->ExecuteJS(view, code);
    }
}