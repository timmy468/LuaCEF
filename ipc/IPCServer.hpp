#pragma once

#include "IPCMessage.hpp"
#include <Windows.h>
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace ipc
{

// Обработчик команд
class IPCMessageHandler
{
  public:
    virtual ~IPCMessageHandler() = default;

    virtual void HandleMessage(HANDLE pipe, CommandType type, const void* payload, uint32_t payloadSize) = 0;
};

class IPCServer
{
  public:
    explicit IPCServer(std::shared_ptr<IPCMessageHandler> handler);
    ~IPCServer();

    // Запустить сервер (блокирует текущий поток)
    bool Start(const std::string& pipe_name = R"(\\.\pipe\LuaCEF_IPC)", int max_clients = 1);

    // Остановить сервер
    void Stop();

    template <typename T> static bool Send(HANDLE pipe, CommandType type, const T& payload);

    // Отправить сообщение клиенту (для ответов из handler'а)
    template <typename T> bool SendToClient(HANDLE client, CommandType type, const T& payload);

    // Проверить работает ли сервер
    bool IsRunning() const
    {
        return is_running_;
    }

  private:
    // Поток обработки каждого клиента
    void ClientThreadProc(HANDLE pipe_handle);

    std::shared_ptr<IPCMessageHandler> handler_;
    std::string pipe_name_;
    bool is_running_;
    std::vector<std::thread> client_threads_;
};

template <typename T> bool IPCServer::SendToClient(HANDLE client, CommandType type, const T& payload)
{
    MessageHeader header;

    header.type = type;
    header.payloadSize = sizeof(T);

    DWORD written{};

    if (!WriteFile(client, &header, sizeof(header), &written, nullptr))
    {
        return false;
    }

    if (!WriteFile(client, &payload, sizeof(payload), &written, nullptr))
    {
        return false;
    }

    return true;
}

template <typename T> bool IPCServer::Send(HANDLE pipe, CommandType type, const T& payload)
{
    MessageHeader header;

    header.type = type;
    header.payloadSize = sizeof(T);

    DWORD written{};

    if (!WriteFile(pipe, &header, sizeof(header), &written, nullptr))
    {
        return false;
    }

    if (!WriteFile(pipe, &payload, sizeof(payload), &written, nullptr))
    {
        return false;
    }

    return true;
}

} // namespace ipc