#include <Windows.h>

#include <filesystem>

#include "Host.hpp"

#include "Logger.hpp"

void CreateDebugConsole()
{
    AllocConsole();

    FILE* file{};

    freopen_s(&file, "CONOUT$", "w", stdout);
    freopen_s(&file, "CONOUT$", "w", stderr);
    freopen_s(&file, "CONIN$", "r", stdin);

    SetConsoleTitleA("LuaCEF Host");

    ShowWindow(GetConsoleWindow(), SW_SHOW);
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    // todo
    CreateDebugConsole();

    Logger::Instance().SetLogFilePath(std::filesystem::current_path() / "luacef" / "host.log");

    Host host;

    host.Initialize();

    host.Run();

    return 0;
}