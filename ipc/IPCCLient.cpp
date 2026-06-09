#include "IPCClient.hpp"
#include <iostream>

namespace ipc
{

IPCClient::IPCClient() : pipe_handle_(INVALID_HANDLE_VALUE)
{
}

IPCClient::~IPCClient()
{
    Disconnect();
}

bool IPCClient::Connect(const std::string& pipe_name)
{
    pipe_name_ = pipe_name;

    // Подождать пока сервер запустится (max 30 секунд)
    for (int i = 0; i < 100; ++i)
    {
        pipe_handle_ =
            CreateFileA(pipe_name_.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);

        if (pipe_handle_ != INVALID_HANDLE_VALUE)
        {
            // Установить режим блокирующего ввода/вывода
            DWORD mode = PIPE_READMODE_MESSAGE;
            if (!SetNamedPipeHandleState(pipe_handle_, &mode, nullptr, nullptr))
            {
                Logger::Instance().Log("Failed to set pipe handle state: {}", GetLastError());
                CloseHandle(pipe_handle_);
                pipe_handle_ = INVALID_HANDLE_VALUE;
                return false;
            }

            Logger::Instance().Log("Connected to IPC server");
            return true;
        }

        if (GetLastError() != ERROR_PIPE_BUSY)
        {
            Logger::Instance().Log("Failed to connect to pipe: {}", GetLastError());
            return false;
        }

        // Попробовать снова через 100ms
        Sleep(100);
    }

    Logger::Instance().Log("Timeout waiting for IPC server to start");
    return false;
}

void IPCClient::Disconnect()
{
    if (pipe_handle_ != INVALID_HANDLE_VALUE)
    {
        CloseHandle(pipe_handle_);
        pipe_handle_ = INVALID_HANDLE_VALUE;
    }
}

bool IPCClient::HasData() const
{
    DWORD bytesAvailable{};

    return PeekNamedPipe(pipe_handle_, nullptr, 0, nullptr, &bytesAvailable, nullptr) && bytesAvailable;
}

bool IPCClient::ReceiveHeader(MessageHeader& header)
{
    if (pipe_handle_ == INVALID_HANDLE_VALUE)
        return false;

    DWORD read{};

    if (!ReadFile(pipe_handle_, &header, sizeof(header), &read, nullptr))
    {
        Logger::Instance().Log("ReadFile failed. handle={} error={}", reinterpret_cast<uintptr_t>(pipe_handle_),
                               GetLastError());
        return false;
    }

    return read == sizeof(header);
}

} // namespace ipc