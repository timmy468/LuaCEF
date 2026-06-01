#include "Plugin.hpp"

PLUGIN_ENTRY_POINT(Plugin);

#define API_EXPORT extern "C" __declspec(dllexport)

extern "C"
{
    API_EXPORT bool LuaCEF_IsInitialized()
    {
        return Plugin::GetInstance().IsInitialized();
    }

    API_EXPORT WebView* LuaCEF_Create(const char* url, int x, int y, int width, int height)
    {
        if (!Plugin::GetInstance().IsInitialized())
            return nullptr;

        return Plugin::GetInstance().Create(url, x, y, width, height);
    }

    API_EXPORT WebView* LuaCEF_CreateFullscreen(const char* url)
    {
        if (!Plugin::GetInstance().IsInitialized())
            return nullptr;

        return Plugin::GetInstance().CreateFullscreen(url);
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
}