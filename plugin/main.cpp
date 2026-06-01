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
        return Plugin::GetInstance().IsAnyInputActive();
    }

    API_EXPORT WebView* LuaCEF_Create(const char* url, int x, int y, int width, int height,
                                      WebView::FrameCallback onCreate)
    {
        if (!Plugin::GetInstance().IsInitialized())
            return nullptr;

        return Plugin::GetInstance().Create(url, x, y, width, height, onCreate);
    }

    API_EXPORT WebView* LuaCEF_CreateFullscreen(const char* url, WebView::FrameCallback onCreate)
    {
        if (!Plugin::GetInstance().IsInitialized())
            return nullptr;

        return Plugin::GetInstance().CreateFullscreen(url, onCreate);
    }

    API_EXPORT void LuaCEF_Close(WebView* view)
    {
        if (!Plugin::GetInstance().IsInitialized())
            return;

        Plugin::GetInstance().Close(view);
    }

    API_EXPORT IDirect3DTexture9* LuaCEF_GetTexture(WebView* view)
    {
        if (!Plugin::GetInstance().IsInitialized())
            return nullptr;

        if (!view)
            return nullptr;

        return view->GetTexture();
    }

    API_EXPORT bool LuaCEF_IsInputActive(WebView* view)
    {
        if (!Plugin::GetInstance().IsInitialized())
            return false;

        if (!view)
            return false;

        return view->InputEnabled();
    }

    API_EXPORT void LuaCEF_SetInputActive(WebView* view, bool active)
    {
        if (!Plugin::GetInstance().IsInitialized())
            return;

        if (!view)
            return;

        view->SetInputEnabled(active);
    }

    API_EXPORT const char* LuaCEF_GetURL(WebView* view)
    {
        if (!Plugin::GetInstance().IsInitialized())
            return nullptr;

        if (!view)
            return nullptr;

        auto& url_ = view->GetURL();
        return url_.c_str();
    }

    API_EXPORT void LuaCEF_LoadURL(WebView* view, const char* url)
    {
        if (!Plugin::GetInstance().IsInitialized())
            return;

        if (!view)
            return;

        view->LoadURL(url);
    }
}