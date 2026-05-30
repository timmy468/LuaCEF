#pragma once

#include <memory>

#include <sol.hpp>

#include "binding/LuaBinding.hpp"
#include "web/WebCore.hpp"

class Plugin final
{
  private:
    std::shared_ptr<WebCore> webCore_;
    std::unique_ptr<LuaBinding> luaBinding_;

  public:
    void OnAttach(sol::table module);
    void OnDetach();

  private:
    void OnRenderInit();
    void OnDrawingEvent();
};