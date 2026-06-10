#include <Windows.h>
#include <shellapi.h>

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
    CreateDebugConsole();

    Logger::Instance().SetLogFilePath(std::filesystem::current_path() / "luacef" / "host.log");

    DWORD parentPid = 0;

    LPWSTR* argv = nullptr;
    int argc = 0;

    argv = CommandLineToArgvW(GetCommandLineW(), &argc);

    if (argv)
    {
        for (int i = 1; i < argc; ++i)
        {
            std::wstring arg = argv[i];

            constexpr wchar_t prefix[] = L"--parent-pid=";

            if (arg.starts_with(prefix))
            {
                parentPid = std::wcstoul(arg.c_str() + std::wcslen(prefix), nullptr, 10);
            }
        }

        LocalFree(argv);
    }

    Logger::Instance().Log("Parent PID: {}", parentPid);

    Host host;

    if (!host.Initialize(parentPid))
        return 1;

    host.Run();

    return 0;
}