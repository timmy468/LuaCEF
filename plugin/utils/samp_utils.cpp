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
    v037r5,
    v03dl,
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
    case 0xCBC90:
        return SampVersion::v037r5;
    case 0xFDB60:
        return SampVersion::v03dl;
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
        case SampVersion::v037r5:
            sampapi::v037r5::RefGame()->SetCursorMode(sampapi::v037r5::CURSOR_LOCKCAM, 0);
            break;
        case SampVersion::v03dl:
            sampapi::v03dl::RefGame()->SetCursorMode(sampapi::v03dl::CURSOR_LOCKCAM, 0);
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
        case SampVersion::v037r5:
            sampapi::v037r5::RefGame()->SetCursorMode(sampapi::v037r5::CURSOR_NONE, 0);
            break;
        case SampVersion::v03dl:
            sampapi::v03dl::RefGame()->SetCursorMode(sampapi::v03dl::CURSOR_NONE, 0);
            break;
        }
    }
}

uintptr_t GetWndProcHookAddress()
{
    switch (GetVersion())
    {
    case SampVersion::v037r1:
        return GetSampHandle() + 0x5DB40;
    case SampVersion::v037r3:
        return GetSampHandle() + 0x60EE0;
    case SampVersion::v037r5:
        return GetSampHandle() + 0x61650;
    case SampVersion::v03dl:
        return GetSampHandle() + 0x610D0;
    }

    return 0;
}

uintptr_t GetDrawHookAddress()
{
    switch (GetVersion())
    {
    case SampVersion::v037r1:
        return GetSampHandle() + 0x7165E;
    case SampVersion::v037r3:
        return GetSampHandle() + 0x75564;
    case SampVersion::v037r5:
        return GetSampHandle() + 0x75C73;
    case SampVersion::v03dl:
        return GetSampHandle() + 0x756F4;
    }

    return 0;
}
} // namespace samp_utils