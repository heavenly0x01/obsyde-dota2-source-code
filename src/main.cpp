#include "common.h"
#include "mem.h"
#include "game.h"
#include "hook.h"
#include <Shellapi.h>

static DWORD WINAPI ScanThread(LPVOID) {
    __try {
        game::ScanLoop();
    } __except (EXCEPTION_EXECUTE_HANDLER) {
    }
    return 0;
}

static DWORD WINAPI InitThread(LPVOID) {
    while (cfg::running.load()) {
        game::TryInit();
        Sleep(game::g_sys.ready ? 1000 : 400);
    }
    return 0;
}
static DWORD WINAPI MainThread(LPVOID) {
    __try {
        ShellExecuteA(nullptr, "open", "https://t.me/Heavenly0x", nullptr, nullptr, SW_SHOWNORMAL);

        bool haveClient = false;
        for (int i = 0; i < 600 && cfg::running.load(); ++i) {
            if (mem::ModuleBase("client.dll")) {
                haveClient = true;
                break;
            }
            Sleep(200);
        }
        if (!haveClient) {
            while (cfg::running.load()) Sleep(500);
            return 0;
        }

        InstallHooks();

        HANDLE scan = CreateThread(nullptr, 0, ScanThread, nullptr, 0, nullptr);
        if (scan) CloseHandle(scan);

        HANDLE init = CreateThread(nullptr, 0, InitThread, nullptr, 0, nullptr);
        if (init) CloseHandle(init);

        while (cfg::running.load()) Sleep(500);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        HANDLE t = CreateThread(nullptr, 0, MainThread, hModule, 0, nullptr);
        if (t) CloseHandle(t);
    }
    return TRUE;
}

