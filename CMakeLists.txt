cmake_minimum_required(VERSION 3.15)
project(LuaCEF)

add_compile_definitions(
    WIN32_LEAN_AND_MEAN
    NOMINMAX
)

set(CEF_ROOT "${CMAKE_SOURCE_DIR}/third_party/cef")
set(CEF_RUNTIME_LIBRARY_FLAG
    "/MD"
    CACHE STRING ""
    FORCE)
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

set(LUACEF_DEPLOY_DIR "${CMAKE_BINARY_DIR}/deploy")
set(LUACEF_CEF_DIR "${LUACEF_DEPLOY_DIR}/luacef")

add_subdirectory(common)
add_subdirectory(ipc)
add_subdirectory(host)
add_subdirectory(plugin)
add_subdirectory(subprocess)

foreach(CONFIG Debug Release RelWithDebInfo MinSizeRel)
    string(TOUPPER ${CONFIG} CONFIG_UPPER)

    set_target_properties(LuaCEFPlugin PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${LUACEF_DEPLOY_DIR}/$<CONFIG>"
    )

    set_target_properties(LuaCEFHost PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${LUACEF_DEPLOY_DIR}/$<CONFIG>/luacef"
    )

    set_target_properties(LuaCEFSubprocess PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${LUACEF_DEPLOY_DIR}/$<CONFIG>/luacef"
    )
endforeach()