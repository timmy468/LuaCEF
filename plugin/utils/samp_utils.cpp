#include "samp_utils.hpp"

#include <cstdint>

#include <Windows.h>
#include <sampapi/CGame.h>

namespace samp_utils
{
enum SampVersion
{
    v037r1,
    v037r3,
    unknown
};

std::uintptr_t GetSampHandle()
{
    return reinterpret_cast<std::uintptr_t>(GetModuleHandleA("samp.dll"));
}

std::uintptr_t GetModuleEntryPoint(std::uintptr_t module)
{
    return reinterpret_cast<IMAGE_NT_HEADERS*>(module + reinterpret_cast<IMAGE_DOS_HEADER*>(module)->e_lfanew)
        ->OptionalHeader.AddressOfEntryPoint;
}

SampVersion GetVersion()
{
    switch (GetModuleEntryPoint(GetSampHandle()))
    {
    case 0x31DF13:
        return SampVersion::v037r1;
    case 0xCC4D0:
        return SampVersion::v037r3;
    default:
        return SampVersion::unknown;
    }
}

void ToggleCursor(bool show)
{
    if (show)
    {
        switch (GetVersion())
        {
        case SampVersion::v037r1:
            sampapi::v037r1::RefGame()->SetCursorMode(sampapi::v037r1::CURSOR_LOCKCAM, 0);
            break;
        case SampVersion::v037r3:
            sampapi::v037r3::RefGame()->SetCursorMode(sampapi::v037r3::CURSOR_LOCKCAM, 0);
            break;
        }
    }
    else
    {
        switch (GetVersion())
        {
        case SampVersion::v037r1:
            sampapi::v037r1::RefGame()->SetCursorMode(sampapi::v037r1::CURSOR_NONE, 0);
            break;
        case SampVersion::v037r3:
            sampapi::v037r3::RefGame()->SetCursorMode(sampapi::v037r3::CURSOR_NONE, 0);
            break;
        }
    }
}
} // namespace samp_utils