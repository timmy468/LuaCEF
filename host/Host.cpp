#include "Host.hpp"

#include <filesystem>
#include <format>
#include <thread>

#include <Windows.h>
#include <cef_app.h>

bool Host::Initialize()
{
    InitCef();

    // Создать обработчик IPC команд
    ipcHandler_ = std::make_shared<IPCBrowserHandler>();

    // Запустить IPC сервер
    ipcServer_ = std::make_shared<ipc::IPCServer>(ipcHandler_);

    // Запустить сервер в отдельном потоке
    std::thread server_thread([this]() { ipcServer_->Start(R"(\\.\pipe\LuaCEF_IPC)", 8); });
    server_thread.detach(); // Отпустить поток

    return true;
}

void Host::Run()
{
    while (true)
    {
        Sleep(100);
    }
}

void Host::InitCef()
{
    std::string work_path = std::filesystem::current_path().string() + "\\luacef";
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
}