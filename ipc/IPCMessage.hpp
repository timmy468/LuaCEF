#pragma once

#include <cstdint>

namespace ipc
{
enum class CommandType : uint16_t
{
    CreateWebView = 1,
    CreateWebViewResponse,
    CloseWebView,
    SetRect,
    LoadURL,
    ReloadPage,
    GoBack,
    GoForward,
    ExecuteJS,

    MouseMove,
    MouseClick,
    MouseWheel,

    Key,

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

struct SetRectCmd
{
    uint32_t id;

    int32_t x;
    int32_t y;

    int32_t width;
    int32_t height;
};

struct LoadURLCmd
{
    uint32_t id;

    char url[512];
};

struct ReloadPageCmd
{
    uint32_t id;

    bool ignoreCache;
};

struct GoBackCmd
{
    uint32_t id;
};

struct GoForwardCmd
{
    uint32_t id;
};

struct ExecuteJSCmd
{
    uint32_t id;

    char code[4096];
};

struct MouseMoveCmd
{
    uint32_t id;

    int32_t x;
    int32_t y;
};

struct MouseClickCmd
{
    uint32_t id;

    int32_t x;
    int32_t y;

    uint32_t button;
    bool mouseUp;
};

struct MouseWheelCmd
{
    uint32_t id;

    int32_t x;
    int32_t y;

    int32_t delta;
};

struct KeyCmd
{
    uint32_t id;

    uint32_t keycode;
    uint32_t nativeKeycode;

    int32_t modifiers;
    int32_t type;
};

#pragma pack(push, 1)

struct MessageHeader
{
    CommandType type;
    uint32_t payloadSize;
};

#pragma pack(pop)
} // namespace ipc