add_executable(LuaCEFSubprocess main.cpp)

set_target_properties(LuaCEFSubprocess PROPERTIES
    OUTPUT_NAME "cef-subprocess"
    WIN32_EXECUTABLE TRUE 
    LINK_FLAGS "/ENTRY:WinMainCRTStartup"
)