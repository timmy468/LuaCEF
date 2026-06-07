#pragma once

#include <Windows.h>

#include <iostream>
#include <string>

#include "IPCMessage.hpp"

namespace ipc
{

class IPCClient
{
  public:
    IPCClient();
    ~IPCClient();

    bool Connect(const std::string& pipe_name = R"(\\.\pipe\LuaCEF_IPC)");
    void Disconnect();

    bool HasData() const;

    template <typename T> bool Send(CommandType type, const T& payload);

    bool ReceiveHeader(MessageHeader& header);

    template <typename T> bool ReceivePayload(T& payload);

    bool IsConnected() const
    {
        return pipe_handle_ != INVALID_HANDLE_VALUE;
    }

  private:
    HANDLE pipe_handle_;
    std::string pipe_name_;
};

template <typename T> bool IPCClient::Send(CommandType type, const T& payload)
{
    if (pipe_handle_ == INVALID_HANDLE_VALUE)
        return false;

    MessageHeader header;

    header.type = type;
    header.payloadSize = sizeof(T);

    DWORD written{};

    // std::println(std::cout, "Sending message of type {} {}", static_cast<int>(type), pipe_handle_);

    if (!WriteFile(pipe_handle_, &header, sizeof(header), &written, nullptr))
    {
        std::println(std::cerr, "WriteFile failed. handle={} error={}", reinterpret_cast<uintptr_t>(pipe_handle_),
                     GetLastError());

        return false;
    }

    // std::println(std::cout, "Sending message of type2 {} {}", static_cast<int>(type), pipe_handle_);

    if (written != sizeof(header))
        return false;

    // std::println(std::cout, "Sending message of type3 {} {}", static_cast<int>(type), pipe_handle_);

    if (!WriteFile(pipe_handle_, &payload, sizeof(T), &written, nullptr))
    {
        return false;
    }

    // std::println(std::cout, "Sending message of type4 {} {}", static_cast<int>(type), pipe_handle_);

    return written == sizeof(T);
}

template <typename T> bool IPCClient::ReceivePayload(T& payload)
{
    DWORD read{};

    if (!ReadFile(pipe_handle_, &payload, sizeof(T), &read, nullptr))
    {
        return false;
    }

    return read == sizeof(T);
}
} // namespace ipc