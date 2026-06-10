#pragma once

#include <atomic>
#include <cstdint>

struct FrameHeader
{
    std::atomic_uint32_t sequence{};

    std::atomic_uint32_t width{};
    std::atomic_uint32_t height{};
};

// todo: move to config
constexpr uint32_t MAX_WIDTH = 1920;
constexpr uint32_t MAX_HEIGHT = 1080;

struct SharedFrame
{
    FrameHeader header;

    uint8_t pixels[MAX_WIDTH * MAX_HEIGHT * 4];
};