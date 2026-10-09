#pragma once

#include "imgui.h"

namespace theme {

inline const ImU32 Purple      = IM_COL32(245, 245, 247, 255);
inline const ImU32 PurpleLight = IM_COL32(255, 255, 255, 255);
inline const ImU32 PurpleDeep  = IM_COL32(205, 205, 212, 255);
inline const ImU32 PurpleDim   = IM_COL32(125, 125, 133, 255);

inline const ImU32 Black       = IM_COL32(  0,   0,   0, 255);
inline const ImU32 Ink         = IM_COL32( 10,  10,  12, 255);
inline const ImU32 Panel       = IM_COL32(  8,   8,  10, 150);
inline const ImU32 PanelSolid  = IM_COL32( 12,  12,  15, 235);
inline const ImU32 Border      = IM_COL32(255, 255, 255,  22);
inline const ImU32 BorderHi    = IM_COL32(255, 255, 255,  42);
inline const ImU32 Accent      = IM_COL32(124, 108, 255, 255);
inline const ImU32 AccentLight = IM_COL32(163, 150, 255, 255);
inline const ImU32 White       = IM_COL32(248, 248, 250, 255);
inline const ImU32 Dim         = IM_COL32(140, 140, 148, 255);

inline ImU32 WithAlpha(ImU32 c, int a) {
    return (c & 0x00FFFFFFu) | ((ImU32)a << 24);
}

void ApplyStyle();
void LoadFonts();
ImFont* FontSmall();
ImFont* FontBig();

}

