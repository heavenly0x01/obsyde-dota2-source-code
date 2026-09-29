#include "hook.h"
#include "theme.h"
#include "imgui.h"

void theme::ApplyStyle() {
    ImGuiStyle& s = ImGui::GetStyle();

    s.WindowRounding    = 8.f;
    s.ChildRounding     = 6.f;
    s.FrameRounding     = 4.f;
    s.PopupRounding     = 6.f;
    s.GrabRounding      = 4.f;
    s.TabRounding       = 6.f;
    s.ScrollbarRounding = 6.f;

    s.WindowPadding  = ImVec2(12, 10);
    s.FramePadding   = ImVec2(10, 4);
    s.ItemSpacing    = ImVec2(10, 6);
    s.ItemInnerSpacing = ImVec2(8, 4);
    s.ScrollbarSize  = 10.f;
    s.IndentSpacing  = 16.f;

    s.WindowBorderSize = 1.f;
    s.ChildBorderSize  = 1.f;
    s.FrameBorderSize  = 0.f;
    s.PopupBorderSize   = 1.f;

    auto set = [&](ImGuiCol i, ImVec4 v) {
        if ((int)i >= 0 && (int)i < (int)ImGuiCol_COUNT) s.Colors[i] = v;
    };

    const ImVec4 winBg      (0.035f, 0.020f, 0.050f, 0.80f);
    const ImVec4 childBg    (0.055f, 0.030f, 0.075f, 0.60f);
    const ImVec4 popupBg    (0.045f, 0.025f, 0.065f, 0.96f);
    const ImVec4 border     (0.430f, 0.160f, 0.850f, 0.85f);
    const ImVec4 frame      (0.160f, 0.090f, 0.240f, 0.85f);
    const ImVec4 frameHov   (0.270f, 0.145f, 0.410f, 0.95f);
    const ImVec4 frameAct   (0.380f, 0.190f, 0.580f, 1.00f);
    const ImVec4 purple     (0.660f, 0.330f, 0.970f, 1.00f);
    const ImVec4 purpleLite (0.755f, 0.520f, 0.990f, 1.00f);
    const ImVec4 purpleDim  (0.360f, 0.220f, 0.520f, 1.00f);
    const ImVec4 text       (0.930f, 0.900f, 1.000f, 1.00f);
    const ImVec4 textDim    (0.560f, 0.510f, 0.660f, 1.00f);

    set(ImGuiCol_Text,                 text);
    set(ImGuiCol_TextDisabled,         textDim);
    set(ImGuiCol_WindowBg,             winBg);
    set(ImGuiCol_ChildBg,              childBg);
    set(ImGuiCol_PopupBg,              popupBg);
    set(ImGuiCol_Border,               border);
    set(ImGuiCol_BorderShadow,         ImVec4(0, 0, 0, 0.55f));
    set(ImGuiCol_FrameBg,              frame);
    set(ImGuiCol_FrameBgHovered,       frameHov);
    set(ImGuiCol_FrameBgActive,        frameAct);
    set(ImGuiCol_TitleBg,              winBg);
    set(ImGuiCol_TitleBgActive,        winBg);
    set(ImGuiCol_TitleBgCollapsed,     winBg);
    set(ImGuiCol_MenuBarBg,            winBg);
    set(ImGuiCol_ScrollbarBg,          ImVec4(0.02f, 0.01f, 0.03f, 0.60f));
    set(ImGuiCol_ScrollbarGrab,        purpleDim);
    set(ImGuiCol_ScrollbarGrabHovered, purple);
    set(ImGuiCol_ScrollbarGrabActive,  purpleLite);
    set(ImGuiCol_CheckMark,            purple);
    set(ImGuiCol_SliderGrab,           purple);
    set(ImGuiCol_SliderGrabActive,     purpleLite);
    set(ImGuiCol_Button,               frame);
    set(ImGuiCol_ButtonHovered,        frameHov);
    set(ImGuiCol_ButtonActive,         frameAct);
    set(ImGuiCol_Header,               ImVec4(0.260f, 0.140f, 0.400f, 0.90f));
    set(ImGuiCol_HeaderHovered,        ImVec4(0.330f, 0.180f, 0.500f, 0.95f));
    set(ImGuiCol_HeaderActive,         ImVec4(0.400f, 0.210f, 0.620f, 1.00f));
    set(ImGuiCol_Separator,            purpleDim);
    set(ImGuiCol_SeparatorHovered,     purple);
    set(ImGuiCol_SeparatorActive,      purpleLite);
    set(ImGuiCol_ResizeGrip,           purpleDim);
    set(ImGuiCol_ResizeGripHovered,    purple);
    set(ImGuiCol_ResizeGripActive,     purpleLite);
    set(ImGuiCol_Tab,                  frame);
    set(ImGuiCol_TabHovered,           frameHov);
    set(ImGuiCol_TabSelected,          purpleDim);
    set(ImGuiCol_PlotLines,            purple);
    set(ImGuiCol_PlotHistogram,        purple);
    set(ImGuiCol_TableHeaderBg,        ImVec4(0.10f, 0.05f, 0.16f, 1.f));
    set(ImGuiCol_TableBorderStrong,    border);
    set(ImGuiCol_TableBorderLight,     purpleDim);
    set(ImGuiCol_TextSelectedBg,       ImVec4(0.45f, 0.22f, 0.75f, 0.45f));
    set(ImGuiCol_DragDropTarget,       purpleLite);
    set(ImGuiCol_NavCursor,            purpleLite);
    set(ImGuiCol_ModalWindowDimBg,     ImVec4(0.f, 0.f, 0.f, 0.55f));
}

static void Section(const char* title) {
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.755f, 0.520f, 0.990f, 1.f));
    ImGui::TextUnformatted(title);
    ImGui::PopStyleColor();

    ImVec2 a = ImGui::GetCursorScreenPos();
    float w = ImGui::GetContentRegionAvail().x;
    ImGui::GetWindowDrawList()->AddLine(a, ImVec2(a.x + w, a.y),
                                        IM_COL32(110, 55, 190, 140), 1.f);
    ImGui::Dummy(ImVec2(0.f, 5.f));
}

static void Hint(const char* t) {
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.560f, 0.510f, 0.660f, 1.f));
    ImGui::TextWrapped("%s", t);
    ImGui::PopStyleColor();
}

static bool s_capture = false;
static int  s_openReq = -1;

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

    if (s_capture) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.755f, 0.520f, 0.990f, 1.f));
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
        if (ImGui::Button("set key")) s_capture = true;
        ImGui::SameLine();
        if (ImGui::Button("clear")) {
            b.vk = 0;
            if (b.holding && b.target) {
                *b.target = b.holdSaved;
                b.holding = false;
            }
        }
        ImGui::SameLine();
        if (ImGui::Button(b.hold ? "mode: hold" : "mode: toggle")) b.hold = !b.hold;
    }

    ImGui::Spacing();
    if (ImGui::Button("done")) ImGui::CloseCurrentPopup();
}

static bool Toggle(const char* label, bool* v, int bi = -1) {
    ImGui::PushID(label);

    float h = 24.f;
    float w = ImGui::GetContentRegionAvail().x;
    if (w < 80.f) w = 80.f;
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImDrawList* dl = ImGui::GetWindowDrawList();

    bool hover = ImGui::IsMouseHoveringRect(p, ImVec2(p.x + w, p.y + h));

    ImU32 track = *v ? IM_COL32(96, 48, 168, 235) : IM_COL32(26, 15, 40, 235);
    ImU32 border = hover ? IM_COL32(168, 85, 247, 255) : IM_COL32(96, 50, 158, 210);
    dl->AddRectFilled(p, ImVec2(p.x + w, p.y + h), track, h * 0.5f);
    dl->AddRect(p, ImVec2(p.x + w, p.y + h), border, h * 0.5f, 0, 1.3f);

    float kw = 40.f, pad = 3.f;
    float kx = *v ? (p.x + w - kw - pad) : (p.x + pad);
    dl->AddRectFilled(ImVec2(kx, p.y + pad), ImVec2(kx + kw, p.y + h - pad),
                      *v ? IM_COL32(208, 165, 255, 255) : IM_COL32(112, 82, 150, 255),
                      (h - pad * 2.f) * 0.5f);

    ImFont* f = theme::FontSmall();
    float fsz = f ? f->LegacySize : 14.f;
    dl->AddText(ImVec2(p.x + 11.f, p.y + (h - fsz) * 0.5f),
                IM_COL32(236, 230, 255, 255), label);
    const char* st = *v ? "ON" : "OFF";
    float stw = f ? f->CalcTextSizeA(fsz, FLT_MAX, 0.f, st).x : 20.f;
    float stx = p.x + w - kw - pad - stw - 9.f;
    dl->AddText(ImVec2(stx, p.y + (h - fsz) * 0.5f),
                *v ? IM_COL32(210, 180, 255, 255) : IM_COL32(130, 118, 155, 255), st);

    if (bi >= 0 && binds::items[bi].vk) {
        const char* kn = binds::KeyName(binds::items[bi].vk);
        float knw = f ? f->CalcTextSizeA(fsz, FLT_MAX, 0.f, kn).x : 10.f;
        dl->AddText(ImVec2(stx - knw - 9.f, p.y + (h - fsz) * 0.5f),
                    IM_COL32(168, 85, 247, 255), kn);
    }

    ImGui::Dummy(ImVec2(w, h));
    bool pressed = hover && ImGui::IsMouseClicked(ImGuiMouseButton_Left);
    if (pressed) *v = !*v;

    if (bi >= 0 && hover && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
        s_openReq = bi;

    ImGui::PopID();
    return pressed;
}

static void TabTitle(const char* t) {
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.755f, 0.520f, 0.990f, 1.f));
    ImGui::SetWindowFontScale(1.15f);
    ImGui::TextUnformatted(t);
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopStyleColor();

    ImVec2 a = ImGui::GetCursorScreenPos();
    float w = ImGui::GetContentRegionAvail().x;
    ImGui::GetWindowDrawList()->AddLine(a, ImVec2(a.x + w, a.y),
                                        IM_COL32(110, 55, 190, 140), 1.f);
    ImGui::Dummy(ImVec2(0.f, 6.f));
}

void DrawMenuWindow() {
    static bool s_styled = false;
    if (!s_styled) { s_styled = true; theme::ApplyStyle(); }

    static int s_tab = 0;
    const char* kTabs[5] = { "ESP", "VISION", "OVERLAY", "AUTO", "HELP" };

    ImGui::SetNextWindowPos(ImVec2(30, 30), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(520.f, 430.f), ImGuiCond_FirstUseEver);

    const ImGuiWindowFlags fl =
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoSavedSettings;

    ImGui::Begin("obsyde", nullptr, fl);

    ImVec2 wp = ImGui::GetWindowPos();
    ImVec2 ws = ImGui::GetWindowSize();
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImFont* big = theme::FontBig();
    float titleH = big ? big->LegacySize : 20.f;

    if (big) {
        const char* logo = "obsyde";
        ImVec2 ts = big->CalcTextSizeA(titleH, 400.f, 0.f, logo);
        dl->AddText(big, titleH, ImVec2(wp.x + 16.f, wp.y + 9.f),
                    IM_COL32(168, 85, 247, 255), logo);
        (void)ts;
    }
    ImVec2 under(wp.x + 14.f, wp.y + 9.f + titleH + 5.f);
    dl->AddLine(under, ImVec2(wp.x + ws.x - 14.f, under.y),
                IM_COL32(110, 55, 190, 170), 1.4f);

    float headerH = 9.f + titleH + 12.f;
    ImGui::SetCursorScreenPos(wp);
    ImGui::InvisibleButton("##hdr_drag", ImVec2(ws.x - 34.f, headerH));
    if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left, 1.f)) {
        ImVec2 d = ImGui::GetIO().MouseDelta;
        ImGui::SetWindowPos(ImVec2(wp.x + d.x, wp.y + d.y));
    }

    ImGui::SetCursorScreenPos(ImVec2(wp.x + ws.x - 30.f, wp.y + 7.f));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.43f, 0.16f, 0.85f, 0.9f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.66f, 0.33f, 0.97f, 1.f));
    if (ImGui::Button("x", ImVec2(22.f, 22.f)))
        cfg::menuOpen = false;
    ImGui::PopStyleColor(3);

    const float navW = 108.f, navGap = 14.f, navX = wp.x + 12.f;
    float navY = wp.y + headerH + 8.f;
    for (int i = 0; i < 5; ++i) {
        ImGui::SetCursorScreenPos(ImVec2(navX, navY + i * 34.f));
        bool sel = (s_tab == i);
        if (sel) {
            ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.38f, 0.19f, 0.58f, 1.f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.46f, 0.24f, 0.70f, 1.f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.55f, 0.30f, 0.82f, 1.f));
        }
        char id[24];
        snprintf(id, sizeof(id), "##tab%d", i);
        if (ImGui::Button(kTabs[i], ImVec2(navW, 28.f))) s_tab = i;
        if (sel) ImGui::PopStyleColor(3);
    }

    dl->AddLine(ImVec2(navX + navW + navGap * 0.5f, navY),
                ImVec2(navX + navW + navGap * 0.5f, wp.y + ws.y - 34.f),
                IM_COL32(96, 50, 158, 160), 1.f);

    float cx = navX + navW + navGap;
    float cw = wp.x + ws.x - 14.f - cx;
    float cy = navY;
    float chh = wp.y + ws.y - 30.f - cy;
    ImGui::SetCursorScreenPos(ImVec2(cx, cy));
    ImGui::BeginChild("##content", ImVec2(cw, chh), false,
                      ImGuiWindowFlags_NoBackground);

    switch (s_tab) {
    case 0:
        TabTitle("ESP");
        Toggle("heroes",            &cfg::espHeroes, 4);
        Toggle("boxes",             &cfg::espBoxes);
        Toggle("skill cooldowns",   &cfg::espCds);
        Toggle("distance",          &cfg::espDist);
        Toggle("skill preview",     &cfg::skillPreview);
        Toggle("all wards (WARD!!)", &cfg::espWards, 5);
        Toggle("roshan",            &cfg::espRoshan);
        Toggle("last hit marker",   &cfg::lastHit, 3);
        Toggle("deny marker",       &cfg::deny);
        ImGui::Spacing();
        Hint("right click a row - bind a key to it");
        break;

    case 1:
        TabTitle("VISION");
        Toggle("remove fog of war", &cfg::vbe, 6);
        Toggle("purple glow",       &cfg::glow, 7);
        ImGui::Spacing();
        Hint("fog switch is written on the game mode entity");
        Hint("glow is client side, best effort in offline");
        break;

    case 2:
        TabTitle("OVERLAY");
        Toggle("dota plus panel",   &cfg::dotaPlus, 9);
        Toggle("keybinds widget",   &cfg::showKeybinds);
        ImGui::Spacing();
        Hint("open this menu and drag the panel to move it");
        Hint("keybinds widget sits top right: name, key, T/H");
        Hint("open this menu and drag the widget to move it");
        break;

    case 3:
        TabTitle("AUTO");
        Toggle("auto accept",       &cfg::autoAccept, 2);
        Toggle("dodge alerts",      &cfg::dodger);
        Toggle("auto dodge",        &cfg::autoDodge, 8);
        Toggle("farm bot (last hit)", &cfg::farmBot, 0);
        Toggle("auto attack",       &cfg::farmAuto, 1);
        ImGui::Spacing();
        Hint("auto accept takes the match the moment it is found");
        Hint("bot clicks the mouse, runs only while the game is focused");
        break;

    case 4:
    default:
        TabTitle("HELP");
        Hint("LH  - creep you can last hit now");
        Hint("LH 2 - needs 2 hits first");
        Hint("DENY - friendly creep below 50%");
        Hint("1: 20s - skill 1 ready in 20 s");
        Hint("arrow at the screen edge - target is off screen");
        Hint("own wards are dim, enemy wards scream WARD!!");
        Hint("right click any row to bind a key (esc cancels)");
        Hint("keybinds widget shows name: key and mode T/H");
        break;
    }

    ImGui::EndChild();

    if (s_openReq >= 0) {
        cfg::rebindIdx = s_openReq;
        s_capture = true;
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
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.560f, 0.510f, 0.660f, 1.f));
    float tw = ImGui::CalcTextSize(foot).x;
    ImGui::SetCursorScreenPos(ImVec2(wp.x + (ws.x - tw) * 0.5f, wp.y + ws.y - 22.f));
    ImGui::TextUnformatted(foot);
    ImGui::PopStyleColor();

    ImGui::End();
}
