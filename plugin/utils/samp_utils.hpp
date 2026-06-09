#pragma once

#include <cstdint>

namespace samp_utils
{
void ToggleCursor(bool show);
uintptr_t GetWndProcHookAddress();
} // namespace samp_utils