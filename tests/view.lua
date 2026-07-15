---@diagnostic disable: undefined-global

local luacef = require("luacef")
local imgui = require("mimgui")
local vkeys = require("vkeys")
local windowsMessage = require("windows.message")

local webView = nil
local webView2 = nil

function main()
    while not isSampAvailable() do
        wait(100)
    end

    while not luacef.inited() do
        wait(100)
    end

    sampAddChatMessage("LuaCEF version: " .. luacef.VERSION, 0xFFFF00)

    local pathUrl = getWorkingDirectory() .. "\\resource\\testcef\\index.html"
    webView = luacef.create(
        pathUrl, 500, 500, 500, 500, function(_)
            sampAddChatMessage("[1] WebView created!", 0x00FF00)
        end)

    webView:setEnabledFocusMode(false)
    webView:setOffscreenRenderingEnabled(true)

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

        if httpStatusCode == 200 then
            webView:setFPS(120)
            webView:setInput(true)
            webView:executeJS([[document.getElementById("title").style.color = "red";]])
        end
    end)

    webView:addOnCloseCallback(function(_)
        sampAddChatMessage("WebView closed!", 0x00FF00)
        notifications:remove(webView)
    end)

    --
    webView2 = luacef.create(
        "https://music.wntn.ru", 0, 0, 500, 500, function(_)
            sampAddChatMessage("[2] WebView created!", 0x00FF00)
            -- webView:loadURL("https://www.youtube.com/")
        end)

    webView2:addOnLoadingCallback(function(_, httpStatusCode)
        sampAddChatMessage("[2] Loading status: " .. httpStatusCode, 0x00FF00)
        if httpStatusCode == 200 then
            webView2:setFPS(120)
            webView2:setInput(true)
        end
    end)

    wait(-1)
end

imgui.OnFrame(
    function() return true end,
    function(player)
        if webView ~= nil then
            local size = imgui.ImVec2(500, 500)
            local start = imgui.ImVec2(500, 500)
            local texture = webView:getTexture()
            imgui.GetBackgroundDrawList():AddImage(texture, start,
                start + size)
        end
    end
).HideCursor = true

addEventHandler("onWindowMessage", function(msg, wparam, lparam)
    if msg == windowsMessage.WM_KEYDOWN or msg == windowsMessage.WM_KEYUP then
        if wparam == vkeys.VK_F9 then
            if webView2 then
                consumeWindowMessage(true, false)
                if msg == windowsMessage.WM_KEYUP then
                    if webView2:isRenderEnabled() then
                        webView2:setRenderEnabled(false)
                        webView2:setInput(false)
                    else
                        webView2:setRenderEnabled(true)
                        webView2:setInput(true)
                    end
                end
            end
        end
    end
end)
