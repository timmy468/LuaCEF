local ffi = require("ffi")

ffi.cdef [[
    typedef void* WebView;
    typedef void* Texture;

    typedef void(__stdcall* frame_callback_t)(WebView view);

    bool LuaCEF_IsInitialized();
    bool IsAnyInputActive();
    WebView LuaCEF_Create(const char* url, int x, int y, int width, int height, frame_callback_t onCreate);
    WebView LuaCEF_CreateFullscreen(const char* url, frame_callback_t onCreate);
    void LuaCEF_Close(WebView view);

    Texture LuaCEF_GetTexture(WebView view);

    bool LuaCEF_IsInputActive(WebView view);
    void LuaCEF_SetInput(WebView view, bool enable);
    const char* LuaCEF_GetURL(WebView view);
    void LuaCEF_LoadURL(WebView view, const char* url);
]]

if getModuleHandle("luacef.asi") == 0 then
    error("plugin not found")
end

local CEF_LIB = ffi.load("luacef.asi")

local WebView = {}

function WebView:new(frame, cb)
    local obj = setmetatable({}, { __index = self })
    obj.frame = frame
    obj.cb = cb
    return obj
end

function WebView:close()
    CEF_LIB.LuaCEF_Close(self.frame)
end

function WebView:getTexture()
    return CEF_LIB.LuaCEF_GetTexture(self.frame)
end

function WebView:isInputActive()
    return CEF_LIB.LuaCEF_IsInputActive(self.frame)
end

function WebView:setInput(enable)
    CEF_LIB.LuaCEF_SetInput(self.frame, enable)
end

function WebView:getURL()
    return ffi.string(CEF_LIB.LuaCEF_GetURL(self.frame))
end

function WebView:loadURL(url)
    CEF_LIB.LuaCEF_LoadURL(self.frame, url)
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
    local frame = CEF_LIB.LuaCEF_Create(url, x, y, width, height, ffi.cast("frame_callback_t", cb))
    local obj = WebView:new(frame, cb)
    MODULE_.frames_[frame] = obj
    return obj
end

function MODULE_.createFullscreen(url, cb)
    local frame = CEF_LIB.LuaCEF_CreateFullscreen(url, ffi.cast("frame_callback_t", cb))
    local obj = WebView:new(frame, cb)
    MODULE_.frames_[frame] = obj
    return obj
end

return MODULE_
