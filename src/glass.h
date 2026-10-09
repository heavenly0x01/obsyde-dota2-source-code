#pragma once

#include <Windows.h>
#include <d3d11.h>
#include "imgui.h"

namespace glass {

extern bool enabled;

void Init(ID3D11Device* dev, ID3D11DeviceContext* ctx);
void Shutdown();

void OnPresent(IDXGISwapChain* sc);
void AddRect(const ImVec2& a, const ImVec2& b, float rounding);
void SetAlpha(float a);

}

