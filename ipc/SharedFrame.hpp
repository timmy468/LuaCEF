#pragma once

#include <atomic>
#include <cstdint>

struct FrameHeader
{
    std::atomic_uint32_t sequence{};

    uint32_t width{};
    uint32_t height{};
};

constexpr uint32_t MAX_WIDTH = 1920;
constexpr uint32_t MAX_HEIGHT = 1080;

struct SharedFrame
{
    FrameHeader header;

    uint8_t pixels[MAX_WIDTH * MAX_HEIGHT * 4];
};