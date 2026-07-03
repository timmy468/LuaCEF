---@diagnostic disable: undefined-global

local luacef = require("luacef")
local imgui = require("mimgui")
local vkeys = require("vkeys")
local windowsMessage = require("windows.message")

local webView = nil
local webView2 = nil
local webView3 = nil

function main()
    while not isSampAvailable() do
        wait(100)
    end

    while not luacef.inited() do
        wait(100)
    end

    sampAddChatMessage("LuaCEF version: " .. luacef.VERSION, 0xFFFF00)

    local w, h = getScreenResolution()
    local notificationW = 300
    local notificationY = 
    webView = luacef.create(
        "http://localhost:5173/", 500, 500, 500, 500, function(_)
            sampAddChatMessage("[1] WebView created!", 0x00FF00)
        end)

    webView:setEnabledFocusMode(false)
    webView:setRenderEnabled(false)

    webView:addBindFunction("test", function(_, args)
        for k, v in pairs(args) do
            print(k, v)
        end
    end)

    webView:addBindFunction("inputChanged", function(_, args)
        sampAddChatMessage("Input: " .. args[1], -1)
    end)

    webView:addOnLoadingCallback(function(_, httpStatusCode)
        sampAddChatMessage("[1] Loading status: " .. httpStatusCode, 0x00FF00)

        if httpStatusCode == 20 then
            webView:setInput(true)
            webView:setFPS(120)
            webView:executeJS([[document.getElementById("title").style.color = "red";]])
        end
    end)

    -- webView:addOnCloseCallback(function(_)
    --     sampAddChatMessage("WebView closed!", 0x00FF00)
    -- end)

    -- "file:///C:/Users/egaaa/Videos/36094040.mp4"

    -- webView2 = luacef.create(
    --     "https://music.wntn.ru", 0, 0, 500, 500, function(_)
    --         sampAddChatMessage("[2] WebView created!", 0x00FF00)
    --         -- webView:loadURL("https://www.youtube.com/")
    --     end)


    -- webView2:addOnLoadingCallback(function(_, httpStatusCode)
    --     webView2:setInput(true)
    --     webView2:setFPS(120)
    --     sampAddChatMessage("[2] Loading status: " .. httpStatusCode, 0x00FF00)
    -- end)

    -- webView3 = luacef.create(
    --     "http://192.168.0.197:5173/", 0, 0, 500, 500, function(_)
    --         sampAddChatMessage("[3] WebView created!", 0x00FF00)
    --         -- webView:loadURL("https://www.youtube.com/")
    --     end)


    -- webView3:addOnLoadingCallback(function(_, httpStatusCode)
    --     webView3:setInput(true)
    --     webView3:setFPS(120)
    --     sampAddChatMessage("[3] Loading status: " .. httpStatusCode, 0x00FF00)
    -- end)

    wait(-1)
end

imgui.OnFrame(
    function() return true end,
    function(player)
        if webView ~= nil then
            local w, h = getScreenResolution()
            local viewW, viewH = 500, 100
            local paddingX, paddingY = 30, 30
            local start = imgui.ImVec2(w - paddingX - viewW, h - paddingY - viewH)
            local texture = webView:getTexture()
            imgui.GetBackgroundDrawList():AddImage(texture, start,
                start + imgui.ImVec2(viewW, viewH))

            -- local texture2 = webView2:getTexture()
            -- imgui.GetBackgroundDrawList():AddImage(texture2, imgui.ImVec2(0, 0),
            --     imgui.ImVec2(000, 000) + imgui.ImVec2(w / 2, h / 2))
        end
    end
).HideCursor = true

addEventHandler("onWindowMessage", function(msg, wparam, lparam)
    if msg == windowsMessage.WM_KEYDOWN or msg == windowsMessage.WM_KEYUP then
        if wparam == vkeys.VK_F9 then
            if webView then
                consumeWindowMessage(true, false)
                if msg == windowsMessage.WM_KEYUP then
                    -- webView:close()
                    -- webView2:close()

                    -- if webView:isRenderEnabled() then
                    --     webView:setRenderEnabled(false)
                    --     webView:setInput(false)
                    -- else
                    --     webView:setRenderEnabled(true)
                    --     webView:setInput(true)
                    -- end

                    -- if webView2:isRenderEnabled() then
                    --     webView2:setRenderEnabled(false)
                    --     webView2:setInput(false)
                    -- else
                    --     webView2:setRenderEnabled(true)
                    --     webView2:setInput(true)
                    -- end
                end
            end
        end
    end
end)
