#include "glass.h"
#include <cmath>
#include <cstring>
#include <vector>

namespace glass {

bool enabled = true;

static ID3D11Device*        g_dev = nullptr;
static ID3D11DeviceContext* g_ctx = nullptr;

static const int kStages = 4;
static ID3D11Texture2D*          g_rt [kStages] = {};
static ID3D11RenderTargetView*   g_rtv[kStages] = {};
static ID3D11ShaderResourceView* g_srv[kStages] = {};
static UINT g_w = 0, g_h = 0;

typedef HRESULT(WINAPI* PFN_D3DCOMPILE)(LPCVOID, SIZE_T, LPCSTR, const void*,
                                        void*, LPCSTR, LPCSTR, UINT, UINT,
                                        ID3DBlob**, ID3DBlob**);
static PFN_D3DCOMPILE g_compile = nullptr;

static ID3D11VertexShader*    g_vsBlur = nullptr;
static ID3D11PixelShader*     g_psBlur = nullptr;
static ID3D11VertexShader*    g_vsQuad = nullptr;
static ID3D11PixelShader*     g_psQuad = nullptr;
static ID3D11InputLayout*     g_ilQuad = nullptr;
static ID3D11Buffer*          g_cb     = nullptr;
static ID3D11SamplerState*    g_smp    = nullptr;
static ID3D11BlendState*      g_bs     = nullptr;
static ID3D11RasterizerState* g_rs     = nullptr;
static ID3D11Buffer*          g_vb     = nullptr;
static UINT                   g_vbCap  = 0;
static ID3D11Buffer*          g_ib     = nullptr;
static UINT                   g_ibCap  = 0;

struct Rect { ImVec2 a, b; float r; };
static std::vector<Rect> g_rects;

#define GR(x) if (x) { (x)->Release(); (x) = nullptr; }

static const char* kHlsl =
"cbuffer CB : register(b0) { float4 cb; };\n"
"struct O { float4 p : SV_Position; float2 uv : TEXCOORD0; };\n"
"Texture2D    t : register(t0);\n"
"SamplerState s : register(s0);\n"
"O VSBlur(uint id : SV_VertexID) {\n"
"  O o; float2 uv = float2((id << 1) & 2, id & 2);\n"
"  o.uv = uv; o.p = float4(uv.x * 2 - 1, 1 - uv.y * 2, 0, 1); return o; }\n"
"float4 PSBlur(O i) : SV_Target {\n"
"  float2 o = cb.zw * cb.xy;\n"
"  float4 c  = t.Sample(s, i.uv + float2( o.x,  o.y));\n"
"  c        += t.Sample(s, i.uv + float2(-o.x,  o.y));\n"
"  c        += t.Sample(s, i.uv + float2( o.x, -o.y));\n"
"  c        += t.Sample(s, i.uv + float2(-o.x, -o.y));\n"
"  return c * 0.25; }\n"
"struct V { float2 p : POSITION; float2 uv : TEXCOORD0; };\n"
"O VSQuad(V v) {\n"
"  O o; o.p = float4(v.p.x / cb.x * 2 - 1, 1 - v.p.y / cb.y * 2, 0, 1);\n"
"  o.uv = v.uv; return o; }\n"
"float4 PSQuad(O i) : SV_Target { float4 c = t.Sample(s, i.uv); c.a = cb.z; return c; }\n";

static bool CompileShaders() {
    if (!g_compile) {
        HMODULE hc = LoadLibraryA("d3dcompiler_47.dll");
        if (!hc) return false;
        g_compile = (PFN_D3DCOMPILE)GetProcAddress(hc, "D3DCompile");
        if (!g_compile) return false;
    }

    ID3DBlob* vb1 = nullptr;
    ID3DBlob* pb1 = nullptr;
    ID3DBlob* vb2 = nullptr;
    ID3DBlob* pb2 = nullptr;
    ID3DBlob* err = nullptr;

    if (FAILED(g_compile(kHlsl, strlen(kHlsl) + 1, "glass", nullptr, nullptr,
                         "VSBlur", "vs_4_0", 0, 0, &vb1, &err)) || !vb1) { GR(err); return false; }
    if (FAILED(g_compile(kHlsl, strlen(kHlsl) + 1, "glass", nullptr, nullptr,
                         "PSBlur", "ps_4_0", 0, 0, &pb1, &err)) || !pb1) { GR(vb1); GR(err); return false; }
    if (FAILED(g_compile(kHlsl, strlen(kHlsl) + 1, "glass", nullptr, nullptr,
                         "VSQuad", "vs_4_0", 0, 0, &vb2, &err)) || !vb2) { GR(vb1); GR(pb1); GR(err); return false; }
    if (FAILED(g_compile(kHlsl, strlen(kHlsl) + 1, "glass", nullptr, nullptr,
                         "PSQuad", "ps_4_0", 0, 0, &pb2, &err)) || !pb2) { GR(vb1); GR(pb1); GR(vb2); GR(err); return false; }
    GR(err);

    g_dev->CreateVertexShader(vb1->GetBufferPointer(), vb1->GetBufferSize(), nullptr, &g_vsBlur);
    g_dev->CreatePixelShader (pb1->GetBufferPointer(), pb1->GetBufferSize(), nullptr, &g_psBlur);
    g_dev->CreateVertexShader(vb2->GetBufferPointer(), vb2->GetBufferSize(), nullptr, &g_vsQuad);
    g_dev->CreatePixelShader (pb2->GetBufferPointer(), pb2->GetBufferSize(), nullptr, &g_psQuad);

    D3D11_INPUT_ELEMENT_DESC il[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    g_dev->CreateInputLayout(il, 2, vb2->GetBufferPointer(), vb2->GetBufferSize(), &g_ilQuad);

    GR(vb1); GR(pb1); GR(vb2); GR(pb2);

    D3D11_BUFFER_DESC cbd{};
    cbd.ByteWidth = 16; cbd.Usage = D3D11_USAGE_DEFAULT; cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    g_dev->CreateBuffer(&cbd, nullptr, &g_cb);

    D3D11_SAMPLER_DESC sd{};
    sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sd.AddressU = sd.AddressV = sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sd.MaxAnisotropy = 1; sd.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    sd.MaxLOD = D3D11_FLOAT32_MAX;
    g_dev->CreateSamplerState(&sd, &g_smp);

    D3D11_BLEND_DESC bd{};
    bd.RenderTarget[0].BlendEnable = TRUE;
    bd.RenderTarget[0].SrcBlend  = D3D11_BLEND_SRC_ALPHA;
    bd.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    bd.RenderTarget[0].BlendOp   = D3D11_BLEND_OP_ADD;
    bd.RenderTarget[0].SrcBlendAlpha  = D3D11_BLEND_ONE;
    bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
    bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    g_dev->CreateBlendState(&bd, &g_bs);

    D3D11_RASTERIZER_DESC rd{};
    rd.FillMode = D3D11_FILL_SOLID; rd.CullMode = D3D11_CULL_NONE;
    rd.DepthClipEnable = TRUE;
    g_dev->CreateRasterizerState(&rd, &g_rs);

    return g_vsBlur && g_psBlur && g_vsQuad && g_psQuad && g_ilQuad && g_cb && g_smp;
}

static void RebuildChain(UINT w, UINT h) {
    for (int i = 0; i < kStages; ++i) { GR(g_srv[i]); GR(g_rtv[i]); GR(g_rt[i]); }
    g_w = w; g_h = h;
    for (int i = 0; i < kStages; ++i) {
        UINT cw = w >> (i + 1), ch = h >> (i + 1);
        if (cw < 4 || ch < 4) break;
        D3D11_TEXTURE2D_DESC td{};
        td.Width = cw; td.Height = ch;
        td.MipLevels = 1; td.ArraySize = 1;
        td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        td.SampleDesc.Count = 1;
        td.Usage = D3D11_USAGE_DEFAULT;
        td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        if (FAILED(g_dev->CreateTexture2D(&td, nullptr, &g_rt[i]))) break;
        g_dev->CreateRenderTargetView(g_rt[i], nullptr, &g_rtv[i]);
        g_dev->CreateShaderResourceView(g_rt[i], nullptr, &g_srv[i]);
    }
}

void Init(ID3D11Device* dev, ID3D11DeviceContext* ctx) {
    g_dev = dev; g_ctx = ctx;
}

void Shutdown() {
    for (int i = 0; i < kStages; ++i) { GR(g_srv[i]); GR(g_rtv[i]); GR(g_rt[i]); }
    GR(g_vsBlur); GR(g_psBlur); GR(g_vsQuad); GR(g_psQuad); GR(g_ilQuad);
    GR(g_cb); GR(g_smp); GR(g_bs); GR(g_rs); GR(g_vb); GR(g_ib);
    g_vbCap = g_ibCap = 0;
    g_w = g_h = 0;
}

void AddRect(const ImVec2& a, const ImVec2& b, float rounding) {
    if (b.x - a.x < 4.f || b.y - a.y < 4.f) return;
    g_rects.push_back(Rect{ a, b, rounding });
}

static float g_alpha = 1.f;
void SetAlpha(float a) { g_alpha = a < 0.f ? 0.f : (a > 1.f ? 1.f : a); }

static void BuildRounded(std::vector<float>& vb, std::vector<unsigned short>& ib,
                         const Rect& rc, float sw, float sh) {
    const int SEG = 6;
    float x0 = rc.a.x, y0 = rc.a.y, x1 = rc.b.x, y1 = rc.b.y;
    float r = rc.r;
    if (r < 1.f) r = 1.f;
    if (r > (x1 - x0) * 0.5f) r = (x1 - x0) * 0.5f;
    if (r > (y1 - y0) * 0.5f) r = (y1 - y0) * 0.5f;

    unsigned short base = (unsigned short)(vb.size() / 4);
    float cx = (x0 + x1) * 0.5f, cy = (y0 + y1) * 0.5f;
    vb.push_back(cx); vb.push_back(cy); vb.push_back(cx / sw); vb.push_back(cy / sh);

    const float HP = 1.5707963f;
    float corners[4][3] = {
        { x1 - r, y0 + r, -HP },
        { x1 - r, y1 - r,  0.f },
        { x0 + r, y1 - r,  HP },
        { x0 + r, y0 + r,  2.f * HP },
    };
    for (int c = 0; c < 4; ++c)
        for (int s = 0; s <= SEG; ++s) {
            float a = corners[c][2] + HP * (float)s / (float)SEG;
            float px = corners[c][0] + cosf(a) * r;
            float py = corners[c][1] + sinf(a) * r;
            vb.push_back(px); vb.push_back(py); vb.push_back(px / sw); vb.push_back(py / sh);
        }

    unsigned short per = (unsigned short)(4 * (SEG + 1));
    for (unsigned short k = 1; k < per; ++k) {
        ib.push_back(base); ib.push_back((unsigned short)(base + k)); ib.push_back((unsigned short)(base + k + 1));
    }
    ib.push_back(base); ib.push_back((unsigned short)(base + per)); ib.push_back((unsigned short)(base + 1));
}

static bool EnsureBuffers(UINT nv, UINT ni) {
    if (nv > g_vbCap) {
        GR(g_vb);
        D3D11_BUFFER_DESC d{};
        d.ByteWidth = nv * 16; d.Usage = D3D11_USAGE_DYNAMIC;
        d.BindFlags = D3D11_BIND_VERTEX_BUFFER; d.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        if (FAILED(g_dev->CreateBuffer(&d, nullptr, &g_vb))) return false;
        g_vbCap = nv;
    }
    if (ni > g_ibCap) {
        GR(g_ib);
        D3D11_BUFFER_DESC d{};
        d.ByteWidth = ni * 2; d.Usage = D3D11_USAGE_DYNAMIC;
        d.BindFlags = D3D11_BIND_INDEX_BUFFER; d.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        if (FAILED(g_dev->CreateBuffer(&d, nullptr, &g_ib))) return false;
        g_ibCap = ni;
    }
    return true;
}

void OnPresent(IDXGISwapChain* sc) {
    std::vector<Rect> rects;
    rects.swap(g_rects);

    if (!g_dev || !g_ctx || !enabled || rects.empty()) return;

    ID3D11Texture2D* bb = nullptr;
    if (FAILED(sc->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&bb)) || !bb) return;

    D3D11_TEXTURE2D_DESC bd{};
    bb->GetDesc(&bd);
    if (bd.SampleDesc.Count > 1) { bb->Release(); return; }
    if (!g_vsBlur && !CompileShaders())        { bb->Release(); return; }
    if (bd.Width != g_w || bd.Height != g_h) RebuildChain(bd.Width, bd.Height);
    if (!g_rt[0] || !g_srv[0])                 { bb->Release(); return; }

    ID3D11ShaderResourceView* bbSrv = nullptr;
    if (FAILED(g_dev->CreateShaderResourceView(bb, nullptr, &bbSrv))) { bb->Release(); return; }
    ID3D11RenderTargetView* bbRtv = nullptr;
    if (FAILED(g_dev->CreateRenderTargetView(bb, nullptr, &bbRtv)))  { GR(bbSrv); bb->Release(); return; }

    ID3D11RenderTargetView* nullRtv[1] = { nullptr };
    g_ctx->OMSetRenderTargets(1, nullRtv, nullptr);

    static const float kOff[kStages] = { 0.5f, 1.0f, 1.5f, 2.0f };
    ID3D11ShaderResourceView* src = bbSrv;
    int last = -1;

    g_ctx->IASetInputLayout(nullptr);
    g_ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    g_ctx->VSSetShader(g_vsBlur, nullptr, 0);
    g_ctx->PSSetShader(g_psBlur, nullptr, 0);
    g_ctx->PSSetSamplers(0, 1, &g_smp);
    g_ctx->RSSetState(g_rs);
    g_ctx->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);

    for (int i = 0; i < kStages && g_rtv[i]; ++i) {
        float srcW = (float)(g_w >> i), srcH = (float)(g_h >> i);
        D3D11_VIEWPORT vp{};
        vp.Width = (float)(g_w >> (i + 1));
        vp.Height = (float)(g_h >> (i + 1));
        vp.MaxDepth = 1.f;
        g_ctx->RSSetViewports(1, &vp);
        g_ctx->OMSetRenderTargets(1, &g_rtv[i], nullptr);

        float cb[4] = { 1.f / srcW, 1.f / srcH, kOff[i], kOff[i] };
        g_ctx->UpdateSubresource(g_cb, 0, nullptr, cb, 0, 0);
        g_ctx->VSSetConstantBuffers(0, 1, &g_cb);
        g_ctx->PSSetConstantBuffers(0, 1, &g_cb);
        g_ctx->PSSetShaderResources(0, 1, &src);
        g_ctx->Draw(3, 0);

        ID3D11ShaderResourceView* nullSrv = nullptr;
        g_ctx->PSSetShaderResources(0, 1, &nullSrv);

        src = g_srv[i];
        last = i;
    }

    if (last >= 0) {
        std::vector<float> vb;
        std::vector<unsigned short> ib;
        for (auto& r : rects) BuildRounded(vb, ib, r, (float)g_w, (float)g_h);

        if (EnsureBuffers((UINT)vb.size() / 4, (UINT)ib.size())) {
            D3D11_MAPPED_SUBRESOURCE mv, mi;
            if (SUCCEEDED(g_ctx->Map(g_vb, 0, D3D11_MAP_WRITE_DISCARD, 0, &mv))) {
                memcpy(mv.pData, vb.data(), vb.size() * sizeof(float));
                g_ctx->Unmap(g_vb, 0);
            }
            if (SUCCEEDED(g_ctx->Map(g_ib, 0, D3D11_MAP_WRITE_DISCARD, 0, &mi))) {
                memcpy(mi.pData, ib.data(), ib.size() * sizeof(unsigned short));
                g_ctx->Unmap(g_ib, 0);
            }

            D3D11_VIEWPORT vp{};
            vp.Width = (float)g_w; vp.Height = (float)g_h; vp.MaxDepth = 1.f;
            g_ctx->RSSetViewports(1, &vp);
            g_ctx->OMSetRenderTargets(1, &bbRtv, nullptr);

            float cb[4] = { (float)g_w, (float)g_h, g_alpha, 0.f };
            g_ctx->UpdateSubresource(g_cb, 0, nullptr, cb, 0, 0);
            g_ctx->VSSetConstantBuffers(0, 1, &g_cb);
            g_ctx->PSSetConstantBuffers(0, 1, &g_cb);

            g_ctx->IASetInputLayout(g_ilQuad);
            g_ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            UINT stride = 16, off0 = 0;
            g_ctx->IASetVertexBuffers(0, 1, &g_vb, &stride, &off0);
            g_ctx->IASetIndexBuffer(g_ib, DXGI_FORMAT_R16_UINT, 0);
            g_ctx->VSSetShader(g_vsQuad, nullptr, 0);
            g_ctx->PSSetShader(g_psQuad, nullptr, 0);
            g_ctx->PSSetShaderResources(0, 1, &g_srv[last]);
            g_ctx->OMSetBlendState(g_bs, nullptr, 0xFFFFFFFF);
            g_ctx->DrawIndexed((UINT)ib.size(), 0, 0);
        }
    }

    ID3D11ShaderResourceView* nullSrv = nullptr;
    g_ctx->PSSetShaderResources(0, 1, &nullSrv);
    g_ctx->OMSetRenderTargets(1, nullRtv, nullptr);

    GR(bbSrv); GR(bbRtv);
    bb->Release();
}

}

