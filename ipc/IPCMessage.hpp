#pragma once

#include <cstdint>

namespace ipc
{
enum class CommandType : uint16_t
{
    CreateWebView = 1,
    CreateWebViewResponse,
    CloseWebView,
    LoadURL,
    ExecuteJS,

    MouseMove,
    MouseClick,

    KeyDown,
    KeyUp,

    Ping,
    Pong,
    Error
};

struct CreateWebViewCmd
{
    uint32_t id;

    int32_t x;
    int32_t y;

    int32_t width;
    int32_t height;

    char url[512];
};

struct CreateWebViewResponse
{
    uint32_t id;
};

struct CloseWebViewCmd
{
    uint32_t id;
};

struct LoadURLCmd
{
    uint32_t id;

    char url[512];
};

struct MouseMoveCmd
{
    uint32_t id;

    int32_t x;
    int32_t y;
};

#pragma pack(push, 1)

struct MessageHeader
{
    CommandType type;
    uint32_t payloadSize;
};

#pragma pack(pop)
} // namespace ipc