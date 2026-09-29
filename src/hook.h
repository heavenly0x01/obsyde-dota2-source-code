#pragma once
#include "game.h"
#include "imgui.h"

void InstallHooks();
void RequestUnload();

void DrawMenuWindow();

namespace view {
void Update();
bool W2S(const Vec3& world, ImVec2& out);
bool W2SRaw(const Vec3& world, ImVec2& out);
extern int W, H;
}
void DrawOverlay(const Frame& f);
void RunAutomation(const Frame& f);
void DrawKeybinds();
void RunAutoAccept();
