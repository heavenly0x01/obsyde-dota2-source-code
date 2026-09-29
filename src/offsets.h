#pragma once

#include <cstdint>

namespace off {

inline constexpr uintptr_t dwViewMatrix            = 0x61C4F10;

inline constexpr uintptr_t esysRva[3]     = { 0x5E52CC0, 0x61A84E0, 0x653DB10 };
inline constexpr uintptr_t esysVtableRva  = 0x480C0F0;
inline constexpr uintptr_t idPagesOff     = 0x10;
inline constexpr int       idStride       = 0x70;
inline constexpr int       entsPerPage    = 512;
inline constexpr int       maxIdPages     = 8;
inline constexpr int       instEntity     = 0x10;
inline constexpr int       idHandleFld    = 0x10;
inline constexpr uint32_t  handleMask     = 0x3FFF;

namespace BaseEntity {
    inline constexpr uintptr_t m_pGameSceneNode = 0x330;
    inline constexpr uintptr_t m_iMaxHealth      = 0x348;
    inline constexpr uintptr_t m_iHealth         = 0x34C;
    inline constexpr uintptr_t m_lifeState       = 0x354;
    inline constexpr uintptr_t m_flSimulationTime= 0x3B8;
    inline constexpr uintptr_t m_iTeamNum        = 0x3E7;
    inline constexpr uintptr_t m_hOwnerEntity    = 0x514;
}

namespace SceneNode {
    inline constexpr uintptr_t m_pOwner      = 0x30;
    inline constexpr uintptr_t m_vecAbsOrigin= 0xD8;
}

namespace ModelEntity {
    inline constexpr uintptr_t m_iViewerID             = 0x77C;
    inline constexpr uintptr_t m_iTeamVisibilityBitmask= 0x780;
    inline constexpr uintptr_t m_bVisibilityDirtyFlag  = 0x789;
    inline constexpr uintptr_t m_Glow                  = 0x8F0;
}

namespace Glow {
    inline constexpr uintptr_t m_fGlowColor         = 0x08;
    inline constexpr uintptr_t m_iGlowType          = 0x30;
    inline constexpr uintptr_t m_iGlowTeam          = 0x34;
    inline constexpr uintptr_t m_nGlowRange         = 0x38;
    inline constexpr uintptr_t m_nGlowRangeMin      = 0x3C;
    inline constexpr uintptr_t m_glowColorOverride  = 0x40;
    inline constexpr uintptr_t m_bFlashing          = 0x44;
    inline constexpr uintptr_t m_bGlowing           = 0x50;
}

namespace NPC {
    inline constexpr uintptr_t m_iCurrentLevel      = 0xBAC;
    inline constexpr uintptr_t m_bIsAncient         = 0xBB0;
    inline constexpr uintptr_t m_bIsBossCreature    = 0xBB1;
    inline constexpr uintptr_t m_bConsideredHero    = 0xBB7;
    inline constexpr uintptr_t m_iAttackRange       = 0xBD8;
    inline constexpr uintptr_t m_iHealthBarOffset   = 0xBFC;
    inline constexpr uintptr_t m_flMana             = 0xC04;
    inline constexpr uintptr_t m_flMaxMana          = 0xC08;
    inline constexpr uintptr_t m_vecAbilities       = 0xC30;
    inline constexpr uintptr_t m_bIsIllusion        = 0xC2C;
    inline constexpr uintptr_t m_flInvisibilityLevel= 0xC64;
    inline constexpr uintptr_t m_iszUnitName        = 0xC78;
    inline constexpr uintptr_t m_iDamageMin         = 0xD20;
    inline constexpr uintptr_t m_iDamageMax         = 0xD24;
    inline constexpr uintptr_t m_iDamageBonus       = 0xD28;
    inline constexpr uintptr_t m_iMoveSpeed         = 0xBEC;
    inline constexpr uintptr_t m_nPlayerOwnerID     = 0x12C8;
    inline constexpr uintptr_t m_flLastAttackTime   = 0x12D0;
    inline constexpr uintptr_t m_flDeathTime        = 0x152C;
    inline constexpr uintptr_t m_flPhysicalArmorValue = 0x1534;
    inline constexpr uintptr_t m_bSuppressGlow      = 0x1314;
}

namespace Hero {
    inline constexpr uintptr_t m_flRespawnTime      = 0x19D8;
    inline constexpr uintptr_t m_iPlayerID          = 0x1A68;
    inline constexpr uintptr_t m_bLifeState         = 0x1A34;
}

namespace Ability {
    inline constexpr uintptr_t m_bHidden            = 0x617;
    inline constexpr uintptr_t m_iLevel             = 0x628;
    inline constexpr uintptr_t m_bInAbilityPhase    = 0x634;
    inline constexpr uintptr_t m_fCooldown          = 0x638;
    inline constexpr uintptr_t m_flCooldownLength   = 0x63C;
    inline constexpr uintptr_t m_iManaCost          = 0x640;
    inline constexpr uintptr_t m_flCastStartTime    = 0x650;
}

namespace Ctrl {
    inline constexpr uintptr_t vtableRva = 0x48A0CE8;
    inline constexpr uintptr_t m_bIsLocalPlayerController = 0x770;
    inline constexpr uintptr_t m_nPlayerID          = 0x908;
    inline constexpr uintptr_t m_hAssignedHero      = 0x90C;
    inline constexpr uintptr_t m_hActiveAbility     = 0x994;
}

namespace RoshanSpawner {
    inline constexpr uintptr_t m_iLastKillerTeam    = 0x5F0;
    inline constexpr uintptr_t m_iKillCount         = 0x5F4;
    inline constexpr uintptr_t m_vRoshanAltLocation = 0x5F8;
    inline constexpr uintptr_t m_hRoshan            = 0x604;
}

namespace RulesProxy {
    inline constexpr uintptr_t m_pGameRules         = 0x5F0;
}
namespace Rules {
    inline constexpr uintptr_t m_nStartingGold      = 0x58;
    inline constexpr uintptr_t m_iGameMode          = 0xE4;
    inline constexpr uintptr_t m_nGameState         = 0x7C;
    inline constexpr uintptr_t m_flGameStartTime    = 0x594;
    inline constexpr uintptr_t m_nRoshanRespawnPhase      = 0xCA8;
    inline constexpr uintptr_t m_flRoshanRespawnPhaseEndTime = 0xCAC;
    inline constexpr uintptr_t m_hGameModeEntity          = 0xE8;
}

namespace GameMode {
    inline constexpr uintptr_t m_bFogOfWarDisabled = 0x605;
    inline constexpr uintptr_t m_bUseUnseenFOW     = 0x606;
}

namespace Data {
    inline constexpr uintptr_t m_roshanSpawnInfo    = 0x1E20;
    inline constexpr uintptr_t m_nNextPowerRuneType = 0x1E38;
}
namespace RoshanPhase {
    inline constexpr uintptr_t m_eRoshanPhase        = 0x08;
    inline constexpr uintptr_t m_flRoshanPhaseStartTime = 0x0C;
    inline constexpr uintptr_t m_flRoshanPhaseEndTime   = 0x10;
}

namespace PlayerVisibility {
    inline constexpr uintptr_t m_flVisibilityStrength     = 0x5F0;
    inline constexpr uintptr_t m_flFogDistanceMultiplier  = 0x5F4;
    inline constexpr uintptr_t m_flFogMaxDensityMultiplier= 0x5F8;
    inline constexpr uintptr_t m_bStartDisabled           = 0x600;
    inline constexpr uintptr_t m_bIsEnabled               = 0x601;
}

enum ERoshanPhase : uint32_t {
    ROSHAN_ALIVE          = 0,
    ROSHAN_BASE_TIMER     = 1,
    ROSHAN_VARIABLE_TIMER = 2,
};

}
