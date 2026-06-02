#include "ProcessManager.hpp"
#include <filesystem>
#include <iostream>

ProcessManager::ProcessManager() = default;

ProcessManager::~ProcessManager()
{
    Stop();
}

bool ProcessManager::Start()
{
    if (IsRunning())
    {
        std::cout << "Subprocess already running (PID: " << process_id_ << ")" << std::endl;
        return true;
    }

    subprocess_path_ = GetSubprocessPath();
    if (subprocess_path_.empty())
    {
        std::cerr << "Failed to find subprocess executable" << std::endl;
        return false;
    }

    std::cout << "Starting subprocess: " << subprocess_path_ << std::endl;

    // Создать процесс
    STARTUPINFOA startup_info{};
    startup_info.cb = sizeof(STARTUPINFOA);
    startup_info.dwFlags = STARTF_USESHOWWINDOW;
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
        std::cerr << "Failed to create subprocess process: " << GetLastError() << std::endl;
        return false;
    }

    process_handle_ = process_info.hProcess;
    process_id_ = process_info.dwProcessId;

    CloseHandle(process_info.hThread); // Закрыть handle главного потока

    std::cout << "Subprocess started (PID: " << process_id_ << ")" << std::endl;

    // Запустить поток для мониторинга
    monitor_running_ = true;
    monitor_thread_ = std::thread([this]() { MonitorThreadProc(); });

    return true;
}

void ProcessManager::Stop()
{
    if (!IsRunning())
        return;

    std::cout << "Stopping subprocess..." << std::endl;

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

    std::cout << "Subprocess stopped" << std::endl;
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
        "cef-host.exe",
    };

    for (const auto& path : possible_paths)
    {
        if (std::filesystem::exists(path))
        {
            std::cout << "Found subprocess at: " << path << std::endl;
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
            std::cerr << "Subprocess crashed or terminated unexpectedly" << std::endl;
            process_handle_ = nullptr;
            process_id_ = 0;
            break;
        }

        Sleep(500); // Проверять каждые 500ms
    }
}