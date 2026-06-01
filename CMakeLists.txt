cmake_minimum_required(VERSION 3.15)
project(LuaCEF)

set(CEF_ROOT "${CMAKE_SOURCE_DIR}/third_party/cef")
add_subdirectory(
    ${CEF_ROOT}
    ${CMAKE_BINARY_DIR}/cef
)
include_directories(
    ${CEF_ROOT}/include
    ${CEF_ROOT}
)
link_libraries(
    libcef_dll_wrapper
    ${CEF_ROOT}/Release/libcef.lib
)

add_subdirectory(plugin)
add_subdirectory(subprocess)