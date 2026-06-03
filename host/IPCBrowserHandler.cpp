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

        auto browser = CefRefPtr<BrowserView>(new BrowserView(cmd->id, cmd->width, cmd->height));
        // browser->AddOnCreateCallback(
        //     [pipe, this](BrowserView* view)
        //     {
        //         ipc::CreateWebViewResponse response{};

        //         response.id = view->GetWebviewId();

        //         std::cout << "Sending CreateWebViewResponse" << std::endl;
        //         if (!ipc::IPCServer::Send(pipe, ipc::CommandType::CreateWebViewResponse, response))
        //         {
        //             std::cerr << "Failed to send CreateWebViewResponse" << std::endl;
        //         }
        //         else
        //         {
        //             std::cout << "CreateWebViewResponse sent" << std::endl;
        //         }
        //     });

        ipc::CreateWebViewResponse response{};

        response.id = browser->GetWebviewId();

        std::cout << "Sending CreateWebViewResponse" << std::endl;
        if (!ipc::IPCServer::Send(pipe, ipc::CommandType::CreateWebViewResponse, response))
        {
            std::cerr << "Failed to send CreateWebViewResponse" << std::endl;
        }
        else
        {
            std::cout << "CreateWebViewResponse sent" << std::endl;
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
                CefPostTask(TID_UI, base::BindOnce([](CefRefPtr<CefBrowser> browser)
                                                   { browser->GetHost()->CloseBrowser(true); }, browser));
            }

            UnregisterBrowser(cmd->id);
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
            auto browser = it->second->GetBrowser();

            if (browser)
            {
                browser->GetMainFrame()->LoadURL(cmd->url);
            }
        }

        break;
    }

        // case ipc::CommandType::ExecuteJS:
        // {
        //     if (payloadSize != sizeof(ipc::ExecuteJSCmd))
        //         return;

        //     auto* cmd = static_cast<const ipc::ExecuteJSCmd*>(payload);

        //     auto it = browsers_.find(cmd->id);

        //     if (it != browsers_.end())
        //     {
        //         auto browser = it->second->GetBrowser();

        //         if (browser)
        //         {
        //             browser->GetMainFrame()->ExecuteJavaScript(cmd->code, {}, 0);
        //         }
        //     }

        //     break;
        // }

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