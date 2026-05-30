// #include "Plugin.hpp"

// PLUGIN_ENTRY_POINT(Plugin)

#include "binding/lua_module.h"
#include <sol.hpp>

sol::table open(sol::this_state ts)
{
    sol::state_view lua(ts);
    auto module = lua.create_table();

    return module;
}

SOL_MODULE_ENTRYPOINT(open);
