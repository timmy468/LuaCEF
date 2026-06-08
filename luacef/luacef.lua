local ffi = require("ffi")
local encoding = require("encoding")

encoding.default = "CP1251"

ffi.cdef [[
    typedef void* RemoteWebView;
    typedef void* Texture;

    typedef void(__stdcall* frame_callback_t)(RemoteWebView view);
    typedef void(__stdcall* loading_callback_t)(RemoteWebView view, int httpStatusCode);
    typedef void(__stdcall* bind_function_callback_t)(RemoteWebView view, const char* args);

    // Plugin
    bool LuaCEF_IsInitialized();
    bool IsAnyInputActive();
    RemoteWebView LuaCEF_Create(const char* url, int x, int y, int width, int height, frame_callback_t onCreate);
    RemoteWebView LuaCEF_CreateFullscreen(const char* url, frame_callback_t onCreate);

    // WebCore
    void LuaCEF_Update();
    void LuaCEF_OnD3DLost();
    void LuaCEF_OnD3DReset();

    // RemoteWebView
    void LuaCEF_Close(RemoteWebView view);

    Texture LuaCEF_GetTexture(RemoteWebView view);

    void LuaCEF_AddOnAfterCreateCallback(RemoteWebView view, frame_callback_t callback);
    void LuaCEF_AddOnLoadingCallback(RemoteWebView view, loading_callback_t callback);
    void LuaCEF_AddOnCloseCallback(RemoteWebView view, frame_callback_t callback);
    void LuaCEF_AddBindFunction(RemoteWebView view, const char* name, bind_function_callback_t callback);

    bool LuaCEF_IsInputActive(RemoteWebView view);
    void LuaCEF_SetInputActive(RemoteWebView view, bool enable);
    void LuaCEF_SetRenderEnabled(RemoteWebView view, bool enable);
    bool LuaCEF_IsRenderEnabled(RemoteWebView view);
    void LuaCEF_SetOffscreenRenderingEnabled(RemoteWebView view, bool enable);
    bool LuaCEF_IsOffscreenRenderingEnabled(RemoteWebView view);

    const char* LuaCEF_GetURL(RemoteWebView view);
    void LuaCEF_LoadURL(RemoteWebView view, const char* url);

    void LuaCEF_ReloadPage(RemoteWebView view, bool ignoreCache);
    void LuaCEF_GoBack(RemoteWebView view);
    void LuaCEF_GoForward(RemoteWebView view);

    void LuaCEF_ExecuteJS(RemoteWebView view, const char* code);
]]

if getModuleHandle("luacef.asi") == 0 then
    error("plugin not found")
end

local CEF_LIB = ffi.load("luacef.asi")

local RemoteWebView = {}

function RemoteWebView:new(frame, cb)
    local obj = setmetatable({}, { __index = self })
    obj.frame = frame
    obj.cb = cb
    return obj
end

function RemoteWebView:close()
    CEF_LIB.LuaCEF_Close(self.frame)
end

function RemoteWebView:addOnAfterCreateCallback(cb)
    CEF_LIB.LuaCEF_AddOnAfterCreateCallback(self.frame, ffi.cast("frame_callback_t", function(...)
        local success, err = pcall(cb, ...)
        if not success then
            print(err)
        end
    end))
end

function RemoteWebView:addOnLoadingCallback(cb)
    CEF_LIB.LuaCEF_AddOnLoadingCallback(self.frame, ffi.cast("loading_callback_t", function(...)
        local success, err = pcall(cb, ...)
        if not success then
            print(err)
        end
    end))
end

function RemoteWebView:addOnCloseCallback(cb)
    CEF_LIB.LuaCEF_AddOnCloseCallback(self.frame, ffi.cast("frame_callback_t", function(...)
        local success, err = pcall(cb, ...)
        if not success then
            print(err)
        end
    end))
end

function RemoteWebView:getTexture()
    return CEF_LIB.LuaCEF_GetTexture(self.frame)
end

function RemoteWebView:isInputActive()
    return CEF_LIB.LuaCEF_IsInputActive(self.frame)
end

function RemoteWebView:setInput(enable)
    CEF_LIB.LuaCEF_SetInputActive(self.frame, enable)
end

function RemoteWebView:setRenderEnabled(enable)
    CEF_LIB.LuaCEF_SetRenderEnabled(self.frame, enable)
end

function RemoteWebView:isRenderEnabled()
    return CEF_LIB.LuaCEF_IsRenderEnabled(self.frame)
end

function RemoteWebView:setOffscreenRenderingEnabled(enable)
    CEF_LIB.LuaCEF_SetOffscreenRenderingEnabled(self.frame, enable)
end

function RemoteWebView:isOffscreenRenderingEnabled()
    return CEF_LIB.LuaCEF_IsOffscreenRenderingEnabled(self.frame)
end

function RemoteWebView:getURL()
    return ffi.string(CEF_LIB.LuaCEF_GetURL(self.frame))
end

function RemoteWebView:loadURL(url)
    CEF_LIB.LuaCEF_LoadURL(self.frame, url)
end

function RemoteWebView:reloadPage(ignoreCache)
    CEF_LIB.LuaCEF_ReloadPage(self.frame, ignoreCache)
end

function RemoteWebView:goBack()
    CEF_LIB.LuaCEF_GoBack(self.frame)
end

function RemoteWebView:goForward()
    CEF_LIB.LuaCEF_GoForward(self.frame)
end

function RemoteWebView:executeJS(code)
    CEF_LIB.LuaCEF_ExecuteJS(self.frame, code)
end

function RemoteWebView:addBindFunction(name, cb)
    CEF_LIB.LuaCEF_AddBindFunction(self.frame, name, ffi.cast("bind_function_callback_t", function(_, args)
        local success, err = pcall(cb, _, decodeJson(encoding.UTF8:decode(ffi.string(args))))
        if not success then
            print(err)
        end
    end))
end

local MODULE_ = {
    frames_ = {}
}

MODULE_.VERSION = "1.0.0"

function MODULE_.inited()
    return CEF_LIB.LuaCEF_IsInitialized()
end

function MODULE_.isAnyInputActive()
    return CEF_LIB.IsAnyInputActive()
end

function MODULE_.create(url, x, y, width, height, cb)
    local frame = CEF_LIB.LuaCEF_Create(url, x, y, width, height, ffi.cast("frame_callback_t", function(...)
        local success, err = pcall(cb, ...)
        if not success then
            print(err)
        end
    end))
    local obj = RemoteWebView:new(frame, cb)
    MODULE_.frames_[frame] = obj
    return obj
end

function MODULE_.createFullscreen(url, cb)
    local frame = CEF_LIB.LuaCEF_CreateFullscreen(url, ffi.cast("frame_callback_t", function(...)
        local success, err = pcall(cb, ...)
        if not success then
            print(err)
        end
    end))
    local obj = RemoteWebView:new(frame, cb)
    MODULE_.frames_[frame] = obj
    return obj
end

addEventHandler("onD3DPresent", function()
    CEF_LIB.LuaCEF_Update()
end)

addEventHandler("onD3DDeviceLost", function()
    CEF_LIB.LuaCEF_OnD3DLost()
end)

addEventHandler("onD3DDeviceReset", function()
    CEF_LIB.LuaCEF_OnD3DReset()
end)

addEventHandler("onScriptTerminate", function(scr)
    if scr == script.this then
        for frame, obj in pairs(MODULE_.frames_) do
            obj:close()
        end
    end
end)

jit.off(_, false)

return MODULE_
