#include "Plugin.hpp"

PLUGIN_ENTRY_POINT(Plugin);

#define API_EXPORT extern "C" __declspec(dllexport)

using frame_callback_t = void(__stdcall*)(RemoteWebView*);

using loading_callback_t = void(__stdcall*)(RemoteWebView*, int);

using bind_function_callback_t = void(__stdcall*)(RemoteWebView*, const char*);

extern "C"
{
    API_EXPORT const char* CEF_GetVersion()
    {
        return "1.1.1";
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
                                            frame_callback_t onCreate)
    {
        if (!Plugin::GetInstance().IsInitialized())
            return nullptr;

        return Plugin::GetInstance().Create(url, x, y, width, height,
                                            [onCreate](RemoteWebView* view) { onCreate(view); });
    }

    API_EXPORT RemoteWebView* LuaCEF_CreateFullscreen(const char* url, frame_callback_t onCreate)
    {
        if (!Plugin::GetInstance().IsInitialized())
            return nullptr;

        return Plugin::GetInstance().CreateFullscreen(url, [onCreate](RemoteWebView* view) { onCreate(view); });
    }

    API_EXPORT void LuaCEF_Update()
    {
        if (!Plugin::GetInstance().IsInitialized())
            return;

        Plugin::GetInstance().Update();
    }

    API_EXPORT void LuaCEF_OnD3DLost()
    {
        if (!Plugin::GetInstance().IsInitialized())
            return;

        Plugin::GetInstance().OnD3DLost();
    }

    API_EXPORT void LuaCEF_OnD3DReset()
    {
        if (!Plugin::GetInstance().IsInitialized())
            return;

        Plugin::GetInstance().OnD3DReset();
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

    API_EXPORT void LuaCEF_AddOnAfterCreateCallback(RemoteWebView* view, frame_callback_t callback)
    {
        if (!Plugin::GetInstance().IsInitialized())
            return;

        if (!view)
            return;

        view->AddOnAfterCreateCallback([callback](RemoteWebView* view) { callback(view); });
    }

    API_EXPORT void LuaCEF_AddOnLoadingCallback(RemoteWebView* view, loading_callback_t callback)
    {
        if (!Plugin::GetInstance().IsInitialized())
            return;

        if (!view)
            return;

        view->AddOnLoadingCallback([callback](RemoteWebView* view, int httpStatusCode)
                                   { callback(view, httpStatusCode); });
    }

    API_EXPORT void LuaCEF_AddOnCloseCallback(RemoteWebView* view, frame_callback_t callback)
    {
        if (!Plugin::GetInstance().IsInitialized())
            return;

        if (!view)
            return;

        view->AddOnCloseCallback([callback](RemoteWebView* view) { callback(view); });
    }

    API_EXPORT void LuaCEF_AddBindFunction(RemoteWebView* view, const char* name, bind_function_callback_t callback)
    {
        if (!Plugin::GetInstance().IsInitialized())
            return;

        if (!view)
            return;

        view->AddBindFunction(name, [callback](RemoteWebView* view, const std::string& args)
                              { callback(view, args.c_str()); });
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

    API_EXPORT bool LuaCEF_IsFocused(RemoteWebView* view)
    {
        if (!Plugin::GetInstance().IsInitialized())
            return false;

        if (!view)
            return false;

        return view->IsFocused();
    }

    API_EXPORT void LuaCEF_SetFocused(RemoteWebView* view, bool active)
    {
        if (!Plugin::GetInstance().IsInitialized())
            return;

        if (!view)
            return;

        view->SetFocused(active);
    }

    API_EXPORT void LuaCEF_SetEnabledFocusMode(RemoteWebView* view, bool enable)
    {
        if (!Plugin::GetInstance().IsInitialized())
            return;

        if (!view)
            return;

        view->SetEnabledFocusMode(enable);
    }

    API_EXPORT bool LuaCEF_IsEnabledFocusMode(RemoteWebView* view)
    {
        if (!Plugin::GetInstance().IsInitialized())
            return false;

        if (!view)
            return false;

        return view->IsEnabledFocusMode();
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

    API_EXPORT void LuaCEF_SetFPS(RemoteWebView* view, int fps)
    {
        if (!Plugin::GetInstance().IsInitialized())
            return;

        if (!view)
            return;

        Plugin::GetInstance().GetWebCore()->SetFPS(view, fps);
    }

    API_EXPORT void LuaCEF_ClearCallbacks(RemoteWebView* view)
    {
        if (!Plugin::GetInstance().IsInitialized())
            return;

        if (!view)
            return;

        view->ClearCallbacks();
    }
}