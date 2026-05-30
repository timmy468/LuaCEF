cmake_minimum_required(VERSION 3.15)
project(LuaCEF)

add_library(${PROJECT_NAME} SHARED)

target_sources(${PROJECT_NAME} PRIVATE
    src/web/WebCore.cpp
    src/web/WebView.cpp

    src/main.cpp
    src/Plugin.cpp
)

find_package(plugin-sdk REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE
    pluginsdk::gtasa
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)

set_target_properties(${PROJECT_NAME} PROPERTIES
    SUFFIX .asi
)

# --- DirectX SDK (June 2010) ---
# Можно задавать:
#  - через -DDXSDK_PATH=...
#  - или через env DXSDK_DIR (как в установщике DXSDK)
set(DXSDK_PATH "" CACHE PATH "Path to DirectX SDK (June 2010), must contain Include/ and Lib/x86")

if(NOT DXSDK_PATH)
  if(DEFINED ENV{DXSDK_DIR})
    # у DXSDK_DIR обычно со слэшем на конце, но нам это не мешает
    set(DXSDK_PATH "$ENV{DXSDK_DIR}")
  else()
    set(DXSDK_PATH "C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)")
  endif()
endif()

# Нормализуем слэши (для Windows путей)
file(TO_CMAKE_PATH "${DXSDK_PATH}" DXSDK_PATH)

if(EXISTS "${DXSDK_PATH}/Include" AND EXISTS "${DXSDK_PATH}/Lib/x86")
  add_library(dx9sdk INTERFACE)
  target_include_directories(dx9sdk INTERFACE "${DXSDK_PATH}/Include")
  target_link_directories(dx9sdk INTERFACE "${DXSDK_PATH}/Lib/x86")
  target_link_libraries(dx9sdk INTERFACE
    d3d9
    $<$<CONFIG:Debug>:d3dx9d>
    $<$<CONFIG:Release>:d3dx9>
  )
else()
  message(FATAL_ERROR "DirectX SDK (June 2010) not found. DXSDK_PATH='${DXSDK_PATH}'. Expected Include/ and Lib/x86/")
endif()

target_link_libraries(${PROJECT_NAME} PUBLIC dx9sdk)

add_library(lua INTERFACE)
target_include_directories(lua INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/lua
)
target_link_libraries(lua INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/lua/lua51.lib
)

add_library(sol2 INTERFACE)

target_include_directories(sol2 INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/sol2
)

target_link_libraries(${PROJECT_NAME} PUBLIC lua sol2)