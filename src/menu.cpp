#include "hook.h"
#include "theme.h"
#include "glass.h"
#include "imgui.h"

#include <cfloat>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <unordered_map>

void theme::ApplyStyle() {
    ImGuiStyle& s = ImGui::GetStyle();

    s.WindowRounding    = 14.f;
    s.ChildRounding     = 10.f;
    s.FrameRounding     = 6.f;
    s.PopupRounding     = 10.f;
    s.GrabRounding      = 8.f;
    s.TabRounding       = 8.f;
    s.ScrollbarRounding = 8.f;

    s.WindowPadding     = ImVec2(14, 10);
    s.FramePadding      = ImVec2(10, 4);
    s.ItemSpacing       = ImVec2(10, 8);
    s.ItemInnerSpacing  = ImVec2(8, 4);
    s.ScrollbarSize     = 8.f;
    s.IndentSpacing     = 16.f;

    s.WindowBorderSize  = 0.f;
    s.ChildBorderSize   = 0.f;
    s.FrameBorderSize   = 0.f;
    s.PopupBorderSize   = 1.f;

    auto set = [&](ImGuiCol i, ImVec4 v) {
        if ((int)i >= 0 && (int)i < (int)ImGuiCol_COUNT) s.Colors[i] = v;
    };

    set(ImGuiCol_Text,             ImVec4(0.96f, 0.96f, 0.97f, 1.00f));
    set(ImGuiCol_TextDisabled,     ImVec4(0.55f, 0.55f, 0.58f, 1.00f));
    set(ImGuiCol_WindowBg,         ImVec4(0.058f, 0.058f, 0.070f, 0.72f));
    set(ImGuiCol_ChildBg,          ImVec4(0.115f, 0.115f, 0.140f, 0.85f));
    set(ImGuiCol_PopupBg,          ImVec4(0.075f, 0.075f, 0.090f, 0.95f));
    set(ImGuiCol_Border,           ImVec4(1.f, 1.f, 1.f, 0.11f));
    set(ImGuiCol_BorderShadow,     ImVec4(0.f, 0.f, 0.f, 0.50f));
    set(ImGuiCol_FrameBg,          ImVec4(1.f, 1.f, 1.f, 0.06f));
    set(ImGuiCol_FrameBgHovered,   ImVec4(1.f, 1.f, 1.f, 0.10f));
    set(ImGuiCol_FrameBgActive,    ImVec4(1.f, 1.f, 1.f, 0.14f));
    set(ImGuiCol_TitleBg,          ImVec4(0.03f, 0.03f, 0.04f, 0.60f));
    set(ImGuiCol_TitleBgActive,    ImVec4(0.03f, 0.03f, 0.04f, 0.60f));
    set(ImGuiCol_TitleBgCollapsed, ImVec4(0.03f, 0.03f, 0.04f, 0.40f));
    set(ImGuiCol_MenuBarBg,        ImVec4(0.03f, 0.03f, 0.04f, 0.60f));
    set(ImGuiCol_ScrollbarBg,      ImVec4(0.f, 0.f, 0.f, 0.f));
    set(ImGuiCol_ScrollbarGrab,        ImVec4(1.f, 1.f, 1.f, 0.15f));
    set(ImGuiCol_ScrollbarGrabHovered, ImVec4(1.f, 1.f, 1.f, 0.25f));
    set(ImGuiCol_ScrollbarGrabActive,  ImVec4(1.f, 1.f, 1.f, 0.35f));
    set(ImGuiCol_CheckMark,        ImVec4(0.96f, 0.96f, 0.97f, 1.f));
    set(ImGuiCol_SliderGrab,       ImVec4(0.96f, 0.96f, 0.97f, 1.f));
    set(ImGuiCol_SliderGrabActive, ImVec4(1.f, 1.f, 1.f, 1.f));
    set(ImGuiCol_Button,           ImVec4(1.f, 1.f, 1.f, 0.07f));
    set(ImGuiCol_ButtonHovered,    ImVec4(1.f, 1.f, 1.f, 0.12f));
    set(ImGuiCol_ButtonActive,     ImVec4(1.f, 1.f, 1.f, 0.18f));
    set(ImGuiCol_Header,           ImVec4(1.f, 1.f, 1.f, 0.08f));
    set(ImGuiCol_HeaderHovered,    ImVec4(1.f, 1.f, 1.f, 0.12f));
    set(ImGuiCol_HeaderActive,     ImVec4(1.f, 1.f, 1.f, 0.16f));
    set(ImGuiCol_Separator,        ImVec4(1.f, 1.f, 1.f, 0.10f));
    set(ImGuiCol_SeparatorHovered, ImVec4(1.f, 1.f, 1.f, 0.20f));
    set(ImGuiCol_SeparatorActive,  ImVec4(1.f, 1.f, 1.f, 0.30f));
    set(ImGuiCol_ResizeGrip,       ImVec4(1.f, 1.f, 1.f, 0.10f));
    set(ImGuiCol_ResizeGripHovered,ImVec4(1.f, 1.f, 1.f, 0.20f));
    set(ImGuiCol_ResizeGripActive, ImVec4(1.f, 1.f, 1.f, 0.30f));
    set(ImGuiCol_Tab,              ImVec4(1.f, 1.f, 1.f, 0.06f));
    set(ImGuiCol_TabHovered,       ImVec4(1.f, 1.f, 1.f, 0.12f));
    set(ImGuiCol_TabSelected,      ImVec4(1.f, 1.f, 1.f, 0.14f));
    set(ImGuiCol_PlotLines,        ImVec4(0.96f, 0.96f, 0.97f, 1.f));
    set(ImGuiCol_PlotHistogram,    ImVec4(0.96f, 0.96f, 0.97f, 1.f));
    set(ImGuiCol_TableHeaderBg,    ImVec4(1.f, 1.f, 1.f, 0.06f));
    set(ImGuiCol_TableBorderStrong,ImVec4(1.f, 1.f, 1.f, 0.10f));
    set(ImGuiCol_TableBorderLight, ImVec4(1.f, 1.f, 1.f, 0.05f));
    set(ImGuiCol_TextSelectedBg,   ImVec4(1.f, 1.f, 1.f, 0.20f));
    set(ImGuiCol_DragDropTarget,   ImVec4(1.f, 1.f, 1.f, 0.60f));
    set(ImGuiCol_NavCursor,        ImVec4(1.f, 1.f, 1.f, 0.60f));
    set(ImGuiCol_ModalWindowDimBg, ImVec4(0.f, 0.f, 0.f, 0.55f));
}

static float Anim(ImGuiID id, float target, float speed = 14.f) {
    static std::unordered_map<ImGuiID, float> m;
    float& v = m[id];
    float k = 1.f - expf(-speed * ImGui::GetIO().DeltaTime);
    v += (target - v) * k;
    return v;
}

static float s_fade = 1.f;

static ImU32 FA(ImU32 c) {
    int a = (int)(((c >> 24) & 0xFF) * s_fade);
    return (c & 0x00FFFFFFu) | ((ImU32)a << 24);
}

static ImU32 WA(ImU32 c, int a) { return FA(theme::WithAlpha(c, a)); }

static ImU32 Mix(ImU32 a, ImU32 b, float t) {
    if (t < 0.f) t = 0.f;
    if (t > 1.f) t = 1.f;
    int ar = a & 0xFF, ag = (a >> 8) & 0xFF, ab = (a >> 16) & 0xFF, aa = (a >> 24) & 0xFF;
    int br = b & 0xFF, bg = (b >> 8) & 0xFF, bb = (b >> 16) & 0xFF, ba = (b >> 24) & 0xFF;
    int ra = (int)((aa + (ba - aa) * t) * s_fade);
    return IM_COL32((int)(ar + (br - ar) * t), (int)(ag + (bg - ag) * t),
                    (int)(ab + (bb - ab) * t), ra);
}

static ImFont* F() { return theme::FontSmall(); }

static float TextW(const char* s, float size = 0.f) {
    ImFont* f = F();
    if (!f) return 0.f;
    float sz = size > 0.f ? size : f->LegacySize;
    return f->CalcTextSizeA(sz, FLT_MAX, 0.f, s).x;
}

static void TabIcon(ImDrawList* dl, int idx, ImVec2 c, ImU32 col) {
    const float HP = 1.5707963f;
    switch (idx) {
    case 0:
        dl->PathArcTo(c, 6.f, HP, 2.f * HP, 8); dl->PathStroke(col, 0, 1.4f);
        dl->PathArcTo(c, 6.f, 0.f, HP, 8);      dl->PathStroke(col, 0, 1.4f);
        dl->AddCircleFilled(c, 2.f, col, 8);
        break;
    case 1:
        dl->AddCircle(c, 3.f, col, 10, 1.4f);
        for (int k = 0; k < 8; ++k) {
            float a = HP * (float)k * 0.5f;
            dl->AddLine(ImVec2(c.x + cosf(a) * 4.5f, c.y + sinf(a) * 4.5f),
                        ImVec2(c.x + cosf(a) * 6.5f, c.y + sinf(a) * 6.5f), col, 1.4f);
        }
        break;
    case 2:
        dl->AddRect(ImVec2(c.x - 6.f, c.y - 4.f), ImVec2(c.x + 6.f, c.y + 4.f), col, 2.f, 0, 1.4f);
        dl->AddCircle(c, 2.2f, col, 10, 1.4f);
        break;
    case 3:
        dl->AddRect(ImVec2(c.x - 6.f, c.y - 5.f), ImVec2(c.x + 3.f, c.y + 1.f), col, 2.f, 0, 1.4f);
        dl->AddRect(ImVec2(c.x - 3.f, c.y - 1.f), ImVec2(c.x + 6.f, c.y + 5.f), col, 2.f, 0, 1.4f);
        break;
    case 4:
        {
            ImVec2 p[4] = { { c.x + 1.f, c.y - 6.f }, { c.x - 3.f, c.y + 1.f },
                            { c.x + 0.f, c.y + 1.f }, { c.x - 1.f, c.y + 6.f } };
            dl->AddPolyline(p, 4, col, 0, 1.4f);
        }
        break;
    default:
        break;
    }
}

static struct {
    float x = 0, y0 = 0, gap = 10, colW = 0;
    float yy[2] = { 0, 0 };
    int last = 0;
} G;

static void GridStart(ImVec2 p, float w, float gap) {
    G.x = p.x; G.y0 = p.y; G.gap = gap;
    G.colW = (w - gap) * 0.5f;
    G.yy[0] = G.yy[1] = 0;
}

static void CardBegin(const char* id) {
    G.last = (G.yy[0] <= G.yy[1]) ? 0 : 1;
    ImGui::SetCursorScreenPos(ImVec2(G.x + G.last * (G.colW + G.gap), G.y0 + G.yy[G.last]));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(18.f, 14.f));
    ImGui::BeginChild(id, ImVec2(G.colW, 0), ImGuiChildFlags_AutoResizeY, 0);
}

static void CardEnd() {
    ImVec2 cwp = ImGui::GetWindowPos();
    ImVec2 cws = ImGui::GetWindowSize();
    ImDrawList* cdl = ImGui::GetWindowDrawList();
    cdl->AddRect(ImVec2(cwp.x + 0.5f, cwp.y + 0.5f), ImVec2(cwp.x + cws.x - 0.5f, cwp.y + cws.y - 0.5f),
                 WA(theme::White, 50), 9.5f, 0, 1.f);
    cdl->AddRect(ImVec2(cwp.x - 1.f, cwp.y - 1.f), ImVec2(cwp.x + cws.x + 1.f, cwp.y + cws.y + 1.f),
                 WA(theme::White, 15), 10.5f, 0, 1.5f);
    ImGui::EndChild();
    ImGui::PopStyleVar();
    G.yy[G.last] += ImGui::GetItemRectSize().y + G.gap;
}

static int s_openReq = -1;

static void CheckRow(const char* label, bool* v, int bind = -1) {
    ImGuiID id = ImGui::GetID(label);
    ImVec2 p = ImGui::GetCursorScreenPos();
    float w = ImGui::GetContentRegionAvail().x;
    float h = 24.f;
    ImDrawList* dl = ImGui::GetWindowDrawList();

    float hov = Anim(id ^ 0x9e37, ImGui::IsMouseHoveringRect(p, ImVec2(p.x + w, p.y + h)) ? 1.f : 0.f, 20.f);
    if (hov > 0.02f)
        dl->AddRectFilled(ImVec2(p.x - 6.f, p.y), ImVec2(p.x + w + 6.f, p.y + h),
                          WA(theme::White, (int)(10 * hov)), 5.f);

    float on = Anim(id, *v ? 1.f : 0.f, 16.f);
    dl->AddText(ImVec2(p.x + 1.f, p.y + 4.f), Mix(theme::Dim, theme::White, on), label);

    if (bind >= 0 && binds::items[bind].vk) {
        float lw = TextW(label);
        dl->AddText(F(), 11.f, ImVec2(p.x + lw + 8.f, p.y + 6.f),
                    WA(theme::AccentLight, 160), binds::KeyName(binds::items[bind].vk));
    }

    float sw = 30.f, sh = 15.f, rad = sh * 0.5f;
    ImVec2 b0(p.x + w - sw, p.y + (h - sh) * 0.5f), b1(b0.x + sw, b0.y + sh);
    dl->AddRectFilled(b0, b1, Mix(WA(theme::White, 16), theme::Accent, on), rad);
    dl->AddRect(b0, b1, Mix(WA(theme::White, 30), theme::AccentLight, on), rad, 0, 1.f);
    float kx = b0.x + rad + on * (sw - sh);
    dl->AddCircleFilled(ImVec2(kx, b0.y + rad), rad - 2.4f,
                        Mix(WA(theme::White, 150), theme::White, on), 14);

    ImGui::InvisibleButton(label, ImVec2(w, h));
    if (ImGui::IsItemClicked()) *v = !*v;
    if (bind >= 0 && ImGui::IsItemClicked(ImGuiMouseButton_Right)) s_openReq = bind;
}

static void SliderRow(const char* label, float* v, float mn, float mx, const char* fmt) {
    ImGuiID id = ImGui::GetID(label);
    ImVec2 p = ImGui::GetCursorScreenPos();
    float w = ImGui::GetContentRegionAvail().x;
    float h = 24.f;
    ImDrawList* dl = ImGui::GetWindowDrawList();

    dl->AddText(ImVec2(p.x + 1.f, p.y + 4.f), FA(theme::White), label);

    float sw = 110.f;
    float sx = p.x + w - sw;
    char buf[32];
    snprintf(buf, sizeof(buf), fmt, *v);
    float tw = TextW(buf, 12.f);
    dl->AddText(F(), 12.f, ImVec2(sx - tw - 8.f, p.y + 5.f), FA(theme::Dim), buf);

    float ty = p.y + h * 0.5f;
    float frac = (*v - mn) / (mx - mn);
    if (frac < 0.f) frac = 0.f;
    if (frac > 1.f) frac = 1.f;

    dl->AddRectFilled(ImVec2(sx, ty - 1.f), ImVec2(sx + sw, ty + 1.f),
                      WA(theme::White, 30), 1.f);
    dl->AddRectFilled(ImVec2(sx, ty - 1.f), ImVec2(sx + sw * frac, ty + 1.f),
                      FA(theme::Accent), 1.f);
    dl->AddCircleFilled(ImVec2(sx + sw * frac, ty), 4.f, FA(theme::White), 12);

    ImGui::InvisibleButton(label, ImVec2(w, h));
    if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
        float t = (ImGui::GetIO().MousePos.x - sx) / sw;
        if (t < 0.f) t = 0.f;
        if (t > 1.f) t = 1.f;
        *v = mn + t * (mx - mn);
    }
    (void)id;
}

static void ItemCard(const char* id, const char* label, bool* v, int bind = -1) {
    CardBegin(id);
    CheckRow(label, v, bind);
    CardEnd();
}

static bool s_capture = false;

static bool Btn(const char* label, float pad = 12.f, float h = 22.f) {
    ImVec2 p = ImGui::GetCursorScreenPos();
    float bw = TextW(label) + pad * 2.f;
    ImGui::InvisibleButton(label, ImVec2(bw, h));
    bool hov = ImGui::IsItemHovered();
    bool act = ImGui::IsItemActive();
    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddRectFilled(p, ImVec2(p.x + bw, p.y + h),
                      WA(theme::White, act ? 30 : (hov ? 17 : 9)), 6.f);
    dl->AddRect(p, ImVec2(p.x + bw, p.y + h),
                WA(theme::White, hov ? 70 : 40), 6.f, 0, 1.f);
    dl->AddText(ImVec2(p.x + pad, p.y + (h - 13.f) * 0.5f), FA(theme::White), label);
    return ImGui::IsItemClicked();
}

static void RebindPopup(int bi) {
    KeyBind& b = binds::items[bi];

    ImGui::Text("bind - %s", b.name);
    ImGui::Spacing();

    char cur[80];
    if (b.vk) snprintf(cur, sizeof(cur), "%s   %s", binds::KeyName(b.vk),
                       b.hold ? "hold" : "toggle");
    else      snprintf(cur, sizeof(cur), "unbound");
    ImGui::Text("current: %s", cur);
    ImGui::Spacing();

    if (s_capture && cfg::menuOpen) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.96f, 0.96f, 0.97f, 1.f));
        ImGui::TextUnformatted("press any key...   esc - cancel");
        ImGui::PopStyleColor();

        for (int k = 3; k < 256; ++k) {
            if (k == VK_ESCAPE) continue;
            if (!binds::Edge[k]) continue;
            b.vk = k;
            if (b.holding && b.target) {
                *b.target = b.holdSaved;
                b.holding = false;
            }
            s_capture = false;
            break;
        }
        if (s_capture && binds::Edge[VK_ESCAPE]) s_capture = false;
    } else {
        if (Btn("set key")) s_capture = true;
        ImGui::SameLine();
        if (Btn("clear")) {
            b.vk = 0;
            if (b.holding && b.target) {
                *b.target = b.holdSaved;
                b.holding = false;
            }
        }
        ImGui::SameLine();
        if (Btn(b.hold ? "mode: hold" : "mode: toggle")) b.hold = !b.hold;
    }

    ImGui::Spacing();
    if (Btn("done")) ImGui::CloseCurrentPopup();
}

static void TabESP() {
    ItemCard("c_hero", "Heroes ESP", &cfg::espHeroes, 4);
    ItemCard("c_boxes", "Boxes", &cfg::espBoxes);
    ItemCard("c_cds", "Skill cooldowns", &cfg::espCds);
    ItemCard("c_dist", "Distance", &cfg::espDist);
    ItemCard("c_prev", "Skill preview", &cfg::skillPreview);
    ItemCard("c_mark", "Last hit", &cfg::lastHit, 3);
    ItemCard("c_deny", "Deny markers", &cfg::deny);
    ItemCard("c_ros", "Roshan", &cfg::espRoshan);
    ItemCard("c_ward", "Wards", &cfg::espWards, 5);
}

static void TabAuto() {
    ItemCard("c_farm", "Farm bot", &cfg::farmBot, 0);
    ItemCard("c_atk", "Auto attack", &cfg::farmAuto, 1);
    ItemCard("c_acc", "Auto accept", &cfg::autoAccept, 2);
    ItemCard("c_dod", "Dodge alerts", &cfg::dodger);
    ItemCard("c_adod", "Auto dodge", &cfg::autoDodge, 8);
}

static void TabVision() {
    ItemCard("c_fog", "Remove fog", &cfg::vbe, 6);
    ItemCard("c_glow", "Enemy glow", &cfg::glow, 7);
}

static void TabCamera() {
    ItemCard("c_zoom", "Zoom out", &cfg::camEnable, 10);
    CardBegin("c_cdist");
    SliderRow("Distance", &cfg::camDistance, 200.f, 3000.f, "%.0f");
    CardEnd();
}

static void TabOverlay() {
    ItemCard("c_plus", "Dota plus panel", &cfg::dotaPlus, 9);
    ItemCard("c_kb", "Keybinds widget", &cfg::showKeybinds);
    ItemCard("c_glass", "Glass blur", &glass::enabled);
}

struct NavItem { int icon; const char* label; int tab; };
struct NavGroup { const char* cap; NavItem items[3]; int n; };

static const NavGroup kNav[] = {
    { "MAIN",      { { 0, "ESP",     0 }, { 4, "Auto",    1 } }, 2 },
    { "VISUALS",   { { 1, "Vision",  2 }, { 2, "Camera",  3 }, { 3, "Overlay", 4 } }, 3 },
};

static const char* kTabLabel[5] = { "ESP", "Auto", "Vision", "Camera", "Overlay" };

static int s_tab = 0;
static float s_tabT = 1.f;
static float s_appear = 0.f;

bool MenuFading() { return s_appear > 0.001f; }

void DrawMenuWindow() {
    static bool s_styled = false;
    if (!s_styled) { s_styled = true; theme::ApplyStyle(); }

    float dt = ImGui::GetIO().DeltaTime;

    float rate = cfg::menuOpen ? 13.f : 22.f;
    s_appear += ((cfg::menuOpen ? 1.f : 0.f) - s_appear) * (1.f - expf(-rate * dt));
    if (s_appear < 0.003f) s_appear = 0.f;
    if (s_appear > 0.997f) s_appear = 1.f;
    if (s_appear <= 0.f) { glass::SetAlpha(1.f); s_fade = 1.f; return; }
    float e = s_appear * s_appear * (3.f - 2.f * s_appear);
    s_fade = e;

    glass::SetAlpha(cfg::menuOpen ? e : e * e);

    const float W = 700.f, H = 480.f;
    static ImVec2 s_center = ImVec2(30.f + W * 0.5f, 30.f + H * 0.5f);
    ImGui::SetNextWindowPos(ImVec2(30, 30), ImGuiCond_FirstUseEver);
    if (s_appear < 1.f) {
        float sc = 0.965f + 0.035f * e;
        ImGui::SetNextWindowSize(ImVec2(W * sc, H * sc), ImGuiCond_Always);
        ImGui::SetNextWindowPos(ImVec2(s_center.x - W * sc * 0.5f, s_center.y - H * sc * 0.5f),
                                ImGuiCond_Always);
    } else {
        ImGui::SetNextWindowSize(ImVec2(W, H), ImGuiCond_Always);
    }

    ImGuiWindowFlags fl =
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoFocusOnAppearing;
    if (!cfg::menuOpen)
        fl |= ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoNav;

    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, e);
    ImGui::Begin("obsyde", nullptr, fl);

    ImVec2 wp = ImGui::GetWindowPos();
    ImVec2 ws = ImGui::GetWindowSize();
    if (s_appear >= 1.f)
        s_center = ImVec2(wp.x + ws.x * 0.5f, wp.y + ws.y * 0.5f);
    glass::AddRect(wp, ImVec2(wp.x + ws.x, wp.y + ws.y), 14.f);
    ImDrawList* dl = ImGui::GetWindowDrawList();

    dl->AddRect(ImVec2(wp.x + 0.5f, wp.y + 0.5f), ImVec2(wp.x + ws.x - 0.5f, wp.y + ws.y - 0.5f),
                WA(theme::White, 52), 14.f, 0, 1.2f);
    dl->AddRect(ImVec2(wp.x - 1.5f, wp.y - 1.5f), ImVec2(wp.x + ws.x + 1.5f, wp.y + ws.y + 1.5f),
                WA(theme::White, 15), 15.5f, 0, 2.f);
    ImFont* big = theme::FontBig();
    float titleH = big ? big->LegacySize : 20.f;

    if (big) {
        const char* logo = "obsyde";
        dl->AddText(big, titleH, ImVec2(wp.x + 14.f, wp.y + 9.f), FA(theme::White), logo);
    }
    ImVec2 under(wp.x + 14.f, wp.y + 9.f + titleH + 5.f);
    dl->AddLine(under, ImVec2(wp.x + ws.x - 14.f, under.y),
                WA(theme::White, 40), 1.4f);

    float headerH = 9.f + titleH + 12.f;
    ImGui::SetCursorScreenPos(wp);
    ImGui::InvisibleButton("##hdr_drag", ImVec2(ws.x - 36.f, headerH));
    if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left, 1.f)) {
        ImVec2 d = ImGui::GetIO().MouseDelta;
        ImGui::SetWindowPos(ImVec2(wp.x + d.x, wp.y + d.y));
    }

    ImVec2 xb(wp.x + ws.x - 36.f, wp.y + 5.f);
    ImGui::SetCursorScreenPos(xb);
    ImGui::InvisibleButton("##close", ImVec2(22.f, 22.f));
    {
        bool hov = ImGui::IsItemHovered();
        bool act = ImGui::IsItemActive();
        if (act || hov)
            dl->AddRectFilled(xb, ImVec2(xb.x + 22.f, xb.y + 22.f),
                              WA(theme::White, act ? 30 : 15), 6.f);
        dl->AddRect(xb, ImVec2(xb.x + 22.f, xb.y + 22.f),
                    WA(theme::White, hov ? 90 : 45), 6.f, 0, 1.f);
        dl->AddLine(ImVec2(xb.x + 8.f, xb.y + 8.f), ImVec2(xb.x + 14.f, xb.y + 14.f),
                    FA(hov ? theme::White : theme::Dim), 1.4f);
        dl->AddLine(ImVec2(xb.x + 14.f, xb.y + 8.f), ImVec2(xb.x + 8.f, xb.y + 14.f),
                    FA(hov ? theme::White : theme::Dim), 1.4f);
        if (ImGui::IsItemClicked()) cfg::menuOpen = false;
    }

    const float navW = 150.f;
    float navX = wp.x + 14.f;
    float navY = wp.y + headerH + 8.f;

    for (int g = 0; g < 2; ++g) {
        const NavGroup& grp = kNav[g];
        dl->AddText(F(), 10.f, ImVec2(navX + 38.f, navY), WA(theme::White, 70), grp.cap);
        navY += 16.f;

        for (int i = 0; i < grp.n; ++i) {
            const NavItem& it = grp.items[i];
            ImGuiID id = ImGui::GetID(it.label);
            ImVec2 bp(navX, navY);
            ImVec2 br(navX + navW, navY + 30.f);

            bool hov = ImGui::IsMouseHoveringRect(bp, br);
            float t = Anim(id, s_tab == it.tab ? 1.f : 0.f, 16.f);
            float h = Anim(id ^ 0x51f, hov ? 1.f : 0.f, 22.f);

            if (h > 0.02f)
                dl->AddRectFilled(ImVec2(bp.x + 4.f, bp.y + 1.f), ImVec2(br.x - 4.f, br.y - 1.f),
                                  WA(theme::White, (int)(14 * h)), 7.f);
            if (t > 0.02f)
                dl->AddRectFilled(ImVec2(bp.x + 2.f, bp.y + 6.f), ImVec2(bp.x + 4.f, br.y - 6.f),
                                  WA(theme::Accent, (int)(255 * t)), 1.f);

            ImVec2 c(bp.x + 20.f, bp.y + 15.f);
            ImU32 ic = Mix(theme::Dim, theme::White, t > h ? t : h);
            TabIcon(dl, it.icon, c, ic);
            dl->AddText(ImVec2(bp.x + 38.f, bp.y + 8.f), Mix(theme::Dim, theme::White, t), it.label);

            ImGui::SetCursorScreenPos(bp);
            ImGui::InvisibleButton(it.label, ImVec2(navW, 30.f));
            if (ImGui::IsItemClicked() && s_tab != it.tab) { s_tab = it.tab; s_tabT = 0.f; }

            navY += 32.f;
        }
        navY += 8.f;
    }

    dl->AddLine(ImVec2(navX + navW + 6.f, wp.y + headerH + 6.f),
                ImVec2(navX + navW + 6.f, wp.y + ws.y - 30.f),
                WA(theme::White, 22), 1.f);

    float cx = navX + navW + 18.f;
    float cw = wp.x + ws.x - 14.f - cx;
    float cy = navY = wp.y + headerH + 8.f;
    float chh = wp.y + ws.y - 30.f - cy;

    s_tabT += ImGui::GetIO().DeltaTime * 6.f;
    if (s_tabT > 1.f) s_tabT = 1.f;
    float slide = (1.f - s_tabT) * 6.f;

    (void)chh;

    {
        ImVec2 p(cx, cy);
        dl->AddText(ImVec2(p.x, p.y + slide), FA(theme::Dim), "obsyde");
        float w1 = TextW("obsyde");
        dl->AddText(ImVec2(p.x + w1 + 6.f, p.y + slide), FA(theme::Dim), "/");
        dl->AddText(ImVec2(p.x + w1 + 16.f, p.y + slide), FA(theme::AccentLight), kTabLabel[s_tab]);
    }

    GridStart(ImVec2(cx, cy + 26.f + slide), cw, 12.f);

    switch (s_tab) {
    case 0: TabESP();     break;
    case 1: TabAuto();    break;
    case 2: TabVision();  break;
    case 3: TabCamera();  break;
    default: TabOverlay(); break;
    }

    if (s_openReq >= 0) {
        cfg::rebindIdx = s_openReq;
        s_openReq = -1;
        ImGui::OpenPopup("rebind");
    }
    if (cfg::rebindIdx >= 0) {
        if (ImGui::BeginPopup("rebind")) {
            RebindPopup(cfg::rebindIdx);
            ImGui::EndPopup();
        } else {
            cfg::rebindIdx = -1;
            s_capture = false;
        }
    }

    const char* foot = "INSERT - menu   END - unload";
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.55f, 0.55f, 0.58f, 1.f));
    float tw = ImGui::CalcTextSize(foot).x;
    ImGui::SetCursorScreenPos(ImVec2(wp.x + (ws.x - tw) * 0.5f, wp.y + ws.y - 22.f));
    ImGui::TextUnformatted(foot);
    ImGui::PopStyleColor();

    ImGui::End();
    ImGui::PopStyleVar();
}

