#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <Shellapi.h>
#include <TlHelp32.h>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <string>

static void Purple() {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
                            FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
}
static void White() {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
                            FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}
static void Green() {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
                            FOREGROUND_GREEN | FOREGROUND_INTENSITY);
}
static void Red() {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
                            FOREGROUND_RED | FOREGROUND_INTENSITY);
}

static std::string ExeDir() {
    char buf[MAX_PATH] = {};
    GetModuleFileNameA(nullptr, buf, MAX_PATH);
    std::string p(buf);
    size_t s = p.find_last_of("\\/");
    return s == std::string::npos ? std::string(".") : p.substr(0, s);
}

static DWORD FindPid(const char* exe) {
    DWORD pid = 0;
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return 0;
    PROCESSENTRY32 pe{};
    pe.dwSize = sizeof(pe);
    if (Process32First(snap, &pe)) {
        do {
            if (_stricmp(pe.szExeFile, exe) == 0) { pid = pe.th32ProcessID; break; }
        } while (Process32Next(snap, &pe));
    }
    CloseHandle(snap);
    return pid;
}

static uintptr_t ModuleBase(DWORD pid, const char* name) {
    uintptr_t base = 0;
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
    if (snap == INVALID_HANDLE_VALUE) return 0;
    MODULEENTRY32 me{};
    me.dwSize = sizeof(me);
    if (Module32First(snap, &me)) {
        do {
            if (_stricmp(me.szModule, name) == 0) { base = (uintptr_t)me.modBaseAddr; break; }
        } while (Module32Next(snap, &me));
    }
    CloseHandle(snap);
    return base;
}

static bool ModuleLoaded(DWORD pid, const char* name) {
    return ModuleBase(pid, name) != 0;
}

static bool Inject(DWORD pid, const std::string& dllPath, std::string& err) {
    if (GetFileAttributesA(dllPath.c_str()) == INVALID_FILE_ATTRIBUTES) {
        err = "dll not found: " + dllPath;
        return false;
    }

    HANDLE hp = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION |
                            PROCESS_VM_WRITE | PROCESS_QUERY_INFORMATION,
                            FALSE, pid);
    if (!hp) {
        err = "OpenProcess failed (error " + std::to_string(GetLastError()) + ")";
        return false;
    }

    uintptr_t remoteK32 = 0;
    for (int i = 0; i < 100 && !remoteK32; ++i) {
        remoteK32 = ModuleBase(pid, "kernel32.dll");
        if (!remoteK32) Sleep(100);
    }
    if (!remoteK32) {
        err = "kernel32 not found in target";
        CloseHandle(hp);
        return false;
    }

    uintptr_t localK32 = (uintptr_t)GetModuleHandleA("kernel32.dll");
    uintptr_t localLoad = (uintptr_t)GetProcAddress((HMODULE)localK32, "LoadLibraryA");
    if (!localLoad) {
        err = "LoadLibraryA not found";
        CloseHandle(hp);
        return false;
    }
    uintptr_t remoteLoad = remoteK32 + (localLoad - localK32);

    SIZE_T len = dllPath.size() + 1;
    void* remoteStr = VirtualAllocEx(hp, nullptr, len, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!remoteStr) {
        err = "VirtualAllocEx failed";
        CloseHandle(hp);
        return false;
    }

    SIZE_T written = 0;
    if (!WriteProcessMemory(hp, remoteStr, dllPath.c_str(), len, &written) || written != len) {
        err = "WriteProcessMemory failed";
        VirtualFreeEx(hp, remoteStr, 0, MEM_RELEASE);
        CloseHandle(hp);
        return false;
    }

    HANDLE th = CreateRemoteThread(hp, nullptr, 0,
                                   (LPTHREAD_START_ROUTINE)remoteLoad,
                                   remoteStr, 0, nullptr);
    if (!th) {
        err = "CreateRemoteThread failed (error " + std::to_string(GetLastError()) + ")";
        VirtualFreeEx(hp, remoteStr, 0, MEM_RELEASE);
        CloseHandle(hp);
        return false;
    }

    DWORD wr = WaitForSingleObject(th, 30000);
    DWORD code = 0;
    GetExitCodeThread(th, &code);
    CloseHandle(th);
    VirtualFreeEx(hp, remoteStr, 0, MEM_RELEASE);
    CloseHandle(hp);

    if (wr != WAIT_OBJECT_0) {
        err = "LoadLibrary timed out";
        return false;
    }
    if (code == 0) {
        err = "LoadLibrary returned NULL";
        return false;
    }

    for (int i = 0; i < 50 && !ModuleLoaded(pid, "obsdota2.dll"); ++i) Sleep(100);
    if (!ModuleLoaded(pid, "obsdota2.dll")) {
        err = "module not visible in target";
        return false;
    }
    return true;
}

static bool LaunchDota(std::string& err) {
    const char* candidates[] = {
        "C:\\Program Files (x86)\\Steam\\steamapps\\common\\dota 2 beta\\game\\bin\\win64\\dota2.exe",
        "D:\\Steam\\steamapps\\common\\dota 2 beta\\game\\bin\\win64\\dota2.exe",
        "E:\\Steam\\steamapps\\common\\dota 2 beta\\game\\bin\\win64\\dota2.exe",
        "C:\\Program Files\\Steam\\steamapps\\common\\dota 2 beta\\game\\bin\\win64\\dota2.exe",
    };
    for (const char* p : candidates) {
        if (GetFileAttributesA(p) != INVALID_FILE_ATTRIBUTES) {
            ShellExecuteA(nullptr, "open", p, nullptr, nullptr, SW_SHOWNORMAL);
            return true;
        }
    }
    SHELLEXECUTEINFOA sei{};
    sei.cbSize = sizeof(sei);
    sei.fMask = SEE_MASK_FLAG_NO_UI;
    sei.lpVerb = "open";
    sei.lpFile = "steam://rungameid/570";
    sei.nShow = SW_SHOWNORMAL;
    if (ShellExecuteExA(&sei)) return true;
    err = "dota2.exe not found";
    return false;
}

static void PrintStatus(DWORD pid) {
    if (ModuleLoaded(pid, "obsdota2.dll"))
        Green(), printf("[+] already loaded\n");
    else
        White(), printf("[*] not loaded yet\n");
}

int main(int argc, char** argv) {
    SetConsoleTitleA("obsyde");
    Purple();
    printf("obsyde\n");
    White();
    printf("-----------------------------\n");
    printf("1. Start dota2 and launch cheat\n");
    printf("2: Launch cheat (if open dota2)\n");
    printf("-----------------------------\n");

    DWORD pid = 0;
    std::string err;

    if (argc >= 3 && strcmp(argv[1], "--pid") == 0) {
        pid = (DWORD)strtoul(argv[2], nullptr, 10);
        if (!pid) { Red(); printf("[-] bad pid\n"); return 1; }
        if (Inject(pid, ExeDir() + "\\obsdota2.dll", err)) {
            Green(); printf("[+] injected into pid %lu\n", pid);
        } else {
            Red(); printf("[-] %s\n", err.c_str());
            return 1;
        }
        return 0;
    }

    printf("> ");
    char line[16] = {};
    if (!fgets(line, sizeof(line), stdin)) return 1;
    int choice = atoi(line);

    if (choice == 1) {
        White();
        if (!LaunchDota(err)) { Red(); printf("[-] %s\n", err.c_str()); return 1; }
        printf("[*] waiting for dota2...\n");
        for (int i = 0; i < 240 && !pid; ++i) {
            pid = FindPid("dota2.exe");
            if (!pid) Sleep(1000);
        }
        if (!pid) { Red(); printf("[-] dota2 did not start\n"); return 1; }
        printf("[*] process pid %lu, waiting for loader...\n", pid);
        for (int i = 0; i < 120 && !ModuleBase(pid, "kernel32.dll"); ++i) Sleep(500);
        White();
        if (Inject(pid, ExeDir() + "\\obsdota2.dll", err)) {
            Green(); printf("[+] injected - open dota2, INSERT for menu\n");
        } else {
            Red(); printf("[-] %s\n", err.c_str());
            return 1;
        }
    } else if (choice == 2) {
        pid = FindPid("dota2.exe");
        if (!pid) {
            Red(); printf("[-] dota2 is not running\n");
            return 1;
        }
        printf("[*] dota2 pid %lu\n", pid);
        PrintStatus(pid);
        White();
        if (Inject(pid, ExeDir() + "\\obsdota2.dll", err)) {
            Green(); printf("[+] injected - INSERT for menu\n");
        } else {
            Red(); printf("[-] %s\n", err.c_str());
            return 1;
        }
    } else {
        Red();
        printf("[-] unknown choice\n");
        return 1;
    }

    return 0;
}
