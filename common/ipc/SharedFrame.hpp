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

struct SharedFrame
{
    FrameHeader header;

    DirtyRect dirtyRects[MAX_DIRTY_RECTS];

    inline uint8_t* GetPixelBuffer()
    {
        return reinterpret_cast<uint8_t*>(reinterpret_cast<char*>(this) + sizeof(SharedFrame));
    }
};