#pragma once

#include <atomic>
#include <cstdint>

struct DirtyRect
{
    int32_t x;
    int32_t y;
    int32_t width;
    int32_t height;
};

constexpr uint32_t MAX_DIRTY_RECTS = 128;

struct FrameHeader
{
    std::atomic_uint32_t sequence{};

    std::atomic_uint32_t width{};
    std::atomic_uint32_t height{};

    std::atomic_uint32_t dirtyRectCount{};
};

// todo: move to config
constexpr uint32_t MAX_WIDTH = 1920;
constexpr uint32_t MAX_HEIGHT = 1080;

struct SharedFrame
{
    FrameHeader header;

    DirtyRect dirtyRects[MAX_DIRTY_RECTS];

    uint8_t pixels[MAX_WIDTH * MAX_HEIGHT * 4];
};