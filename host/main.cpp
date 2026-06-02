#include <Windows.h>

#include "Host.hpp"

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    Host host;

    host.Initialize();

    host.Run();

    return 0;
}