add_library(cef-common INTERFACE)

add_subdirectory(ipc)

target_include_directories(cef-common INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

include(FetchContent)
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.14.1
)
FetchContent_MakeAvailable(spdlog)

target_link_libraries(cef-common INTERFACE
    spdlog::spdlog
)