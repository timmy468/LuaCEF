#include "ProcessManager.hpp"

#include <filesystem>
#include <format>
#include <iostream>

#include <TlHelp32.h>

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
        Logger::Instance().Log("Subprocess already running (PID: {})", process_id_.load());

        return true;
    }

    subprocess_path_ = GetSubprocessPath();

    if (subprocess_path_.empty())
    {
        Logger::Instance().Log("Failed to find subprocess executable");

        return false;
    }

    Logger::Instance().Log("Subprocess path: {}", subprocess_path_);

    STARTUPINFOA startup_info{};
    startup_info.cb = sizeof(startup_info);
    startup_info.dwFlags = STARTF_USESHOWWINDOW;
#ifdef NDEBUG
    startup_info.wShowWindow = SW_HIDE;
#endif

    PROCESS_INFORMATION process_info{};

    const auto parentPid = GetCurrentProcessId();
    std::string cmdLine = std::format("\"{}\" --parent-pid={}", subprocess_path_, parentPid);

    if (!CreateProcessA(nullptr, cmdLine.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr,
                        &startup_info, &process_info))
    {
        Logger::Instance().Log("CreateProcess failed: {}", GetLastError());

        return false;
    }

    Logger::Instance().Log("Subprocess created (PID: {}), cmdline: {}", process_id_.load(), cmdLine);

    process_handle_ = process_info.hProcess;
    process_id_ = process_info.dwProcessId;

    CloseHandle(process_info.hThread);

    Logger::Instance().Log("Subprocess created (PID: {})", process_id_.load());

    monitor_running_ = true;

    monitor_thread_ = std::thread([this]() { MonitorThreadProc(); });

    return true;
}

void ProcessManager::Stop()
{
    monitor_running_ = false;

    if (monitor_thread_.joinable())
    {
        monitor_thread_.join();
    }

    if (!process_handle_)
        return;

    Logger::Instance().Log("Stopping subprocess (PID: {})", process_id_.load());

    DWORD exit_code{};

    if (GetExitCodeProcess(process_handle_, &exit_code) && exit_code == STILL_ACTIVE)
    {
        WaitForSingleObject(process_handle_, 1000);

        GetExitCodeProcess(process_handle_, &exit_code);

        if (exit_code == STILL_ACTIVE)
        {
            Logger::Instance().Log("Force terminating subprocess");

            TerminateProcess(process_handle_, 0);

            WaitForSingleObject(process_handle_, 5000);
        }
    }

    CloseHandle(process_handle_);

    process_handle_ = nullptr;
    process_id_ = 0;

    Logger::Instance().Log("Subprocess stopped");
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
        HANDLE process = process_handle_;

        if (!process)
            break;

        DWORD exit_code{};

        if (!GetExitCodeProcess(process, &exit_code))
        {
            Logger::Instance().Log("GetExitCodeProcess failed");

            break;
        }

        if (exit_code != STILL_ACTIVE)
        {
            Logger::Instance().Log("Subprocess exited with code {}", exit_code);

            break;
        }

        Sleep(500);
    }
}