add_library(cef-ipc STATIC)

target_sources(cef-ipc PRIVATE
    IPCServer.cpp
    IPCClient.cpp
)

target_include_directories(cef-ipc PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)