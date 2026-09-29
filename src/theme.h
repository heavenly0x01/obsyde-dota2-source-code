#pragma once
#include "imgui.h"

namespace theme {

inline const ImU32 Purple      = IM_COL32(168,  85, 247, 255);
inline const ImU32 PurpleLight = IM_COL32(192, 132, 252, 255);
inline const ImU32 PurpleDeep  = IM_COL32(109,  40, 217, 255);
inline const ImU32 PurpleDim   = IM_COL32( 91,  55, 130, 255);
inline const ImU32 Black       = IM_COL32(  0,   0,   0, 255);
inline const ImU32 Ink         = IM_COL32( 12,   6,  18, 255);
inline const ImU32 White       = IM_COL32(245, 240, 255, 255);
inline const ImU32 Dim         = IM_COL32(150, 130, 175, 255);

inline ImU32 WithAlpha(ImU32 c, int a) {
    return (c & 0x00FFFFFFu) | ((ImU32)a << 24);
}

void ApplyStyle();
void LoadFonts();
ImFont* FontSmall();
ImFont* FontBig();

}
