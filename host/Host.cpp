#include "Host.hpp"

#include <filesystem>
#include <format>

#include <Windows.h>
#include <cef_app.h>

bool Host::Initialize()
{
    constexpr int id = 0; // TODO: generate unique ID for each view
    auto name = std::format("LuaCEF_Frame_{}", id);

    mapping_ = CreateFileMappingA(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, sizeof(SharedFrame), name.c_str());

    frame_ = static_cast<SharedFrame*>(MapViewOfFile(mapping_, FILE_MAP_ALL_ACCESS, 0, 0, 0));
    memset(frame_, 0, sizeof(SharedFrame));

    //
    std::string work_path = std::filesystem::current_path().string();
    std::filesystem::create_directories(work_path + "\\cache");

    CefSettings settings;

    CefString(&settings.browser_subprocess_path).FromASCII(std::string(work_path + "\\cef-subprocess.exe").c_str());
    CefString(&settings.log_file) = work_path + "\\luacef.log";
    CefString(&settings.cache_path).FromASCII(std::string(work_path + "\\cache").c_str());
    CefString(&settings.locales_dir_path).FromASCII(std::string(work_path + "\\locales").c_str());
    CefString(&settings.resources_dir_path) = work_path;
    CefString(&settings.locale).FromASCII("ru");
    CefString(&settings.accept_language_list).FromASCII("en-US,en,ru-RU,ru");

    settings.no_sandbox = 1;
    settings.multi_threaded_message_loop = 1;
    settings.windowless_rendering_enabled = 1;

    settings.background_color = 0;
    settings.log_severity = LOGSEVERITY_DEBUG;
    settings.command_line_args_disabled = 1;

    CefMainArgs args(GetModuleHandle(nullptr));

    CefInitialize(args, settings, nullptr, nullptr);

    // Create the browser view
    browserView_ = new BrowserView(frame_, 1280, 720);

    CefWindowInfo info;

    info.SetAsWindowless(nullptr);

    CefBrowserSettings browserSettings;

    CefBrowserHost::CreateBrowser(info, browserView_, "https://google.com", browserSettings, nullptr, nullptr);

    return true;
}

void Host::Run()
{
    while (true)
    {
        Sleep(100);
    }
}