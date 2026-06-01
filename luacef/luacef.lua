local ffi = require("ffi")

ffi.cdef [[
    int LuaCEF_Test();
    // bool LuaCEF_Inited();
]]

local MODULE_ = {}

MODULE_.VERSION = "1.0.0"

return MODULE_
