#include "WebCore.hpp"

#include <filesystem>
#include <iostream>
#include <string>

#include <CSprite2d.h>

RemoteWebView* gWebView = nullptr;

void WebCore::Initialize(IDirect3DDevice9* device)
{
    device_ = device;

    // todo: test
    gWebView = new RemoteWebView(device_, 0, 0, 1280, 720);
    webViews_.push_back(gWebView);
}

WebCore::~WebCore()
{
    Shutdown();
}

void WebCore::Shutdown()
{
    // for (auto& view : webViews_)
    //     Close(view);

    // Don't call CefShutdown() here to avoid freeze.
    // TerminateProcess (during quit) is called before CCore destruction anyways.
    // CefShutdown();
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

// WebView* WebCore::Create(const std::string& url, int x, int y, int w, int h, WebView::FrameCallback onCreate)
// {
//     auto webView = new WebView(device_, x, y, w, h, onCreate);
//     webView->LoadURL(url);

//     webViews_.push_back(webView);

//     return webView;
// }

// void WebCore::Close(WebView* view)
// {
//     auto it = std::find(webViews_.begin(), webViews_.end(), view);
//     if (it != webViews_.end())
//     {
//         (*it)->Close();
//         delete *it;
//         webViews_.erase(it);
//     }
// }

void WebCore::Update()
{
    UpdateTextures();
    Render();
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