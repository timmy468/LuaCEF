#pragma once

#include <Windows.h>
#include <memory>
#include <string>
#include <thread>

// Управление процессом subprocess
class ProcessManager
{
  public:
    ProcessManager();
    ~ProcessManager();

    // Запустить subprocess
    bool Start();

    // Остановить subprocess
    void Stop();

    // Проверить работает ли процесс
    bool IsRunning() const;

    // Получить PID процесса
    DWORD GetProcessId() const
    {
        return process_id_;
    }

    // Получить HANDLE процесса
    HANDLE GetProcessHandle() const
    {
        return process_handle_;
    }

  private:
    HANDLE process_handle_{nullptr};
    DWORD process_id_{0};
    std::string subprocess_path_;

    // Поток для мониторинга процесса
    std::thread monitor_thread_;
    bool monitor_running_{false};

    // Получить путь к subprocess
    std::string GetSubprocessPath();

    // Поток для мониторинга
    void MonitorThreadProc();
};