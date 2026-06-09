#include "WebCore.hpp"

#include <filesystem>
#include <iostream>
#include <string>

#include <CSprite2d.h>

#include "utils/samp_utils.hpp"

#include "Logger.hpp"

void WebCore::Initialize(IDirect3DDevice9* device)
{
    device_ = device;

    processManager_ = std::make_unique<ProcessManager>();
    if (!processManager_->Start())
    {
        Logger::Instance().Log("Failed to start CEF subprocess");
        device_ = nullptr;
        return;
    }

    // Подождать пока subprocess инициализируется (max 5 секунд)
    for (int i = 0; i < 50; ++i)
    {
        Sleep(100);

        // Создать IPC клиент и подключиться к subprocess
        ipcClient_ = std::make_shared<ipc::IPCClient>();
        if (ipcClient_->Connect())
        {
            Logger::Instance().Log("WebCore initialized, connected to subprocess");

            // Запустить поток для получения сообщений
            StartMessageReceiveThread();
            return;
        }
    }

    Logger::Instance().Log("Failed to connect to CEF subprocess (timeout)");
    processManager_->Stop();
    processManager_ = nullptr;
    device_ = nullptr;
}

WebCore::~WebCore()
{
    Shutdown();
}

void WebCore::Shutdown()
{
    messageThreadRunning_ = false;
    if (messageThread_.joinable())
    {
        messageThread_.join();
    }

    for (auto* view : webViews_)
    {
        delete view;
    }

    webViews_.clear();
    webviewIdMap_.clear();

    ipcClient_.reset();

    if (processManager_)
    {
        processManager_->Stop();
        processManager_.reset();
    }
}

RemoteWebView* WebCore::Create(const std::string& url, int x, int y, int w, int h,
                               RemoteWebView::FrameCallback onCreate)
{
    ipc::CreateWebViewCmd cmd{};

    cmd.id = nextBrowserId_++;
    cmd.x = x;
    cmd.y = y;

    cmd.width = w;
    cmd.height = h;

    strcpy_s(cmd.url, sizeof(cmd.url), url.c_str());

    auto* webView = new RemoteWebView(ipcClient_, cmd.id, device_, x, y, w, h, onCreate);
    webViews_.push_back(webView);

    webviewIdMap_[cmd.id] = webView;

    AddTaskToMessageThread(
        [this, cmd]()
        {
            Logger::Instance().Log("Sending CreateWebView command");
            if (!ipcClient_->Send(ipc::CommandType::CreateWebView, cmd))
            {
                Logger::Instance().Log("Failed to send CreateWebView");

                return;
            }
            Logger::Instance().Log("CreateWebView command sent");
        });

    Logger::Instance().Log("WebView created with id {}", cmd.id);

    return webView;
}

void WebCore::Close(RemoteWebView* view)
{
    auto it = std::find(webViews_.begin(), webViews_.end(), view);
    if (it != webViews_.end())
    {
        if (view->IsClosing())
            return;

        view->Close();

        ipc::CloseWebViewCmd cmd{};
        cmd.id = view->GetBrowserId();

        AddTaskToMessageThread(
            [this, cmd]()
            {
                Logger::Instance().Log("Sending CloseWebView command");
                if (!ipcClient_->Send(ipc::CommandType::CloseWebView, cmd))
                {
                    Logger::Instance().Log("Failed to send CloseWebView command");
                }
                Logger::Instance().Log("CloseWebView command sent");
            });
    }
}

void WebCore::LoadURL(RemoteWebView* view, const std::string& url)
{
    auto it = std::find(webViews_.begin(), webViews_.end(), view);
    if (it != webViews_.end())
    {
        (*it)->SetURL(url);

        ipc::LoadURLCmd cmd{};
        cmd.id = view->GetBrowserId();
        strcpy_s(cmd.url, sizeof(cmd.url), url.c_str());

        AddTaskToMessageThread(
            [this, cmd]()
            {
                Logger::Instance().Log("Sending LoadURL command");
                if (!ipcClient_->Send(ipc::CommandType::LoadURL, cmd))
                {
                    Logger::Instance().Log("Failed to send LoadURL command");
                }
                Logger::Instance().Log("LoadURL command sent");
            });
    }
}

void WebCore::ReloadPage(RemoteWebView* view, bool ignoreCache)
{
    auto it = std::find(webViews_.begin(), webViews_.end(), view);
    if (it != webViews_.end())
    {
        ipc::ReloadPageCmd cmd{};
        cmd.id = view->GetBrowserId();
        cmd.ignoreCache = ignoreCache;

        AddTaskToMessageThread(
            [this, cmd]()
            {
                Logger::Instance().Log("Sending ReloadPage command");
                if (!ipcClient_->Send(ipc::CommandType::ReloadPage, cmd))
                {
                    Logger::Instance().Log("Failed to send ReloadPage command");
                }
                Logger::Instance().Log("ReloadPage command sent");
            });
    }
}

void WebCore::GoBack(RemoteWebView* view)
{
    auto it = std::find(webViews_.begin(), webViews_.end(), view);
    if (it != webViews_.end())
    {
        ipc::GoBackCmd cmd{};
        cmd.id = view->GetBrowserId();

        AddTaskToMessageThread(
            [this, cmd]()
            {
                Logger::Instance().Log("Sending GoBack command");
                if (!ipcClient_->Send(ipc::CommandType::GoBack, cmd))
                {
                    Logger::Instance().Log("Failed to send GoBack command");
                }
                Logger::Instance().Log("GoBack command sent");
            });
    }
}

void WebCore::GoForward(RemoteWebView* view)
{
    auto it = std::find(webViews_.begin(), webViews_.end(), view);
    if (it != webViews_.end())
    {
        ipc::GoForwardCmd cmd{};
        cmd.id = view->GetBrowserId();

        AddTaskToMessageThread(
            [this, cmd]()
            {
                Logger::Instance().Log("Sending GoForward command");
                if (!ipcClient_->Send(ipc::CommandType::GoForward, cmd))
                {
                    Logger::Instance().Log("Failed to send GoForward command");
                }
                Logger::Instance().Log("GoForward command sent");
            });
    }
}

void WebCore::ExecuteJS(RemoteWebView* view, const std::string& code)
{
    auto it = std::find(webViews_.begin(), webViews_.end(), view);
    if (it != webViews_.end())
    {
        ipc::ExecuteJSCmd cmd{};
        cmd.id = view->GetBrowserId();
        strcpy_s(cmd.code, sizeof(cmd.code), code.c_str());

        AddTaskToMessageThread(
            [this, cmd]()
            {
                Logger::Instance().Log("Sending ExecuteJS command");
                if (!ipcClient_->Send(ipc::CommandType::ExecuteJS, cmd))
                {
                    Logger::Instance().Log("Failed to send ExecuteJS command");
                }
                Logger::Instance().Log("ExecuteJS command sent");
            });
    }
}

bool WebCore::IsAnyInputActive() const
{
    for (auto* view : webViews_)
    {
        if (view->InputEnabled())
            return true;
    }

    return false;
}

void WebCore::Update(std::shared_ptr<IRenderManager> renderManager)
{
    ProcessGameThreadTasks();

    ProcessInput();

    UpdateTextures();
    Render(renderManager);
}

void WebCore::Render(std::shared_ptr<IRenderManager> renderManager)
{
    // TODO: fix SF & white screen
    for (auto& view : webViews_)
    {
        if (view->OffscreenRenderingEnabled() || !view->RenderEnabled())
            continue;

        // if (!view->IsLoaded())
        //     continue;

        if (auto texture = view->GetTexture())
        {
            renderManager->RenderTexture(view->GetTexture(), view->GetX(), view->GetY(), view->GetWidth(),
                                         view->GetHeight());
        }
    }
}

void WebCore::UpdateTextures()
{

    for (auto& view : webViews_)
        view->Update();
}

bool WebCore::OnWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    bool retval = false;

    if (msg == WM_CHAR)
    {
        wchar_t wch;
        MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, reinterpret_cast<char*>(&wParam), 1, &wch, 1);
        wParam = wch;
    }

    if (msg == WM_ACTIVATEAPP)
    {
        const auto gameActive = wParam != 0;

        // fix for games that don't have cursor
        if (!gameActive)
        {
            if (IsAnyInputActive())
                samp_utils::ToggleCursor(false);
        }

        ipc::PauseCmd cmd{};
        cmd.paused = !gameActive;

        AddTaskToMessageThread([this, cmd] { ipcClient_->Send(ipc::CommandType::Pause, cmd); });
    }

    for (auto& [id, webView] : webviewIdMap_)
    {
        if (webView->IsClosing())
            continue;

        if (msg == WM_MOUSEMOVE)
        {
            POINT point = {LOWORD(lParam), HIWORD(lParam)};
            ScreenToClient(hwnd, &point);

            ipc::MouseMoveCmd cmd{};
            cmd.id = id;
            cmd.x = point.x - webView->GetX();
            cmd.y = point.y - webView->GetY();

            AddTaskToMessageThread(
                [this, cmd]()
                {
                    if (!ipcClient_->Send(ipc::CommandType::MouseMove, cmd))
                    {
                        Logger::Instance().Log("Failed to send MouseMove command");
                    }
                });

            continue;
        }
        else if (msg >= WM_LBUTTONDOWN && msg <= WM_MBUTTONUP)
        {
            POINT point = {LOWORD(lParam), HIWORD(lParam)};
            ScreenToClient(hwnd, &point);

            {
                POINT p;
                GetCursorPos(&p);
                ScreenToClient(hwnd, &p);

                bool inside = p.x >= webView->GetX() && p.x < webView->GetX() + webView->GetWidth() &&
                              p.y >= webView->GetY() && p.y < webView->GetY() + webView->GetHeight();
                webView->SetFocused(inside);
            }

            if (!webView->IsFocused())
                continue;

            ipc::MouseClickCmd cmd{};
            cmd.id = id;
            cmd.x = point.x - webView->GetX();
            cmd.y = point.y - webView->GetY();

            switch (msg)
            {
            case WM_LBUTTONDOWN:
                cmd.mouseUp = false;
                cmd.button = 0;
                break;
            case WM_LBUTTONUP:
                cmd.mouseUp = true;
                cmd.button = 0;
                break;
            case WM_MBUTTONDOWN:
                cmd.mouseUp = false;
                cmd.button = 1;
                break;
            case WM_MBUTTONUP:
                cmd.mouseUp = true;
                cmd.button = 1;
                break;
            case WM_RBUTTONDOWN:
                cmd.mouseUp = false;
                cmd.button = 2;
                break;
            case WM_RBUTTONUP:
                cmd.mouseUp = true;
                cmd.button = 2;
                break;
            default:
                retval = false;
                continue;
            }

            AddTaskToMessageThread(
                [this, cmd]()
                {
                    if (!ipcClient_->Send(ipc::CommandType::MouseClick, cmd))
                    {
                        Logger::Instance().Log("Failed to send MouseClick command");
                    }
                });
            retval = true;
            continue;
        }
        else if (msg == WM_MOUSEWHEEL)
        {
            if (!webView->IsFocused())
                continue;

            POINT point = {LOWORD(lParam), HIWORD(lParam)};
            ScreenToClient(hwnd, &point);

            ipc::MouseWheelCmd cmd{};
            cmd.id = id;
            cmd.x = point.x - webView->GetX();
            cmd.y = point.y - webView->GetY();
            cmd.delta = GET_WHEEL_DELTA_WPARAM(wParam);

            AddTaskToMessageThread(
                [this, cmd]()
                {
                    if (!ipcClient_->Send(ipc::CommandType::MouseWheel, cmd))
                    {
                        Logger::Instance().Log("Failed to send MouseWheel command");
                    }
                });
            retval = true;
            continue;
        }

        if (msg >= WM_KEYDOWN && msg <= WM_CHAR)
        {
            if (!webView->IsFocused())
                continue;

            int keycode = static_cast<int>(wParam);
            int modifiers = 0;

            if (GetKeyState(VK_SHIFT) & 0x8000)
                modifiers |= 1 << 1;
            if (GetKeyState(VK_CONTROL) & 0x8000)
                modifiers |= 1 << 2;
            if (GetKeyState(VK_MENU) & 0x8000)
                modifiers |= 1 << 3;

            ipc::KeyCmd cmd{};
            cmd.id = id;
            cmd.keycode = keycode;
            cmd.nativeKeycode = static_cast<int>(lParam);
            cmd.modifiers = modifiers;

            if (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN)
            {
                cmd.type = 0;
            }
            else if (msg == WM_KEYUP || msg == WM_SYSKEYUP)
            {
                cmd.type = 2;
            }
            else if (msg == WM_CHAR)
            {
                cmd.type = 3;
            }

            AddTaskToMessageThread(
                [this, cmd]()
                {
                    if (!ipcClient_->Send(ipc::CommandType::Key, cmd))
                    {
                        Logger::Instance().Log("Failed to send Key command");
                    }
                });

            retval = true;
            continue;
        }
    }
    return retval;
}

void WebCore::OnDeviceLost()
{
    for (auto& view : webViews_)
        view->OnDeviceLost();
}

void WebCore::OnDeviceReset()
{
    for (auto& view : webViews_)
        view->OnDeviceReset();
}

void WebCore::StartMessageReceiveThread()
{
    messageThreadRunning_ = true;
    messageThread_ = std::thread([this]() { MessageReceiveThreadProc(); });
}

void WebCore::MessageReceiveThreadProc()
{
    Logger::Instance().Log("Message thread started");

    while (messageThreadRunning_)
    {
        if (!ipcClient_ || !ipcClient_->IsConnected())
            break;

        ProcessMessageThreadTasks();

        if (!ipcClient_->HasData())
            continue;

        ipc::MessageHeader header{};

        if (!ipcClient_->ReceiveHeader(header))
        {
            Logger::Instance().Log("Receive failed");
            continue;
        }

        switch (header.type)
        {
        case ipc::CommandType::CreateWebViewResponse:
        {
            if (header.payloadSize != sizeof(ipc::CreateWebViewResponse))
            {
                Logger::Instance().Log("Invalid CreateWebViewResponse payload size");
                break;
            }

            ipc::CreateWebViewResponse response{};

            if (!ipcClient_->ReceivePayload(response))
            {
                Logger::Instance().Log("Failed to receive CreateWebViewResponse payload");
                break;
            }

            const uint32_t browserId = response.id;

            Logger::Instance().Log("Attaching browser with id {}", browserId);

            AddTaskToGameThread(
                [this, browserId]()
                {
                    auto it = webviewIdMap_.find(browserId);
                    if (it != webviewIdMap_.end())
                    {
                        it->second->AttachBrowser();
                    }
                });

            break;
        }

        case ipc::CommandType::CloseWebViewResponse:
        {
            if (header.payloadSize != sizeof(ipc::CloseWebViewResponse))
            {
                Logger::Instance().Log("Invalid CloseWebViewResponse payload size");
                break;
            }

            ipc::CloseWebViewResponse response{};

            if (!ipcClient_->ReceivePayload(response))
            {
                Logger::Instance().Log("Failed to receive CloseWebViewResponse payload");
                break;
            }

            const uint32_t browserId = response.id;

            Logger::Instance().Log("Closing browser with id {}", browserId);

            AddTaskToGameThread(
                [this, browserId]()
                {
                    auto it = webviewIdMap_.find(browserId);

                    if (it != webviewIdMap_.end())
                    {
                        auto* view = it->second;

                        view->OnClose();

                        webViews_.erase(std::remove(webViews_.begin(), webViews_.end(), view), webViews_.end());

                        webviewIdMap_.erase(it);

                        delete view;
                    }
                });

            break;
        }

        case ipc::CommandType::LoadingResponse:
        {
            if (header.payloadSize != sizeof(ipc::LoadingResponse))
            {
                Logger::Instance().Log("Invalid LoadingResponse payload size");
                break;
            }

            ipc::LoadingResponse response{};

            if (!ipcClient_->ReceivePayload(response))
            {
                Logger::Instance().Log("Failed to receive LoadingResponse payload");
                break;
            }

            const uint32_t browserId = response.id;

            Logger::Instance().Log("Loading browser with id {}", browserId);

            AddTaskToGameThread(
                [this, browserId, response]()
                {
                    auto it = webviewIdMap_.find(browserId);
                    if (it != webviewIdMap_.end())
                    {
                        it->second->OnLoading(response.httpStatusCode);
                    }
                });

            break;
        }

        case ipc::CommandType::AfterCreateResponse:
        {
            if (header.payloadSize != sizeof(ipc::AfterCreateResponse))
            {
                Logger::Instance().Log("Invalid AfterCreateResponse payload size");
                break;
            }

            ipc::AfterCreateResponse response{};

            if (!ipcClient_->ReceivePayload(response))
            {
                Logger::Instance().Log("Failed to receive AfterCreateResponse payload");
                break;
            }

            const uint32_t browserId = response.id;

            Logger::Instance().Log("AfterCreateResponse browser with id {}", browserId);

            AddTaskToGameThread(
                [this, browserId]()
                {
                    auto it = webviewIdMap_.find(browserId);
                    if (it != webviewIdMap_.end())
                    {
                        it->second->OnAfterCreate();
                    }
                });

            break;
        }

        case ipc::CommandType::URLChangedResponse:
        {
            if (header.payloadSize != sizeof(ipc::URLChangedResponse))
            {
                Logger::Instance().Log("Invalid URLChangedResponse payload size");
                break;
            }

            ipc::URLChangedResponse response{};

            if (!ipcClient_->ReceivePayload(response))
            {
                Logger::Instance().Log("Failed to receive URLChangedResponse payload");
                break;
            }

            const uint32_t browserId = response.id;
            const std::string url = response.url;

            Logger::Instance().Log("URLChangedResponse browser with id {}", browserId);

            AddTaskToGameThread(
                [this, browserId, url]()
                {
                    auto it = webviewIdMap_.find(browserId);
                    if (it != webviewIdMap_.end())
                    {
                        it->second->OnURLChanged(url);
                    }
                });

            break;
        }

        case ipc::CommandType::BindFunctionResponse:
        {
            if (header.payloadSize != sizeof(ipc::BindFunctionResponse))
            {
                Logger::Instance().Log("Invalid BindFunctionResponse payload size");
                break;
            }

            ipc::BindFunctionResponse response{};

            if (!ipcClient_->ReceivePayload(response))
            {
                Logger::Instance().Log("Failed to receive BindFunctionResponse payload");
                break;
            }

            const uint32_t browserId = response.browserId;
            const std::string function = response.function;
            const std::string args = response.args;

            Logger::Instance().Log("BindFunctionResponse browser with id {}", browserId);

            AddTaskToGameThread(
                [this, browserId, function, args]()
                {
                    auto it = webviewIdMap_.find(browserId);
                    if (it != webviewIdMap_.end())
                    {
                        it->second->OnBindFunctionCall(function, args);
                    }
                });

            break;
        }

        default:
        {
            // std::array<uint8_t, 8192> dummy{};

            // if (header.payloadSize)
            // {
            //     ipcClient_->ReceiveRaw(dummy.data(), header.payloadSize);
            // }

            break;
        }
        }
        Sleep(10);
    }
}

void WebCore::ProcessGameThreadTasks()
{
    std::lock_guard lock(gameThreadTasks_.mutex);

    for (auto& task : gameThreadTasks_.tasks)
    {
        task();
    }

    gameThreadTasks_.tasks.clear();
}

void WebCore::AddTaskToGameThread(std::function<void()> task)
{
    std::lock_guard lock(gameThreadTasks_.mutex);

    gameThreadTasks_.tasks.push_back(task);
}

void WebCore::ProcessMessageThreadTasks()
{
    std::lock_guard lock(messageThreadTasks_.mutex);

    for (auto& task : messageThreadTasks_.tasks)
    {
        task();
    }

    messageThreadTasks_.tasks.clear();
}

void WebCore::AddTaskToMessageThread(std::function<void()> task)
{
    std::lock_guard lock(messageThreadTasks_.mutex);

    messageThreadTasks_.tasks.push_back(task);
}

void WebCore::ProcessInput()
{
    if (!IsAnyInputActive())
        return;

    samp_utils::ToggleCursor(true);
}