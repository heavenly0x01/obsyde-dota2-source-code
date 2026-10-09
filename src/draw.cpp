#include "hook.h"
#include "theme.h"
#include "offsets.h"
#include "glass.h"
#include "imgui.h"

#include <cfloat>

namespace input {
void ClickAt(HWND hwnd, int cx, int cy, bool right);
void Tick();
}

static float g_mat[16] = {};
static int   g_mode = 0;
static bool  g_matOk = false;

static bool Project(int mode, const Vec3& w, float& sx, float& sy, float& pw);

namespace view {

int W = 0;
int H = 0;

bool W2S(const Vec3& world, ImVec2& out) {
    if (!g_matOk || W <= 0 || H <= 0) return false;
    float sx = 0, sy = 0, pw = 0;
    if (!Project(g_mode, world, sx, sy, pw)) return false;
    if (pw <= 0.f) return false;
    if (sx < -0.20f * W || sx > 1.20f * W) return false;
    if (sy < -0.20f * H || sy > 1.20f * H) return false;
    out.x = sx;
    out.y = sy;
    return true;
}

bool W2SRaw(const Vec3& world, ImVec2& out) {
    if (!g_matOk || W <= 0 || H <= 0) return false;
    float sx = 0, sy = 0, pw = 0;
    if (!Project(g_mode, world, sx, sy, pw)) return false;
    if (pw <= 0.f) return false;
    out.x = sx;
    out.y = sy;
    return true;
}

void Update() {
    ImGuiIO& io = ImGui::GetIO();
    W = (int)io.DisplaySize.x;
    H = (int)io.DisplaySize.y;
    if (W <= 0 || H <= 0) return;
    if (!game::g_sys.ready) return;

    struct Mat4 { float m[16]; };
    Mat4 tmp;
    if (!mem::Read(game::g_sys.clientBase + off::dwViewMatrix, tmp)) return;

    double sum = 0.0;
    bool finite = true;
    for (int i = 0; i < 16; ++i) {
        float v = tmp.m[i];
        if (!(v == v) || fabsf(v) > 1e6f) { finite = false; break; }
        sum += fabsf(v);
    }
    if (!finite || sum < 0.5f) return;

    memcpy(g_mat, tmp.m, sizeof(g_mat));
    g_matOk = true;
}

}

static bool Project(int mode, const Vec3& w, float& sx, float& sy, float& pw) {
    const float* m = g_mat;
    float px, py;

    if (mode & 2) {
        px = m[0] * w.x + m[4] * w.y + m[8]  * w.z + m[12];
        py = m[1] * w.x + m[5] * w.y + m[9]  * w.z + m[13];
        pw = m[3] * w.x + m[7] * w.y + m[11] * w.z + m[15];
    } else {
        px = m[0] * w.x + m[1] * w.y + m[2]  * w.z + m[3];
        py = m[4] * w.x + m[5] * w.y + m[6]  * w.z + m[7];
        pw = m[12] * w.x + m[13] * w.y + m[14] * w.z + m[15];
    }

    if (fabsf(pw) < 1e-6f) return false;

    float nx = px / pw;
    float ny = py / pw;
    if (mode & 1) ny = -ny;

    sx = (nx * 0.5f + 0.5f) * (float)view::W;
    sy = (0.5f - ny * 0.5f) * (float)view::H;
    return true;
}

static void ResolveMode(const Frame& f) {
    if (!g_matOk) return;

    float nA = fabsf(g_mat[12]) + fabsf(g_mat[13]) + fabsf(g_mat[14]);
    float nB = fabsf(g_mat[3])  + fabsf(g_mat[7])  + fabsf(g_mat[11]);

    int mode;
    if (nA < nB * 0.5f)      mode = 0;
    else if (nB < nA * 0.5f) mode = 2;
    else                     mode = g_mode & 2;

    float ax, ay, aw, bx, by, bw;
    Vec3 up{ f.localPos.x, f.localPos.y, f.localPos.z + 150.f };
    if (Project(mode, f.localPos, ax, ay, aw) && Project(mode, up, bx, by, bw) &&
        aw > 0.f && bw > 0.f && (by - ay) > 1.f)
        mode |= 1;

    if (mode != g_mode)
        g_mode = mode;
}

static void DText(ImDrawList* dl, const char* s, ImVec2 p, ImU32 col, float size = 0.f) {
    ImFont* f = theme::FontSmall();
    if (!f || !s || !*s) return;
    float sz = size > 0.f ? size : f->LegacySize;
    dl->AddText(f, sz, ImVec2(p.x + 1.f, p.y + 1.f), IM_COL32(0, 0, 0, 215), s);
    dl->AddText(f, sz, p, col, s);
}

static float DTextW(const char* s, float size = 0.f) {
    ImFont* f = theme::FontSmall();
    if (!f) return 0.f;
    float sz = size > 0.f ? size : f->LegacySize;
    return f->CalcTextSizeA(sz, FLT_MAX, 0.f, s).x;
}

static void DTextC(ImDrawList* dl, const char* s, float cx, float y, ImU32 col, float size = 0.f) {
    DText(dl, s, ImVec2(cx - DTextW(s, size) * 0.5f, y), col, size);
}

static void DrawBox(ImDrawList* dl, ImVec2 a, ImVec2 b, ImU32 col, float thick = 1.6f) {
    float w = b.x - a.x, h = b.y - a.y;
    if (w < 4.f || h < 4.f) return;
    float len = (w < h ? w : h) * 0.28f;
    if (len < 5.f) len = 5.f;

    dl->AddLine(ImVec2(a.x, a.y),         ImVec2(a.x + len, a.y),         col, thick);
    dl->AddLine(ImVec2(a.x, a.y),         ImVec2(a.x, a.y + len),         col, thick);
    dl->AddLine(ImVec2(b.x, a.y),         ImVec2(b.x - len, a.y),         col, thick);
    dl->AddLine(ImVec2(b.x, a.y),         ImVec2(b.x, a.y + len),         col, thick);
    dl->AddLine(ImVec2(a.x, b.y),         ImVec2(a.x + len, b.y),         col, thick);
    dl->AddLine(ImVec2(a.x, b.y),         ImVec2(a.x, b.y - len),         col, thick);
    dl->AddLine(ImVec2(b.x, b.y),         ImVec2(b.x - len, b.y),         col, thick);
    dl->AddLine(ImVec2(b.x, b.y),         ImVec2(b.x, b.y - len),         col, thick);
}

static void DrawBar(ImDrawList* dl, ImVec2 a, float w, float h, float frac,
                    ImU32 fill, bool withText = false, const char* text = nullptr) {
    if (frac < 0.f) frac = 0.f;
    if (frac > 1.f) frac = 1.f;
    dl->AddRectFilled(a, ImVec2(a.x + w, a.y + h), IM_COL32(0, 0, 0, 190), 1.f);
    if (frac > 0.f)
        dl->AddRectFilled(a, ImVec2(a.x + w * frac, a.y + h), fill, 1.f);
    dl->AddRect(a, ImVec2(a.x + w, a.y + h), IM_COL32(0, 0, 0, 220), 1.f, 0, 1.f);
    if (withText && text && *text)
        DTextC(dl, text, a.x + w * 0.5f, a.y - 14.f, theme::White);
}

static void DrawSegTicks(ImDrawList* dl, ImVec2 a, float w, float h,
                         float maxV, float segV) {
    if (!(segV > 1.f) || !(maxV > 0.f)) return;
    int n = (int)(maxV / segV);
    if (n < 2 || n > 80) return;
    for (int k = 1; k < n; ++k) {
        float x = a.x + w * ((float)k * segV / maxV);
        dl->AddLine(ImVec2(x, a.y + 1.f), ImVec2(x, a.y + h - 1.f),
                    IM_COL32(0, 0, 0, 185), 1.f);
    }
}

static void DrawStar(ImDrawList* dl, ImVec2 c, float r, ImU32 col) {
    ImVec2 pts[10];
    for (int i = 0; i < 10; ++i) {
        float ang = -1.5707963f + (float)i * 0.62831853f;
        float rr  = (i & 1) ? r * 0.44f : r;
        pts[i] = ImVec2(c.x + cosf(ang) * rr, c.y + sinf(ang) * rr);
    }
    dl->AddConvexPolyFilled(pts, 10, col);
    dl->AddPolyline(pts, 10, IM_COL32(0, 0, 0, 220), 0, 1.6f);
}

static void FmtTime(float sec, char* out, int cap) {
    if (!(sec >= 0.f)) sec = 0.f;
    if (sec > 35999.f) sec = 35999.f;
    int m = (int)(sec / 60.f);
    int s = (int)(sec - (float)m * 60.f);
    snprintf(out, cap, "%d:%02d", m, s);
}

static void EdgeArrow(ImDrawList* dl, const ImVec2& sp, ImU32 col, float size = 9.f) {
    float cx = view::W * 0.5f, cy = view::H * 0.5f;
    float m = 46.f;
    float minX = m, maxX = (float)view::W - m;
    float minY = m, maxY = (float)view::H - m;

    float dx = sp.x - cx, dy = sp.y - cy;
    if (dx == 0.f && dy == 0.f) return;

    float t = 1.f;
    if (dx > 0.f)      { float tt = (maxX - cx) / dx; if (tt < t) t = tt; }
    else if (dx < 0.f) { float tt = (minX - cx) / dx; if (tt < t) t = tt; }
    if (dy > 0.f)      { float tt = (maxY - cy) / dy; if (tt < t) t = tt; }
    else if (dy < 0.f) { float tt = (minY - cy) / dy; if (tt < t) t = tt; }
    if (t < 0.f) t = 0.f;

    float px = cx + dx * t, py = cy + dy * t;
    float ang = atan2f(dy, dx);
    ImVec2 tip(px + cosf(ang) * size * 0.6f, py + sinf(ang) * size * 0.6f);
    ImVec2 c1 (px + cosf(ang + 2.5f) * size, py + sinf(ang + 2.5f) * size);
    ImVec2 c2 (px + cosf(ang - 2.5f) * size, py + sinf(ang - 2.5f) * size);
    dl->AddTriangleFilled(tip, c1, c2, col);
    dl->AddCircle(ImVec2(px, py), size * 1.4f,
                  IM_COL32(col & 0xFF, (col >> 8) & 0xFF, (col >> 16) & 0xFF, 90), 12, 1.2f);
}

static void DrawHero(const Frame& f, const FrameUnit& u, bool enemy, ImDrawList* dl) {
    ImVec2 base, top;
    if (!view::W2S(u.pos, base)) {
        ImVec2 rp;
        if (view::W2SRaw(u.pos, rp))
            EdgeArrow(dl, rp, enemy ? IM_COL32(255, 90, 230, 245)
                                    : IM_COL32(175, 125, 255, 175));
        return;
    }
    Vec3 tp{ u.pos.x, u.pos.y, u.pos.z + u.hbOffset };
    if (!view::W2S(tp, top))
        top = ImVec2(base.x, base.y - view::H * 0.12f);

    float h = base.y - top.y;
    if (h < 8.f || h > view::H * 1.5f) return;
    float w = h * 0.42f;
    if (w < 18.f) w = 18.f;

    ImVec2 a(top.x - w * 0.5f, top.y);
    ImVec2 b(top.x + w * 0.5f, base.y);

    ImU32 main  = enemy ? theme::Purple : theme::PurpleDim;
    ImU32 soft  = enemy ? theme::PurpleLight : theme::WithAlpha(theme::PurpleLight, 130);

    if (cfg::espBoxes) DrawBox(dl, a, b, main);

    char buf[96];

    float barW = w < 34.f ? 34.f : w;
    float bx = top.x - barW * 0.5f;
    float barY = a.y - 15.f;

    float hpFrac = (u.maxHp > 0) ? (float)u.hp / (float)u.maxHp : 0.f;
    DrawBar(dl, ImVec2(bx, barY), barW, 8.f, hpFrac,
            enemy ? theme::Purple : theme::PurpleDim, false, nullptr);
    if (cfg::lastHit && u.dmgOut > 1.f && u.alive)
        DrawSegTicks(dl, ImVec2(bx, barY), barW, 8.f, (float)u.maxHp, u.dmgOut);

    if (u.maxMana > 0.f) {
        float mf = u.mana / u.maxMana;
        if (mf < 0.f) mf = 0.f; if (mf > 1.f) mf = 1.f;
        dl->AddRectFilled(ImVec2(bx, barY - 4.f), ImVec2(bx + barW * mf, barY - 1.f),
                          IM_COL32(120, 90, 240, 230));
    }

    snprintf(buf, sizeof(buf), "%d", u.hp);
    DText(dl, buf, ImVec2(bx + barW + 3.f, barY - 1.f), soft, 12.f);

    if (cfg::espDist) {
        snprintf(buf, sizeof(buf), "%d", (int)u.dist);
        DText(dl, buf, ImVec2(b.x + 4.f, b.y - 14.f), soft);
    }

    if (!u.alive) {
        if (u.respawn >= 0) snprintf(buf, sizeof(buf), "DEAD %ds", u.respawn);
        else                snprintf(buf, sizeof(buf), "DEAD");
        DTextC(dl, buf, top.x, barY - 26.f, theme::White, 17.f);
        return;
    }

    if (u.nick[0]) snprintf(buf, sizeof(buf), "%s L%d", u.nick, u.level);
    else           snprintf(buf, sizeof(buf), "L%d", u.level);
    if (u.illusion) strncat(buf, " ILL", sizeof(buf) - strlen(buf) - 1);
    if (u.invis > 0.4f) strncat(buf, " INVIS", sizeof(buf) - strlen(buf) - 1);
    DTextC(dl, buf, top.x, barY - 44.f, enemy ? theme::White : theme::Dim, 17.f);

    if (cfg::espCds && u.abilN > 0) {
        char lines[6][40];
        ImU32 cols[6];
        int shown = 0;
        for (int i = 0; i < u.abilN && shown < 6; ++i) {
            const AbilityInfo& ab = u.abil[i];
            if (ab.phase) {
                snprintf(lines[shown], sizeof(lines[shown]), "%d: CAST", i + 1);
                cols[shown] = theme::PurpleLight;
            } else if (ab.cd > 0.05f) {
                int secs = (int)(ab.cd + 0.5f);
                snprintf(lines[shown], sizeof(lines[shown]), "%d: %ds", i + 1, secs);
                cols[shown] = theme::WithAlpha(theme::White, 225);
            } else continue;
            ++shown;
        }
        if (shown > 0) {
            float ly = (barY - 50.f) - shown * 17.f;
            for (int i = 0; i < shown; ++i) {
                DTextC(dl, lines[i], top.x, ly, cols[i], 16.f);
                ly += 17.f;
            }
        }
    }
}

static void DrawCreepMarkers(const Frame& f, const FrameUnit& u, ImDrawList* dl) {

    bool lh = cfg::lastHit && u.alive && u.hits > 0;
    bool dn = cfg::deny    && u.alive && u.canDeny;
    if (!lh && !dn) return;

    ImVec2 p;
    Vec3 tp{ u.pos.x, u.pos.y, u.pos.z + u.hbOffset };
    if (!view::W2S(tp, p)) return;

    const float kStarR = 9.f;
    const float kBarW  = 76.f, kBarH = 9.f;
    const float kGap   = 3.f;

    ImU32 col = theme::White;
    float bx = floorf(p.x - kBarW * 0.5f);
    float by = floorf(p.y - kBarH) - 2.f;

    if (lh) {
        if      (u.canLastHit) col = IM_COL32( 60, 255, 110, 255);
        else if (u.hits <= 3)  col = IM_COL32(255, 220,  40, 255);
        else                   col = IM_COL32(255, 220,  40, 220);

        dl->AddRectFilled(ImVec2(bx - 1.f, by - 1.f), ImVec2(bx + kBarW + 1.f, by + kBarH + 1.f),
                          IM_COL32(0, 0, 0, 220));
        float frac = (u.maxHp > 0) ? (float)u.hp / (float)u.maxHp : 0.f;
        if (frac < 0.f) frac = 0.f;
        if (frac > 1.f) frac = 1.f;
        if (frac > 0.f)
            dl->AddRectFilled(ImVec2(bx, by),
                              ImVec2(bx + floorf(kBarW * frac + 0.5f), by + kBarH), col);
        dl->AddRect(ImVec2(bx - 0.5f, by - 0.5f), ImVec2(bx + kBarW + 0.5f, by + kBarH + 0.5f),
                    IM_COL32(0, 0, 0, 255), 0.f, 0, 1.f);
        if (u.dmgOut > 1.f) {
            int n = (int)((float)u.maxHp / u.dmgOut);
            if (n >= 2 && n <= 80)
                for (int k = 1; k < n; ++k) {
                    float x = floorf(bx + kBarW * ((float)k * u.dmgOut / (float)u.maxHp)) + 0.5f;
                    dl->AddLine(ImVec2(x, by + 1.f), ImVec2(x, by + kBarH - 1.f),
                                IM_COL32(0, 0, 0, 200), 1.f);
                }
        }

        ImVec2 c{ p.x, by - kGap - kStarR };
        DrawStar(dl, c, kStarR, col);

        if (!u.canLastHit && u.hits > 1) {
            char n[8];
            snprintf(n, sizeof(n), "%d", u.hits);
            DTextC(dl, n, p.x + 18.f, c.y - 7.f, theme::White, 14.f);
        }
    }

    if (dn) {
        char txt[] = "DENY";
        float tw = DTextW(txt, 13.f);
        float dbx = p.x - (tw + 10.f) * 0.5f;
        float dby = lh ? by + kBarH + 3.f : p.y;
        dl->AddRectFilled(ImVec2(dbx, dby), ImVec2(dbx + tw + 10.f, dby + 16.f),
                          theme::WithAlpha(theme::Black, 160), 4.f);
        dl->AddRect(ImVec2(dbx, dby), ImVec2(dbx + tw + 10.f, dby + 16.f),
                    theme::WithAlpha(theme::White, 70), 4.f, 0, 1.f);
        DText(dl, txt, ImVec2(dbx + 5.f, dby + 1.f),
              theme::WithAlpha(theme::White, 235), 13.f);
    }
}

static void DrawWard(const Frame& f, const FrameUnit& u, ImDrawList* dl) {
    bool enemy = (u.team != f.localTeam);
    ImVec2 p;
    if (!view::W2S(u.pos, p)) {
        ImVec2 rp;
        if (view::W2SRaw(u.pos, rp))
            EdgeArrow(dl, rp, enemy ? IM_COL32(255, 90, 230, 250)
                                    : IM_COL32(175, 125, 255, 185));
        return;
    }

    ImU32 col = enemy ? IM_COL32(255, 90, 230, 255) : IM_COL32(180, 130, 255, 200);
    const char* label = enemy ? "WARD!!" : "ward";

    DTextC(dl, label, p.x, p.y - 48.f, col, enemy ? 19.f : 15.f);
    if (cfg::espDist) {
        char buf[32];
        snprintf(buf, sizeof(buf), "%d", (int)u.dist);
        DTextC(dl, buf, p.x, p.y - 30.f, theme::PurpleLight);
    }
    dl->AddCircleFilled(p, 5.f, col, 12);
    dl->AddCircle(p, 7.5f, IM_COL32(col & 0xFF, (col >> 8) & 0xFF, (col >> 16) & 0xFF, 150),
                  12, 1.8f);

    dl->AddCircle(p, 11.f, IM_COL32(col & 0xFF, (col >> 8) & 0xFF, (col >> 16) & 0xFF, 90),
                  12, 1.2f);
}

static void DrawRoshanMarker(const Frame& f, ImDrawList* dl) {
    if (!cfg::espRoshan || !f.roshanPosValid) return;
    if (f.roshanAlive) return;

    ImVec2 p;
    if (!view::W2S(f.roshanPos, p)) return;

    char buf[64];
    if (f.roshanEtaLo >= 0.f) {
        if ((int)f.roshanEtaLo == (int)f.roshanEtaHi) {
            FmtTime(f.roshanEtaLo, buf, sizeof(buf));
        } else {
            char lo[16], hi[16];
            FmtTime(f.roshanEtaLo, lo, sizeof(lo));
            FmtTime(f.roshanEtaHi, hi, sizeof(hi));
            snprintf(buf, sizeof(buf), "%s..%s", lo, hi);
        }
        char out[80];
        snprintf(out, sizeof(out), "ROSHAN %s", buf);
        DTextC(dl, out, p.x, p.y - 44.f, theme::PurpleLight, 17.f);
    } else {
        DTextC(dl, "ROSHAN", p.x, p.y - 44.f, theme::WithAlpha(theme::PurpleLight, 150), 16.f);
    }
    dl->AddCircle(p, 7.f, theme::WithAlpha(theme::Purple, 160), 14, 1.6f);
    dl->AddLine(ImVec2(p.x - 5.f, p.y), ImVec2(p.x + 5.f, p.y), theme::PurpleLight, 1.6f);
}

struct SpellShape {
    bool  draw   = false;
    bool  line   = false;
    bool  meteor = false;
    bool  atSelf = false;
    float radius = 320.f;
    float len    = 0.f;
    float width  = 200.f;
    char  label[32] = {};
};

static bool EndsWith(const char* s, const char* suf) {
    if (!s || !suf) return false;
    size_t a = strlen(s), b = strlen(suf);
    return a >= b && strcmp(s + a - b, suf) == 0;
}

static SpellShape ClassifySpell(const char* cls) {
    SpellShape sp;
    if (!cls || !*cls) { sp.draw = true; return sp; }

    const char* p = strstr(cls, "Ability_");
    const char* n = p ? p + 8 : cls;

    auto has = [n](const char* s) { return strstr(n, s) != nullptr; };

    if (has("Special") || has("Tidebringer") || has("GhostWalk") ||
        has("AttributeBonus") || has("Warcry") || has("GodsStrength") ||
        EndsWith(n, "Invoke"))
        return sp;

    sp.draw = true;

    struct LineRule { const char* key; float len; float width; };
    static const LineRule kLines[] = {
        { "Earthshaker_Fissure",    1350.f, 300.f },
        { "Pudge_MeatHook",         1300.f, 160.f },
        { "Mirana_Arrow",           2600.f, 120.f },
        { "Windranger_Powershot",   1875.f, 140.f },
        { "Magnus_Shockwave",       1250.f, 200.f },
        { "Lion_Impale",             850.f, 150.f },
        { "Invoker_Tornado",        1600.f, 200.f },
        { "Invoker_DeafeningBlast", 1100.f, 260.f },
        { "SandKing_Burrowstrike",   700.f, 200.f },
        { "Mars_Spear",             1000.f, 160.f },
    };
    for (const auto& r : kLines) {
        if (has(r.key)) {
            sp.line = true;
            sp.len = r.len;
            sp.width = r.width;
            break;
        }
    }

    struct CircleRule { const char* key; float radius; };
    static const CircleRule kCircles[] = {
        { "Kunkka_Torrent",            250.f },
        { "Kunkka_GhostShip",          450.f },
        { "Invoker_SunStrike",         200.f },
        { "Invoker_EMP",               600.f },
        { "Lina_LightStrikeArray",     400.f },
        { "CrystalMaiden_CrystalNova", 425.f },
        { "FacelessVoid_Chronosphere", 500.f },
        { "WitchDoctor_Maledict",      180.f },
    };
    if (!sp.line) {
        for (const auto& r : kCircles) {
            if (has(r.key)) { sp.radius = r.radius; break; }
        }
    }

    if (has("ChaosMeteor")) {
        sp.meteor = true;
        sp.radius = 300.f;
        sp.len = 1025.f;
        sp.width = 200.f;
    }
    if (has("BerserkersCall"))  { sp.atSelf = true; sp.radius = 325.f; }
    if (has("Epicenter"))       { sp.atSelf = true; sp.radius = 600.f; }
    if (has("IceWall"))         { sp.atSelf = true; sp.radius = 350.f; }

    if (p) {
        size_t i = 0;
        for (; n[i] && i + 1 < sizeof(sp.label); ++i)
            sp.label[i] = (n[i] == '_') ? ' ' : n[i];
        sp.label[i] = 0;
    }
    return sp;
}

static void GroundPoly(ImDrawList* dl, const Vec3* w, int n, ImU32 fill, ImU32 border) {
    ImVec2 pts[48];
    if (n < 3 || n > 48) return;
    for (int i = 0; i < n; ++i) {
        ImVec2 p;
        if (!view::W2SRaw(w[i], p)) return;
        if (fabsf(p.x) > view::W * 8.f || fabsf(p.y) > view::H * 8.f) return;
        pts[i] = p;
    }
    dl->AddConvexPolyFilled(pts, n, fill);
    dl->AddPolyline(pts, n, border, ImDrawFlags_Closed, 1.6f);
}

static void GroundCircle(ImDrawList* dl, const Vec3& c, float r, ImU32 fill, ImU32 border) {
    enum { N = 40 };
    ImVec2 pts[N];
    int m = 0;
    for (int i = 0; i < N; ++i) {
        float a = 6.2831853f * (float)i / (float)N;
        Vec3 w{ c.x + cosf(a) * r, c.y + sinf(a) * r, c.z };
        ImVec2 p;
        if (view::W2SRaw(w, p) && fabsf(p.x) < view::W * 8.f && fabsf(p.y) < view::H * 8.f)
            pts[m++] = p;
    }
    if (m < 12) return;
    dl->AddConvexPolyFilled(pts, m, fill);
    dl->AddPolyline(pts, m, border, ImDrawFlags_Closed, 1.6f);
}

static void GroundBand(ImDrawList* dl, const Vec3& from, const Vec3& dir,
                       float len, float halfW, float zEnd, ImU32 fill, ImU32 border) {
    Vec3 e{ from.x + dir.x * len, from.y + dir.y * len, zEnd };
    Vec3 q[4] = {
        { from.x - dir.y * halfW, from.y + dir.x * halfW, from.z },
        { e.x    - dir.y * halfW, e.y    + dir.x * halfW, e.z },
        { e.x    + dir.y * halfW, e.y    - dir.x * halfW, e.z },
        { from.x + dir.y * halfW, from.y - dir.x * halfW, from.z },
    };
    GroundPoly(dl, q, 4, fill, border);
}

static void DrawSkillPreview(const Frame& f, const FrameUnit& u, ImDrawList* dl) {
    if (u.alive && u.mdirOk) {
        GroundBand(dl, Vec3{ u.pos.x - u.mdir.x * 40.f, u.pos.y - u.mdir.y * 40.f, u.pos.z },
                   u.mdir, 460.f, 34.f, u.pos.z,
                   IM_COL32(168, 85, 247, 45), IM_COL32(192, 132, 252, 95));
    }

    const AbilityInfo* cast = nullptr;
    for (int i = 0; i < u.abilN; ++i) {
        if (u.abil[i].phase) { cast = &u.abil[i]; break; }
    }
    if (!cast) return;

    SpellShape sp = ClassifySpell(cast->cls);
    if (!sp.draw) return;

    Vec3 aim{};
    bool hasAim = false;
    const FrameUnit* bestHero = nullptr;
    const FrameUnit* bestCreep = nullptr;
    float bh = 4000.f, bc = 1900.f;
    for (const auto& t : f.units) {
        if (t.addr == u.addr || t.team == u.team || !t.alive) continue;
        float d = VecDist(t.pos, u.pos);
        if (t.kind == UnitKind::Hero) {
            if (d < bh) { bh = d; bestHero = &t; }
        } else if (t.kind == UnitKind::Creep || t.kind == UnitKind::Ward) {
            if (d < bc) { bc = d; bestCreep = &t; }
        }
    }
    const FrameUnit* best = bestHero ? bestHero : bestCreep;
    if (best) { aim = best->pos; hasAim = true; }
    if (!hasAim) aim = u.pos;

    Vec3 dir{};
    if (hasAim) {
        float dx = aim.x - u.pos.x, dy = aim.y - u.pos.y;
        float d = sqrtf(dx * dx + dy * dy);
        if (d > 1.f) dir = Vec3{ dx / d, dy / d, 0.f };
    }
    if (dir.x == 0.f && dir.y == 0.f) {
        if (u.mdirOk) {
            dir = u.mdir;
        } else {
            float dx = f.localPos.x - u.pos.x, dy = f.localPos.y - u.pos.y;
            float d = sqrtf(dx * dx + dy * dy);
            dir = (d > 1.f) ? Vec3{ dx / d, dy / d, 0.f } : Vec3{ 1.f, 0.f, 0.f };
        }
    }

    ImU32 fill   = IM_COL32(168, 85, 247, 70);
    ImU32 border = IM_COL32(192, 132, 252, 215);
    Vec3 center = sp.atSelf ? u.pos : aim;

    if (sp.line) {
        GroundBand(dl, u.pos, dir, sp.len, sp.width * 0.5f, center.z, fill, border);
    } else {
        GroundCircle(dl, center, sp.radius, fill, border);
    }

    if (sp.meteor) {
        Vec3 s{ aim.x + dir.x * 60.f, aim.y + dir.y * 60.f, aim.z };
        GroundBand(dl, s, dir, sp.len, sp.width * 0.5f, aim.z,
                   IM_COL32(168, 85, 247, 55), IM_COL32(192, 132, 252, 170));
    }

    if (sp.label[0]) {
        Vec3 tp{ center.x, center.y, center.z + 130.f };
        ImVec2 p;
        if (view::W2S(tp, p)) DTextC(dl, sp.label, p.x, p.y, theme::White, 15.f);
    }
}

static void DrawDotaPlus(const Frame& f, ImDrawList* dl) {
    if (!cfg::dotaPlus) return;
    if (view::W < 500) return;

    const float pw = 460.f, ph = 54.f;
    static float s_x = -10000.f, s_y = -10000.f;
    if (s_x < -9999.f) { s_x = (view::W - pw) * 0.5f; s_y = 10.f; }

    ImGuiIO& io = ImGui::GetIO();
    static bool s_drag = false;
    bool hover = cfg::menuOpen &&
                 io.MousePos.x >= s_x && io.MousePos.x <= s_x + pw &&
                 io.MousePos.y >= s_y && io.MousePos.y <= s_y + ph;
    if (cfg::menuOpen) {
        if (hover && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) s_drag = true;
        if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) s_drag = false;
        if (s_drag) {
            s_x += io.MouseDelta.x;
            s_y += io.MouseDelta.y;
        }
    } else s_drag = false;
    if (s_x < 0.f) s_x = 0.f;
    if (s_y < 0.f) s_y = 0.f;
    if (s_x > view::W - pw) s_x = (float)view::W - pw;
    if (s_y > view::H - ph) s_y = (float)view::H - ph;

    float x = s_x, y = s_y;

    glass::AddRect(ImVec2(x, y), ImVec2(x + pw, y + ph), 10.f);
    dl->AddRectFilled(ImVec2(x, y), ImVec2(x + pw, y + ph), theme::Panel, 10.f);
    dl->AddRect(ImVec2(x, y), ImVec2(x + pw, y + ph), theme::Border, 10.f, 0, 1.f);
    if (hover || s_drag)
        dl->AddRect(ImVec2(x - 2.f, y - 2.f), ImVec2(x + pw + 2.f, y + ph + 2.f),
                    theme::WithAlpha(theme::White, s_drag ? 90 : 45), 11.f, 0, 1.f);

    char buf[64];

    float t = (f.gameStart > 0.f) ? (f.now - f.gameStart) : f.now;
    if (t < 0.f) t = 0.f;
    FmtTime(t, buf, sizeof(buf));
    DTextC(dl, buf, x + pw * 0.5f, y + 7.f, theme::White, 21.f);

    if (f.roshanAlive) {
        snprintf(buf, sizeof(buf), "ROSHAN UP");
        DText(dl, buf, ImVec2(x + 12.f, y + 9.f), theme::PurpleLight);
        if (f.roshanMaxHp > 0) {
            snprintf(buf, sizeof(buf), "%d/%d", f.roshanHp, f.roshanMaxHp);
            DText(dl, buf, ImVec2(x + 12.f, y + 27.f), theme::Dim);
        }
    } else if (f.roshanEtaLo >= 0.f) {
        if ((int)f.roshanEtaLo == (int)f.roshanEtaHi)
            FmtTime(f.roshanEtaLo, buf, sizeof(buf));
        else {
            char lo[16], hi[16];
            FmtTime(f.roshanEtaLo, lo, sizeof(lo));
            FmtTime(f.roshanEtaHi, hi, sizeof(hi));
            snprintf(buf, sizeof(buf), "%s-%s", lo, hi);
        }
        char out[48];
        snprintf(out, sizeof(out), "ROSHAN %s", buf);
        DText(dl, out, ImVec2(x + 12.f, y + 9.f), theme::PurpleLight);
        DText(dl, "respawn timer", ImVec2(x + 12.f, y + 29.f), theme::Dim);
    } else {
        DText(dl, "ROSHAN ?", ImVec2(x + 12.f, y + 9.f), theme::Dim);
        DText(dl, "unknown", ImVec2(x + 12.f, y + 29.f), theme::Dim);
    }

    float sc[4] = {};
    for (const auto& u : f.units) {
        if (u.kind != UnitKind::Hero || !u.alive) continue;
        if (u.team < 2 || u.team > 3) continue;
        sc[u.team] += (float)u.hp + (float)u.level * 140.f + u.mana * 0.4f;
    }
    float mine = sc[f.localTeam >= 2 && f.localTeam <= 3 ? f.localTeam : 2];
    float foe  = sc[f.localTeam == 2 ? 3 : 2];
    float total = mine + foe;
    float prob = (total > 1.f) ? mine / total : 0.5f;

    char pct[24];
    snprintf(pct, sizeof(pct), "%d%%", (int)(prob * 100.f + 0.5f));
    DText(dl, pct, ImVec2(x + pw - 12.f - DTextW(pct, 15.f), y + 8.f),
          theme::PurpleLight, 15.f);
    DText(dl, "win chance", ImVec2(x + pw - 12.f - DTextW("win chance"), y + 28.f),
          theme::Dim);

    float bx = x + pw * 0.5f - 70.f, bw = 140.f;
    float byy = y + 14.f;
    dl->AddRectFilled(ImVec2(bx, byy), ImVec2(bx + bw, byy + 6.f),
                      IM_COL32(255, 255, 255, 26), 3.f);
    dl->AddRectFilled(ImVec2(bx, byy), ImVec2(bx + bw * prob, byy + 6.f),
                      theme::Purple, 3.f);

    if (f.maxHp > 0) {
        char hp[48];
        snprintf(hp, sizeof(hp), "YOU  %d/%d  |  %d/%d  |  L%d",
                 f.hp, f.maxHp, f.mana, f.maxMana, f.level);
        DTextC(dl, hp, x + pw * 0.5f, y + ph - 17.f, theme::WithAlpha(theme::White, 235), 13.f);
    }
}

static bool g_dodgeWarn = false;

static void DrawAlerts(ImDrawList* dl) {
    if (g_dodgeWarn && (cfg::dodger || cfg::autoDodge)) {
        float cx = view::W * 0.5f, cy = view::H * 0.62f;
        glass::AddRect(ImVec2(cx - 74.f, cy - 6.f), ImVec2(cx + 74.f, cy + 30.f), 8.f);
        dl->AddRectFilled(ImVec2(cx - 74.f, cy - 6.f), ImVec2(cx + 74.f, cy + 30.f),
                          theme::WithAlpha(theme::Ink, 200), 8.f);
        dl->AddRect(ImVec2(cx - 74.f, cy - 6.f), ImVec2(cx + 74.f, cy + 30.f),
                    theme::Border, 8.f, 0, 1.f);
        DTextC(dl, "DODGE!", cx, cy, IM_COL32(255, 120, 120, 255), 26.f);
    }
}

static void WriteGlow(uintptr_t e, bool on) {
    uintptr_t g = e + off::ModelEntity::m_Glow;
    if (on) {
        mem::Write<uint8_t>(e + off::NPC::m_bSuppressGlow, 0);
        mem::Write<int32_t>(g + off::Glow::m_iGlowType, 3);
        mem::Write<uint32_t>(g + off::Glow::m_nGlowRange, 32000u);
        mem::Write<uint32_t>(g + off::Glow::m_glowColorOverride, 0xFFF755A8u);
        mem::Write<float>(g + off::Glow::m_fGlowColor + 0x0, 0.66f);
        mem::Write<float>(g + off::Glow::m_fGlowColor + 0x4, 0.33f);
        mem::Write<float>(g + off::Glow::m_fGlowColor + 0x8, 0.97f);
        mem::Write<bool>(g + off::Glow::m_bGlowing, true);
    } else {
        mem::Write<bool>(g + off::Glow::m_bGlowing, false);
        mem::Write<int32_t>(g + off::Glow::m_iGlowType, 0);
        mem::Write<uint32_t>(g + off::Glow::m_nGlowRange, 0);
    }
}

static void ApplyVision(const Frame& f) {
    static bool  s_readFog  = false;
    static float s_fogStr = 1.f, s_fogDist = 1.f, s_fogDen = 1.f;
    static uint8_t s_fogEn = 1, s_fogStart = 0;
    static bool  s_vbeOn  = false;
    static bool  s_glowOn = false;

    bool vbe  = cfg::vbe;
    bool glow = cfg::glow;

    static bool    s_fogRead = false;
    static uint8_t s_fogOrig = 0, s_unseenOrig = 0;
    if (game::g_modeEnt) {
        if (!s_fogRead) {
            s_fogRead = true;
            mem::Read(game::g_modeEnt + off::GameMode::m_bFogOfWarDisabled, s_fogOrig);
            mem::Read(game::g_modeEnt + off::GameMode::m_bUseUnseenFOW, s_unseenOrig);
        }
        if (vbe) {
            mem::Write<uint8_t>(game::g_modeEnt + off::GameMode::m_bFogOfWarDisabled, 1);
            mem::Write<uint8_t>(game::g_modeEnt + off::GameMode::m_bUseUnseenFOW, 0);
        } else {
            mem::Write<uint8_t>(game::g_modeEnt + off::GameMode::m_bFogOfWarDisabled, s_fogOrig);
            mem::Write<uint8_t>(game::g_modeEnt + off::GameMode::m_bUseUnseenFOW, s_unseenOrig);
        }
    }

    if (game::g_visEnt) {
        uintptr_t v = game::g_visEnt;
        if (!s_readFog) {
            s_readFog = true;
            mem::Read(v + off::PlayerVisibility::m_flVisibilityStrength, s_fogStr);
            mem::Read(v + off::PlayerVisibility::m_flFogDistanceMultiplier, s_fogDist);
            mem::Read(v + off::PlayerVisibility::m_flFogMaxDensityMultiplier, s_fogDen);
            mem::Read(v + off::PlayerVisibility::m_bIsEnabled, s_fogEn);
            mem::Read(v + off::PlayerVisibility::m_bStartDisabled, s_fogStart);
        }
        if (vbe) {
            mem::Write<float>(v + off::PlayerVisibility::m_flVisibilityStrength, 1.f);
            mem::Write<float>(v + off::PlayerVisibility::m_flFogDistanceMultiplier, 8.f);
            mem::Write<float>(v + off::PlayerVisibility::m_flFogMaxDensityMultiplier, 0.05f);
        } else if (s_vbeOn) {
            mem::Write<float>(v + off::PlayerVisibility::m_flVisibilityStrength, s_fogStr);
            mem::Write<float>(v + off::PlayerVisibility::m_flFogDistanceMultiplier, s_fogDist);
            mem::Write<float>(v + off::PlayerVisibility::m_flFogMaxDensityMultiplier, s_fogDen);
        }
    }

    for (const auto& u : f.units) {
        bool enemy = (u.team != f.localTeam) && u.team >= 2 && f.localTeam >= 2;

        if (vbe && enemy && (u.kind == UnitKind::Hero || u.kind == UnitKind::Ward ||
                             u.kind == UnitKind::Courier)) {
            mem::Write<uint32_t>(u.addr + off::ModelEntity::m_iTeamVisibilityBitmask, 0xFFFFFFFFu);
            mem::Write<bool>(u.addr + off::ModelEntity::m_bVisibilityDirtyFlag, true);
        }

        if (glow && enemy && u.kind == UnitKind::Hero && u.alive)
            WriteGlow(u.addr, true);
        else if (!glow && s_glowOn && enemy && u.kind == UnitKind::Hero)
            WriteGlow(u.addr, false);
    }

    s_vbeOn  = vbe;
    s_glowOn = glow;
}

void DrawOverlay(const Frame& f) {
    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    if (!dl || !f.ok) return;
    if (f.gameState == 6 || f.gameState == 7) return;

    ResolveMode(f);

    static int  s_visTick = 0;
    static bool s_vbePrev = false, s_glowPrev = false;
    ++s_visTick;
    bool visChanged = (cfg::vbe != s_vbePrev) || (cfg::glow != s_glowPrev);
    if (visChanged || ((s_visTick % 5) == 0 && (cfg::vbe || cfg::glow)))
        ApplyVision(f);
    s_vbePrev  = cfg::vbe;
    s_glowPrev = cfg::glow;

    for (const auto& u : f.units) {
        switch (u.kind) {
        case UnitKind::Hero: {

            float dx = u.pos.x - f.localPos.x;
            float dy = u.pos.y - f.localPos.y;
            float dz = u.pos.z - f.localPos.z;
            bool self = (u.team == f.localTeam) &&
                        (dx * dx + dy * dy + dz * dz) < 1.0f;
            if (self) break;
            if (cfg::skillPreview && u.team != f.localTeam)
                DrawSkillPreview(f, u, dl);
            if (cfg::espHeroes) DrawHero(f, u, u.team != f.localTeam, dl);
            break;
        }
        case UnitKind::Creep:
        case UnitKind::Building:
        case UnitKind::Boss:
            DrawCreepMarkers(f, u, dl);
            break;
        case UnitKind::Ward:
            if (cfg::espWards) DrawWard(f, u, dl);
            break;
        case UnitKind::Roshan:
            if (cfg::espRoshan && u.alive) {
                FrameUnit copy = u;
                DrawHero(f, copy, true, dl);
                ImVec2 p;
                Vec3 tp{ u.pos.x, u.pos.y, u.pos.z + u.hbOffset };
                if (view::W2S(tp, p)) {
                    DTextC(dl, "ROSHAN", p.x, p.y - 24.f, theme::PurpleLight, 17.f);
                }
            }
            break;
        default:
            break;
        }
    }

    DrawRoshanMarker(f, dl);
    DrawDotaPlus(f, dl);
    DrawAlerts(dl);
}

static HWND GameHwnd() {
    static HWND s_h = nullptr;
    if (s_h && IsWindow(s_h)) return s_h;

    DWORD pid = GetCurrentProcessId();
    HWND best = nullptr;
    long bestArea = 0;

    for (HWND w = FindWindowA(nullptr, nullptr); w;
         w = FindWindowExA(nullptr, w, nullptr, nullptr)) {
        DWORD wp = 0;
        GetWindowThreadProcessId(w, &wp);
        if (wp != pid || !IsWindowVisible(w)) continue;
        RECT r;
        if (!GetClientRect(w, &r)) continue;
        long area = (long)(r.right - r.left) * (long)(r.bottom - r.top);
        if (area > bestArea) { bestArea = area; best = w; }
    }
    if (best) s_h = best;
    return best;
}

void RunAutomation(const Frame& f) {
    input::Tick();

    static DWORD s_clickAt = 0;
    static DWORD s_dodgeAt = 0;
    g_dodgeWarn = false;

    if (!f.ok) return;
    if (f.gameState == 6 || f.gameState == 7) return;
    if (!cfg::farmBot && !cfg::farmAuto && !cfg::dodger && !cfg::autoDodge) return;

    HWND hwnd = GameHwnd();
    if (!hwnd || GetForegroundWindow() != hwnd) return;
    if (cfg::menuOpen) return;

    DWORD now = GetTickCount();

    if (cfg::dodger || cfg::autoDodge) {
        for (const auto& u : f.units) {
            if (u.kind != UnitKind::Hero || u.team == f.localTeam) continue;
            if (!u.alive || u.dist > 900.f) continue;

            bool casting = false;
            for (int i = 0; i < u.abilN; ++i)
                if (u.abil[i].phase) { casting = true; break; }
            if (!casting) continue;

            g_dodgeWarn = true;

            if (cfg::autoDodge && now - s_dodgeAt > 700) {

                float dx = f.localPos.x - u.pos.x;
                float dy = f.localPos.y - u.pos.y;
                float len = sqrtf(dx * dx + dy * dy);
                if (len < 1.f) { dx = 1.f; dy = 0.f; len = 1.f; }
                float ux = dx / len, uy = dy / len;
                const float step = 420.f;
                Vec3 cand[4] = {
                    { f.localPos.x - uy * step, f.localPos.y + ux * step, f.localPos.z },
                    { f.localPos.x + uy * step, f.localPos.y - ux * step, f.localPos.z },
                    { f.localPos.x + ux * step, f.localPos.y + uy * step, f.localPos.z },
                    { f.localPos.x - ux * step, f.localPos.y - uy * step, f.localPos.z },
                };

                int best = -1;
                float bestD = 0.f;
                for (int i = 0; i < 4; ++i) {
                    ImVec2 sp;
                    if (!view::W2SRaw(cand[i], sp)) continue;
                    if (sp.x > 70.f && sp.x < view::W - 70.f &&
                        sp.y > 70.f && sp.y < view::H - 70.f) { best = i; break; }
                    float mx = sp.x - view::W * 0.5f, my = sp.y - view::H * 0.5f;
                    float d = mx * mx + my * my;
                    if (best < 0 || d < bestD) { bestD = d; best = i; }
                }

                ImVec2 sp;
                if (best >= 0 && view::W2SRaw(cand[best], sp)) {
                    if (sp.x < 24.f) sp.x = 24.f;
                    if (sp.x > view::W - 24.f) sp.x = (float)view::W - 24.f;
                    if (sp.y < 24.f) sp.y = 24.f;
                    if (sp.y > view::H - 24.f) sp.y = (float)view::H - 24.f;
                    input::ClickAt(hwnd, (int)sp.x, (int)sp.y, true);
                    s_dodgeAt = now;
                }
            }
            break;
        }
    }

    if (!cfg::farmBot && !cfg::farmAuto) return;
    if (now - s_clickAt < 450) return;

    const FrameUnit* target = nullptr;

    if (cfg::farmBot) {
        for (const auto& u : f.units) {
            if (!u.canLastHit || !u.alive) continue;
            if (!target || u.dist < target->dist) target = &u;
        }
    }

    if (!target && cfg::farmAuto) {
        for (const auto& u : f.units) {
            if (u.kind != UnitKind::Creep && u.kind != UnitKind::Building) continue;
            if (!u.alive || u.team == f.localTeam) continue;
            if (u.dist > (float)f.atkRange + 60.f) continue;
            if (!target || u.dist < target->dist) target = &u;
        }
    }

    if (!target) return;

    Vec3 tgt{ target->pos.x, target->pos.y, target->pos.z + 70.f };
    ImVec2 sp;
    if (!view::W2S(tgt, sp)) return;
    if (sp.x < 0.f || sp.y < 0.f || sp.x >= view::W || sp.y >= view::H) return;

    input::ClickAt(hwnd, (int)sp.x, (int)sp.y, true);
    s_clickAt = now;
}

void binds::ProcessBinds() {
    static bool prev[256] = {};
    for (int k = 1; k < 256; ++k) {
        bool d = (GetAsyncKeyState(k) & 0x8000) != 0;
        Down[k] = d;
        Edge[k] = d && !prev[k];
        prev[k] = d;
    }

    if (cfg::rebindIdx >= 0) return;

    for (int i = 0; i < kCount; ++i) {
        KeyBind& b = items[i];
        if (!b.vk || !b.target) continue;
        bool down = Down[b.vk];
        if (b.hold) {
            if (down && !b.holding) {
                b.holdSaved = *b.target;
                *b.target = true;
                b.holding = true;
            } else if (!down && b.holding) {
                *b.target = b.holdSaved;
                b.holding = false;
            }
        } else if (Edge[b.vk]) {
            *b.target = !*b.target;
        }
    }
}

void DrawKeybinds() {
    if (!cfg::showKeybinds) return;

    ImGuiIO& io = ImGui::GetIO();
    if (io.DisplaySize.x < 300.f || io.DisplaySize.y < 200.f) return;

    ImFont* f = theme::FontSmall();
    float fsz = f ? f->LegacySize : 14.f;

    char lines[32][72];
    bool on[32];
    int n = 0;
    float maxw = 0.f;

    for (int i = 0; i < binds::kCount && n < 32; ++i) {
        const KeyBind& b = binds::items[i];
        if (!b.vk || !b.target) continue;
        snprintf(lines[n], sizeof(lines[n]), "%s: %s  %s",
                 b.name, binds::KeyName(b.vk), b.hold ? "H" : "T");
        on[n] = *b.target;
        if (f) {
            float lw = f->CalcTextSizeA(fsz, FLT_MAX, 0.f, lines[n]).x;
            if (lw > maxw) maxw = lw;
        }
        ++n;
    }
    if (n == 0) return;

    const char* title = "keybinds";
    if (f) {
        float tw = f->CalcTextSizeA(fsz, FLT_MAX, 0.f, title).x;
        if (tw > maxw) maxw = tw;
    }

    const float pad = 9.f;
    const float lineH = fsz + 4.f;
    const float w = maxw + pad * 2.f;
    const float h = pad + fsz + 5.f + n * lineH + pad - 3.f;

    static float s_x = -10000.f, s_y = -10000.f;
    if (s_x < -9999.f) { s_x = io.DisplaySize.x - w - 14.f; s_y = 14.f; }

    static bool s_drag = false;
    bool hover = cfg::menuOpen &&
                 io.MousePos.x >= s_x && io.MousePos.x <= s_x + w &&
                 io.MousePos.y >= s_y && io.MousePos.y <= s_y + h;
    if (cfg::menuOpen) {
        if (hover && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) s_drag = true;
        if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) s_drag = false;
        if (s_drag) {
            s_x += io.MouseDelta.x;
            s_y += io.MouseDelta.y;
        }
    } else s_drag = false;

    if (s_x < 0.f) s_x = 0.f;
    if (s_y < 0.f) s_y = 0.f;
    if (s_x > io.DisplaySize.x - w) s_x = io.DisplaySize.x - w;
    if (s_y > io.DisplaySize.y - h) s_y = io.DisplaySize.y - h;

    const float x = s_x, y = s_y;

    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    if (!dl) return;

    glass::AddRect(ImVec2(x, y), ImVec2(x + w, y + h), 8.f);
    dl->AddRectFilled(ImVec2(x, y), ImVec2(x + w, y + h), theme::Panel, 8.f);
    dl->AddRect(ImVec2(x, y), ImVec2(x + w, y + h), theme::Border, 8.f, 0, 1.f);
    if (hover || s_drag)
        dl->AddRect(ImVec2(x - 2.f, y - 2.f), ImVec2(x + w + 2.f, y + h + 2.f),
                    theme::WithAlpha(theme::White, s_drag ? 90 : 45), 9.f, 0, 1.f);
    dl->AddText(f, fsz, ImVec2(x + pad + 1.f, y + pad + 1.f),
                IM_COL32(0, 0, 0, 215), title);
    dl->AddText(f, fsz, ImVec2(x + pad, y + pad), theme::White, title);

    float ly = y + pad + fsz + 5.f;
    for (int i = 0; i < n; ++i) {
        ImU32 c = on[i] ? IM_COL32(235, 235, 240, 255) : theme::Dim;
        dl->AddText(f, fsz, ImVec2(x + pad + 1.f, ly + 1.f), IM_COL32(0, 0, 0, 200), lines[i]);
        dl->AddText(f, fsz, ImVec2(x + pad, ly), c, lines[i]);
        ly += lineH;
    }
}

static bool CodePageOk(uintptr_t p) {
    if ((p & 0xFFF) > 0xFFC) return false;
    MEMORY_BASIC_INFORMATION mbi{};
    if (VirtualQuery((LPCVOID)p, &mbi, sizeof(mbi)) != sizeof(mbi)) return false;
    if (mbi.State != MEM_COMMIT) return false;
    if (mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS)) return false;
    DWORD pr = mbi.Protect & 0xFF;
    return pr == PAGE_EXECUTE || pr == PAGE_EXECUTE_READ ||
           pr == PAGE_EXECUTE_READWRITE || pr == PAGE_EXECUTE_WRITECOPY;
}

static bool LooksLikeFunc(uintptr_t p) {
    if (!CodePageOk(p)) return false;
    const unsigned char* b = (const unsigned char*)p;
    if (b[0] == 0x40 && (b[1] & 0xF0) == 0x50) return true;
    if (b[0] == 0x55 || b[0] == 0x53 || b[0] == 0x56 || b[0] == 0x57) return true;
    if (b[0] == 0xE9 || b[0] == 0xEB) return true;
    if (b[0] == 0x33 && (b[1] & 0xC7) == 0xC0) return true;
    if (b[0] == 0x48 || b[0] == 0x49 || b[0] == 0x4C || b[0] == 0x4D) {
        if (b[1] == 0x83 && b[2] == 0xEC) return true;
        if (b[1] == 0x8B && b[2] == 0xEC) return true;
        if (b[1] == 0x8B && (b[2] & 0xC7) == 0x05) return true;
        if (b[1] == 0x8B && (b[2] & 0xC7) == 0x0D) return true;
        if (b[1] == 0x89 && (b[2] & 0xC7) == 0x44) return true;
        if (b[1] == 0x8D) return true;
        if (b[1] == 0x63) return true;
    }
    return false;
}

static void AutoAcceptTick() {
    uintptr_t base = game::g_sys.clientBase;
    if (!base) return;

    static DWORD s_sentAt = 0;
    DWORD now = GetTickCount();

    typedef void* (*GetMatchFn)(void*);
    typedef bool  (*SendReadyFn)(void*, int);
    typedef void* (*GetGlobalFn)();

    GetMatchFn getMatch = (GetMatchFn)(base + 0x1D7A810);
    void* mm = getMatch((void*)(base + 0x63CAE18));
    void* st = mm ? *(void**)((char*)mm + 0x18) : nullptr;
    if (!st) return;

    int phase = *(int*)((char*)st + 0x1A8);
    if (phase != 15) return;
    if (s_sentAt && now - s_sentAt < 2000) return;

    GetGlobalFn getGlobal = (GetGlobalFn)(base + 0x2286C90);
    void* g = getGlobal();
    if (!g) return;

    SendReadyFn sendReady = (SendReadyFn)(base + 0x2292C40);
    sendReady(g, 1);
    s_sentAt = now;
}

void RunAutoAccept() {
    if (!cfg::autoAccept) return;
    uintptr_t base = game::g_sys.clientBase;
    if (!base) return;
    static int s_valid = 0;
    if (!s_valid) {
        s_valid = (LooksLikeFunc(base + 0x1D7A810) &&
                   LooksLikeFunc(base + 0x2286C90) &&
                   LooksLikeFunc(base + 0x2292C40)) ? 1 : -1;
        if (s_valid < 0) cfg::autoAccept = false;
    }
    if (s_valid < 0) return;
    __try {
        AutoAcceptTick();
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        cfg::autoAccept = false;
        s_valid = -1;
    }
}

