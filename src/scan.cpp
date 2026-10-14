#include "common.h"
#include "mem.h"
#include "offsets.h"
#include "scan.h"
#include <unordered_map>
#include <vector>
#include <cmath>
#include <cstring>

namespace scan {

const FieldBind g_fields[] = {
    { "m_pGameSceneNode",   &off::BaseEntity::m_pGameSceneNode,  { nullptr, nullptr, nullptr } },
    { "m_iMaxHealth",       &off::BaseEntity::m_iMaxHealth,      { nullptr, nullptr, nullptr } },
    { "m_iHealth",          &off::BaseEntity::m_iHealth,         { "m_iMaxHealth", "m_iTeamNum", nullptr } },
    { "m_lifeState",        &off::BaseEntity::m_lifeState,       { nullptr, nullptr, nullptr } },
    { "m_flSimulationTime", &off::BaseEntity::m_flSimulationTime,{ nullptr, nullptr, nullptr } },
    { "m_flAnimTime",       &off::BaseEntity::m_flAnimTime,      { "m_flSimulationTime", "m_lifeState", "m_iHealth" } },
    { "m_iTeamNum",         &off::BaseEntity::m_iTeamNum,        { nullptr, nullptr, nullptr } },
    { "m_hOwnerEntity",     &off::BaseEntity::m_hOwnerEntity,    { nullptr, nullptr, nullptr } },

    { "m_pOwner",           &off::SceneNode::m_pOwner,           { "m_vecAbsOrigin", nullptr, nullptr } },
    { "m_vecAbsOrigin",     &off::SceneNode::m_vecAbsOrigin,     { nullptr, nullptr, nullptr } },

    { "m_iViewerID",              &off::ModelEntity::m_iViewerID,             { nullptr, nullptr, nullptr } },
    { "m_iTeamVisibilityBitmask", &off::ModelEntity::m_iTeamVisibilityBitmask,{ nullptr, nullptr, nullptr } },
    { "m_bVisibilityDirtyFlag",   &off::ModelEntity::m_bVisibilityDirtyFlag,  { nullptr, nullptr, nullptr } },
    { "m_Glow",                   &off::ModelEntity::m_Glow,                  { nullptr, nullptr, nullptr } },

    { "m_fGlowColor",        &off::Glow::m_fGlowColor,        { nullptr, nullptr, nullptr } },
    { "m_iGlowType",         &off::Glow::m_iGlowType,         { nullptr, nullptr, nullptr } },
    { "m_iGlowTeam",         &off::Glow::m_iGlowTeam,         { nullptr, nullptr, nullptr } },
    { "m_nGlowRange",        &off::Glow::m_nGlowRange,        { nullptr, nullptr, nullptr } },
    { "m_nGlowRangeMin",     &off::Glow::m_nGlowRangeMin,     { nullptr, nullptr, nullptr } },
    { "m_glowColorOverride", &off::Glow::m_glowColorOverride, { nullptr, nullptr, nullptr } },
    { "m_bFlashing",         &off::Glow::m_bFlashing,         { nullptr, nullptr, nullptr } },
    { "m_bGlowing",          &off::Glow::m_bGlowing,          { nullptr, nullptr, nullptr } },

    { "m_iCurrentLevel",       &off::NPC::m_iCurrentLevel,       { nullptr, nullptr, nullptr } },
    { "m_bIsAncient",          &off::NPC::m_bIsAncient,          { nullptr, nullptr, nullptr } },
    { "m_bIsBossCreature",     &off::NPC::m_bIsBossCreature,     { nullptr, nullptr, nullptr } },
    { "m_bConsideredHero",     &off::NPC::m_bConsideredHero,     { nullptr, nullptr, nullptr } },
    { "m_iAttackRange",        &off::NPC::m_iAttackRange,        { nullptr, nullptr, nullptr } },
    { "m_iHealthBarOffset",    &off::NPC::m_iHealthBarOffset,    { nullptr, nullptr, nullptr } },
    { "m_flMana",              &off::NPC::m_flMana,              { "m_flMaxMana", "m_iAttackRange", "m_iMoveSpeed" } },
    { "m_flMaxMana",           &off::NPC::m_flMaxMana,           { nullptr, nullptr, nullptr } },
    { "m_vecAbilities",        &off::NPC::m_vecAbilities,        { nullptr, nullptr, nullptr } },
    { "m_bIsIllusion",         &off::NPC::m_bIsIllusion,         { nullptr, nullptr, nullptr } },
    { "m_flInvisibilityLevel", &off::NPC::m_flInvisibilityLevel, { nullptr, nullptr, nullptr } },
    { "m_iszUnitName",         &off::NPC::m_iszUnitName,         { nullptr, nullptr, nullptr } },
    { "m_iDamageMin",          &off::NPC::m_iDamageMin,          { nullptr, nullptr, nullptr } },
    { "m_iDamageMax",          &off::NPC::m_iDamageMax,          { nullptr, nullptr, nullptr } },
    { "m_iDamageBonus",        &off::NPC::m_iDamageBonus,        { nullptr, nullptr, nullptr } },
    { "m_iMoveSpeed",          &off::NPC::m_iMoveSpeed,          { nullptr, nullptr, nullptr } },
    { "m_nPlayerOwnerID",      &off::NPC::m_nPlayerOwnerID,      { nullptr, nullptr, nullptr } },
    { "m_flLastAttackTime",    &off::NPC::m_flLastAttackTime,    { nullptr, nullptr, nullptr } },
    { "m_flDeathTime",         &off::NPC::m_flDeathTime,         { "m_flPhysicalArmorValue", "m_flLastAttackTime", nullptr } },
    { "m_flPhysicalArmorValue",&off::NPC::m_flPhysicalArmorValue,{ nullptr, nullptr, nullptr } },
    { "m_bSuppressGlow",       &off::NPC::m_bSuppressGlow,       { nullptr, nullptr, nullptr } },

    { "m_flRespawnTime",     &off::Hero::m_flRespawnTime,    { "m_bLifeState", "m_bDisableWearables", "m_iPlayerID" } },
    { "m_iPlayerID",         &off::Hero::m_iPlayerID,        { nullptr, nullptr, nullptr } },
    { "m_bLifeState",        &off::Hero::m_bLifeState,       { nullptr, nullptr, nullptr } },
    { "m_bDisableWearables", &off::Hero::m_bDisableWearables,{ nullptr, nullptr, nullptr } },

    { "m_bHiddenByEquipmentSlot", &off::Wearable::m_bHiddenByEquipmentSlot, { nullptr, nullptr, nullptr } },

    { "m_flCameraZoomAmount", &off::Camera::m_flCameraZoomAmount, { nullptr, nullptr, nullptr } },

    { "m_bHidden",          &off::Ability::m_bHidden,          { "m_flCastStartTime", "m_iLevel", "m_fCooldown" } },
    { "m_iLevel",           &off::Ability::m_iLevel,           { nullptr, nullptr, nullptr } },
    { "m_bInAbilityPhase",  &off::Ability::m_bInAbilityPhase,  { nullptr, nullptr, nullptr } },
    { "m_fCooldown",        &off::Ability::m_fCooldown,        { nullptr, nullptr, nullptr } },
    { "m_flCooldownLength", &off::Ability::m_flCooldownLength, { nullptr, nullptr, nullptr } },
    { "m_iManaCost",        &off::Ability::m_iManaCost,        { nullptr, nullptr, nullptr } },
    { "m_flCastStartTime",  &off::Ability::m_flCastStartTime,  { nullptr, nullptr, nullptr } },

    { "m_bIsLocalPlayerController", &off::Ctrl::m_bIsLocalPlayerController, { nullptr, nullptr, nullptr } },
    { "m_nPlayerID",          &off::Ctrl::m_nPlayerID,     { "m_hAssignedHero", "m_bIsLocalPlayerController", "m_hActiveAbility" } },
    { "m_hAssignedHero",      &off::Ctrl::m_hAssignedHero, { nullptr, nullptr, nullptr } },
    { "m_hActiveAbility",     &off::Ctrl::m_hActiveAbility,{ nullptr, nullptr, nullptr } },

    { "m_iLastKillerTeam",    &off::RoshanSpawner::m_iLastKillerTeam,    { nullptr, nullptr, nullptr } },
    { "m_iKillCount",         &off::RoshanSpawner::m_iKillCount,         { nullptr, nullptr, nullptr } },
    { "m_vRoshanAltLocation", &off::RoshanSpawner::m_vRoshanAltLocation, { nullptr, nullptr, nullptr } },
    { "m_hRoshan",            &off::RoshanSpawner::m_hRoshan,            { nullptr, nullptr, nullptr } },

    { "m_pGameRules",         &off::RulesProxy::m_pGameRules, { nullptr, nullptr, nullptr } },

    { "m_nStartingGold",              &off::Rules::m_nStartingGold,              { nullptr, nullptr, nullptr } },
    { "m_iGameMode",                  &off::Rules::m_iGameMode,                  { nullptr, nullptr, nullptr } },
    { "m_nGameState",                 &off::Rules::m_nGameState,                 { nullptr, nullptr, nullptr } },
    { "m_flGameStartTime",            &off::Rules::m_flGameStartTime,            { nullptr, nullptr, nullptr } },
    { "m_nRoshanRespawnPhase",        &off::Rules::m_nRoshanRespawnPhase,        { nullptr, nullptr, nullptr } },
    { "m_flRoshanRespawnPhaseEndTime",&off::Rules::m_flRoshanRespawnPhaseEndTime,{ nullptr, nullptr, nullptr } },
    { "m_hGameModeEntity",            &off::Rules::m_hGameModeEntity,            { nullptr, nullptr, nullptr } },

    { "m_bFogOfWarDisabled", &off::GameMode::m_bFogOfWarDisabled, { nullptr, nullptr, nullptr } },
    { "m_bUseUnseenFOW",     &off::GameMode::m_bUseUnseenFOW,     { nullptr, nullptr, nullptr } },

    { "m_roshanSpawnInfo",    &off::Data::m_roshanSpawnInfo,    { nullptr, nullptr, nullptr } },
    { "m_nNextPowerRuneType", &off::Data::m_nNextPowerRuneType, { nullptr, nullptr, nullptr } },

    { "m_eRoshanPhase",          &off::RoshanPhase::m_eRoshanPhase,          { nullptr, nullptr, nullptr } },
    { "m_flRoshanPhaseStartTime",&off::RoshanPhase::m_flRoshanPhaseStartTime,{ nullptr, nullptr, nullptr } },
    { "m_flRoshanPhaseEndTime",  &off::RoshanPhase::m_flRoshanPhaseEndTime,  { nullptr, nullptr, nullptr } },

    { "m_flVisibilityStrength",      &off::PlayerVisibility::m_flVisibilityStrength,      { nullptr, nullptr, nullptr } },
    { "m_flFogDistanceMultiplier",   &off::PlayerVisibility::m_flFogDistanceMultiplier,   { nullptr, nullptr, nullptr } },
    { "m_flFogMaxDensityMultiplier", &off::PlayerVisibility::m_flFogMaxDensityMultiplier, { nullptr, nullptr, nullptr } },
    { "m_bStartDisabled",            &off::PlayerVisibility::m_bStartDisabled,            { "m_bIsEnabled", "m_flVisibilityStrength", "m_flFogDistanceMultiplier" } },
    { "m_bIsEnabled",                &off::PlayerVisibility::m_bIsEnabled,                { nullptr, nullptr, nullptr } },
};

const int g_fieldCount = (int)(sizeof(g_fields) / sizeof(g_fields[0]));

namespace {

struct Range {
    uintptr_t lo = 0;
    uintptr_t hi = 0;
};

struct Hit {
    uintptr_t addr = 0;
    const Range* r = nullptr;
};

struct Cand {
    uint32_t s = 0;
    uint32_t o = 0;
};

std::vector<Range> g_exec;
std::vector<Range> g_nonexec;
std::vector<Range> g_write;
uintptr_t g_base = 0;
uint32_t g_size = 0;
bool s_static = false;
bool s_esys = false;
bool s_vm = false;
DWORD s_nextLive = 0;

void FillRanges() {
    mem::ModSec secs[64];
    int ns = mem::ModSections(g_base, secs, 64);
    for (int i = 0; i < ns; ++i) {
        if (secs[i].va >= g_size) continue;
        uint32_t vs = secs[i].vsize;
        if (vs > g_size - secs[i].va) vs = g_size - secs[i].va;
        if (!vs) continue;
        Range r;
        r.lo = g_base + secs[i].va;
        r.hi = r.lo + vs;
        if (secs[i].flags & 0x20000000u) {
            g_exec.push_back(r);
        } else {
            g_nonexec.push_back(r);
            if (secs[i].flags & 0x80000000u) g_write.push_back(r);
        }
    }
}

int HexV(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

size_t ParsePat(const char* s, uint8_t* b, char* m) {
    size_t n = 0;
    while (*s && n < 32) {
        if (*s == ' ') { ++s; continue; }
        if (s[0] == '?' && s[1] == '?') {
            b[n] = 0;
            m[n] = '?';
            s += 2;
        } else {
            int h = HexV(s[0]);
            int l = HexV(s[1]);
            if (h < 0 || l < 0) break;
            b[n] = (uint8_t)(h * 16 + l);
            m[n] = 'x';
            s += 2;
        }
        ++n;
    }
    return n;
}

bool MatchAt(const uint8_t* p, const uint8_t* b, const char* m, size_t n) {
    for (size_t i = 0; i < n; ++i)
        if (m[i] == 'x' && p[i] != b[i]) return false;
    return true;
}

uintptr_t FindOne(const std::vector<Range>& rs, const uint8_t* b, const char* m, size_t n) {
    if (!n) return 0;
    for (const Range& r : rs) {
        if (r.hi <= r.lo || r.hi - r.lo < n) continue;
        for (uintptr_t a = r.lo; a + n <= r.hi; ++a)
            if (MatchAt((const uint8_t*)a, b, m, n)) return a;
    }
    return 0;
}

void FindAll(const std::vector<Range>& rs, const uint8_t* b, const char* m, size_t n,
             std::vector<Hit>& out, size_t cap) {
    if (!n) return;
    for (const Range& r : rs) {
        if (r.hi <= r.lo || r.hi - r.lo < n) continue;
        for (uintptr_t a = r.lo; a + n <= r.hi; ++a) {
            if (!MatchAt((const uint8_t*)a, b, m, n)) continue;
            Hit h;
            h.addr = a;
            h.r = &r;
            out.push_back(h);
            if (out.size() >= cap) return;
        }
    }
}

uintptr_t ModeVote(const std::unordered_map<uintptr_t, int>& v) {
    uintptr_t best = 0;
    int bn = -1;
    for (const auto& kv : v)
        if (kv.second > bn) { bn = kv.second; best = kv.first; }
    return best;
}

void ResolveAA() {
    uint8_t pb[32];
    char pm[32];
    size_t n;

    n = ParsePat("48 89 5C 24 08 44 8B 59 14 33 D2 4C 8B C9", pb, pm);
    uintptr_t gm = FindOne(g_exec, pb, pm, n);

    n = ParsePat("48 89 5C 24 08 55 56 57 48 8D 6C 24 B9 48 81 EC D0 00 00 00", pb, pm);
    uintptr_t sr = FindOne(g_exec, pb, pm, n);

    n = ParsePat("48 8D 88 08 07 00 00 E8", pb, pm);
    std::vector<Hit> hits;
    FindAll(g_exec, pb, pm, n, hits, 4096);

    if (hits.empty()) { off::getMatchRva = 0; return; }

    std::unordered_map<uintptr_t, int> vp;
    std::unordered_map<uintptr_t, int> vm;
    for (const Hit& h : hits) {
        uintptr_t a = h.addr;
        if (a + 12 <= h.r->hi) {
            int32_t d;
            memcpy(&d, (const void*)(a + 8), 4);
            uintptr_t t = a + 12 + (intptr_t)d;
            if (t >= h.r->lo && t + 8 <= h.r->hi) vp[t]++;
        }
        if (a >= h.r->lo + 5 && *(const uint8_t*)(a - 5) == 0xE8) {
            int32_t d;
            memcpy(&d, (const void*)(a - 4), 4);
            uintptr_t t = a + (intptr_t)d;
            if (t >= h.r->lo && t + 8 <= h.r->hi) vm[t]++;
        }
    }

    if (!gm) gm = ModeVote(vp);
    uintptr_t gg = ModeVote(vm);
    int gv = 0;
    auto it = vm.find(gg);
    if (it != vm.end()) gv = it->second;

    if (!gm || !gg || gv < 5 || gv * 2 < (int)hits.size()) { off::getMatchRva = 0; return; }

    uint8_t th[8];
    memcpy(th, (const void*)gg, 8);
    if (th[0] != 0x48 || th[1] != 0x8D || (th[2] & 0xC7) != 0x05 || th[7] != 0xC3) {
        off::getMatchRva = 0;
        return;
    }
    int32_t disp;
    memcpy(&disp, th + 3, 4);
    uintptr_t singleton = gg + 7 + (intptr_t)disp;
    if (singleton < g_base || (singleton - g_base) + 0x710 > g_size) { off::getMatchRva = 0; return; }

    off::getMatchRva = gm - g_base;
    off::getGlobalRva = gg - g_base;
    off::containerRva = singleton + 0x708 - g_base;
    off::sendReadyRva = sr ? sr - g_base : 0;
}

void ResolveVtables() {
    uint32_t es = rtti::VtableByName("CGameEntitySystem");
    if (es) off::esysVtableRva = es;
    uint32_t cv = rtti::VtableByName("C_DOTAPlayerController");
    if (cv) off::Ctrl::vtableRva = cv;
}

void ResolveSchema() {
    const int nf = g_fieldCount;
    size_t flen[128];
    for (int i = 0; i < nf; ++i) flen[i] = strlen(g_fields[i].name);

    std::vector<std::vector<Cand>> c(nf);

    for (const Range& r : g_nonexec) {
        for (uintptr_t p = r.lo; p + 0x18 <= r.hi; p += 8) {
            uintptr_t v = *(const uintptr_t*)p;
            if (v < g_base) continue;
            uint32_t srva = (uint32_t)(v - g_base);
            if ((uintptr_t)srva != v - g_base || srva + 2 > g_size) continue;
            const char* s = (const char*)v;
            if (s[0] != 'm' || s[1] != '_') continue;
            size_t avail = g_size - srva;
            size_t sl = 0;
            while (sl < avail && sl < 64 && s[sl]) ++sl;
            if (sl >= 64 || sl >= avail) continue;
            int fi = -1;
            for (int i = 0; i < nf; ++i) {
                if (flen[i] == sl && memcmp(s, g_fields[i].name, sl) == 0) { fi = i; break; }
            }
            if (fi < 0) continue;
            uint32_t fo;
            memcpy(&fo, (const void*)(p + 0x10), 4);
            if (fo == 0 || fo >= 0x8000) continue;
            Cand cd;
            cd.s = (uint32_t)(p - g_base);
            cd.o = fo;
            c[fi].push_back(cd);
        }
    }

    uint32_t res[128];
    bool ok[128];
    memset(res, 0, sizeof(res));
    memset(ok, 0, sizeof(ok));

    for (int i = 0; i < nf; ++i) {
        if (c[i].empty()) continue;
        uint32_t sv[64];
        int sn[64];
        int ns = 0;
        for (const Cand& cd : c[i]) {
            int k = -1;
            for (int j = 0; j < ns; ++j)
                if (sv[j] == cd.o) { k = j; break; }
            if (k < 0) {
                if (ns >= 64) break;
                sv[ns] = cd.o;
                sn[ns] = 1;
                ++ns;
            } else {
                sn[k]++;
            }
        }
        if (ns == 1) { ok[i] = true; res[i] = sv[0]; continue; }
        if (ns <= 1) continue;
        if (g_fields[i].anchors[0]) continue;
        int bi = 0;
        for (int j = 1; j < ns; ++j) if (sn[j] > sn[bi]) bi = j;
        bool dom = true;
        for (int j = 0; j < ns; ++j)
            if (j != bi && sn[j] >= sn[bi]) { dom = false; break; }
        if (dom) { ok[i] = true; res[i] = sv[bi]; }
    }

    for (int i = 0; i < nf; ++i) {
        if (ok[i] || c[i].empty()) continue;
        if (!g_fields[i].anchors[0]) continue;
        for (int ai = 0; ai < 3; ++ai) {
            const char* an = g_fields[i].anchors[ai];
            if (!an) break;
            int aj = -1;
            for (int j = 0; j < nf; ++j)
                if (strcmp(g_fields[j].name, an) == 0) { aj = j; break; }
            if (aj < 0 || !ok[aj]) continue;
            uint32_t as[32];
            int na = 0;
            for (const Cand& cd : c[aj])
                if (cd.o == res[aj] && na < 32) as[na++] = cd.s;
            if (!na) continue;
            int bi = -1;
            uint64_t bd = ~(uint64_t)0;
            for (size_t x = 0; x < c[i].size(); ++x) {
                for (int k = 0; k < na; ++k) {
                    int64_t d = (int64_t)c[i][x].s - (int64_t)as[k];
                    uint64_t ad = d < 0 ? (uint64_t)(-d) : (uint64_t)d;
                    if ((ad & 0x1F) != 0) continue;
                    if (ad > 0x2000) continue;
                    if (ad < bd) { bd = ad; bi = (int)x; }
                }
            }
            if (bi >= 0) { ok[i] = true; res[i] = c[i][bi].o; break; }
        }
    }

    for (int i = 0; i < nf; ++i)
        if (ok[i]) *g_fields[i].var = res[i];
}

void EsGlobalsImpl(uintptr_t base, std::vector<uintptr_t>& out) {
    out.clear();
    if (!base) return;
    uint32_t size = mem::ModuleSize(base);
    if (!size) return;
    mem::ModSec secs[64];
    int ns = mem::ModSections(base, secs, 64);
    for (int i = 0; i < ns; ++i) {
        if (!(secs[i].flags & 0x80000000u) || (secs[i].flags & 0x20000000u)) continue;
        if (secs[i].va >= size) continue;
        uint32_t vs = secs[i].vsize;
        if (vs > size - secs[i].va) vs = size - secs[i].va;
        uintptr_t lo = base + secs[i].va;
        uintptr_t hi = lo + vs;
        for (uintptr_t p = lo; p + 8 <= hi; p += 8) {
            uintptr_t v = *(const uintptr_t*)p;
            if (!mem::ValidPtr(v)) continue;
            if (v >= base && v < base + size) continue;
            out.push_back(p - base);
        }
    }
}

bool TryEs() {
    std::vector<uintptr_t> cand;
    EsGlobalsImpl(g_base, cand);
    uintptr_t found[8];
    int nf = 0;
    for (uintptr_t rva : cand) {
        uintptr_t v = 0;
        if (!mem::Read(g_base + rva, v)) continue;
        if (!mem::ValidPtr(v)) continue;
        uintptr_t vp = 0;
        if (!mem::Read(v, vp)) continue;
        if (vp != g_base + off::esysVtableRva) continue;
        found[nf++] = rva;
        if (nf >= 8) break;
    }
    if (!nf) return false;
    for (int i = 0; i < 8; ++i) off::esysRva[i] = (i < nf) ? found[i] : 0;
    return true;
}

bool ShapeOk(uintptr_t a) {
    float v;
    if (!mem::Read(a, v) || std::fabs(v) < 0.3f || std::fabs(v) > 5.f) return false;
    if (!mem::Read(a + 4, v) || std::fabs(v) > 1e-4f) return false;
    if (!mem::Read(a + 8, v) || std::fabs(v) > 1e-4f) return false;
    if (!mem::Read(a + 16, v) || std::fabs(v) > 1e-4f) return false;
    if (!mem::Read(a + 48, v) || std::fabs(v) > 1e-4f) return false;
    if (!mem::Read(a + 20, v) || std::fabs(v) < 0.3f || std::fabs(v) > 5.f) return false;
    if (!mem::Read(a + 40, v) || !(v > -5.f && v < -0.01f)) return false;
    float m[16];
    if (!mem::Read(a, m)) return false;
    for (int i = 0; i < 16; ++i)
        if (!std::isfinite(m[i])) return false;
    if (std::fabs(m[9] - m[13]) > 0.01f) return false;
    if (std::fabs(m[10] - m[14]) > 0.01f) return false;
    if (std::fabs(m[9]) < 0.01f) return false;
    if (std::fabs(m[15]) < 10.f) return false;
    return true;
}

uintptr_t PickByRefs(const std::vector<uintptr_t>& cands) {
    std::vector<int> counts(cands.size(), 0);
    for (const Range& r : g_exec) {
        for (uintptr_t a = r.lo; a + 8 <= r.hi; ++a) {
            const uint8_t* p = (const uint8_t*)a;
            int dispIdx = -1;
            uint8_t b0 = p[0];
            if (b0 >= 0x40 && b0 <= 0x4F) {
                uint8_t o1 = p[1];
                if ((o1 == 0x8B || o1 == 0x8D || o1 == 0x89) && (p[2] & 0xC7) == 0x05) {
                    dispIdx = 3;
                } else if (o1 == 0x0F && (p[3] & 0xC7) == 0x05 &&
                           (p[2] == 0x10 || p[2] == 0x11 || p[2] == 0x28 || p[2] == 0x29 ||
                            p[2] == 0xB6 || p[2] == 0xB7 || p[2] == 0xBE || p[2] == 0xBF)) {
                    dispIdx = 4;
                }
            } else if (b0 == 0x8B || b0 == 0x8D || b0 == 0x89) {
                if ((p[1] & 0xC7) == 0x05) dispIdx = 2;
            } else if (b0 == 0x0F) {
                if ((p[2] & 0xC7) == 0x05 &&
                    (p[1] == 0x10 || p[1] == 0x11 || p[1] == 0x28 || p[1] == 0x29 ||
                     p[1] == 0xB6 || p[1] == 0xB7 || p[1] == 0xBE || p[1] == 0xBF)) {
                    dispIdx = 3;
                }
            } else if (b0 == 0xF2 || b0 == 0xF3 || b0 == 0x66) {
                if (p[1] == 0x0F && (p[2] == 0x10 || p[2] == 0x11) && (p[3] & 0xC7) == 0x05)
                    dispIdx = 4;
            }
            if (dispIdx < 0) continue;
            int32_t d;
            memcpy(&d, p + dispIdx, 4);
            uintptr_t tgt = a + (uintptr_t)dispIdx + 4 + (intptr_t)d;
            for (size_t ci = 0; ci < cands.size(); ++ci)
                if (tgt == g_base + cands[ci]) counts[ci]++;
        }
    }
    size_t bi = 0;
    for (size_t i = 1; i < counts.size(); ++i)
        if (counts[i] > counts[bi]) bi = i;
    return cands[bi];
}

bool TryViewMatrix() {
    std::vector<uintptr_t> cands;
    for (const Range& r : g_write) {
        for (uintptr_t a = r.lo; a + 64 <= r.hi; a += 4) {
            if (!ShapeOk(a)) continue;
            cands.push_back(a - g_base);
            if (cands.size() >= 16) break;
        }
        if (cands.size() >= 16) break;
    }
    if (cands.empty()) return false;
    uintptr_t best = cands[0];
    if (cands.size() > 1) best = PickByRefs(cands);
    off::dwViewMatrix = best;
    return true;
}

void StaticPass() {
    FillRanges();
    rtti::Build(g_base);
    ResolveVtables();
    ResolveAA();
    ResolveSchema();
}

}

void EsGlobals(uintptr_t base, std::vector<uintptr_t>& out) {
    EsGlobalsImpl(base, out);
}

void Resolve(uintptr_t base) {
    if (!base) return;
    uint32_t size = mem::ModuleSize(base);
    if (!size) return;

    if (!s_static) {
        g_base = base;
        g_size = size;
        StaticPass();
        s_static = true;
    }
    if (s_esys && s_vm) return;
    if (s_nextLive && GetTickCount() < s_nextLive) return;
    s_nextLive = GetTickCount() + 3000;
    if (!s_esys) s_esys = TryEs();
    if (!s_vm) s_vm = TryViewMatrix();
}

void Tick() {
    Resolve(mem::ModuleBase("client.dll"));
}

}
