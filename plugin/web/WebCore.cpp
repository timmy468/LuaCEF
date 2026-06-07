#include "WebCore.hpp"

#include <filesystem>
#include <iostream>
#include <string>

#include <CSprite2d.h>

#include "utils/samp_utils.hpp"

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
        ipcClient_ = std::make_shared<ipc::IPCClient>();
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
        view->Close();

        ipc::CloseWebViewCmd cmd{};
        cmd.id = view->GetBrowserId();

        AddTaskToMessageThread(
            [this, cmd]()
            {
                std::println(std::cout, "Sending CloseWebView command");
                if (!ipcClient_->Send(ipc::CommandType::CloseWebView, cmd))
                {
                    std::cerr << "Failed to send CloseWebView command" << std::endl;
                }
                std::println(std::cout, "CloseWebView command sent");
            });

        // todo: handle this
        // delete *it;
        // webViews_.erase(it);
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

void WebCore::Update()
{
    ProcessGameThreadTasks();

    ProcessInput();

    UpdateTextures();
    Render();
}

void WebCore::Render()
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
            std::println(std::cout, "Rendering texture {} {} {} {}", view->GetX(), view->GetY(), view->GetWidth(),
                         view->GetHeight());

            static RwRaster* raster{};
            if (!raster)
            {
                raster = RwRasterCreate(view->GetWidth(), view->GetHeight(), 0, rwRASTERTYPETEXTURE);
            }

            // RasterExtOffset
            {
                auto offset = *reinterpret_cast<IDirect3DTexture9***>(0xB4E9E0);
                auto& rasterTexture = *reinterpret_cast<IDirect3DTexture9**>((uintptr_t)raster + (uintptr_t)offset);
                rasterTexture = texture;
            }

            RwRenderStateSet(rwRENDERSTATETEXTURERASTER, raster);

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

bool WebCore::OnWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_CHAR)
    {
        wchar_t wch;
        MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, reinterpret_cast<char*>(&wParam), 1, &wch, 1);
        wParam = wch;
    }

    for (auto& [id, webView] : webviewIdMap_)
    {
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
                        std::cerr << "Failed to send MouseMove command" << std::endl;
                    }
                });

            return true;
        }
        else if (msg >= WM_LBUTTONDOWN && msg <= WM_MBUTTONUP)
        {
            POINT point = {LOWORD(lParam), HIWORD(lParam)};
            ScreenToClient(hwnd, &point);

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
                return false;
            }

            AddTaskToMessageThread(
                [this, cmd]()
                {
                    if (!ipcClient_->Send(ipc::CommandType::MouseClick, cmd))
                    {
                        std::cerr << "Failed to send MouseClick command" << std::endl;
                    }
                });
            return true;
        }
        else if (msg == WM_MOUSEWHEEL)
        {
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
                        std::cerr << "Failed to send MouseWheel command" << std::endl;
                    }
                });
            return true;
        }

        if (msg >= WM_KEYDOWN && msg <= WM_CHAR)
        {
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
                        std::cerr << "Failed to send Key command" << std::endl;
                    }
                });

            return true;
        }
    }
    return false;
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