add_executable(LuaCEFSubprocess main.cpp)

target_compile_features(LuaCEFSubprocess PRIVATE cxx_std_23)

set_target_properties(LuaCEFSubprocess PROPERTIES
    OUTPUT_NAME "cef-subprocess"
    WIN32_EXECUTABLE TRUE 
    LINK_FLAGS "/ENTRY:WinMainCRTStartup"
)