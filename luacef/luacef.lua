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
    bool LuaCEF_IsFocused(RemoteWebView view);
    void LuaCEF_SetFocused(RemoteWebView view, bool enable);
    void LuaCEF_SetEnabledFocusMode(RemoteWebView view, bool enable);
    bool LuaCEF_IsEnabledFocusMode(RemoteWebView view);
    void LuaCEF_SetRenderEnabled(RemoteWebView view, bool enable);
    bool LuaCEF_IsRenderEnabled(RemoteWebView view);
    void LuaCEF_SetOffscreenRenderingEnabled(RemoteWebView view, bool enable);
    bool LuaCEF_IsOffscreenRenderingEnabled(RemoteWebView view);

    const char* LuaCEF_GetURL(RemoteWebView view);
    void LuaCEF_LoadURL(RemoteWebView view, const char* url);
    void LuaCEF_SetRect(RemoteWebView* view, int x, int y, int width, int height);

    void LuaCEF_ReloadPage(RemoteWebView view, bool ignoreCache);
    void LuaCEF_GoBack(RemoteWebView view);
    void LuaCEF_GoForward(RemoteWebView view);

    void LuaCEF_ExecuteJS(RemoteWebView view, const char* code);
    void LuaCEF_SetFPS(RemoteWebView view, int fps);

    void LuaCEF_ClearCallbacks(RemoteWebView view);
]]

if getModuleHandle("luacef.asi") == 0 then
    error("plugin not found")
end

local MODULE_ = {
    _frames = {},
    _lib = ffi.load("luacef.asi")
}

---@class RemoteWebView
local RemoteWebView = {}

function RemoteWebView:_release()
    self._callbacks = {}
end

function RemoteWebView:_storeCallback(cb)
    if not self._callbacks then
        self._callbacks = {}
    end

    table.insert(self._callbacks, cb)
    return cb
end

function RemoteWebView:new(frame, cb)
    local obj = setmetatable({}, { __index = self })
    obj._frame = frame

    obj._callbacks = {}

    return obj
end

---Закрыть браузер.
function RemoteWebView:close()
    if self._closed then
        return
    end

    self._closed = true

    MODULE_._lib.LuaCEF_Close(self._frame)

    MODULE_._frames[self._frame] = nil
end

---Добавить callback, вызываемый после создания браузера.
---@param cb fun(view: RemoteWebView)
function RemoteWebView:addOnAfterCreateCallback(cb)
    local callback = ffi.cast("frame_callback_t", function(...)
        local success, err = pcall(cb, ...)
        if not success then
            print(err)
        end
    end)

    self:_storeCallback(callback)

    MODULE_._lib.LuaCEF_AddOnAfterCreateCallback(self._frame, callback)
end

---Добавить callback окончания загрузки страницы.
---@param cb fun(view: RemoteWebView, httpStatusCode: integer)
function RemoteWebView:addOnLoadingCallback(cb)
    local callback = ffi.cast("loading_callback_t", function(...)
        local success, err = pcall(cb, ...)
        if not success then
            print(err)
        end
    end)

    self:_storeCallback(callback)

    MODULE_._lib.LuaCEF_AddOnLoadingCallback(self._frame, callback)
end

---Добавить callback закрытия браузера.
---@param cb fun(view: RemoteWebView)
function RemoteWebView:addOnCloseCallback(cb)
    local callback = ffi.cast("frame_callback_t", function(...)
        local success, err = pcall(cb, ...)
        if not success then
            print(err)
        end
    end)

    self:_storeCallback(callback)

    MODULE_._lib.LuaCEF_AddOnCloseCallback(self._frame, callback)
end

---Получить Direct3D текстуру браузера.
---@return userdata texture
function RemoteWebView:getTexture()
    return MODULE_._lib.LuaCEF_GetTexture(self._frame)
end

---Проверить активен ли ввод.
---@return boolean
function RemoteWebView:isInputActive()
    return MODULE_._lib.LuaCEF_IsInputActive(self._frame)
end

---Включить или выключить ввод.
---@param enable boolean
function RemoteWebView:setInput(enable)
    MODULE_._lib.LuaCEF_SetInputActive(self._frame, enable)
end

---Проверить активен ли ввод (в фокусе).
---Автоматически фокусируется при нажатии в области браузера.
---@return boolean
function RemoteWebView:isFocused()
    return MODULE_._lib.LuaCEF_IsFocused(self._frame)
end

---Включить или выключить фокус.
---@param enable boolean
function RemoteWebView:setFocused(enable)
    MODULE_._lib.LuaCEF_SetFocused(self._frame, enable)
end

---Включить режим фокусировки
---@param enable boolean
function RemoteWebView:setEnabledFocusMode(enable)
    MODULE_._lib.LuaCEF_SetEnabledFocusMode(self._frame, enable)
end

---Проверить включен ли режим фокусировки
---@return boolean
function RemoteWebView:isEnabledFocusMode()
    return MODULE_._lib.LuaCEF_IsEnabledFocusMode(self._frame)
end

---Включить или выключить рендеринг (вне OSR).
---@param enable boolean
function RemoteWebView:setRenderEnabled(enable)
    MODULE_._lib.LuaCEF_SetRenderEnabled(self._frame, enable)
end

---Проверить включен ли рендеринг.
---@return boolean
function RemoteWebView:isRenderEnabled()
    return MODULE_._lib.LuaCEF_IsRenderEnabled(self._frame)
end

---Включить или выключить offscreen rendering.
---@param enable boolean
function RemoteWebView:setOffscreenRenderingEnabled(enable)
    MODULE_._lib.LuaCEF_SetOffscreenRenderingEnabled(self._frame, enable)
end

---Проверить используется ли offscreen rendering.
---@return boolean
function RemoteWebView:isOffscreenRenderingEnabled()
    return MODULE_._lib.LuaCEF_IsOffscreenRenderingEnabled(self._frame)
end

---Получить текущий URL.
---@return string
function RemoteWebView:getURL()
    return ffi.string(MODULE_._lib.LuaCEF_GetURL(self._frame))
end

---Загрузить URL.
---@param url string
function RemoteWebView:loadURL(url)
    MODULE_._lib.LuaCEF_LoadURL(self._frame, url)
end

---Установить прямоугольник браузера.
---@param x integer
---@param y integer
---@param width integer
---@param height integer
function RemoteWebView:setRect(x, y, width, height)
    MODULE_._lib.LuaCEF_SetRect(self._frame, x, y, width, height)
end

---Перезагрузить страницу.
---@param ignoreCache boolean
function RemoteWebView:reloadPage(ignoreCache)
    MODULE_._lib.LuaCEF_ReloadPage(self._frame, ignoreCache)
end

---Перейти назад.
function RemoteWebView:goBack()
    MODULE_._lib.LuaCEF_GoBack(self._frame)
end

---Перейти вперед.
function RemoteWebView:goForward()
    MODULE_._lib.LuaCEF_GoForward(self._frame)
end

---Выполнить JavaScript код.
---@param code string
function RemoteWebView:executeJS(code)
    MODULE_._lib.LuaCEF_ExecuteJS(self._frame, code)
end

---Зарегистрировать функцию для вызова из JavaScript.
---
---JS:
---```javascript
---lua.call("test", "hello", 123)
---```
---
---Lua:
---```lua
---view:addBindFunction("test", function(view, args)
---    print(args[1])
---    print(args[2])
---end)
---```
---
---@param name string
---@param cb fun(view: RemoteWebView, args: any[])
function RemoteWebView:addBindFunction(name, cb)
    local callback = ffi.cast(
        "bind_function_callback_t",
        function(_, args)
            local success, err = pcall(
                cb,
                _,
                decodeJson(
                    encoding.UTF8:decode(
                        ffi.string(args)
                    )
                )
            )

            if not success then
                print(err)
            end
        end
    )

    self:_storeCallback(callback)

    MODULE_._lib.LuaCEF_AddBindFunction(
        self._frame,
        name,
        callback
    )
end

---Установить FPS для браузера.
---@param fps integer
function RemoteWebView:setFPS(fps)
    MODULE_._lib.LuaCEF_SetFPS(self._frame, fps)
end

---Очистить все callback'и браузера.
function RemoteWebView:clearCallbacks()
    MODULE_._lib.LuaCEF_ClearCallbacks(self._frame)
    self._callbacks = {}
end

---Версия библиотеки.
---@type string
MODULE_.VERSION = "1.0.0"

---Проверить инициализирован ли LuaCEF.
---@return boolean
function MODULE_.inited()
    return MODULE_._lib.LuaCEF_IsInitialized()
end

---Проверить активен ли ввод хотя бы в одном браузере.
---@return boolean
function MODULE_.isAnyInputActive()
    return MODULE_._lib.IsAnyInputActive()
end

---Создать браузер.
---
---```lua
---local browser = LuaCEF.create(
---    "https://google.com",
---    100, 100,
---    800, 600,
---    function(view)
---        print("Created")
---    end
---)
---```
---
---@param url string
---@param x integer
---@param y integer
---@param width integer
---@param height integer
---@param cb? fun(view: RemoteWebView)
---@return RemoteWebView
function MODULE_.create(url, x, y, width, height, cb)
    local createCallback

    if cb then
        createCallback = ffi.cast(
            "frame_callback_t",
            function(...)
                local success, err = pcall(cb, ...)
                if not success then
                    print(err)
                end
            end
        )
    end

    local frame = MODULE_._lib.LuaCEF_Create(
        url,
        x,
        y,
        width,
        height,
        createCallback
    )

    local obj = RemoteWebView:new(frame)

    if createCallback then
        obj:_storeCallback(createCallback)
    end

    MODULE_._frames[frame] = obj

    return obj
end

---Создать полноэкранный браузер.
---@param url string
---@param cb? fun(view: RemoteWebView)
---@return RemoteWebView
function MODULE_.createFullscreen(url, cb)
    local createCallback

    if cb then
        createCallback = ffi.cast(
            "frame_callback_t",
            function(...)
                local success, err = pcall(cb, ...)
                if not success then
                    print(err)
                end
            end
        )
    end

    local frame = MODULE_._lib.LuaCEF_CreateFullscreen(
        url,
        createCallback
    )

    local obj = RemoteWebView:new(frame)

    if createCallback then
        obj:_storeCallback(createCallback)
    end

    MODULE_._frames[frame] = obj

    return obj
end

addEventHandler("onD3DPresent", function()
    MODULE_._lib.LuaCEF_Update()
end)

addEventHandler("onD3DDeviceLost", function()
    MODULE_._lib.LuaCEF_OnD3DLost()
end)

addEventHandler("onD3DDeviceReset", function()
    MODULE_._lib.LuaCEF_OnD3DReset()
end)

addEventHandler("onScriptTerminate", function(scr)
    if scr ~= script.this then
        return
    end

    for _, obj in pairs(MODULE_._frames) do
        pcall(function()
            obj:clearCallbacks()
            obj:close()
        end)
    end

    MODULE_._frames = {}
end)

jit.off(_, false)

return MODULE_
