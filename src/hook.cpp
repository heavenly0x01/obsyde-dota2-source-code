#include "hook.h"
#include "theme.h"
#include "offsets.h"

#include <d3d11.h>
#include <dxgi.h>
#include <dxgi1_2.h>
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

static HRESULT __stdcall hkPresent(IDXGISwapChain* sc, UINT sync, UINT flags);
static HRESULT __stdcall hkPresent1(IDXGISwapChain1* sc, UINT sync, UINT flags, const DXGI_PRESENT_PARAMETERS* p);
static HRESULT __stdcall hkResize(IDXGISwapChain* sc, UINT bc, UINT w, UINT h, DXGI_FORMAT fmt, UINT flags);
static LRESULT CALLBACK    hkWndProc(HWND, UINT, WPARAM, LPARAM);

using PresentFn  = HRESULT(__stdcall*)(IDXGISwapChain*, UINT, UINT);
using Present1Fn = HRESULT(__stdcall*)(IDXGISwapChain1*, UINT, UINT, const DXGI_PRESENT_PARAMETERS*);
using ResizeFn   = HRESULT(__stdcall*)(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT);

struct HookedVt {
    void** vt = nullptr;
    void*  origPresent = nullptr;
    void*  origPresent1 = nullptr;
    void*  origResize = nullptr;
    bool   p1 = false;
};

static HookedVt  g_vts[4];
static int       g_vtCount = 0;

static ID3D11Device*           g_device = nullptr;
static ID3D11DeviceContext*    g_ctx = nullptr;
static ID3D11RenderTargetView* g_rtv = nullptr;
static HWND                    g_hwnd = nullptr;
static bool                    g_init = false;
static bool                    g_initFailed = false;

static WNDPROC g_origWndProc = nullptr;
static bool    g_hooked = false;

static Frame g_frame;

static ImFont* g_fontSmall = nullptr;
static ImFont* g_fontBig = nullptr;

ImFont* theme::FontSmall() { return g_fontSmall; }
ImFont* theme::FontBig()   { return g_fontBig; }

void theme::LoadFonts() {
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;

    static const ImWchar ranges[] = { 0x0020, 0x00FF, 0x0400, 0x04FF, 0 };
    ImFontConfig cfg{};
    cfg.OversampleH = 2;
    cfg.OversampleV = 2;

    static const char* kSmall[] = {
        "C:\\Windows\\Fonts\\segoeuib.ttf",
        "C:\\Windows\\Fonts\\arialbd.ttf",
        "C:\\Windows\\Fonts\\segoeui.ttf",
    };
    for (const char* p : kSmall) {
        g_fontSmall = io.Fonts->AddFontFromFileTTF(p, 15.5f, &cfg, ranges);
        if (g_fontSmall) break;
    }
    static const char* kBig[] = {
        "C:\\Windows\\Fonts\\segoeuib.ttf",
        "C:\\Windows\\Fonts\\arialbd.ttf",
    };
    for (const char* p : kBig) {
        g_fontBig = io.Fonts->AddFontFromFileTTF(p, 22.0f, &cfg, ranges);
        if (g_fontBig) break;
    }
    if (!g_fontSmall) g_fontSmall = io.Fonts->AddFontDefault();
    if (!g_fontBig)   g_fontBig = g_fontSmall;
}

namespace input {

static bool ToScreen(HWND hwnd, int cx, int cy, int& sx, int& sy) {
    POINT p{ cx, cy };
    if (!ClientToScreen(hwnd, &p)) return false;
    sx = p.x;
    sy = p.y;
    return true;
}

static void MoveTo(int x, int y) {
    int vx = GetSystemMetrics(SM_XVIRTUALSCREEN);
    int vy = GetSystemMetrics(SM_YVIRTUALSCREEN);
    int vw = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int vh = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    if (vw <= 1 || vh <= 1) return;

    INPUT in{};
    in.type = INPUT_MOUSE;
    in.mi.dx = (LONG)llround(((double)(x - vx) * 65535.0) / (double)(vw - 1));
    in.mi.dy = (LONG)llround(((double)(y - vy) * 65535.0) / (double)(vh - 1));
    in.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_VIRTUALDESK;
    SendInput(1, &in, sizeof(INPUT));
}

static void Click(bool right) {
    INPUT in[2]{};
    in[0].type = INPUT_MOUSE;
    in[0].mi.dwFlags = right ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_LEFTDOWN;
    in[1].type = INPUT_MOUSE;
    in[1].mi.dwFlags = right ? MOUSEEVENTF_RIGHTUP : MOUSEEVENTF_RIGHTUP;
    if (!right) in[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;
    SendInput(2, in, sizeof(INPUT));
}

static DWORD g_moveAt = 0;
static bool  g_pendingClick = false;
static bool  g_pendingRight = false;
static int   g_pendingX = 0, g_pendingY = 0;
static HWND  g_pendingHwnd = nullptr;

void ClickAt(HWND hwnd, int cx, int cy, bool right) {
    int sx = 0, sy = 0;
    if (!ToScreen(hwnd, cx, cy, sx, sy)) return;
    g_pendingX = sx;
    g_pendingY = sy;
    g_pendingRight = right;
    g_pendingHwnd = hwnd;
    MoveTo(sx, sy);
    g_moveAt = GetTickCount();
    g_pendingClick = true;
}

void Tick() {
    if (!g_pendingClick) return;
    DWORD dt = GetTickCount() - g_moveAt;
    if (dt < 40) return;
    g_pendingClick = false;
    if (cfg::menuOpen) return;
    if (dt > 800) return;
    if (g_pendingHwnd && GetForegroundWindow() != g_pendingHwnd) return;
    SetCursorPos(g_pendingX, g_pendingY);
    Click(g_pendingRight);
}

}

static bool InModule(void* p, const char* name) {
    uintptr_t b = mem::ModuleBase(name);
    if (!b) return false;
    uintptr_t a = (uintptr_t)p;
    return a >= b && a < b + mem::ModuleSize(b);
}

static bool LooksLikeCode(void* p) {
    return InModule(p, "dxgi.dll") || InModule(p, "d3d11.dll") || InModule(p, "d3d10.dll");
}

static void PatchEntry(void** slot, void* fn, void*& orig) {
    DWORD old = 0;
    orig = *slot;
    VirtualProtect(slot, sizeof(void*), PAGE_EXECUTE_READWRITE, &old);
    *slot = fn;
    VirtualProtect(slot, sizeof(void*), old, &old);
}

static void PatchVTable(void** vt, bool withPresent1) {
    if (!vt) return;

    HookedVt* h = nullptr;
    for (int i = 0; i < g_vtCount; ++i)
        if (g_vts[i].vt == vt) { h = &g_vts[i]; break; }

    if (!h) {
        if (g_vtCount >= 4) return;
        h = &g_vts[g_vtCount++];
        h->vt = vt;
        PatchEntry(&vt[8],  (void*)hkPresent, h->origPresent);
        PatchEntry(&vt[13], (void*)hkResize,  h->origResize);
    }

    if (withPresent1 && !h->p1 && LooksLikeCode(vt[21])) {
        PatchEntry(&vt[21], (void*)hkPresent1, h->origPresent1);
        h->p1 = true;
    }
}

static HookedVt* FindVt(void** vt) {
    for (int i = 0; i < g_vtCount; ++i)
        if (g_vts[i].vt == vt) return &g_vts[i];
    return nullptr;
}

static void CreateDummySwapChain(bool flip, void** outVt) {
    WNDCLASSEXA wc{};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = DefWindowProcA;
    wc.hInstance = GetModuleHandleA(nullptr);
    wc.lpszClassName = "obsyde_dx_dummy";
    RegisterClassExA(&wc);

    HWND tw = CreateWindowExA(0, wc.lpszClassName, "", WS_OVERLAPPEDWINDOW,
                              0, 0, 2, 2, nullptr, nullptr, wc.hInstance, nullptr);
    if (!tw) return;

    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferCount = flip ? 2 : 1;
    sd.BufferDesc.Width = 2;
    sd.BufferDesc.Height = 2;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = tw;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.SwapEffect = flip ? DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL : DXGI_SWAP_EFFECT_DISCARD;

    IDXGISwapChain* sc = nullptr;
    ID3D11Device* dev = nullptr;
    ID3D11DeviceContext* ctx = nullptr;
    D3D_FEATURE_LEVEL fl;

    HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
                                               nullptr, 0, D3D11_SDK_VERSION,
                                               &sd, &sc, &dev, &fl, &ctx);
    if (SUCCEEDED(hr) && sc)
        *outVt = *(void***)sc;

    if (sc) sc->Release();
    if (dev) dev->Release();
    if (ctx) ctx->Release();
    DestroyWindow(tw);
}

void InstallHooks() {
    if (g_hooked) return;

    void* vtA = nullptr;
    void* vtB = nullptr;
    CreateDummySwapChain(false, &vtA);
    CreateDummySwapChain(true, &vtB);

    PatchVTable((void**)vtA, false);
    PatchVTable((void**)vtB, true);

    g_hooked = g_vtCount > 0;
}

static void CreateRTV(IDXGISwapChain* sc) {
    if (g_rtv) { g_rtv->Release(); g_rtv = nullptr; }
    ID3D11Texture2D* bb = nullptr;
    if (SUCCEEDED(sc->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&bb)) && bb) {
        g_device->CreateRenderTargetView(bb, nullptr, &g_rtv);
        bb->Release();
    }
}

static void InitImGui(IDXGISwapChain* sc) {
    DXGI_SWAP_CHAIN_DESC d{};
    if (FAILED(sc->GetDesc(&d))) return;
    g_hwnd = d.OutputWindow;

    if (FAILED(sc->GetDevice(__uuidof(ID3D11Device), (void**)&g_device)) || !g_device)
        return;
    g_device->GetImmediateContext(&g_ctx);
    CreateRTV(sc);
    if (!g_rtv) return;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    theme::LoadFonts();

    ImGui_ImplWin32_Init(g_hwnd);
    ImGui_ImplDX11_Init(g_device, g_ctx);

    g_origWndProc = (WNDPROC)SetWindowLongPtrA(g_hwnd, GWLP_WNDPROC, (LONG_PTR)hkWndProc);

    g_init = true;
}

static LRESULT CALLBACK hkWndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (cfg::menuOpen && ImGui_ImplWin32_WndProcHandler(hWnd, msg, wp, lp))
        return TRUE;

    if (cfg::menuOpen) {
        switch (msg) {
        case WM_MOUSEMOVE:
        case WM_LBUTTONDOWN: case WM_LBUTTONUP: case WM_LBUTTONDBLCLK:
        case WM_RBUTTONDOWN: case WM_RBUTTONUP: case WM_RBUTTONDBLCLK:
        case WM_MBUTTONDOWN: case WM_MBUTTONUP: case WM_MBUTTONDBLCLK:
        case WM_XBUTTONDOWN: case WM_XBUTTONUP:
        case WM_MOUSEWHEEL:  case WM_MOUSEHWHEEL:
        case WM_KEYDOWN:     case WM_KEYUP:
        case WM_SYSKEYDOWN:  case WM_SYSKEYUP:
        case WM_CHAR:        case WM_SYSCHAR:
        case WM_INPUT:
            return TRUE;
        default:
            break;
        }
    }
    return CallWindowProcA(g_origWndProc, hWnd, msg, wp, lp);
}

static void DoFrame() {
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    if (GetAsyncKeyState(cfg::menuKey) & 1) {
        cfg::menuOpen = !cfg::menuOpen;
        ImGui::GetIO().MouseDrawCursor = cfg::menuOpen;
        if (cfg::menuOpen) {
            ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouseCursorChange;
        } else {
            ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
        }
    }
    if (GetAsyncKeyState(cfg::unloadKey) & 1)
        RequestUnload();

    binds::ProcessBinds();

    if (cfg::menuOpen && g_hwnd)
        ClipCursor(nullptr);

    uintptr_t ctrl = game::g_sys.localCtrl;
    if (mem::ValidPtr(ctrl))
        game::BuildFrame(g_frame, ctrl);

    view::Update();

    if (cfg::menuOpen) DrawMenuWindow();
    DrawOverlay(g_frame);
    DrawKeybinds();
    RunAutomation(g_frame);
    RunAutoAccept();

    ImGui::Render();
    if (g_rtv && g_ctx) {
        g_ctx->OMSetRenderTargets(1, &g_rtv, nullptr);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    }
}

static HRESULT __stdcall hkPresent(IDXGISwapChain* sc, UINT sync, UINT flags) {
    HookedVt* h = FindVt(*(void***)sc);
    PresentFn orig = h ? (PresentFn)h->origPresent : nullptr;
    if (!orig) return S_OK;

    if (cfg::running.load()) {
        __try {
            if (!g_init && !g_initFailed) {
                InitImGui(sc);
                if (!g_init) g_initFailed = true;
            }
            if (g_init) DoFrame();
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            g_initFailed = true;
        }
    }
    return orig(sc, sync, flags);
}

static HRESULT __stdcall hkPresent1(IDXGISwapChain1* sc, UINT sync, UINT flags, const DXGI_PRESENT_PARAMETERS* p) {
    HookedVt* h = FindVt(*(void***)sc);
    Present1Fn orig = h ? (Present1Fn)h->origPresent1 : nullptr;
    if (!orig) return S_OK;

    if (cfg::running.load()) {
        __try {
            if (!g_init && !g_initFailed) {
                InitImGui(sc);
                if (!g_init) g_initFailed = true;
            }
            if (g_init) DoFrame();
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            g_initFailed = true;
        }
    }
    return orig(sc, sync, flags, p);
}

static HRESULT __stdcall hkResize(IDXGISwapChain* sc, UINT bc, UINT w, UINT h, DXGI_FORMAT fmt, UINT flags) {
    HookedVt* hv = FindVt(*(void***)sc);
    ResizeFn orig = hv ? (ResizeFn)hv->origResize : nullptr;
    if (!orig) return S_OK;

    if (g_rtv) { g_rtv->Release(); g_rtv = nullptr; }
    HRESULT hr = orig(sc, bc, w, h, fmt, flags);
    if (g_device) CreateRTV(sc);
    return hr;
}

static DWORD WINAPI UnloadThread(LPVOID mod) {
    cfg::running.store(false);
    Sleep(1200);

    for (int i = 0; i < g_vtCount; ++i) {
        HookedVt& h = g_vts[i];
        if (!h.vt) continue;
        DWORD old = 0;
        if (h.origPresent) {
            VirtualProtect(&h.vt[8], sizeof(void*), PAGE_EXECUTE_READWRITE, &old);
            h.vt[8] = h.origPresent;
            VirtualProtect(&h.vt[8], sizeof(void*), old, &old);
        }
        if (h.origResize) {
            VirtualProtect(&h.vt[13], sizeof(void*), PAGE_EXECUTE_READWRITE, &old);
            h.vt[13] = h.origResize;
            VirtualProtect(&h.vt[13], sizeof(void*), old, &old);
        }
        if (h.p1 && h.origPresent1) {
            VirtualProtect(&h.vt[21], sizeof(void*), PAGE_EXECUTE_READWRITE, &old);
            h.vt[21] = h.origPresent1;
            VirtualProtect(&h.vt[21], sizeof(void*), old, &old);
        }
    }

    if (g_origWndProc && g_hwnd)
        SetWindowLongPtrA(g_hwnd, GWLP_WNDPROC, (LONG_PTR)g_origWndProc);

    Sleep(400);
    if (g_init) {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
        g_init = false;
    }
    if (g_rtv) { g_rtv->Release(); g_rtv = nullptr; }
    if (g_ctx) { g_ctx->Release(); g_ctx = nullptr; }
    if (g_device) { g_device->Release(); g_device = nullptr; }

    Sleep(700);
    FreeLibraryAndExitThread((HMODULE)mod, 0);
    return 0;
}

void RequestUnload() {
    if (!cfg::running.load()) return;
    HMODULE mod = nullptr;
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                       (LPCSTR)&UnloadThread, &mod);
    HANDLE t = CreateThread(nullptr, 0, UnloadThread, mod, 0, nullptr);
    if (t) CloseHandle(t);
}
