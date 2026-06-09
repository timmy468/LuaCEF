#include "ProcessManager.hpp"

#include <filesystem>
#include <iostream>

#include <TlHelp32.h>

#include "Logger.hpp"

ProcessManager::ProcessManager() = default;

ProcessManager::~ProcessManager()
{
    Stop();
}

void ProcessManager::KillOldHosts()
{
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (snapshot == INVALID_HANDLE_VALUE)
        return;

    PROCESSENTRY32 entry{};
    entry.dwSize = sizeof(entry);

    if (Process32First(snapshot, &entry))
    {
        do
        {
            if (_stricmp(entry.szExeFile, "cef-host.exe") == 0)
            {
                HANDLE process = OpenProcess(PROCESS_TERMINATE | PROCESS_QUERY_INFORMATION, FALSE, entry.th32ProcessID);

                if (process)
                {
                    Logger::Instance().Log("Killing old cef-host.exe (PID: {})", entry.th32ProcessID);

                    TerminateProcess(process, 0);
                    WaitForSingleObject(process, 3000);

                    CloseHandle(process);
                }
            }
        } while (Process32Next(snapshot, &entry));
    }

    CloseHandle(snapshot);
}

bool ProcessManager::Start()
{
    if (IsRunning())
    {
        Logger::Instance().Log("Subprocess already running (PID: {})", process_id_);

        return true;
    }

    KillOldHosts();

    subprocess_path_ = GetSubprocessPath();

    if (subprocess_path_.empty())
    {
        Logger::Instance().Log("Failed to find subprocess executable");

        return false;
    }

    Logger::Instance().Log("Subprocess path: {}", subprocess_path_);

    STARTUPINFOA startup_info{};
    startup_info.cb = sizeof(startup_info);
    // startup_info.dwFlags = STARTF_USESHOWWINDOW;
    startup_info.wShowWindow = SW_HIDE;

    PROCESS_INFORMATION process_info{};

    if (!CreateProcessA(subprocess_path_.c_str(), nullptr, nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr,
                        &startup_info, &process_info))
    {
        Logger::Instance().Log("CreateProcess failed: {}", GetLastError());

        return false;
    }

    process_handle_ = process_info.hProcess;
    process_id_ = process_info.dwProcessId;

    CloseHandle(process_info.hThread);

    Logger::Instance().Log("Subprocess created (PID: {})", process_id_);

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

    Logger::Instance().Log("Stopping subprocess (PID: {})", process_id_);

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