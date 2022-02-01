#include <win32.h>
void main()
{

    DWORD old;
    VirtualProtect((PVOID)&FreeConsole, 1, PAGE_EXECUTE_READWRITE, &old);
    *(BYTE*)(&FreeConsole) = 0xC3;
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
    freopen("CONIN$", "r", stdin);
    HWND ConsoleHandle = GetConsoleWindow();
    SetConsoleTitleA("[Marlyn]");
    ::SetWindowPos(ConsoleHandle, HWND_TOPMOST, 0, 0, 0, 0, SWP_DRAWFRAME | SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
    ::ShowWindow(ConsoleHandle, SW_NORMAL);

    rbx::scan_game();
    start_imgui();
}

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        std::thread(main).detach();
        break;
    }
    return TRUE;
}