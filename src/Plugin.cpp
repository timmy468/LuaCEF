#include "Plugin.hpp"
#include <plugin.h>

void Plugin::OnAttach(sol::table module)
{
    webCore_ = std::make_shared<WebCore>();

    luaBinding_ = std::make_unique<LuaBinding>(module, webCore_);
    luaBinding_->Bind();

    plugin::Events::initRwEvent += [this]() { OnRenderInit(); };
    plugin::Events::drawingEvent += [this]() { OnDrawingEvent(); };
}

void Plugin::OnDetach()
{
}

void Plugin::OnRenderInit()
{
    webCore_->Initialize(GetD3DDevice());
}

void Plugin::OnDrawingEvent()
{
}
