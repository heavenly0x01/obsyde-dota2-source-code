#pragma once
#include "common.h"
#include "mem.h"
#include "offsets.h"
#include <mutex>
#include <vector>

enum class UnitKind : uint8_t {
    Unknown = 0,
    Hero,
    Creep,
    Ward,
    Roshan,
    Building,
    Courier,
    Boss,
};

inline const char* KindTag(UnitKind k) {
    switch (k) {
    case UnitKind::Hero:     return "HERO";
    case UnitKind::Creep:    return "CREEP";
    case UnitKind::Ward:     return "WARD";
    case UnitKind::Roshan:   return "ROSHAN";
    case UnitKind::Building: return "BUILDING";
    case UnitKind::Courier:  return "COURIER";
    case UnitKind::Boss:     return "BOSS";
    default:                 return "";
    }
}

struct StaticUnit {
    uintptr_t    addr = 0;
    UnitKind     kind = UnitKind::Unknown;
    const char*  cls  = nullptr;
    char         name[64] = {};
    char         nick[32] = {};
};

struct AbilityInfo {
    int         level   = 0;
    int         mana    = 0;
    float       cd      = 0.f;
    float       cdLen   = 0.f;
    bool        phase   = false;
    const char* cls     = nullptr;
};

struct FrameUnit {
    uintptr_t   addr = 0;
    UnitKind    kind = UnitKind::Unknown;
    const char* cls  = nullptr;
    char        name[64] = {};
    char        nick[32] = {};

    int   team = 0;
    int   hp = 0, maxHp = 0;
    float mana = 0.f, maxMana = 0.f;
    Vec3  pos{};
    float hbOffset = 200.f;

    bool  alive = false;
    bool  illusion = false;
    float invis = 0.f;
    int   level = 0;
    float armor = 0.f;
    float dmgAvg = 0.f;
    int   atkRange = 0;

    float dist = 0.f;
    int   respawn = -1;

    AbilityInfo abil[16];
    int         abilN = 0;

    bool  mdirOk = false;
    Vec3  mdir{};

    bool  canLastHit = false;
    bool  canDeny = false;
    int   hits = 0;
};

struct Frame {
    bool  ok = false;
    float now = 0.f;

    int   localTeam = 2;
    Vec3  localPos{};
    int   hp = 0, maxHp = 0, mana = 0, maxMana = 0, level = 0;
    float dmgAvg = 0.f;
    int   atkRange = 0;
    bool  localAlive = false;

    uintptr_t rules = 0;
    float     gameStart = -1.f;
    int       gameState = -1;
    int       roshanPhase = -1;
    float     roshanEnd = -1.f;

    bool  roshanAlive = false;
    int   roshanHp = 0, roshanMaxHp = 0;
    Vec3  roshanPos{};
    bool  roshanPosValid = false;
    float roshanEtaLo = -1.f;
    float roshanEtaHi = -1.f;

    bool  lastHitDeny = true;

    std::vector<FrameUnit> units;
};

namespace game {

struct Sys {
    uintptr_t clientBase  = 0;
    uint32_t  imageSize   = 0;
    uintptr_t esys        = 0;
    uintptr_t idPages[off::maxIdPages] = {};
    uint32_t  handleMask  = off::handleMask;
    uintptr_t localCtrl   = 0;
    bool      rttiOk      = false;
    bool      ready       = false;
};

extern Sys g_sys;

extern uintptr_t g_rules;
extern uintptr_t g_rulesProxy;
extern uintptr_t g_roshanSpawner;
extern uintptr_t g_dataEnt;
extern uintptr_t g_visEnt;
extern uintptr_t g_modeEnt;

bool  TryInit();
void  ScanLoop();
void  BuildFrame(Frame& out, uintptr_t localCtrl);

uintptr_t EntityByIndex(int index);
uintptr_t EntityByHandle(uint32_t handle);
void      RefreshModeEntity();

extern Vec3  g_roshanLastPos;
extern bool  g_roshanSeen;

}
