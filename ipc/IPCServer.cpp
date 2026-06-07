#include "IPCServer.hpp"

#include <array>
#include <iostream>

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
    std::println(std::cout, "IPCServer::Start");

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
            std::cerr << "Failed to create named pipe: " << GetLastError() << std::endl;
            is_running_ = false;
            return false;
        }

        std::cout << "IPC Server listening on " << pipe_name_ << std::endl;

        // Ждать подключения клиента
        if (!ConnectNamedPipe(pipe, nullptr))
        {
            DWORD error = GetLastError();
            if (error != ERROR_PIPE_CONNECTED)
            {
                std::cerr << "Failed to connect named pipe: " << error << std::endl;
                CloseHandle(pipe);
                is_running_ = false;
                return false;
            }
        }

        std::cout << "Client connected" << std::endl;

        // Создать поток для обработки клиента
        client_threads_.emplace_back([this](HANDLE pipe_handle) { ClientThreadProc(pipe_handle); }, pipe);
    }

    std::println(std::cout, "IPCServer::Stop");

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
            continue;

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
            std::cerr << "Payload too large" << std::endl;

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

    std::cout << "Client disconnected" << std::endl;
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