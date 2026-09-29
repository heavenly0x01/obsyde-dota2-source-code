#include "game.h"
#include "offsets.h"
#include <unordered_map>
#include <cstring>

namespace game {

Sys g_sys;

uintptr_t g_rules         = 0;
uintptr_t g_rulesProxy    = 0;
uintptr_t g_roshanSpawner = 0;
uintptr_t g_dataEnt       = 0;
uintptr_t g_visEnt        = 0;
uintptr_t g_modeEnt       = 0;

Vec3  g_roshanLastPos{};
bool  g_roshanSeen = false;

static std::mutex          g_lock;
static std::vector<StaticUnit> g_units;


static uintptr_t IdentityAddr(int index) {
    if (index < 0 || index >= off::maxIdPages * off::entsPerPage) return 0;
    uintptr_t page = g_sys.idPages[index >> 9];
    if (!page) return 0;
    return page + (uintptr_t)(index & (off::entsPerPage - 1)) * (uintptr_t)off::idStride;
}

static bool RefreshPages() {
    if (!g_sys.esys) return false;
    bool any = false;
    for (int k = 0; k < off::maxIdPages; ++k) {
        uintptr_t p = 0;
        mem::Read(g_sys.esys + off::idPagesOff + 8ULL * k, p);
        g_sys.idPages[k] = mem::ValidPtr(p) ? p : 0;
        if (g_sys.idPages[k]) any = true;
    }
    return any;
}

static uintptr_t ResolveIndexEx(int index, uint32_t exactHandle) {
    uintptr_t id = IdentityAddr(index);
    if (!id) return 0;
    uint32_t fld = 0;
    if (!mem::Read(id + off::idHandleFld, fld)) return 0;
    if ((fld & g_sys.handleMask) != (uint32_t)index) return 0;
    if (exactHandle && fld != exactHandle) return 0;
    uintptr_t e = 0;
    if (!mem::Read(id, e) || !mem::ValidPtr(e)) return 0;
    uintptr_t back = 0;
    if (!mem::Read(e + off::instEntity, back) || back != id) return 0;
    return e;
}

static uintptr_t ResolveIndex(int index) {
    return ResolveIndexEx(index, 0);
}

uintptr_t EntityByIndex(int index) {
    if (!g_sys.ready) return 0;
    return ResolveIndex(index);
}

uintptr_t EntityByHandle(uint32_t handle) {
    if (!g_sys.ready || handle == 0 || handle == 0xFFFFFFFFu) return 0;
    int idx = (int)(handle & g_sys.handleMask);
    uintptr_t e = ResolveIndexEx(idx, handle);
    if (!e) e = ResolveIndexEx(idx, 0);
    return e;
}

void RefreshModeEntity() {
    if (!g_sys.ready) return;

    if (g_modeEnt) {
        uint8_t b = 0xFF;
        if (mem::Read(g_modeEnt + off::GameMode::m_bFogOfWarDisabled, b) && b <= 1) return;
        g_modeEnt = 0;
    }
    if (!g_rules) return;

    uint32_t h = 0;
    if (!mem::Read(g_rules + off::Rules::m_hGameModeEntity, h)) return;
    uintptr_t e = EntityByHandle(h);
    if (!e) return;
    uintptr_t vptr = 0;
    if (!mem::Read(e, vptr) || vptr < g_sys.clientBase ||
        vptr >= g_sys.clientBase + g_sys.imageSize) return;

    uint64_t lo = 0;
    uint32_t hi = 0;
    if (!mem::Read(e + 0x600, lo) || !mem::Read(e + 0x608, hi)) return;
    const unsigned char* b = (const unsigned char*)&lo;
    for (int i = 0; i < 8; ++i)
        if (b[i] > 1) return;
    const unsigned char* c = (const unsigned char*)&hi;
    for (int i = 0; i < 3; ++i)
        if (c[i] > 1) return;

    g_modeEnt = e;
}

static bool LooksLikeUnitName(const char* s) {
    if (!s || !*s) return false;
    if (!StartsWith(s, "npc_")) return false;
    for (const char* c = s; *c; ++c) {
        char ch = *c;
        bool ok = (ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9') || ch == '_' || ch == '-';
        if (!ok) return false;
    }
    return true;
}

static bool ReadUnitName(uintptr_t e, char* out, int cap) {
    uintptr_t p = 0;
    if (!mem::Read(e + off::NPC::m_iszUnitName, p)) return false;
    if (p < 0x100000000ULL || p > 0x00007FFFFFFFFFFFULL) return false;
    char buf[64] = {};
    if (!mem::ReadStr(p, buf, sizeof(buf))) return false;
    if (!LooksLikeUnitName(buf)) return false;
    strncpy(out, buf, cap - 1);
    out[cap - 1] = 0;
    return true;
}

static void MakeNick(const char* cls, const char* name, char* out, int cap) {
    out[0] = 0;
    static const char* kHeroCls = "C_DOTA_Unit_Hero_";
    if (cls && StartsWith(cls, kHeroCls)) {
        strncpy(out, cls + strlen(kHeroCls), cap - 1);
        out[cap - 1] = 0;
        return;
    }
    static const char* kHeroName = "npc_dota_hero_";
    if (StartsWith(name, kHeroName)) {
        strncpy(out, name + strlen(kHeroName), cap - 1);
        out[cap - 1] = 0;
        return;
    }
}

static bool RulesPlausible(uintptr_t p) {
    if (!mem::ValidPtr(p)) return false;
    int state = mem::ReadOr<int>(p + off::Rules::m_nGameState, -1);
    if (state < 0 || state > 64) return false;
    float t = mem::ReadOr<float>(p + off::Rules::m_flGameStartTime, -1.f);
    if (!(t >= 0.f && t < 300000.f)) return false;
    int gold = mem::ReadOr<int>(p + off::Rules::m_nStartingGold, -1);
    if (gold < 0 || gold > 10000) return false;
    int mode = mem::ReadOr<int>(p + off::Rules::m_iGameMode, -1);
    if (mode < 0 || mode > 1000) return false;
    uint32_t phase = mem::ReadOr<uint32_t>(p + off::Rules::m_nRoshanRespawnPhase, 99);
    return phase <= 2;
}

struct CacheEntry {
    uintptr_t vptr = 0;
    bool      keep = false;
    UnitKind  kind = UnitKind::Unknown;
    const char* cls = nullptr;
    char      name[64] = {};
    char      nick[32] = {};
};

static std::unordered_map<uintptr_t, CacheEntry> g_cache;

static bool Classify(uintptr_t e, uintptr_t vptr, StaticUnit& u) {
    const char* cls = rtti::ClassOf(g_sys.clientBase, vptr);

    if (cls) {
        if (Streq(cls, "C_DOTAGamerulesProxy")) {
            uintptr_t p = 0;
            mem::Read(e + off::RulesProxy::m_pGameRules, p);
            if (RulesPlausible(p)) { g_rules = p; g_rulesProxy = e; }
            return false;
        }
        if (Streq(cls, "C_DOTA_RoshanSpawner")) { g_roshanSpawner = e; return false; }
        if (Streq(cls, "C_DOTA_DataNonSpectator")) { g_dataEnt = e; return false; }
        if (Contains(cls, "PlayerVisibility")) { g_visEnt = e; return false; }
    }

    char name[64] = {};
    bool hasName = ReadUnitName(e, name, sizeof(name));

    UnitKind kind = UnitKind::Unknown;

    if (cls && StartsWith(cls, "C_DOTA_Unit_Hero_")) kind = UnitKind::Hero;
    else if (hasName && StartsWith(name, "npc_dota_hero_")) kind = UnitKind::Hero;

    if (kind == UnitKind::Unknown && hasName) {
        if (Streq(name, "npc_dota_roshan")) kind = UnitKind::Roshan;
        else if (Contains(name, "observer_ward") || Contains(name, "sentry_ward") ||
                 StartsWith(name, "npc_dota_ward")) kind = UnitKind::Ward;
        else if (StartsWith(name, "npc_dota_creep") || StartsWith(name, "npc_dota_neutral") ||
                 StartsWith(name, "npc_dota_siege")) kind = UnitKind::Creep;
        else if (Contains(name, "_tower") || Contains(name, "_fort") ||
                 Contains(name, "_fountain") || Contains(name, "_healer")) kind = UnitKind::Building;
        else if (Contains(name, "courier")) kind = UnitKind::Courier;
        else if (Contains(name, "tormentor") || Contains(name, "miniboss")) kind = UnitKind::Boss;
    }

    if (kind == UnitKind::Unknown) {
        if (cls && (Contains(cls, "_Creep") || StartsWith(cls, "C_DOTA_Unit_Creep"))) kind = UnitKind::Creep;
        else if (cls && Contains(cls, "Building") && StartsWith(cls, "C_DOTA_")) kind = UnitKind::Building;
    }

    if (kind == UnitKind::Unknown) return false;

    u.addr = e;
    u.kind = kind;
    u.cls  = cls;
    strncpy(u.name, name, sizeof(u.name) - 1);
    MakeNick(cls, name, u.nick, sizeof(u.nick));
    return true;
}

static bool DiscoverEntitySystem() {
    uintptr_t base = g_sys.clientBase;
    g_sys.esys = 0;

    for (size_t i = 0; i < sizeof(off::esysRva) / sizeof(off::esysRva[0]); ++i) {
        uintptr_t v = 0;
        if (!mem::Read(base + off::esysRva[i], v) || !mem::ValidPtr(v)) continue;
        uintptr_t vp = 0;
        if (!mem::Read(v, vp) || vp != base + off::esysVtableRva) continue;
        g_sys.esys = v;
        break;
    }
    if (!g_sys.esys)
        return false;

    if (!RefreshPages()) {
        g_sys.esys = 0;
        return false;
    }

    int hits = 0;
    for (int i = 1; i <= off::entsPerPage && hits < 8; ++i)
        if (ResolveIndex(i)) ++hits;
    if (hits < 2) {
        g_sys.esys = 0;
        return false;
    }

    return true;
}

static bool DetectMask(uintptr_t ctrl) {
    uint32_t h = 0;
    if (!mem::Read(ctrl + off::Ctrl::m_hAssignedHero, h)) return false;
    if (h == 0 || h == 0xFFFFFFFFu) return false;

    static const uint32_t kMasks[] = { 0x3FFF, 0x7FFF, 0x1FFF, 0xFFFF, 0xFFF };
    for (uint32_t m : kMasks) {
        int idx = (int)(h & m);
        uintptr_t id = IdentityAddr(idx);
        if (!id) continue;
        uint32_t fld = 0;
        if (!mem::Read(id + off::idHandleFld, fld) || fld != h) continue;
        uintptr_t e = 0;
        if (!mem::Read(id, e) || !mem::ValidPtr(e)) continue;
        uintptr_t back = 0;
        if (!mem::Read(e + off::instEntity, back) || back != id) continue;

        char nm[64] = {};
        if (ReadUnitName(e, nm, sizeof(nm)) && StartsWith(nm, "npc_dota_hero_")) {
            g_sys.handleMask = m;
            return true;
        }
    }
    return false;
}

static DWORD g_nextProbeAt = 0;

static bool TryInitInner() {
    if (g_sys.ready) return true;

    uintptr_t base = mem::ModuleBase("client.dll");
    if (!base) return false;
    if (!g_sys.clientBase) {
        g_sys.clientBase = base;
        g_sys.imageSize = mem::ModuleSize(base);
    }

    if (GetTickCount() < g_nextProbeAt) return false;

    if (!DiscoverEntitySystem()) {
        g_nextProbeAt = GetTickCount() + 3000;
        return false;
    }

    g_sys.ready = true;
    return true;
}

bool TryInit() {
    if (g_sys.ready) return true;

    static std::atomic<bool> s_busy{ false };
    if (s_busy.exchange(true)) return false;
    bool ok = TryInitInner();
    s_busy.store(false);
    return ok;
}

void ScanLoop() {
    while (cfg::running.load()) {
        if (!g_sys.ready) { Sleep(250); continue; }

        RefreshPages();
        RefreshModeEntity();

        std::vector<StaticUnit> next;
        next.reserve(384);

        uintptr_t sample[128];
        int sampleN = 0;
        int found = 0;
        uintptr_t localCtrl = 0;

        const int kMaxIndex = off::maxIdPages * off::entsPerPage;
        for (int i = 1; i <= kMaxIndex; ++i) {
            uintptr_t e = ResolveIndex(i);
            if (!mem::ValidPtr(e)) continue;

            uintptr_t vptr = 0;
            if (!mem::Read(e, vptr)) continue;
            if (vptr < g_sys.clientBase || vptr >= g_sys.clientBase + g_sys.imageSize) continue;

            ++found;
            if (sampleN < 128) sample[sampleN++] = vptr;

            if (vptr == g_sys.clientBase + off::Ctrl::vtableRva) {
                uint8_t loc = 0;
                mem::Read(e + off::Ctrl::m_bIsLocalPlayerController, loc);
                if (loc) localCtrl = e;
            }

            auto it = g_cache.find(e);
            if (it != g_cache.end() && it->second.vptr == vptr) {
                if (it->second.keep) {
                    StaticUnit u;
                    u.addr = e;
                    u.kind = it->second.kind;
                    u.cls = it->second.cls;
                    memcpy(u.name, it->second.name, sizeof(u.name));
                    memcpy(u.nick, it->second.nick, sizeof(u.nick));
                    next.push_back(u);
                }
                continue;
            }

            StaticUnit u;
            bool keep = Classify(e, vptr, u);

            if (g_cache.size() > 24576) g_cache.clear();
            CacheEntry ce;
            ce.vptr = vptr;
            ce.keep = keep;
            ce.kind = u.kind;
            ce.cls = u.cls;
            memcpy(ce.name, u.name, sizeof(ce.name));
            memcpy(ce.nick, u.nick, sizeof(ce.nick));
            g_cache[e] = ce;

            if (keep) next.push_back(u);
        }

        if (!rtti::Ready() && sampleN >= 8)
            rtti::DetectDelta(g_sys.clientBase, sample, sampleN);

        if (!g_rules) {
            for (int i = 1; i < 0x400 && !g_rules; ++i) {
                uintptr_t e = ResolveIndex(i);
                if (!mem::ValidPtr(e)) continue;
                uintptr_t p = 0;
                mem::Read(e + off::RulesProxy::m_pGameRules, p);
                if (RulesPlausible(p)) {
                    g_rules = p;
                    g_rulesProxy = e;
                }
            }
        }

        if (localCtrl != g_sys.localCtrl)
            g_sys.localCtrl = localCtrl;
        static bool s_maskDone = false;
        if (!s_maskDone && g_sys.localCtrl)
            s_maskDone = DetectMask(g_sys.localCtrl);

        {
            static int s_emptyPass = 0;
            if (found == 0) {
                if (++s_emptyPass >= 15 && g_sys.ready) {
                    s_emptyPass = 0;
                    g_sys.ready = false;
                    g_nextProbeAt = 0;
                }
            } else {
                s_emptyPass = 0;
            }
        }

        {
            std::lock_guard<std::mutex> lk(g_lock);
            g_units.swap(next);
        }

        Sleep(200);
    }
}

static void CopyUnits(std::vector<StaticUnit>& out) {
    std::lock_guard<std::mutex> lk(g_lock);
    out = g_units;
}

static int ReadAbilities(uintptr_t npc, AbilityInfo* out, int cap) {

    int cnt = 0;
    uintptr_t vec = 0;
    mem::Read(npc + off::NPC::m_vecAbilities, cnt);
    mem::Read(npc + off::NPC::m_vecAbilities + 8, vec);
    if (cnt <= 0 || cnt > 32) return 0;
    if (vec < 0x100000000ULL || vec > 0x00007FFFFFFFFFFFULL) return 0;

    int n = 0;
    for (int i = 0; i < cnt && n < cap; ++i) {
        uint32_t h = 0;
        if (!mem::Read(vec + 4ULL * i, h)) continue;
        uintptr_t a = EntityByHandle(h);
        if (!mem::ValidPtr(a)) continue;

        int lvl = mem::ReadOr<int>(a + off::Ability::m_iLevel);
        if (lvl <= 0) continue;
        if (mem::ReadOr<bool>(a + off::Ability::m_bHidden)) continue;

        AbilityInfo& ai = out[n++];
        ai.level = lvl;
        ai.mana = mem::ReadOr<int>(a + off::Ability::m_iManaCost);
        ai.cd = mem::ReadOr<float>(a + off::Ability::m_fCooldown);
        ai.cdLen = mem::ReadOr<float>(a + off::Ability::m_flCooldownLength);
        ai.phase = mem::ReadOr<bool>(a + off::Ability::m_bInAbilityPhase);
        uintptr_t avp = 0;
        if (mem::Read(a, avp) && avp) ai.cls = rtti::ClassOf(g_sys.clientBase, avp);
        if (ai.cd < 0.f || ai.cd > 600.f) ai.cd = 0.f;
        if (ai.cdLen < 0.f || ai.cdLen > 900.f) ai.cdLen = 0.f;
    }
    return n;
}

static void Reset(Frame& f) {
    f.ok = false;
    f.now = 0.f;
    f.localTeam = 2;
    f.localPos = Vec3{};
    f.hp = f.maxHp = f.mana = f.maxMana = f.level = 0;
    f.dmgAvg = 0.f;
    f.atkRange = 0;
    f.localAlive = false;
    f.rules = 0;
    f.gameStart = -1.f;
    f.gameState = -1;
    f.roshanPhase = -1;
    f.roshanEnd = -1.f;
    f.roshanAlive = false;
    f.roshanHp = f.roshanMaxHp = 0;
    f.roshanPos = Vec3{};
    f.roshanPosValid = false;
    f.roshanEtaLo = f.roshanEtaHi = -1.f;
    f.lastHitDeny = cfg::deny;
    f.units.clear();
}

static float ArmorMult(float a) {
    return 1.0f - (0.06f * a) / (1.0f + 0.06f * fabsf(a));
}

static bool ReadOrigin(uintptr_t e, Vec3& out) {
    uintptr_t node = 0;
    if (!mem::Read(e + off::BaseEntity::m_pGameSceneNode, node) || node < 0x10000ULL) return false;
    return mem::Read(node + off::SceneNode::m_vecAbsOrigin, out);
}

static bool FillUnit(const StaticUnit& u, FrameUnit& fu, const Frame& f) {
    fu = FrameUnit{};
    fu.addr = u.addr;
    fu.kind = u.kind;
    fu.cls = u.cls;
    memcpy(fu.name, u.name, sizeof(fu.name));
    memcpy(fu.nick, u.nick, sizeof(fu.nick));

    if (!ReadOrigin(u.addr, fu.pos)) return false;
    if (!mem::Read(u.addr + off::BaseEntity::m_iHealth, fu.hp)) return false;
    fu.maxHp = mem::ReadOr<int>(u.addr + off::BaseEntity::m_iMaxHealth);
    if (fu.maxHp <= 0) return false;

    uint8_t team = 0;
    mem::Read(u.addr + off::BaseEntity::m_iTeamNum, team);
    fu.team = team;

    uint8_t life = 1;
    mem::Read(u.addr + off::BaseEntity::m_lifeState, life);
    fu.alive = (life == 0 && fu.hp > 0);

    fu.illusion = mem::ReadOr<bool>(u.addr + off::NPC::m_bIsIllusion, false);
    fu.invis = mem::ReadOr<float>(u.addr + off::NPC::m_flInvisibilityLevel, 0.f);
    fu.level = mem::ReadOr<int>(u.addr + off::NPC::m_iCurrentLevel, 0);
    fu.mana = mem::ReadOr<float>(u.addr + off::NPC::m_flMana, 0.f);
    fu.maxMana = mem::ReadOr<float>(u.addr + off::NPC::m_flMaxMana, 0.f);

    int hbo = mem::ReadOr<int>(u.addr + off::NPC::m_iHealthBarOffset, 0);
    fu.hbOffset = (hbo > 0 && hbo < 1500) ? (float)hbo : 200.f;

    fu.dist = VecDist(fu.pos, f.localPos);

    if (fu.kind == UnitKind::Hero) {
        float v = mem::ReadOr<float>(u.addr + off::Hero::m_flRespawnTime, -1.f);
        if (!fu.alive) {
            float rem = v - f.now;
            if (rem < -1.f) {
                float dt = mem::ReadOr<float>(u.addr + off::NPC::m_flDeathTime, 0.f);
                rem = (dt + v) - f.now;
            }
            if (rem < 0.f) rem = 0.f;
            if (rem > 900.f) rem = 0.f;
            fu.respawn = (int)(rem + 0.5f);
        }
        fu.abilN = ReadAbilities(u.addr, fu.abil, 16);

        struct MoveHist { Vec3 pos{}; float sim = -1.f; Vec3 dir{}; bool ok = false; };
        static std::unordered_map<uintptr_t, MoveHist> s_move;
        float sim = mem::ReadOr<float>(u.addr + off::BaseEntity::m_flSimulationTime, -1.f);
        MoveHist& h = s_move[u.addr];
        if (sim > 0.f && sim != h.sim) {
            if (h.sim > 0.f) {
                float dx = fu.pos.x - h.pos.x, dy = fu.pos.y - h.pos.y;
                float d2 = dx * dx + dy * dy;
                if (d2 > 16.f && d2 < 360000.f) {
                    float d = sqrtf(d2);
                    h.dir = Vec3{ dx / d, dy / d, 0.f };
                    h.ok = true;
                } else {
                    h.ok = false;
                }
            }
            h.pos = fu.pos;
            h.sim = sim;
        }
        if (h.ok) {
            fu.mdirOk = true;
            fu.mdir = h.dir;
        }
    }

    bool target = (fu.kind == UnitKind::Creep || fu.kind == UnitKind::Building || fu.kind == UnitKind::Boss);
    if (target && f.localAlive) {
        float armor = mem::ReadOr<float>(u.addr + off::NPC::m_flPhysicalArmorValue, 0.f);
        fu.armor = armor;
        float dmg = f.dmgAvg * ArmorMult(armor);
        if (dmg <= 1.f) dmg = 1.f;
        bool inRange = fu.dist <= (float)f.atkRange + 75.f;
        if (inRange && fu.team != f.localTeam) {
            fu.canLastHit = fu.hp <= dmg;
            fu.hits = (int)ceilf(fu.hp / dmg);
        }
        if (inRange && fu.kind == UnitKind::Creep && fu.team == f.localTeam && f.lastHitDeny) {
            fu.canDeny = fu.hp < fu.maxHp * 0.5f;
        }
    }
    return true;
}

static bool  s_roshanAlivePrev = false;
static float s_roshanDiedAt = -1.f;

void BuildFrame(Frame& f, uintptr_t ctrl) {
    Reset(f);
    if (!g_sys.ready) return;
    if (!mem::ValidPtr(ctrl)) return;

    uintptr_t vp = 0;
    if (!mem::Read(ctrl, vp) || vp != g_sys.clientBase + off::Ctrl::vtableRva) return;

    RefreshPages();

    uint32_t heroH = 0;
    mem::Read(ctrl + off::Ctrl::m_hAssignedHero, heroH);
    uintptr_t hero = EntityByHandle(heroH);
    if (!mem::ValidPtr(hero)) return;

    f.now = mem::ReadOr<float>(hero + off::BaseEntity::m_flSimulationTime, 0.f);
    f.localTeam = mem::ReadOr<uint8_t>(hero + off::BaseEntity::m_iTeamNum, 2);
    f.hp = mem::ReadOr<int>(hero + off::BaseEntity::m_iHealth, 0);
    f.maxHp = mem::ReadOr<int>(hero + off::BaseEntity::m_iMaxHealth, 0);
    f.mana = (int)mem::ReadOr<float>(hero + off::NPC::m_flMana, 0.f);
    f.maxMana = (int)mem::ReadOr<float>(hero + off::NPC::m_flMaxMana, 0.f);
    f.level = mem::ReadOr<int>(hero + off::NPC::m_iCurrentLevel, 0);

    uint8_t life = 1;
    mem::Read(hero + off::BaseEntity::m_lifeState, life);
    f.localAlive = (life == 0 && f.hp > 0);

    int dmin = mem::ReadOr<int>(hero + off::NPC::m_iDamageMin, 0);
    int dmax = mem::ReadOr<int>(hero + off::NPC::m_iDamageMax, 0);
    int dbon = mem::ReadOr<int>(hero + off::NPC::m_iDamageBonus, 0);
    f.dmgAvg = ((float)dmin + (float)dmax) * 0.5f + (float)dbon;
    if (f.dmgAvg <= 0.f) f.dmgAvg = (float)(dmin + dbon);

    f.atkRange = mem::ReadOr<int>(hero + off::NPC::m_iAttackRange, 0);
    if (f.atkRange <= 0) f.atkRange = 300;

    if (!ReadOrigin(hero, f.localPos)) f.localPos = Vec3{};

    f.rules = g_rules;
    if (g_rules) {
        f.gameState = mem::ReadOr<int>(g_rules + off::Rules::m_nGameState, -1);
        f.gameStart = mem::ReadOr<float>(g_rules + off::Rules::m_flGameStartTime, -1.f);
        f.roshanPhase = (int)mem::ReadOr<uint32_t>(g_rules + off::Rules::m_nRoshanRespawnPhase, 99);
        f.roshanEnd = mem::ReadOr<float>(g_rules + off::Rules::m_flRoshanRespawnPhaseEndTime, -1.f);
    }
    if (g_dataEnt) {
        uintptr_t rp = g_dataEnt + off::Data::m_roshanSpawnInfo;
        uint32_t ph = 99;
        mem::Read(rp + off::RoshanPhase::m_eRoshanPhase, ph);
        if (ph <= 2) {
            f.roshanPhase = (int)ph;
            f.roshanEnd = mem::ReadOr<float>(rp + off::RoshanPhase::m_flRoshanPhaseEndTime, f.roshanEnd);
        }
    }

    static std::vector<StaticUnit> s_units;
    CopyUnits(s_units);

    f.units.reserve(s_units.size());
    for (size_t i = 0; i < s_units.size(); ++i) {
        FrameUnit fu;
        if (FillUnit(s_units[i], fu, f)) f.units.push_back(fu);
    }

    {
        int readyVotes = 0, fullVotes = 0;
        for (auto& u : f.units) {
            if (u.kind != UnitKind::Hero) continue;
            for (int i = 0; i < u.abilN; ++i) {
                float cd = u.abil[i].cd, len = u.abil[i].cdLen;
                if (len < 0.5f) continue;
                if (cd < 0.02f) ++readyVotes;
                if (fabsf(cd - len) < 1e-3f) ++fullVotes;
            }
        }
        if (fullVotes > readyVotes) {
            for (auto& u : f.units) {
                if (u.kind != UnitKind::Hero) continue;
                for (int i = 0; i < u.abilN; ++i) {
                    float& cd = u.abil[i].cd;
                    cd = u.abil[i].cdLen - cd;
                    if (cd < 0.f) cd = 0.f;
                }
            }
        }
    }

    bool seen = false;
    for (auto& u : f.units) {
        if (u.kind != UnitKind::Roshan) continue;
        seen = true;
        f.roshanAlive = u.alive;
        f.roshanHp = u.hp;
        f.roshanMaxHp = u.maxHp;
        f.roshanPos = u.pos;
        f.roshanPosValid = true;
        g_roshanLastPos = u.pos;
        g_roshanSeen = true;
    }

    if (seen) {
        if (!f.roshanAlive && s_roshanAlivePrev) s_roshanDiedAt = f.now;
        s_roshanAlivePrev = f.roshanAlive;
    } else {
        s_roshanAlivePrev = false;
        f.roshanPos = g_roshanLastPos;
        f.roshanPosValid = g_roshanSeen;
    }

    if (!f.roshanAlive) {
        bool exact = (f.roshanPhase > 0 && f.roshanEnd > 0.f && f.roshanEnd > f.now - 1.f);
        if (exact) {
            f.roshanEtaLo = f.roshanEtaHi = f.roshanEnd - f.now;
        } else if (s_roshanDiedAt > 0.f) {
            float elapsed = f.now - s_roshanDiedAt;
            f.roshanEtaLo = 480.f - elapsed;
            f.roshanEtaHi = 660.f - elapsed;
            if (f.roshanEtaHi < 0.f) { f.roshanEtaLo = 0.f; f.roshanEtaHi = 0.f; }
        }
    } else {
        s_roshanDiedAt = -1.f;
    }

    f.ok = true;
}

}
