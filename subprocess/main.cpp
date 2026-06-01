#include "Subprocess.hpp"
#include <Windows.h>

int WINAPI WinMain(HINSTANCE instance, HINSTANCE previous_instance, LPSTR cmd_line, int show)
{
    UNREFERENCED_PARAMETER(previous_instance);
    UNREFERENCED_PARAMETER(cmd_line);

    void* sandbox_info = nullptr;

    CefMainArgs main_args(instance);
    CefRefPtr<Subprocess> app(new Subprocess);

    int exit_code = CefExecuteProcess(main_args, app.get(), sandbox_info);

    if (exit_code >= 0)
        return exit_code;

    return 0;
}