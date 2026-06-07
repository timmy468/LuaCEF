#include "IPCBrowserHandler.hpp"

#include <base/cef_bind.h>
#include <base/cef_callback.h>
#include <wrapper/cef_closure_task.h>

#include <iostream>

IPCBrowserHandler::IPCBrowserHandler()
{
}

IPCBrowserHandler::~IPCBrowserHandler() = default;

void IPCBrowserHandler::AddTask(std::function<void()> task)
{
    std::lock_guard lock(tasks_mutex_);

    tasks_.push_back(task);
}

void IPCBrowserHandler::HandleMessage(HANDLE pipe, ipc::CommandType type, const void* payload, uint32_t payloadSize)
{
    switch (type)
    {
    case ipc::CommandType::CreateWebView:
    {
        if (payloadSize != sizeof(ipc::CreateWebViewCmd))
            return;

        auto* cmd = static_cast<const ipc::CreateWebViewCmd*>(payload);

        std::println(std::cout, "IPC CreateWebView {}", cmd->url);

        auto browser = CefRefPtr<BrowserView>(new BrowserView(cmd->id, cmd->x, cmd->y, cmd->width, cmd->height));

        ipc::CreateWebViewResponse response{};

        response.id = browser->GetWebviewId();

        if (!ipc::IPCServer::Send(pipe, ipc::CommandType::CreateWebViewResponse, response))
        {
            std::cerr << "Failed to send CreateWebViewResponse" << std::endl;
        }

        RegisterBrowser(cmd->id, browser);

        const std::string url = cmd->url;

        CefPostTask(TID_UI, base::BindOnce(
                                [](CefRefPtr<BrowserView> browser, std::string url)
                                {
                                    // CefWindowInfo windowInfo;

                                    // windowInfo.SetAsPopup(nullptr, "LuaCEF Test");

                                    // CefBrowserSettings browserSettings;

                                    // CefBrowserHost::CreateBrowser(windowInfo, browser, url, browserSettings, nullptr,
                                    //                               nullptr);

                                    CefWindowInfo windowInfo;
                                    windowInfo.SetAsWindowless(nullptr);

                                    CefBrowserSettings browserSettings;

                                    CefBrowserHost::CreateBrowser(windowInfo, browser, url, browserSettings, nullptr,
                                                                  nullptr);
                                },
                                browser, url));

        std::println(std::cout, "BrowserView creation requested. id {}, url {}, width {}, height {}", cmd->id, cmd->url,
                     cmd->width, cmd->height);

        break;
    }

    case ipc::CommandType::CloseWebView:
    {
        if (payloadSize != sizeof(ipc::CloseWebViewCmd))
            return;

        auto* cmd = static_cast<const ipc::CloseWebViewCmd*>(payload);

        auto it = browsers_.find(cmd->id);

        if (it != browsers_.end())
        {
            if (auto browser = it->second->GetBrowser())
            {
                // todo: response
                CefPostTask(TID_UI, base::BindOnce([](CefRefPtr<CefBrowser> browser)
                                                   { browser->GetHost()->CloseBrowser(true); }, browser));
            }

            UnregisterBrowser(cmd->id);
        }

        break;
    }

    case ipc::CommandType::SetRect:
    {
        if (payloadSize != sizeof(ipc::SetRectCmd))
            return;

        auto* cmd = static_cast<const ipc::SetRectCmd*>(payload);

        auto it = browsers_.find(cmd->id);

        if (it != browsers_.end())
        {
            CefPostTask(TID_UI, base::BindOnce([](CefRefPtr<BrowserView> browser, int x, int y, int width, int height)
                                               { browser->SetRect(x, y, width, height); }, it->second, cmd->x, cmd->y,
                                               cmd->width, cmd->height));
        }

        break;
    }

    case ipc::CommandType::LoadURL:
    {
        if (payloadSize != sizeof(ipc::LoadURLCmd))
            return;

        auto* cmd = static_cast<const ipc::LoadURLCmd*>(payload);

        auto it = browsers_.find(cmd->id);

        if (it != browsers_.end())
        {
            if (auto browser = it->second->GetBrowser())
            {
                const std::string url = cmd->url;
                // Todo: response
                CefPostTask(TID_UI, base::BindOnce([](CefRefPtr<CefBrowser> browser, std::string url)
                                                   { browser->GetMainFrame()->LoadURL(url); }, browser, url));
            }
        }

        break;
    }

    case ipc::CommandType::ReloadPage:
    {
        if (payloadSize != sizeof(ipc::ReloadPageCmd))
            return;

        auto* cmd = static_cast<const ipc::ReloadPageCmd*>(payload);

        auto it = browsers_.find(cmd->id);

        if (it != browsers_.end())
        {
            if (auto browser = it->second->GetBrowser())
            {
                const bool ignoreCache = cmd->ignoreCache;

                CefPostTask(TID_UI, base::BindOnce(
                                        [](CefRefPtr<CefBrowser> browser, bool ignoreCache)
                                        {
                                            if (ignoreCache)
                                                browser->ReloadIgnoreCache();
                                            else
                                                browser->Reload();
                                        },
                                        browser, ignoreCache));
            }
        }

        break;
    }

    case ipc::CommandType::GoBack:
    {
        if (payloadSize != sizeof(ipc::GoBackCmd))
            return;

        auto* cmd = static_cast<const ipc::GoBackCmd*>(payload);

        auto it = browsers_.find(cmd->id);

        if (it != browsers_.end())
        {
            if (auto browser = it->second->GetBrowser())
            {
                CefPostTask(TID_UI, base::BindOnce([](CefRefPtr<CefBrowser> browser) { browser->GoBack(); }, browser));
            }
        }

        break;
    }

    case ipc::CommandType::GoForward:
    {
        if (payloadSize != sizeof(ipc::GoForwardCmd))
            return;

        auto* cmd = static_cast<const ipc::GoForwardCmd*>(payload);

        auto it = browsers_.find(cmd->id);

        if (it != browsers_.end())
        {
            if (auto browser = it->second->GetBrowser())
            {
                CefPostTask(TID_UI,
                            base::BindOnce([](CefRefPtr<CefBrowser> browser) { browser->GoForward(); }, browser));
            }
        }

        break;
    }

    case ipc::CommandType::ExecuteJS:
    {
        if (payloadSize != sizeof(ipc::ExecuteJSCmd))
            return;

        auto* cmd = static_cast<const ipc::ExecuteJSCmd*>(payload);

        auto it = browsers_.find(cmd->id);

        if (it != browsers_.end())
        {
            if (auto browser = it->second->GetBrowser())
            {
                CefPostTask(TID_UI, base::BindOnce(
                                        [](CefRefPtr<CefBrowser> browser, const std::string& code)
                                        {
                                            if (auto mainFrame = browser->GetMainFrame())
                                            {
                                                mainFrame->ExecuteJavaScript(code, {}, 0);
                                            }
                                        },
                                        browser, cmd->code));
            }
        }

        break;
    }

        // case ipc::CommandType::Ping:
        // {
        //     if (payloadSize != sizeof(ipc::PingCmd))
        //         return;

        //     auto* cmd = static_cast<const ipc::PingCmd*>(payload);

        //     ipc::PongCmd pong{};

        //     pong.timestamp = cmd->timestamp;

        //     server_->SendToClient(pipe, ipc::CommandType::Pong, pong);

        //     break;
        // }

    case ipc::CommandType::MouseMove:
    {
        auto* cmd = static_cast<const ipc::MouseMoveCmd*>(payload);

        std::println(std::cout, "MouseMove command received with id {}, x: {}, y: {}", cmd->id, cmd->x, cmd->y);

        auto it = browsers_.find(cmd->id);
        if (it != browsers_.end())
        {
            if (auto browser = it->second->GetBrowser())
            {
                CefPostTask(TID_UI, base::BindOnce(
                                        [](CefRefPtr<CefBrowser> browser, int x, int y)
                                        {
                                            if (auto host = browser->GetHost())
                                            {
                                                CefMouseEvent mouseEvent;

                                                mouseEvent.x = x;
                                                mouseEvent.y = y;
                                                mouseEvent.modifiers = 0;

                                                host->SendMouseMoveEvent(mouseEvent, false);
                                            }
                                        },
                                        browser, cmd->x, cmd->y));
            }
        }

        break;
    }
    case ipc::CommandType::MouseClick:
    {
        auto* cmd = static_cast<const ipc::MouseClickCmd*>(payload);

        std::println(std::cout, "MouseClick command received with id {}, x: {}, y: {}, button: {}, down: {}", cmd->id,
                     cmd->x, cmd->y, cmd->button, cmd->mouseUp);

        auto it = browsers_.find(cmd->id);
        if (it != browsers_.end())
        {
            if (auto browser = it->second->GetBrowser())
            {
                CefPostTask(TID_UI, base::BindOnce(
                                        [](CefRefPtr<CefBrowser> browser, int x, int y,
                                           CefBrowserHost::MouseButtonType button, bool mouseUp)
                                        {
                                            if (auto host = browser->GetHost())
                                            {
                                                CefMouseEvent mouseEvent;

                                                mouseEvent.x = x;
                                                mouseEvent.y = y;
                                                mouseEvent.modifiers = 0;

                                                host->SendMouseClickEvent(mouseEvent, button, mouseUp, 1);
                                            }
                                        },
                                        browser, cmd->x, cmd->y,
                                        static_cast<CefBrowserHost::MouseButtonType>(cmd->button), cmd->mouseUp));
            }
        }

        break;
    }

    case ipc::CommandType::MouseWheel:
    {
        auto* cmd = static_cast<const ipc::MouseWheelCmd*>(payload);

        std::println(std::cout, "MouseWheel command received with id {}, x: {}, y: {}, delta: {}", cmd->id, cmd->x,
                     cmd->y, cmd->delta);

        auto it = browsers_.find(cmd->id);
        if (it != browsers_.end())
        {
            if (auto browser = it->second->GetBrowser())
            {
                CefPostTask(TID_UI, base::BindOnce(
                                        [](CefRefPtr<CefBrowser> browser, int x, int y, int delta)
                                        {
                                            if (auto host = browser->GetHost())
                                            {
                                                CefMouseEvent mouseEvent;

                                                mouseEvent.x = x;
                                                mouseEvent.y = y;
                                                mouseEvent.modifiers = 0;

                                                host->SendMouseWheelEvent(mouseEvent, 0, delta);
                                            }
                                        },
                                        it->second->GetBrowser(), cmd->x, cmd->y, cmd->delta));
            }
        }

        break;
    }
    case ipc::CommandType::Key:
    {
        auto* cmd = static_cast<const ipc::KeyCmd*>(payload);
        std::println(std::cout,
                     "Key command received with id {}, keycode: {}, nativeKeycode: {}, modifiers: {}, type: {}",
                     cmd->id, cmd->keycode, cmd->nativeKeycode, cmd->modifiers, cmd->type);

        auto it = browsers_.find(cmd->id);
        if (it != browsers_.end())
        {
            if (auto browser = it->second->GetBrowser())
            {
                CefPostTask(TID_UI,
                            base::BindOnce(
                                [](CefRefPtr<CefBrowser> browser, int32_t keycode, int32_t nativeKeycode,
                                   int32_t modifiers, int32_t type)
                                {
                                    if (auto host = browser->GetHost())
                                    {
                                        CefKeyEvent event;

                                        event.windows_key_code = keycode;
                                        event.native_key_code = nativeKeycode;
                                        event.modifiers = modifiers;
                                        event.type = static_cast<cef_key_event_type_t>(type);

                                        host->SendKeyEvent(event);
                                    }
                                },
                                it->second->GetBrowser(), cmd->keycode, cmd->nativeKeycode, cmd->modifiers, cmd->type));
            }
        }

        break;
    }

    default:
    {
        std::println(std::cerr, "Unknown IPC command {}", static_cast<int>(type));

        break;
    }
    }
}

void IPCBrowserHandler::RegisterBrowser(uint32_t id, CefRefPtr<BrowserView> handler)
{
    browsers_[id] = handler;
}

void IPCBrowserHandler::UnregisterBrowser(uint32_t id)
{
    browsers_.erase(id);
}