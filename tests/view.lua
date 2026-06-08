local luacef = require("luacef")
local imgui = require("mimgui")

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

    webView = luacef.create(
        "file:///c:/Games/Advance Games/testcef/index.html", 500, 500, 500, 500, function(_)
            webView:setInput(true)
            sampAddChatMessage("WebView created!", 0x00FF00)
        end)

    webView:addBindFunction("test", function(_, args)
        for k, v in pairs(args) do
            print(k, v)
        end
    end)

    webView:addBindFunction("inputChanged", function(_, args)
        sampAddChatMessage("Input: " .. args[1], -1)
    end)

    webView:addOnLoadingCallback(function(_, httpStatusCode)
        sampAddChatMessage("Loading status: " .. httpStatusCode, 0x00FF00)
        webView:executeJS([[document.getElementById("title").style.color = "red";]])
    end)

    -- webView:addOnCloseCallback(function(_)
    --     sampAddChatMessage("WebView closed!", 0x00FF00)
    -- end)


    -- "file:///C:/Users/egaaa/Videos/36094040.mp4"

    webView2 = luacef.create(
        "https://threejs.org/examples/#webgl_loader_gltf", 0, 0, 500, 500, function(_)
            -- webView:setInput(true)
            sampAddChatMessage("WebView2 created!", 0x00FF00)
            -- webView:loadURL("https://www.youtube.com/")
        end)

    wait(-1)
end

local ffi = require("ffi")

imgui.OnFrame(
    function() return true end,
    function(player)
        if webView ~= nil then
            -- local w, h = getScreenResolution()
            -- local texture = webView:getTexture()
            -- imgui.GetBackgroundDrawList():AddImage(texture, imgui.ImVec2(500, 500),
            --     imgui.ImVec2(500, 500) + imgui.ImVec2(500, 500))

            -- local texture2 = webView2:getTexture()
            -- imgui.GetBackgroundDrawList():AddImage(texture2, imgui.ImVec2(0, 0),
            --     imgui.ImVec2(000, 000) + imgui.ImVec2(w / 2, h / 2))
        end
    end
).HideCursor = true
