#pragma once

#include <sol.hpp>

#include "core/IWebCore.hpp"

class LuaBinding
{
  public:
    LuaBinding(sol::table module, std::shared_ptr<IWebCore> webCore);
    void Bind();

  private:
    sol::table module_;
    std::shared_ptr<IWebCore> webCore_;
};