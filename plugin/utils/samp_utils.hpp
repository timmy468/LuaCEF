#pragma once

#include <cstdint>

namespace samp_utils
{
void ToggleCursor(bool show);
std::uintptr_t GetSampHandle();
uintptr_t GetWndProcHookAddress();
uintptr_t GetDrawHookAddress();
} // namespace samp_utils