#include "ProcessManager.hpp"
#include <filesystem>
#include <iostream>

#include "Logger.hpp"

ProcessManager::ProcessManager() = default;

ProcessManager::~ProcessManager()
{
    Stop();
}

bool ProcessManager::Start()
{
    if (IsRunning())
    {
        Logger::Instance().Log("Subprocess already running (PID: {})", process_id_);
        return true;
    }

    subprocess_path_ = GetSubprocessPath();
    if (subprocess_path_.empty())
    {
        Logger::Instance().Log("Failed to find subprocess executable");
        return false;
    }

    Logger::Instance().Log("Subprocess path: {}", subprocess_path_);

    // Создать процесс
    STARTUPINFOA startup_info{};
    startup_info.cb = sizeof(STARTUPINFOA);
    // todo: fix
    // startup_info.dwFlags = STARTF_USESHOWWINDOW;
    startup_info.wShowWindow = SW_HIDE; // Скрыть окно subprocess

    PROCESS_INFORMATION process_info{};

    if (!CreateProcessA(subprocess_path_.c_str(),
                        nullptr, // Command line
                        nullptr, // Process attributes
                        nullptr, // Thread attributes
                        FALSE,   // Inherit handles
                        CREATE_NEW_CONSOLE,
                        nullptr, // Environment
                        nullptr, // Current directory
                        &startup_info, &process_info))
    {
        Logger::Instance().Log("Failed to create subprocess process: {}", GetLastError());
        return false;
    }

    process_handle_ = process_info.hProcess;
    process_id_ = process_info.dwProcessId;

    CloseHandle(process_info.hThread); // Закрыть handle главного потока

    Logger::Instance().Log("Subprocess created (PID: {})", process_id_);

    // Запустить поток для мониторинга
    monitor_running_ = true;
    monitor_thread_ = std::thread([this]() { MonitorThreadProc(); });

    return true;
}

void ProcessManager::Stop()
{
    if (!IsRunning())
        return;

    Logger::Instance().Log("Stopping subprocess (PID: {})", process_id_);

    monitor_running_ = false;

    if (process_handle_ != nullptr)
    {
        // Попробовать завершить процесс gracefully
        TerminateProcess(process_handle_, 0);

        // Ждать завершения
        WaitForSingleObject(process_handle_, 5000);

        CloseHandle(process_handle_);
        process_handle_ = nullptr;
    }

    process_id_ = 0;

    if (monitor_thread_.joinable())
    {
        monitor_thread_.join();
    }

    Logger::Instance().Log("Subprocess stopped (PID: {})", process_id_);
}

bool ProcessManager::IsRunning() const
{
    if (process_handle_ == nullptr)
        return false;

    DWORD exit_code = 0;
    if (!GetExitCodeProcess(process_handle_, &exit_code))
        return false;

    return exit_code == STILL_ACTIVE;
}

std::string ProcessManager::GetSubprocessPath()
{
    // Искать cef-subprocess.exe в текущей директории или в bin/Release
    std::vector<std::string> possible_paths = {
        "luacef/cef-host.exe",
    };

    for (const auto& path : possible_paths)
    {
        if (std::filesystem::exists(path))
        {
            Logger::Instance().Log("Found subprocess at: {}", path);
            return std::filesystem::absolute(path).string();
        }
    }

    return "";
}

void ProcessManager::MonitorThreadProc()
{
    while (monitor_running_)
    {
        if (!IsRunning())
        {
            Logger::Instance().Log("Subprocess crashed or terminated unexpectedly");
            process_handle_ = nullptr;
            process_id_ = 0;
            break;
        }

        Sleep(500); // Проверять каждые 500ms
    }
}