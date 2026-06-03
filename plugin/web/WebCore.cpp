#include "WebCore.hpp"

#include <filesystem>
#include <iostream>
#include <string>

#include <CSprite2d.h>

void WebCore::Initialize(IDirect3DDevice9* device)
{
    device_ = device;

    processManager_ = std::make_unique<ProcessManager>();
    if (!processManager_->Start())
    {
        std::cerr << "Failed to start CEF subprocess" << std::endl;
        device_ = nullptr;
        return;
    }

    // Подождать пока subprocess инициализируется (max 5 секунд)
    for (int i = 0; i < 50; ++i)
    {
        Sleep(100);

        // Создать IPC клиент и подключиться к subprocess
        ipcClient_ = std::make_unique<ipc::IPCClient>();
        if (ipcClient_->Connect())
        {
            std::cout << "WebCore initialized, connected to subprocess" << std::endl;

            // Запустить поток для получения сообщений
            StartMessageReceiveThread();
            return;
        }
    }

    std::cerr << "Failed to connect to CEF subprocess (timeout)" << std::endl;
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
    if (processManager_)
    {
        processManager_->Stop();
        processManager_ = nullptr;
    }

    // for (auto& view : webViews_)
    //     Close(view);
}

// bool WebCore::IsAnyInputActive() const
// {
//     for (auto* view : webViews_)
//     {
//         if (view->InputEnabled())
//             return true;
//     }

//     return false;
// }

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

    auto* webView = new RemoteWebView(cmd.id, device_, x, y, w, h, onCreate);
    webViews_.push_back(webView);

    webviewIdMap_[cmd.id] = webView;

    AddTaskToMessageThread(
        [this, cmd]()
        {
            std::println(std::cout, "Sending CreateWebView command");
            if (!ipcClient_->Send(ipc::CommandType::CreateWebView, cmd))
            {
                std::cerr << "Failed to send CreateWebView" << std::endl;

                return;
            }
            std::println(std::cout, "CreateWebView command sent");
        });

    std::println(std::cout, "WebView created with id {}", cmd.id);

    return webView;
}

void WebCore::Close(RemoteWebView* view)
{
    auto it = std::find(webViews_.begin(), webViews_.end(), view);
    if (it != webViews_.end())
    {
        (*it)->Close();
        delete *it;
        webViews_.erase(it);
    }
}

void WebCore::Update()
{
    ProcessGameThreadTasks();
    UpdateTextures();
    // Render();
}

void WebCore::Render()
{
    // TODO: fix SF & white screen
    for (auto& view : webViews_)
    {
        if (view->OffscreenRenderingEnabled())
            continue;

        // if (!view->IsLoaded())
        //     continue;

        if (auto texture = view->GetTexture())
        {
            std::println(std::cout, "Rendering texture {} {} {} {}", view->GetX(), view->GetY(), view->GetWidth(),
                         view->GetHeight());

            device_->SetTexture(0, texture);

            CRect rect;
            rect.left = view->GetX();
            rect.top = view->GetY();
            rect.right = view->GetX() + view->GetWidth();
            rect.bottom = view->GetY() + view->GetHeight();
            CSprite2d::DrawTxRect(rect, {255, 255, 255, 255});
        }
    }
}

void WebCore::UpdateTextures()
{

    for (auto& view : webViews_)
        view->Update();
}

// bool WebCore::OnWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
// {
//     if (msg == WM_CHAR)
//     {
//         wchar_t wch;
//         MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, reinterpret_cast<char*>(&wParam), 1, &wch, 1);
//         wParam = wch;
//     }

//     for (auto& webView : webViews_)
//     {
//         auto browser = webView->GetBrowser();

//         if (!browser)
//             continue;

//         auto host = browser->GetHost();

//         if (!host)
//             continue;

//         if (msg >= WM_MOUSEMOVE && msg <= WM_MBUTTONUP || msg == WM_MOUSEWHEEL)
//         {
//             POINT point = {LOWORD(lParam), HIWORD(lParam)};
//             ScreenToClient(hwnd, &point);

//             CefMouseEvent mouse_event;

//             mouse_event.x = point.x;
//             mouse_event.y = point.y;
//             mouse_event.modifiers = 0;

//             switch (msg)
//             {
//             case WM_LBUTTONDOWN:
//                 host->SendMouseClickEvent(mouse_event, MBT_LEFT, false, 1);
//                 break;
//             case WM_LBUTTONUP:
//                 host->SendMouseClickEvent(mouse_event, MBT_LEFT, true, 1);
//                 break;
//             case WM_RBUTTONDOWN:
//                 host->SendMouseClickEvent(mouse_event, MBT_RIGHT, false, 1);
//                 break;
//             case WM_RBUTTONUP:
//                 host->SendMouseClickEvent(mouse_event, MBT_RIGHT, true, 1);
//                 break;
//             case WM_MBUTTONDOWN:
//                 host->SendMouseClickEvent(mouse_event, MBT_MIDDLE, false, 1);
//                 break;
//             case WM_MBUTTONUP:
//                 host->SendMouseClickEvent(mouse_event, MBT_MIDDLE, true, 1);
//                 break;
//             case WM_MOUSEWHEEL:
//             {
//                 int delta = GET_WHEEL_DELTA_WPARAM(wParam);
//                 host->SendMouseWheelEvent(mouse_event, 0, delta);
//                 break;
//             }
//             default:
//             {
//                 host->SendMouseMoveEvent(mouse_event, false);
//                 break;
//             }
//             }
//             return true;
//         }

//         if (msg >= WM_KEYDOWN && msg <= WM_CHAR)
//         {
//             int keycode = static_cast<int>(wParam);
//             int modifiers = 0;

//             if (GetKeyState(VK_SHIFT) & 0x8000)
//                 modifiers |= EVENTFLAG_SHIFT_DOWN;
//             if (GetKeyState(VK_CONTROL) & 0x8000)
//                 modifiers |= EVENTFLAG_CONTROL_DOWN;
//             if (GetKeyState(VK_MENU) & 0x8000)
//                 modifiers |= EVENTFLAG_ALT_DOWN;

//             CefKeyEvent event;

//             event.windows_key_code = keycode;
//             event.native_key_code = static_cast<int>(lParam);
//             event.modifiers = modifiers;

//             if (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN)
//             {
//                 event.type = KEYEVENT_RAWKEYDOWN;
//             }
//             else if (msg == WM_KEYUP || msg == WM_SYSKEYUP)
//             {
//                 event.type = KEYEVENT_KEYUP;
//             }
//             else if (msg == WM_CHAR)
//             {
//                 event.type = KEYEVENT_CHAR;
//             }

//             host->SendKeyEvent(event);

//             return true;
//         }
//     }
//     return false;
// }

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
    std::thread t([this]() { MessageReceiveThreadProc(); });
    t.detach();
}

void WebCore::MessageReceiveThreadProc()
{
    std::cout << "Message thread started\n";

    while (messageThreadRunning_)
    {
        if (!ipcClient_ || !ipcClient_->IsConnected())
            break;

        ProcessMessageThreadTasks();

        if (!ipcClient_->HasData())
            continue;

        ipc::MessageHeader header{};

        std::cout << "Waiting header...\n";
        if (!ipcClient_->ReceiveHeader(header))
        {
            std::cout << "Receive failed\n";
            continue;
        }

        std::cout << "Header received\n";

        switch (header.type)
        {
        case ipc::CommandType::CreateWebViewResponse:
        {
            if (header.payloadSize != sizeof(ipc::CreateWebViewResponse))
            {
                std::cerr << "Invalid CreateWebViewResponse payload size" << std::endl;
                break;
            }

            ipc::CreateWebViewResponse response{};

            if (!ipcClient_->ReceivePayload(response))
            {
                std::cerr << "Failed to receive CreateWebViewResponse payload" << std::endl;
                break;
            }

            const uint32_t browserId = response.id;

            std::cout << "Attaching browser with id " << browserId << std::endl;

            AddTaskToGameThread([this, browserId]() { webviewIdMap_[browserId]->AttachBrowser(); });
        }
            // case ipc::CommandType::FrameReady:
            // {
            //     if (header.payloadSize != sizeof(ipc::FrameReadyCmd))
            //     {
            //         std::cerr << "Invalid FrameReady payload size" << std::endl;

            //         break;
            //     }

            //     ipc::FrameReadyCmd frameReady{};

            //     if (!ipcClient_->ReceivePayload(frameReady))
            //     {
            //         std::cerr << "Failed to receive FrameReady payload" << std::endl;

            //         break;
            //     }

            //     auto it = webviewIdMap_.find(frameReady.id);

            //     if (it != webviewIdMap_.end())
            //     {
            //         it->second->OnRemoteFrameReady(frameReady.width, frameReady.height,
            //         frameReady.shared_memory_offset,
            //                                        frameReady.frame_size);
            //     }

            //     break;
            // }

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