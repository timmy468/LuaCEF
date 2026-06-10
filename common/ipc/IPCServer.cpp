#include "IPCServer.hpp"

#include <array>
#include <iostream>

#include "Logger.hpp"

namespace ipc
{

IPCServer::IPCServer(std::shared_ptr<IPCMessageHandler> handler) : handler_(handler), is_running_(false)
{
}

IPCServer::~IPCServer()
{
    Stop();
}

bool IPCServer::Start(const std::string& pipe_name, int max_clients)
{
    Logger::Instance().Log("IPCServer::Start");

    pipe_name_ = pipe_name;
    is_running_ = true;

    while (is_running_)
    {
        // Создать именованный канал
        constexpr DWORD PIPE_BUFFER_SIZE = 64 * 1024;

        HANDLE pipe = CreateNamedPipeA(pipe_name_.c_str(), PIPE_ACCESS_DUPLEX,
                                       PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, max_clients,
                                       PIPE_BUFFER_SIZE, PIPE_BUFFER_SIZE, 0, nullptr);

        if (pipe == INVALID_HANDLE_VALUE)
        {
            Logger::Instance().Log("Failed to create named pipe: {}", GetLastError());
            is_running_ = false;
            return false;
        }

        Logger::Instance().Log("IPC Server listening on {}", pipe_name_);

        // Ждать подключения клиента
        if (!ConnectNamedPipe(pipe, nullptr))
        {
            DWORD error = GetLastError();
            if (error != ERROR_PIPE_CONNECTED)
            {
                Logger::Instance().Log("Failed to connect named pipe: {}", error);
                CloseHandle(pipe);
                is_running_ = false;
                return false;
            }
        }

        Logger::Instance().Log("Client connected");

        // Создать поток для обработки клиента
        client_threads_.emplace_back([this](HANDLE pipe_handle) { ClientThreadProc(pipe_handle); }, pipe);
    }

    Logger::Instance().Log("IPCServer::Stop");

    return true;
}

void IPCServer::Stop()
{
    is_running_ = false;

    // Ждать завершения всех потоков
    for (auto& thread : client_threads_)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }
    client_threads_.clear();
}

void IPCServer::ClientThreadProc(HANDLE pipe_handle)
{
    constexpr uint32_t MAX_PAYLOAD_SIZE = 8192;

    std::array<uint8_t, MAX_PAYLOAD_SIZE> payloadBuffer;

    while (is_running_)
    {
        ProcessMessageThreadTasks();

        if (!HasData(pipe_handle))
        {
            Sleep(1);
            continue;
        }

        MessageHeader header{};

        DWORD bytesRead{};

        if (!ReadFile(pipe_handle, &header, sizeof(header), &bytesRead, nullptr))
        {
            break;
        }

        if (bytesRead != sizeof(header))
        {
            break;
        }

        if (header.payloadSize > MAX_PAYLOAD_SIZE)
        {
            Logger::Instance().Log("Payload too large");

            break;
        }

        if (header.payloadSize)
        {
            if (!ReadFile(pipe_handle, payloadBuffer.data(), header.payloadSize, &bytesRead, nullptr))
            {
                break;
            }

            if (bytesRead != header.payloadSize)
            {
                break;
            }
        }

        if (handler_)
        {
            handler_->HandleMessage(pipe_handle, header.type, payloadBuffer.data(), header.payloadSize);
        }
    }

    FlushFileBuffers(pipe_handle);

    DisconnectNamedPipe(pipe_handle);

    CloseHandle(pipe_handle);

    Logger::Instance().Log("Client disconnected");
}

bool IPCServer::HasData(HANDLE pipe_handle)
{
    DWORD bytesAvailable{};

    return PeekNamedPipe(pipe_handle, nullptr, 0, nullptr, &bytesAvailable, nullptr) && bytesAvailable;
}

void IPCServer::AddTaskToMessageThread(std::function<void()> task)
{
    std::lock_guard lock(message_thread_tasks_.mutex);

    message_thread_tasks_.tasks.push_back(task);
}

void IPCServer::ProcessMessageThreadTasks()
{
    std::lock_guard lock(message_thread_tasks_.mutex);

    for (auto& task : message_thread_tasks_.tasks)
    {
        task();
    }

    message_thread_tasks_.tasks.clear();
}
} // namespace ipc