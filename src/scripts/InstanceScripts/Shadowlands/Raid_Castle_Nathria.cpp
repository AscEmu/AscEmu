/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Raid_Castle_Nathria.hpp"

#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <set>
#include <utility>
#include <vector>

enum CastleNathriaEvents
{
    // Shriekwing
    EVENT_CN_SHRIEKWING_MURDER_PREY = 1,

    // Sludgefist
    EVENT_CN_SLUDGEFIST_FALLING_RUBBLE,
    EVENT_CN_SLUDGEFIST_HEEDLESS_CHARGE,
    EVENT_CN_SLUDGEFIST_IMPACT,

    // Sun King's Salvation
    EVENT_CN_KAEL_MINISTERS,
    EVENT_CN_KAEL_DARITHOS,
    EVENT_CN_KAEL_CLOAK_OF_FLAMES,
    EVENT_CN_KAEL_SHADE_BLAZING_SURGE,
    EVENT_CN_KAEL_SHADE_PHOENIXES,

    // Lady Inerva Darkvein
    EVENT_CN_INERVA_FOCUS_ANIMA,
    EVENT_CN_INERVA_BOTTLED_ANIMA,
    EVENT_CN_INERVA_SINS,
    EVENT_CN_INERVA_CONCENTRATED_ANIMA,
    EVENT_CN_SINS_SHARED_SUFFERING,
    EVENT_CN_SPECTER_CONDEMN,

    // Sire Denathrius
    EVENT_CN_DENATHRIUS_HUNT,
    EVENT_CN_DENATHRIUS_RAVAGE,
    EVENT_CN_DENATHRIUS_CHORUS,
    EVENT_CN_DENATHRIUS_CABALISTS,
    EVENT_CN_DENATHRIUS_REFLECTION
};

namespace
{
    // how a timed spell picks its target
    enum class CastleNathriaAim : uint8_t
    {
        Victim,
        Self,
        Area,
        Player,
        Ranged
    };

    struct CastleNathriaTimer
    {
        uint32_t spellId;
        CastleNathriaAim aim;
        uint8_t targets;
        uint32_t firstMs;
        uint32_t repeatMs;
        char const* warning;
    };

    struct CastleNathriaTimerSet
    {
        CastleNathriaTimer const* timers;
        size_t count;
    };

    template <size_t N>
    CastleNathriaTimerSet timerSet(CastleNathriaTimer const (&timers)[N])
    {
        return { timers, N };
    }

    float const CastleNathriaFullCircle = 6.28318531f;
    float const CastleNathriaHalfCircle = 3.14159265f;

    // timed spells use event ids from here on, one block per slot
    uint32_t const TimerEventBase = 1000;
    size_t const TimerSlotCount = 3;

    uint32_t const DestroyerBerserkMs = 420000;
    uint32_t const ShriekwingBerserkMs = 480000;
    uint32_t const SludgefistBerserkMs = 420000;
    uint32_t const XymoxBerserkMs = 480000;
    uint32_t const InervaBerserkMs = 480000;
    uint32_t const DenathriusBerserkMs = 600000;
    uint32_t const CouncilBerserkMs = 540000;
    uint32_t const GeneralsBerserkMs = 540000;
    uint32_t const AltimorBerserkMs = 540000;

    uint32_t const DestroyerEnergyPerSecond = 1;

    uint32_t const ShriekwingBloodPerSecond = 1;
    uint32_t const ShriekwingShroudMs = 40000;

    uint32_t const SludgefistRagePerSecond = 2;
    uint32_t const SludgefistGazeToChargeMs = 6000;
    uint32_t const SludgefistChargeToImpactMs = 3000;

    // Kael'thas is healed back from a fraction of his health
    uint32_t const SunKingFriendlyFaction = 35;
    uint32_t const SunKingStartPct = 35;
    uint32_t const SunKingShadeCount = 2;
    uint32_t const SunKingShadeMarks[SunKingShadeCount] = { 45, 90 };
    uint32_t const SunKingWatchMs = 2000;
    float const SunKingStartReach = 45.0f;
    float const SunKingWipeReach = 100.0f;
    float const SunKingInfuserReach = 6.0f;
    uint32_t const SunKingInfuserDrainPct = 1;
    uint32_t const SunKingFontStartPct = 25;
    uint32_t const SunKingFontHealPct = 10;
    uint32_t const SunKingWaveCount = 4;
    uint32_t const SunKingWaveSize = 3;
    uint32_t const SunKingMinisterWaves[SunKingWaveCount][SunKingWaveSize] =
    {
        { NPC_ROCKBOUND_VANQUISHER, NPC_BLEAKWING_ASSASSIN, 0 },
        { NPC_VILE_OCCULTIST, NPC_VILE_OCCULTIST, NPC_PESTERING_FIEND },
        { NPC_SOUL_INFUSER, NPC_SOUL_INFUSER, NPC_BLEAKWING_ASSASSIN },
        { NPC_ROCKBOUND_VANQUISHER, NPC_VILE_OCCULTIST, NPC_SOUL_INFUSER }
    };

    // Xy'mox swaps the relic at these health marks
    int32_t const XymoxExtinctionPct = 70;
    int32_t const XymoxAnnihilationPct = 40;

    // container levels are kept in half percent
    uint8_t const InervaContainerCount = 4;
    uint8_t const InervaDesire = 0;
    uint8_t const InervaBottled = 1;
    uint8_t const InervaSin = 2;
    uint8_t const InervaConcentrated = 3;
    uint32_t const InervaContainerFull = 200;
    uint32_t const InervaIdleGain = 1;
    uint32_t const InervaFocusedGain = 5;
    uint32_t const InervaEnergyPerSecond = 4;
    uint32_t const InervaExposeSpells[3] = { SPELL_INERVA_EXPOSE_DESIRES, SPELL_INERVA_EXPOSE_COGNITION, SPELL_INERVA_EXPOSE_HEART };
    uint32_t const InervaBottledSpells[3] = { SPELL_INERVA_BOTTLED_ANIMA, SPELL_INERVA_LINGERING_ANIMA, SPELL_INERVA_REPLICATING_ANIMA };
    uint32_t const InervaSinSpells[3] = { SPELL_INERVA_LESSER_SINS_AND_SUFFERING, SPELL_INERVA_SINS_AND_SUFFERING, SPELL_INERVA_GREATER_SINS_AND_SUFFERING };
    uint32_t const InervaConcentratedSpells[3] = { SPELL_INERVA_LIGHTLY_CONCENTRATED_ANIMA, SPELL_INERVA_CONCENTRATED_ANIMA, SPELL_INERVA_HIGHLY_CONCENTRATED_ANIMA };

    uint32_t const DenathriusStageSinners = 1;
    uint32_t const DenathriusStageMarch = 2;
    uint32_t const DenathriusStageChorus = 3;
    uint32_t const DenathriusStageIndignation = 4;
    int32_t const DenathriusChorusPct = 70;
    int32_t const DenathriusIndignationPct = 40;
    uint32_t const DenathriusMarchMs = 15000;

    int32_t const CouncilDansePct = 50;
    uint32_t const CouncilDanseMs = 30000;

    int32_t const GeneralsStonePct = 50;
    uint32_t const GeneralsReinforcementMs = 45000;
    uint32_t const GeneralsStoneLimitMs = 60000;

    // members of the encounters fought as one, gargons wait for their release
    struct CastleNathriaMember
    {
        uint32_t dataId;
        uint32_t entry;
        bool joinsPull;
    };

    CastleNathriaMember const CastleNathriaMembers[] =
    {
        { DATA_COUNCIL_OF_BLOOD,        BOSS_CASTELLAN_NIKLAUS,     true },
        { DATA_COUNCIL_OF_BLOOD,        BOSS_BARONESS_FRIEDA,       true },
        { DATA_COUNCIL_OF_BLOOD,        BOSS_LORD_STAVROS,          true },
        { DATA_STONE_LEGION_GENERALS,   BOSS_GENERAL_KAAL,          true },
        { DATA_STONE_LEGION_GENERALS,   BOSS_GENERAL_GRASHAAL,      true },
        { DATA_HUNTSMAN_ALTIMOR,        BOSS_HUNTSMAN_ALTIMOR,      true },
        { DATA_HUNTSMAN_ALTIMOR,        BOSS_MARGORE,               false },
        { DATA_HUNTSMAN_ALTIMOR,        BOSS_BARGAST,               false },
        { DATA_HUNTSMAN_ALTIMOR,        BOSS_HECUTIS,               false }
    };

    // release order of the hunting gargons
    uint32_t const AltimorGargonCount = 3;
    uint32_t const AltimorGargons[AltimorGargonCount] = { BOSS_MARGORE, BOSS_BARGAST, BOSS_HECUTIS };

    // point on a ring around origin, facing the middle
    LocationVector castleNathriaRingPoint(LocationVector const& origin, float minRange, float maxRange)
    {
        float const angle = Util::getRandomFloat(CastleNathriaFullCircle);
        float const range = Util::getRandomFloat(minRange, maxRange);
        float facing = angle + CastleNathriaHalfCircle;
        if (facing > CastleNathriaFullCircle)
            facing -= CastleNathriaFullCircle;

        return LocationVector(origin.x + range * std::cos(angle), origin.y + range * std::sin(angle), origin.z, facing);
    }

    uint32_t castleNathriaPercentOf(uint32_t value, uint32_t pct)
    {
        return static_cast<uint32_t>(static_cast<uint64_t>(value) * pct / 100);
    }

    // resource that fills by a fixed amount every second
    struct CastleNathriaGauge
    {
        uint32_t value = 0;
        uint32_t clockMs = 0;

        void reset()
        {
            value = 0;
            clockMs = 0;
        }

        void fill(uint32_t diff, uint32_t perSecond)
        {
            clockMs += diff;
            while (clockMs >= 1000)
            {
                clockMs -= 1000;
                value += perSecond;
            }
        }

        bool isFull() const { return value >= 100; }
    };

    // creatures Kael'thas' captors bring in
    struct CastleNathriaServant
    {
        WoWGuid guid;
        uint32_t entry;
        bool spent;
    };

    //////////////////////////////////////////////////////////////////////////////////////////
    // timed spells

    CastleNathriaTimer const DestroyerTimers[] =
    {
        { SPELL_DESTROYER_OVERWHELM,            CastleNathriaAim::Victim,   1,  8000,   12000,  nullptr },
        { SPELL_DESTROYER_GLUTTONOUS_MIASMA,    CastleNathriaAim::Ranged,   4,  3000,   24000,  nullptr },
        { SPELL_DESTROYER_VOLATILE_EJECTION,    CastleNathriaAim::Ranged,   1,  12000,  36000,  nullptr },
        { SPELL_DESTROYER_DESOLATE,             CastleNathriaAim::Area,     1,  25000,  40000,  nullptr },
        { SPELL_DESTROYER_EXPUNGE,              CastleNathriaAim::Ranged,   3,  30000,  30000,  nullptr }
    };

    CastleNathriaTimer const ShriekwingThirstTimers[] =
    {
        { SPELL_SHRIEKWING_EXSANGUINATING_BITE, CastleNathriaAim::Victim,   1,  8000,   18000,  nullptr },
        { SPELL_SHRIEKWING_ECHOLOCATION,        CastleNathriaAim::Area,     1,  12000,  26000,  nullptr },
        { SPELL_SHRIEKWING_BLIND_SWIPE,         CastleNathriaAim::Victim,   1,  14000,  24000,  nullptr },
        { SPELL_SHRIEKWING_ECHOING_SCREECH,     CastleNathriaAim::Area,     1,  17000,  30000,  nullptr },
        { SPELL_SHRIEKWING_EARSPLITTING_SHRIEK, CastleNathriaAim::Area,     1,  22000,  30000,  "Shriekwing lets loose an Earsplitting Shriek! Break line of sight!" },
        { SPELL_SHRIEKWING_WAVE_OF_BLOOD,       CastleNathriaAim::Area,     1,  30000,  35000,  nullptr }
    };

    CastleNathriaTimer const ShriekwingTerrorTimers[] =
    {
        { SPELL_SHRIEKWING_ECHOING_SONAR,       CastleNathriaAim::Self,     1,  2000,   8000,   nullptr },
        { SPELL_SHRIEKWING_SHROUDED_SHRIEK,     CastleNathriaAim::Area,     1,  6000,   12000,  nullptr }
    };

    CastleNathriaTimer const SludgefistTimers[] =
    {
        { SPELL_SLUDGEFIST_CHAIN_LINK,          CastleNathriaAim::Area,     1,  10000,  45000,  nullptr },
        { SPELL_SLUDGEFIST_COLOSSAL_ROAR,       CastleNathriaAim::Self,     1,  20000,  30000,  nullptr },
        { SPELL_SLUDGEFIST_DESTRUCTIVE_STOMP,   CastleNathriaAim::Area,     1,  30000,  40000,  nullptr },
        { SPELL_SLUDGEFIST_CHAIN_SLAM,          CastleNathriaAim::Area,     1,  35000,  45000,  nullptr }
    };

    CastleNathriaTimer const KaelShadeTimers[] =
    {
        { SPELL_KAEL_SHADE_FIERY_STRIKE,        CastleNathriaAim::Victim,   1,  6000,   12000,  nullptr },
        { SPELL_KAEL_SHADE_EMBER_BLAST,         CastleNathriaAim::Ranged,   1,  18000,  25000,  "The Shade of Kael'thas gathers an Ember Blast! Share the impact!" },
        { SPELL_KAEL_SHADE_CLOAK_OF_FLAMES,     CastleNathriaAim::Self,     1,  30000,  60000,  nullptr }
    };

    CastleNathriaTimer const RebornPhoenixTimers[] =
    {
        { SPELL_PHOENIX_SMOLDERING_PLUMAGE,     CastleNathriaAim::Area,     1,  5000,   10000,  nullptr }
    };

    CastleNathriaTimer const VanquisherTimers[] =
    {
        { SPELL_VANQUISHER_VANQUISHING_STRIKE,  CastleNathriaAim::Victim,   1,  6000,   14000,  nullptr },
        { SPELL_VANQUISHER_CONCUSSIVE_SMASH,    CastleNathriaAim::Area,     1,  12000,  22000,  nullptr }
    };

    CastleNathriaTimer const AssassinTimers[] =
    {
        { SPELL_ASSASSIN_CRIMSON_FLURRY,        CastleNathriaAim::Ranged,   1,  8000,   16000,  nullptr }
    };

    CastleNathriaTimer const OccultistTimers[] =
    {
        { SPELL_OCCULTIST_SCORNFUL_BLAST,       CastleNathriaAim::Victim,   1,  3000,   6000,   nullptr },
        { SPELL_OCCULTIST_VULGAR_BRAND,         CastleNathriaAim::Ranged,   1,  10000,  20000,  nullptr },
        { SPELL_OCCULTIST_DOOR_OF_SHADOWS,      CastleNathriaAim::Self,     1,  15000,  25000,  nullptr }
    };

    CastleNathriaTimer const FiendTimers[] =
    {
        { SPELL_FIEND_SHATTERING_RUBY,          CastleNathriaAim::Player,   1,  6000,   15000,  nullptr }
    };

    CastleNathriaTimer const DarithosTimers[] =
    {
        { SPELL_DARITHOS_GREATER_CASTIGATION,   CastleNathriaAim::Area,     1,  8000,   15000,  nullptr }
    };

    CastleNathriaTimer const XymoxTimers[] =
    {
        { SPELL_XYMOX_DIMENSIONAL_TEAR,         CastleNathriaAim::Area,     1,  8000,   35000,  nullptr },
        { SPELL_XYMOX_HYPERLIGHT_SPARK,         CastleNathriaAim::Victim,   1,  12000,  18000,  nullptr },
        { SPELL_XYMOX_GLYPH_OF_DESTRUCTION,     CastleNathriaAim::Victim,   1,  15000,  30000,  nullptr },
        { SPELL_XYMOX_STASIS_TRAP,              CastleNathriaAim::Area,     1,  20000,  30000,  nullptr },
        { SPELL_XYMOX_RIFT_BLAST,               CastleNathriaAim::Area,     1,  25000,  35000,  nullptr }
    };

    CastleNathriaTimer const XymoxPhantasmTimers[] =
    {
        { SPELL_XYMOX_CRYSTAL_OF_PHANTASMS,     CastleNathriaAim::Player,   1,  5000,   40000,  nullptr },
        { SPELL_XYMOX_SOUL_SINGE,               CastleNathriaAim::Area,     1,  10000,  25000,  nullptr }
    };

    CastleNathriaTimer const XymoxExtinctionTimers[] =
    {
        { SPELL_XYMOX_ROOT_OF_EXTINCTION,       CastleNathriaAim::Player,   1,  5000,   40000,  nullptr }
    };

    CastleNathriaTimer const XymoxAnnihilationTimers[] =
    {
        { SPELL_XYMOX_EDGE_OF_ANNIHILATION,     CastleNathriaAim::Victim,   1,  5000,   45000,  "Artificer Xy'mox unleashes the Edge of Annihilation! Escape through the wormholes!" },
        { SPELL_XYMOX_AURA_OF_DREAD,            CastleNathriaAim::Player,   1,  10000,  30000,  nullptr }
    };

    CastleNathriaTimer const FleetingSpiritTimers[] =
    {
        { SPELL_SPIRIT_POSSESSION,              CastleNathriaAim::Player,   1,  6000,   30000,  nullptr }
    };

    CastleNathriaTimer const SeedOfExtinctionTimers[] =
    {
        { SPELL_SEED_WITHERING_TOUCH,           CastleNathriaAim::Area,     1,  8000,   10000,  nullptr }
    };

    CastleNathriaTimer const ConjuredManifestationTimers[] =
    {
        { SPELL_MANIFESTATION_CONDEMN,          CastleNathriaAim::Area,     1,  3000,   6000,   nullptr }
    };

    CastleNathriaTimer const HarnessedSpecterTimers[] =
    {
        { SPELL_SPECTER_UNCONSCIONABLE_GUILT,   CastleNathriaAim::Victim,   1,  5000,   15000,  nullptr }
    };

    CastleNathriaTimer const DenathriusSinnersTimers[] =
    {
        { SPELL_DENATHRIUS_CLEANSING_PAIN,      CastleNathriaAim::Victim,   1,  6000,   24000,  nullptr },
        { SPELL_DENATHRIUS_BLOOD_PRICE,         CastleNathriaAim::Area,     1,  20000,  58000,  "Sire Denathrius demands a Blood Price!" },
        { SPELL_DENATHRIUS_COMMAND_RAVAGE,      CastleNathriaAim::Area,     1,  30000,  60000,  nullptr }
    };

    CastleNathriaTimer const DenathriusChorusTimers[] =
    {
        { SPELL_DENATHRIUS_WRACKING_PAIN,       CastleNathriaAim::Victim,   1,  6000,   18000,  nullptr },
        { SPELL_DENATHRIUS_HAND_OF_DESTRUCTION, CastleNathriaAim::Self,     1,  25000,  45000,  "Sire Denathrius raises his Hand of Destruction! Run!" },
        { SPELL_DENATHRIUS_COMMAND_MASSACRE,    CastleNathriaAim::Self,     1,  35000,  45000,  nullptr }
    };

    CastleNathriaTimer const DenathriusIndignationTimers[] =
    {
        { SPELL_DENATHRIUS_SHATTERING_PAIN,     CastleNathriaAim::Self,     1,  10000,  25000,  nullptr },
        { SPELL_DENATHRIUS_FATAL_FINESSE,       CastleNathriaAim::Ranged,   3,  15000,  45000,  nullptr },
        { SPELL_DENATHRIUS_BLOOD_PRICE_FINAL,   CastleNathriaAim::Self,     1,  20000,  55000,  nullptr },
        { SPELL_DENATHRIUS_HAND_OF_DESTRUCTION, CastleNathriaAim::Self,     1,  30000,  45000,  "Sire Denathrius raises his Hand of Destruction! Run!" },
        { SPELL_DENATHRIUS_SINISTER_REFLECTION, CastleNathriaAim::Self,     1,  40000,  60000,  nullptr }
    };

    CastleNathriaTimer const RemorniaTimers[] =
    {
        { SPELL_REMORNIA_CARNAGE,               CastleNathriaAim::Player,   1,  10000,  20000,  nullptr },
        { SPELL_REMORNIA_IMPALE,                CastleNathriaAim::Ranged,   1,  15000,  30000,  nullptr }
    };

    CastleNathriaTimer const EchoOfSinTimers[] =
    {
        { SPELL_ECHO_PAINFUL_MEMORIES,          CastleNathriaAim::Self,     1,  4000,   12000,  nullptr }
    };

    CastleNathriaTimer const CrimsonCabalistTimers[] =
    {
        { SPELL_CABALIST_CRIMSON_CHORUS,        CastleNathriaAim::Player,   1,  5000,   15000,  nullptr },
        { SPELL_CABALIST_DUSK_ELEGY,            CastleNathriaAim::Player,   1,  12000,  25000,  nullptr }
    };

    CastleNathriaTimer const NiklausTimers[] =
    {
        { SPELL_NIKLAUS_DUELISTS_RIPOSTE,       CastleNathriaAim::Victim,   1,  8000,   20000,  nullptr },
        { SPELL_NIKLAUS_SUMMON_DUTIFUL_ATTENDANT, CastleNathriaAim::Self,   1,  15000,  35000,  nullptr }
    };

    CastleNathriaTimer const NiklausInfusedTimers[] =
    {
        { SPELL_NIKLAUS_DREDGER_SERVANTS,       CastleNathriaAim::Self,     1,  10000,  40000,  nullptr }
    };

    CastleNathriaTimer const NiklausEmpoweredTimers[] =
    {
        { SPELL_NIKLAUS_CASTELLANS_CADRE,       CastleNathriaAim::Area,     1,  8000,   45000,  nullptr }
    };

    CastleNathriaTimer const FriedaTimers[] =
    {
        { SPELL_FRIEDA_DRAIN_ESSENCE,           CastleNathriaAim::Ranged,   1,  6000,   20000,  nullptr },
        { SPELL_FRIEDA_DREADBOLT_VOLLEY,        CastleNathriaAim::Area,     1,  10000,  15000,  nullptr }
    };

    CastleNathriaTimer const FriedaInfusedTimers[] =
    {
        { SPELL_FRIEDA_PRIDEFUL_ERUPTION,       CastleNathriaAim::Area,     1,  12000,  30000,  "Baroness Frieda erupts with prideful anima!" }
    };

    CastleNathriaTimer const FriedaEmpoweredTimers[] =
    {
        { SPELL_FRIEDA_SOUL_SPIKES,             CastleNathriaAim::Victim,   1,  8000,   20000,  nullptr },
        { SPELL_FRIEDA_TWISTED_PAIN,            CastleNathriaAim::Player,   1,  15000,  30000,  nullptr }
    };

    CastleNathriaTimer const StavrosTimers[] =
    {
        { SPELL_STAVROS_EVASIVE_LUNGE,          CastleNathriaAim::Victim,   1,  8000,   18000,  nullptr },
        { SPELL_STAVROS_DARK_RECITAL,           CastleNathriaAim::Area,     1,  20000,  45000,  nullptr }
    };

    CastleNathriaTimer const StavrosInfusedTimers[] =
    {
        { SPELL_STAVROS_WALTZ_OF_BLOOD,         CastleNathriaAim::Self,     1,  15000,  30000,  nullptr }
    };

    CastleNathriaTimer const StavrosEmpoweredTimers[] =
    {
        { SPELL_STAVROS_DANCING_FOOLS,          CastleNathriaAim::Area,     1,  10000,  45000,  nullptr }
    };

    CastleNathriaTimer const KaalGroundTimers[] =
    {
        { SPELL_KAAL_SERRATED_SWIPE,            CastleNathriaAim::Victim,   1,  6000,   12000,  nullptr },
        { SPELL_KAAL_WICKED_BLADE,              CastleNathriaAim::Ranged,   1,  15000,  30000,  "General Kaal readies her Wicked Blade!" },
        { SPELL_KAAL_HEART_REND,                CastleNathriaAim::Area,     1,  25000,  40000,  nullptr },
        { SPELL_KAAL_CALL_SHADOW_FORCES,        CastleNathriaAim::Self,     1,  35000,  50000,  nullptr }
    };

    CastleNathriaTimer const KaalAirTimers[] =
    {
        { SPELL_KAAL_RICOCHETING_SHURIKEN,      CastleNathriaAim::Player,   1,  6000,   20000,  nullptr },
        { SPELL_KAAL_WICKED_BLADE,              CastleNathriaAim::Ranged,   1,  10000,  30000,  nullptr },
        { SPELL_KAAL_CALL_SHADOW_FORCES,        CastleNathriaAim::Self,     1,  20000,  50000,  nullptr }
    };

    CastleNathriaTimer const GrashaalGroundTimers[] =
    {
        { SPELL_GRASHAAL_STONE_FIST,            CastleNathriaAim::Victim,   1,  6000,   15000,  nullptr },
        { SPELL_GRASHAAL_CRYSTALIZE,            CastleNathriaAim::Ranged,   1,  12000,  30000,  nullptr },
        { SPELL_GRASHAAL_STONE_BREAKERS_COMBO,  CastleNathriaAim::Victim,   1,  20000,  35000,  nullptr },
        { SPELL_GRASHAAL_REVERBERATING_LEAP,    CastleNathriaAim::Area,     1,  25000,  40000,  nullptr },
        { SPELL_GRASHAAL_SEISMIC_UPHEAVAL,      CastleNathriaAim::Area,     1,  35000,  45000,  nullptr }
    };

    CastleNathriaTimer const GrashaalAirTimers[] =
    {
        { SPELL_GRASHAAL_STONE_SPIKE,           CastleNathriaAim::Player,   1,  5000,   15000,  nullptr },
        { SPELL_GRASHAAL_CRYSTALIZE,            CastleNathriaAim::Ranged,   1,  8000,   30000,  nullptr }
    };

    CastleNathriaTimer const SkirmisherTimers[] =
    {
        { SPELL_SKIRMISHER_WICKED_SLAUGHTER,    CastleNathriaAim::Victim,   1,  6000,   12000,  nullptr }
    };

    CastleNathriaTimer const GoliathTimers[] =
    {
        { SPELL_GOLIATH_RAVENOUS_FEAST,         CastleNathriaAim::Victim,   1,  8000,   15000,  nullptr }
    };

    CastleNathriaTimer const CommandoTimers[] =
    {
        { SPELL_COMMANDO_CLUSTER_BOMBARDMENT,   CastleNathriaAim::Player,   1,  5000,   12000,  nullptr }
    };

    CastleNathriaTimer const AltimorTimers[] =
    {
        { SPELL_ALTIMOR_SPREADSHOT,             CastleNathriaAim::Victim,   1,  6000,   12000,  nullptr },
        { SPELL_ALTIMOR_SINSEEKER,              CastleNathriaAim::Self,     1,  25000,  50000,  "Huntsman Altimor marks his prey with Sinseeker!" }
    };

    CastleNathriaTimer const MargoreTimers[] =
    {
        { SPELL_MARGORE_JAGGED_CLAWS,           CastleNathriaAim::Victim,   1,  5000,   10000,  nullptr },
        { SPELL_MARGORE_VICIOUS_LUNGE,          CastleNathriaAim::Area,     1,  12000,  20000,  nullptr }
    };

    CastleNathriaTimer const BargastTimers[] =
    {
        { SPELL_BARGAST_RIP_SOUL,               CastleNathriaAim::Victim,   1,  10000,  30000,  nullptr },
        { SPELL_BARGAST_SHADES_OF_BARGAST,      CastleNathriaAim::Self,     1,  15000,  60000,  nullptr }
    };

    CastleNathriaTimer const HecutisTimers[] =
    {
        { SPELL_HECUTIS_PETRIFYING_HOWL,        CastleNathriaAim::Area,     1,  10000,  25000,  "Hecutis lets out a Petrifying Howl!" },
        { SPELL_HECUTIS_CHARGE,                 CastleNathriaAim::Ranged,   1,  15000,  30000,  nullptr }
    };

    CastleNathriaTimer const ShadeOfBargastTimers[] =
    {
        { SPELL_BARGAST_SHADE_DEATHLY_ROAR,     CastleNathriaAim::Area,     1,  6000,   15000,  nullptr }
    };
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class CastleNathriaInstanceScript : public InstanceScript
{
public:
    explicit CastleNathriaInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(CastleNathriaEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new CastleNathriaInstanceScript(pMapMgr); }

    // a fallen member must not end the encounter while its partners stand
    bool setBossState(uint32_t id, EncounterStates state) override
    {
        if (state == EncounterStates::Performed && getBossState(id) == EncounterStates::InProgress && !isGroupDown(id, nullptr))
            return false;

        return InstanceScript::setBossState(id, state);
    }

    void OnEncounterStateChange(uint32_t id, uint32_t state) override
    {
        if (id == DATA_STONE_LEGION_GENERALS)
        {
            mGeneralsStage = 1;
            mGeneralsIntermission = false;
        }
        else if (id == DATA_COUNCIL_OF_BLOOD)
        {
            mDanseCalled = false;
        }

        if (state != EncounterStates::Failed)
            return;

        std::set<WoWGuid> brought;
        if (id == DATA_HUNTSMAN_ALTIMOR)
        {
            brought.swap(mBroughtGargons);
            mNextGargon = 0;
            mCurrentGargon = WoWGuid();
        }

        // a wiped group fights again in full, gargons called in by the script go away
        for (Creature* pMember : getGroup(id))
        {
            if (brought.count(pMember->GetNewGUID()) != 0)
                pMember->despawn(1000, 0);
            else if (!pMember->isAlive())
                pMember->despawn(1000, 5000);
        }
    }

    std::vector<Creature*> getGroup(uint32_t dataId)
    {
        std::vector<Creature*> group;
        for (CastleNathriaMember const& member : CastleNathriaMembers)
        {
            if (member.dataId != dataId)
                continue;

            for (Creature* pCreature : getCreatureSetForEntry(member.entry))
                group.push_back(pCreature);
        }

        return group;
    }

    // single boss encounters have no group and are down with their boss
    bool isGroupDown(uint32_t dataId, Creature* pExcept)
    {
        for (Creature* pMember : getGroup(dataId))
        {
            if (pMember != pExcept && pMember->isAlive())
                return false;
        }

        return true;
    }

    // pulling one member pulls the others
    void rallyGroup(uint32_t dataId, CreatureAIScript* pPuller)
    {
        if (mRallying)
            return;

        mRallying = true;
        for (CastleNathriaMember const& member : CastleNathriaMembers)
        {
            if (member.dataId != dataId || !member.joinsPull)
                continue;

            for (Creature* pCreature : getCreatureSetForEntry(member.entry))
            {
                if (pCreature != pPuller->getCreature() && pCreature->isAlive() && !pCreature->isInCombat())
                    pPuller->setZoneWideCombat(pCreature);
            }
        }
        mRallying = false;
    }

    Creature* findLiving(uint32_t entry)
    {
        for (Creature* pCreature : getCreatureSetForEntry(entry))
        {
            if (pCreature->isAlive())
                return pCreature;
        }

        return nullptr;
    }

    void sendAction(uint32_t entry, int32_t action)
    {
        for (Creature* pCreature : getCreatureSetForEntry(entry))
        {
            if (pCreature->isAlive() && pCreature->GetScript() != nullptr)
                pCreature->GetScript()->DoAction(action);
        }
    }

    void sendActionToGroup(uint32_t dataId, int32_t action, Creature* pExcept)
    {
        for (Creature* pMember : getGroup(dataId))
        {
            if (pMember != pExcept && pMember->isAlive() && pMember->GetScript() != nullptr)
                pMember->GetScript()->DoAction(action);
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // The Council of Blood

    // the Danse Macabre is called once per attempt
    bool callDanseMacabre()
    {
        if (mDanseCalled)
            return false;

        mDanseCalled = true;
        sendActionToGroup(DATA_COUNCIL_OF_BLOOD, ACTION_COUNCIL_DANSE_BEGIN, nullptr);
        return true;
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // Stone Legion Generals

    uint8_t getGeneralsStage() const { return mGeneralsStage; }
    bool isGeneralsIntermission() const { return mGeneralsIntermission; }

    void beginGeneralsIntermission(uint32_t stoneEntry, uint32_t partnerEntry)
    {
        if (mGeneralsIntermission)
            return;

        mGeneralsIntermission = true;
        sendAction(stoneEntry, ACTION_GENERALS_TURN_TO_STONE);
        sendAction(partnerEntry, ACTION_GENERALS_CALL_REINFORCEMENTS);
    }

    void endGeneralsIntermission()
    {
        if (!mGeneralsIntermission)
            return;

        mGeneralsIntermission = false;
        if (mGeneralsStage < 3)
            ++mGeneralsStage;

        sendAction(BOSS_GENERAL_KAAL, ACTION_GENERALS_NEXT_STAGE);
        sendAction(BOSS_GENERAL_GRASHAAL, ACTION_GENERALS_NEXT_STAGE);
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // Huntsman Altimor

    bool hasReleasedGargon() const { return mNextGargon > 0; }
    bool isCurrentGargon(Creature* pCreature) const { return !mCurrentGargon.isEmpty() && pCreature->GetNewGUID() == mCurrentGargon; }
    Creature* getCurrentGargon() { return mCurrentGargon.isEmpty() ? nullptr : getCreatureByGuid(mCurrentGargon); }

    // a placed gargon is used, otherwise the next one is called in
    void releaseNextGargon(CreatureAIScript* pCaller)
    {
        if (mNextGargon >= AltimorGargonCount)
            return;

        uint32_t const entry = AltimorGargons[mNextGargon++];
        Creature* pGargon = findLiving(entry);
        if (pGargon == nullptr)
        {
            pGargon = pCaller->spawnCreature(entry, castleNathriaRingPoint(pCaller->getCreature()->GetPosition(), 4.0f, 8.0f));
            if (pGargon != nullptr)
                mBroughtGargons.insert(pGargon->GetNewGUID());
        }

        if (pGargon == nullptr)
            return;

        mCurrentGargon = pGargon->GetNewGUID();
        if (!pGargon->isInCombat())
            pCaller->setZoneWideCombat(pGargon);

        sendAction(BOSS_HUNTSMAN_ALTIMOR, ACTION_ALTIMOR_BIND_GARGON);
    }

private:
    bool mRallying = false;
    bool mDanseCalled = false;

    uint8_t mGeneralsStage = 1;
    bool mGeneralsIntermission = false;

    uint32_t mNextGargon = 0;
    WoWGuid mCurrentGargon;
    std::set<WoWGuid> mBroughtGargons;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event clock, timed spells, berserk and target helpers

class CastleNathriaCombatAI : public CreatureAIScript
{
public:
    explicit CastleNathriaCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        mStage = 1;
        mBerserkLeftMs = 0;
        onEngage();
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        scriptEvents.resetEvents();
        summons.despawnAll();
        clearTimerSets();
        clearStageFlags();
        mStage = 0;
        mBerserkLeftMs = 0;
        onDisengage();
    }

    void onSummonedCreature(Creature* pSummon) override { summons.summon(pSummon); }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!_isInCombat())
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        scriptEvents.updateEvents(diff, 0);
        tickBerserk(diff);
        onCombatTick(diff);

        // due events wait while a cast is running
        while (!_isCasting())
        {
            uint32_t const eventId = scriptEvents.getFinishedEvent();
            if (eventId == 0)
                break;

            if (!fireTimerEvent(eventId))
                onEvent(eventId);
        }
    }

protected:
    virtual void onEngage() {}
    virtual void onDisengage() {}
    virtual void onCombatTick(uint32_t /*diff*/) {}
    virtual void onEvent(uint32_t /*eventId*/) {}
    virtual void onTimerFired(uint32_t /*spellId*/) {}
    virtual void clearStageFlags() {}

    CastleNathriaInstanceScript* getCastle()
    {
        if (getCreature()->GetMapId() != MAP_CASTLE_NATHRIA)
            return nullptr;

        return static_cast<CastleNathriaInstanceScript*>(getInstanceScript());
    }

    // distinct random players, the tank only when nobody else is around
    std::vector<Unit*> pickPlayers(size_t count, bool spareTank)
    {
        Unit* pTank = getCreature()->getAIInterface()->getCurrentTarget();
        std::vector<Unit*> pool;
        std::vector<Unit*> spared;
        for (Object* pObject : getCreature()->getInRangePlayersSet())
        {
            Unit* pPlayer = static_cast<Unit*>(pObject);
            if (pPlayer == nullptr || !pPlayer->isAlive() || getCreature()->getDistance(pPlayer) > 100.0f)
                continue;

            if (spareTank && pPlayer == pTank)
                spared.push_back(pPlayer);
            else
                pool.push_back(pPlayer);
        }

        if (pool.empty())
            pool.swap(spared);

        size_t const wanted = std::min(count, pool.size());
        for (size_t i = 0; i < wanted; ++i)
        {
            size_t const pick = i + Util::getRandomUInt(static_cast<uint32_t>(pool.size() - 1 - i));
            std::swap(pool[i], pool[pick]);
        }

        pool.resize(wanted);
        return pool;
    }

    Unit* pickPlayer(bool spareTank)
    {
        std::vector<Unit*> const picked = pickPlayers(1, spareTank);
        return picked.empty() ? nullptr : picked.front();
    }

    Unit* nearestPlayer(float range)
    {
        Unit* pNearest = nullptr;
        float nearestRange = range;
        for (Object* pObject : getCreature()->getInRangePlayersSet())
        {
            Unit* pPlayer = static_cast<Unit*>(pObject);
            if (pPlayer == nullptr || !pPlayer->isAlive())
                continue;

            float const distance = getCreature()->getDistance(pPlayer);
            if (distance <= nearestRange)
            {
                nearestRange = distance;
                pNearest = pPlayer;
            }
        }

        return pNearest;
    }

    Unit* castAtPlayer(uint32_t spellId, bool spareTank, bool triggered = false)
    {
        Unit* pTarget = pickPlayer(spareTank);
        if (pTarget != nullptr)
            castSpell(pTarget, spellId, triggered);

        return pTarget;
    }

    // summoned add that joins the fight at once
    Creature* callAdd(uint32_t entry, LocationVector const& spot, CreatureSummonDespawnType despawnType = CORPSE_DESPAWN, uint32_t durationMs = 0)
    {
        Creature* pAdd = summonCreature(entry, spot, despawnType, durationMs);
        if (pAdd != nullptr)
            setZoneWideCombat(pAdd);

        return pAdd;
    }

    LocationVector aroundSelf(float minRange, float maxRange)
    {
        return castleNathriaRingPoint(getCreature()->GetPosition(), minRange, maxRange);
    }

    void warn(char const* text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }

    void armBerserk(uint32_t delayMs, uint32_t spellId = SPELL_CASTLE_NATHRIA_BERSERK)
    {
        mBerserkLeftMs = delayMs;
        mBerserkSpell = spellId;
    }

    void armTimers(size_t slot, CastleNathriaTimerSet timers)
    {
        if (slot >= TimerSlotCount)
            return;

        disarmTimers(slot);
        mTimerSets[slot] = timers;
        for (size_t i = 0; i < timers.count; ++i)
            scriptEvents.addEvent(timerEventId(slot, i), timers.timers[i].firstMs);
    }

    void disarmTimers(size_t slot)
    {
        if (slot >= TimerSlotCount)
            return;

        for (size_t i = 0; i < mTimerSets[slot].count; ++i)
            scriptEvents.removeEvent(timerEventId(slot, i));

        mTimerSets[slot] = CastleNathriaTimerSet{ nullptr, 0 };
    }

    void disarmAllTimers()
    {
        for (size_t slot = 0; slot < TimerSlotCount; ++slot)
            disarmTimers(slot);
    }

    // a new stage drops every pending event and timed spell
    void enterStage(uint32_t stage)
    {
        mStage = stage;
        scriptEvents.resetEvents();
        clearTimerSets();
    }

    // out of reach of the raid: no target, no melee
    void setUntouchable(bool apply, bool root = true)
    {
        if (apply)
            getCreature()->addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        else
            getCreature()->removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);

        if (root || !apply)
            setRooted(apply);

        _setMeleeDisabled(apply);
    }

    uint32_t mStage = 0;

private:
    static uint32_t timerEventId(size_t slot, size_t index)
    {
        return static_cast<uint32_t>(TimerEventBase * (slot + 1) + index);
    }

    void clearTimerSets()
    {
        for (size_t slot = 0; slot < TimerSlotCount; ++slot)
            mTimerSets[slot] = CastleNathriaTimerSet{ nullptr, 0 };
    }

    bool fireTimerEvent(uint32_t eventId)
    {
        if (eventId < TimerEventBase)
            return false;

        size_t const slot = eventId / TimerEventBase - 1;
        size_t const index = eventId % TimerEventBase;
        if (slot >= TimerSlotCount || index >= mTimerSets[slot].count)
            return true;

        CastleNathriaTimer const& timer = mTimerSets[slot].timers[index];
        uint32_t const spellId = timer.spellId;
        castTimer(timer);

        if (timer.repeatMs != 0)
            scriptEvents.addEvent(eventId, timer.repeatMs);

        onTimerFired(spellId);
        return true;
    }

    void castTimer(CastleNathriaTimer const& timer)
    {
        if (timer.warning != nullptr)
            warn(timer.warning);

        switch (timer.aim)
        {
            case CastleNathriaAim::Victim:
                castSpellOnVictim(timer.spellId);
                break;
            case CastleNathriaAim::Self:
                castSpellOnSelf(timer.spellId);
                break;
            case CastleNathriaAim::Area:
                castSpellAOE(timer.spellId);
                break;
            default:
            {
                bool const spareTank = timer.aim == CastleNathriaAim::Ranged;
                if (timer.targets <= 1)
                {
                    castAtPlayer(timer.spellId, spareTank);
                    break;
                }

                for (Unit* pTarget : pickPlayers(timer.targets, spareTank))
                    castSpell(pTarget, timer.spellId, true);
                break;
            }
        }
    }

    void tickBerserk(uint32_t diff)
    {
        if (mBerserkLeftMs == 0)
            return;

        if (mBerserkLeftMs > diff)
        {
            mBerserkLeftMs -= diff;
            return;
        }

        mBerserkLeftMs = 0;
        castSpellOnSelf(mBerserkSpell, true);
    }

    CastleNathriaTimerSet mTimerSets[TimerSlotCount] = {};
    uint32_t mBerserkLeftMs = 0;
    uint32_t mBerserkSpell = SPELL_CASTLE_NATHRIA_BERSERK;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame, boss state and group pull

class CastleNathriaBossAI : public CastleNathriaCombatAI
{
public:
    CastleNathriaBossAI(Creature* pCreature, uint32_t dataId) : CastleNathriaCombatAI(pCreature), mDataId(dataId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mDataId, EncounterStates::InProgress);
        }

        if (CastleNathriaInstanceScript* pCastle = getCastle())
            pCastle->rallyGroup(mDataId, this);

        CastleNathriaCombatAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        if (isAlive())
        {
            if (InstanceScript* pInstance = getInstanceScript())
            {
                pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
                pInstance->setBossState(mDataId, EncounterStates::Failed);
            }
        }

        CastleNathriaCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        clearStageFlags();
        beforeDefeatCheck();

        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

            CastleNathriaInstanceScript* pCastle = getCastle();
            if (pCastle == nullptr || pCastle->isGroupDown(mDataId, getCreature()))
                pInstance->setBossState(mDataId, EncounterStates::Performed);
        }

        onDefeat();
        scriptEvents.resetEvents();
        summons.despawnAll();
    }

protected:
    virtual void beforeDefeatCheck() {}
    virtual void onDefeat() {}

    uint32_t mDataId;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Adds driven by their timed spells alone

class CastleNathriaTimedAddAI : public CastleNathriaCombatAI
{
public:
    CastleNathriaTimedAddAI(Creature* pCreature, CastleNathriaTimerSet timers, uint32_t engageAura = 0)
        : CastleNathriaCombatAI(pCreature), mTimers(timers), mEngageAura(engageAura) {}

protected:
    void onEngage() override
    {
        if (mEngageAura != 0)
            castSpellOnSelf(mEngageAura, true);

        armTimers(0, mTimers);
    }

private:
    CastleNathriaTimerSet mTimers;
    uint32_t mEngageAura;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Hungering Destroyer

class CastleNathriaHungeringDestroyerAI : public CastleNathriaBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CastleNathriaHungeringDestroyerAI(c); }
    explicit CastleNathriaHungeringDestroyerAI(Creature* pCreature) : CastleNathriaBossAI(pCreature, DATA_HUNGERING_DESTROYER) {}

protected:
    void onEngage() override
    {
        mEnergy.reset();
        armBerserk(DestroyerBerserkMs);
        castSpellOnSelf(SPELL_DESTROYER_HUNGERING_STRIKES, true);
        armTimers(0, timerSet(DestroyerTimers));
    }

    // Consume at full energy
    void onCombatTick(uint32_t diff) override
    {
        mEnergy.fill(diff, DestroyerEnergyPerSecond);
        if (!mEnergy.isFull() || _isCasting())
            return;

        mEnergy.reset();
        warn("The Hungering Destroyer begins to Consume all life around it!");
        castSpellOnSelf(SPELL_DESTROYER_CONSUME);
    }

private:
    CastleNathriaGauge mEnergy;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shriekwing, shrouded in blood once she is engorged

class CastleNathriaShriekwingAI : public CastleNathriaBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CastleNathriaShriekwingAI(c); }
    explicit CastleNathriaShriekwingAI(Creature* pCreature) : CastleNathriaBossAI(pCreature, DATA_SHRIEKWING) {}

protected:
    void onEngage() override
    {
        armBerserk(ShriekwingBerserkMs);
        beginThirst();
    }

    void onCombatTick(uint32_t diff) override
    {
        if (mStage != 1)
            return;

        mBlood.fill(diff, ShriekwingBloodPerSecond);
        if (mBlood.isFull() && !_isCasting())
            beginTerror();
    }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_CN_SHRIEKWING_MURDER_PREY)
            return;

        castAtPlayer(SPELL_SHRIEKWING_MURDER_PREY, false, true);
        beginThirst();
    }

    void clearStageFlags() override
    {
        if (!mShrouded)
            return;

        mShrouded = false;
        setUntouchable(false);
        _removeAura(SPELL_SHRIEKWING_BLOOD_SHROUD);
    }

private:
    void beginThirst()
    {
        clearStageFlags();
        enterStage(1);
        mBlood.reset();
        armTimers(0, timerSet(ShriekwingThirstTimers));
    }

    // she keeps moving while shrouded
    void beginTerror()
    {
        enterStage(2);
        mShrouded = true;
        warn("Shriekwing is shrouded in blood and stalks her prey through the darkness!");
        castSpellOnSelf(SPELL_SHRIEKWING_BLOOD_SHROUD, true);
        setUntouchable(true, false);
        armTimers(0, timerSet(ShriekwingTerrorTimers));
        scriptEvents.addEvent(EVENT_CN_SHRIEKWING_MURDER_PREY, ShriekwingShroudMs);
    }

    CastleNathriaGauge mBlood;
    bool mShrouded = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Sludgefist, charges the gazed player at full rage

class CastleNathriaSludgefistAI : public CastleNathriaBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CastleNathriaSludgefistAI(c); }
    explicit CastleNathriaSludgefistAI(Creature* pCreature) : CastleNathriaBossAI(pCreature, DATA_SLUDGEFIST) {}

protected:
    void onEngage() override
    {
        mRage.reset();
        mCharging = false;
        mGazeTarget = WoWGuid();
        armBerserk(SludgefistBerserkMs, SPELL_SLUDGEFIST_GRUESOME_RAGE);
        castSpellOnSelf(SPELL_SLUDGEFIST_GIANT_FISTS, true);
        armTimers(0, timerSet(SludgefistTimers));
    }

    void onCombatTick(uint32_t diff) override
    {
        if (mCharging)
            return;

        mRage.fill(diff, SludgefistRagePerSecond);
        if (mRage.isFull() && !_isCasting())
            fixGaze();
    }

    void onTimerFired(uint32_t spellId) override
    {
        if (spellId == SPELL_SLUDGEFIST_DESTRUCTIVE_STOMP)
            scriptEvents.addEvent(EVENT_CN_SLUDGEFIST_FALLING_RUBBLE, 2500);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_CN_SLUDGEFIST_FALLING_RUBBLE:
                for (Unit* pTarget : pickPlayers(3, false))
                    castSpell(pTarget, SPELL_SLUDGEFIST_FALLING_RUBBLE, true);
                break;
            case EVENT_CN_SLUDGEFIST_HEEDLESS_CHARGE:
            {
                Unit* pTarget = gazeTarget();
                if (pTarget == nullptr)
                    pTarget = pickPlayer(true);

                if (pTarget != nullptr)
                    castSpell(pTarget, SPELL_SLUDGEFIST_HEEDLESS_CHARGE);

                scriptEvents.addEvent(EVENT_CN_SLUDGEFIST_IMPACT, SludgefistChargeToImpactMs);
                break;
            }
            case EVENT_CN_SLUDGEFIST_IMPACT:
                castSpellAOE(SPELL_SLUDGEFIST_DESTRUCTIVE_IMPACT, true);
                castSpellOnSelf(SPELL_SLUDGEFIST_COLLAPSING_FOUNDATION, true);
                mRage.reset();
                mCharging = false;
                mGazeTarget = WoWGuid();
                break;
            default:
                break;
        }
    }

private:
    void fixGaze()
    {
        mCharging = true;
        if (Unit* pTarget = pickPlayer(true))
        {
            mGazeTarget = pTarget->GetNewGUID();
            castSpell(pTarget, SPELL_SLUDGEFIST_HATEFUL_GAZE, true);
        }

        warn("Sludgefist fixes his Hateful Gaze upon his prey and prepares a Heedless Charge!");
        scriptEvents.addEvent(EVENT_CN_SLUDGEFIST_HEEDLESS_CHARGE, SludgefistGazeToChargeMs);
    }

    Unit* gazeTarget()
    {
        if (mGazeTarget.isEmpty() || getCreature()->getWorldMap() == nullptr)
            return nullptr;

        Unit* pTarget = getCreature()->getWorldMap()->getUnit(mGazeTarget);
        return pTarget != nullptr && pTarget->isAlive() ? pTarget : nullptr;
    }

    CastleNathriaGauge mRage;
    bool mCharging = false;
    WoWGuid mGazeTarget;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Sun King's Salvation: Kael'thas is healed back while the ministers try to stop it

class CastleNathriaKaelthasAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CastleNathriaKaelthasAI(c); }
    explicit CastleNathriaKaelthasAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        getCreature()->setFaction(SunKingFriendlyFaction);
        setReactState(REACT_PASSIVE);
        setCanEnterCombat(false);
        setRooted(true);
    }

    void OnDespawn() override { dismissServants(); }

    void AIUpdate(unsigned long time_passed) override
    {
        InstanceScript* pInstance = getInstanceScript();
        if (pInstance == nullptr || !isAlive() || pInstance->getBossState(DATA_SUN_KINGS_SALVATION) == EncounterStates::Performed)
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (!mActive)
        {
            if (mWatchMs > diff)
            {
                mWatchMs -= diff;
                return;
            }

            mWatchMs = 1000;
            if (hasPlayerWithin(SunKingStartReach))
                beginSalvation(pInstance);
            return;
        }

        scriptEvents.updateEvents(diff, 0);
        while (uint32_t eventId = scriptEvents.getFinishedEvent())
            onEvent(eventId);

        if (mWatchMs > diff)
        {
            mWatchMs -= diff;
            return;
        }

        mWatchMs = SunKingWatchMs;
        if (!hasPlayerWithin(SunKingWipeReach))
        {
            failSalvation(pInstance);
            return;
        }

        tendEssenceFonts();
        sufferSoulInfusion();
        checkRestoration(pInstance);
    }

private:
    bool hasPlayerWithin(float range)
    {
        for (Object* pObject : getCreature()->getInRangePlayersSet())
        {
            Unit* pPlayer = static_cast<Unit*>(pObject);
            if (pPlayer != nullptr && pPlayer->isAlive() && getCreature()->getDistance(pPlayer) <= range)
                return true;
        }

        return false;
    }

    void beginSalvation(InstanceScript* pInstance)
    {
        Creature* pSelf = getCreature();
        mActive = true;
        mWatchMs = SunKingWatchMs;
        mWave = 0;
        mShadesCalled = 0;
        mShadeGuid = WoWGuid();

        pSelf->m_canRegenerateHP = false;
        pSelf->setHealth(std::max<uint32_t>(1, castleNathriaPercentOf(pSelf->getMaxHealth(), SunKingStartPct)));

        pInstance->setBossState(DATA_SUN_KINGS_SALVATION, EncounterStates::InProgress);
        pInstance->sendUnitEncounter(EncounterFrameEngage, pSelf);
        sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Kael'thas Sunstrider lies weakened! Restore him before the Ministers of Vice can stop you!");

        scriptEvents.addEvent(EVENT_CN_KAEL_MINISTERS, 5000);
        scriptEvents.addEvent(EVENT_CN_KAEL_CLOAK_OF_FLAMES, 30000);
        scriptEvents.addEvent(EVENT_CN_KAEL_DARITHOS, 120000);
    }

    void onEvent(uint32_t eventId)
    {
        switch (eventId)
        {
            case EVENT_CN_KAEL_MINISTERS:
                callMinisters(SunKingMinisterWaves[mWave % SunKingWaveCount]);
                ++mWave;
                scriptEvents.addEvent(EVENT_CN_KAEL_MINISTERS, 45000);
                break;
            case EVENT_CN_KAEL_DARITHOS:
                bringServant(NPC_HIGH_TORTURER_DARITHOS, castleNathriaRingPoint(getCreature()->GetPosition(), 15.0f, 25.0f), 0, true);
                break;
            case EVENT_CN_KAEL_CLOAK_OF_FLAMES:
                // only while no Shade is present
                if (!isShadePresent())
                    castSpellOnSelf(SPELL_KAELTHAS_CLOAK_OF_FLAMES, true);
                scriptEvents.addEvent(EVENT_CN_KAEL_CLOAK_OF_FLAMES, 40000);
                break;
            default:
                break;
        }
    }

    // Soul Infusers walk in on their own, the others attack at once
    void callMinisters(uint32_t const (&wave)[SunKingWaveSize])
    {
        for (uint32_t entry : wave)
        {
            if (entry == 0)
                continue;

            bringServant(entry, castleNathriaRingPoint(getCreature()->GetPosition(), 15.0f, 25.0f), 0, entry != NPC_SOUL_INFUSER);
        }
    }

    Creature* bringServant(uint32_t entry, LocationVector const& spot, uint32_t factionId, bool joinFight)
    {
        Creature* pServant = spawnCreature(entry, spot, factionId);
        if (pServant == nullptr)
            return nullptr;

        mServants.push_back({ pServant->GetNewGUID(), entry, false });
        if (joinFight)
            setZoneWideCombat(pServant);

        return pServant;
    }

    Creature* findServant(WoWGuid const& guid)
    {
        if (guid.isEmpty() || getCreature()->getWorldMap() == nullptr)
            return nullptr;

        return getCreature()->getWorldMap()->getCreature(guid);
    }

    void dismissServants()
    {
        for (CastleNathriaServant const& servant : mServants)
        {
            if (Creature* pServant = findServant(servant.guid))
                pServant->despawn(500, 0);
        }

        mServants.clear();
        mShadeGuid = WoWGuid();
    }

    bool isShadePresent()
    {
        Creature* pShade = findServant(mShadeGuid);
        return pShade != nullptr && pShade->isAlive();
    }

    void restoreHealth(uint32_t pct)
    {
        Creature* pSelf = getCreature();
        uint32_t const maxHealth = pSelf->getMaxHealth();
        pSelf->setHealth(std::min(maxHealth, pSelf->getHealth() + castleNathriaPercentOf(maxHealth, pct)));
    }

    // fallen occultists leave a font, a healed font restores Kael'thas
    void tendEssenceFonts()
    {
        for (size_t i = 0; i < mServants.size(); ++i)
        {
            uint32_t const entry = mServants[i].entry;
            if (mServants[i].spent || (entry != NPC_VILE_OCCULTIST && entry != NPC_ESSENCE_FONT))
                continue;

            Creature* pServant = findServant(mServants[i].guid);
            if (pServant == nullptr)
                continue;

            if (entry == NPC_VILE_OCCULTIST && !pServant->isAlive())
            {
                mServants[i].spent = true;
                if (Creature* pFont = bringServant(NPC_ESSENCE_FONT, pServant->GetPosition(), SunKingFriendlyFaction, false))
                {
                    pFont->m_canRegenerateHP = false;
                    pFont->setHealth(std::max<uint32_t>(1, castleNathriaPercentOf(pFont->getMaxHealth(), SunKingFontStartPct)));
                }
            }
            else if (entry == NPC_ESSENCE_FONT && pServant->isAlive() && pServant->getHealth() >= pServant->getMaxHealth())
            {
                mServants[i].spent = true;
                pServant->castSpell(static_cast<Unit*>(getCreature()), static_cast<uint32_t>(SPELL_FONT_ESSENCE_OVERFLOW), true);
                restoreHealth(SunKingFontHealPct);
                pServant->despawn(1000, 0);
            }
        }
    }

    // every infuser at his side drains him
    void sufferSoulInfusion()
    {
        uint32_t infusers = 0;
        for (CastleNathriaServant const& servant : mServants)
        {
            if (servant.entry != NPC_SOUL_INFUSER)
                continue;

            Creature* pInfuser = findServant(servant.guid);
            if (pInfuser != nullptr && pInfuser->isAlive() && getCreature()->getDistance(pInfuser) <= SunKingInfuserReach + 2.0f)
                ++infusers;
        }

        if (infusers == 0)
            return;

        Creature* pSelf = getCreature();
        uint32_t const drain = castleNathriaPercentOf(pSelf->getMaxHealth(), SunKingInfuserDrainPct * infusers);
        pSelf->setHealth(pSelf->getHealth() > drain ? pSelf->getHealth() - drain : 1);
    }

    void checkRestoration(InstanceScript* pInstance)
    {
        Creature* pSelf = getCreature();
        uint32_t const maxHealth = pSelf->getMaxHealth();
        if (maxHealth == 0)
            return;

        uint32_t const percent = static_cast<uint32_t>(static_cast<uint64_t>(pSelf->getHealth()) * 100 / maxHealth);
        if (mShadesCalled < SunKingShadeCount && percent >= SunKingShadeMarks[mShadesCalled] && !isShadePresent())
        {
            callShade();
            return;
        }

        if (pSelf->getHealth() >= maxHealth && mShadesCalled >= SunKingShadeCount && !isShadePresent())
            completeSalvation(pInstance);
    }

    void callShade()
    {
        ++mShadesCalled;
        sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "A Shade of Kael'thas tears free from his tormented soul!");
        if (Creature* pShade = bringServant(NPC_SHADE_OF_KAELTHAS, castleNathriaRingPoint(getCreature()->GetPosition(), 6.0f, 10.0f), 0, true))
            mShadeGuid = pShade->GetNewGUID();
    }

    void completeSalvation(InstanceScript* pInstance)
    {
        mActive = false;
        scriptEvents.resetEvents();
        dismissServants();
        getCreature()->m_canRegenerateHP = true;

        sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Kael'thas Sunstrider is restored!");
        pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        pInstance->setBossState(DATA_SUN_KINGS_SALVATION, EncounterStates::Performed);
    }

    // nobody left standing nearby: everything goes back to the start
    void failSalvation(InstanceScript* pInstance)
    {
        mActive = false;
        mWatchMs = 5000;
        scriptEvents.resetEvents();
        dismissServants();
        getCreature()->m_canRegenerateHP = true;
        getCreature()->setHealth(getCreature()->getMaxHealth());

        pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        pInstance->setBossState(DATA_SUN_KINGS_SALVATION, EncounterStates::Failed);
    }

    bool mActive = false;
    uint32_t mWatchMs = 1000;
    uint32_t mWave = 0;
    uint32_t mShadesCalled = 0;
    WoWGuid mShadeGuid;
    std::vector<CastleNathriaServant> mServants;
};

class CastleNathriaKaelShadeAI : public CastleNathriaCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CastleNathriaKaelShadeAI(c); }
    explicit CastleNathriaKaelShadeAI(Creature* pCreature) : CastleNathriaCombatAI(pCreature) {}

protected:
    void onEngage() override
    {
        castSpellOnSelf(SPELL_KAEL_SHADE_REFLECTION_OF_GUILT, true);
        armTimers(0, timerSet(KaelShadeTimers));
        scriptEvents.addEvent(EVENT_CN_KAEL_SHADE_PHOENIXES, 10000);
        scriptEvents.addEvent(EVENT_CN_KAEL_SHADE_BLAZING_SURGE, 12000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_CN_KAEL_SHADE_BLAZING_SURGE:
                // heroic and above it strikes a random player's position
                if (isHeroic())
                    castAtPlayer(SPELL_KAEL_SHADE_BLAZING_SURGE_SPREAD, false);
                else
                    castSpellOnVictim(SPELL_KAEL_SHADE_BLAZING_SURGE);
                scriptEvents.addEvent(EVENT_CN_KAEL_SHADE_BLAZING_SURGE, 22000);
                break;
            case EVENT_CN_KAEL_SHADE_PHOENIXES:
                // the phoenixes fade with the Shade
                for (uint8_t i = 0; i < 2; ++i)
                    callAdd(NPC_REBORN_PHOENIX, aroundSelf(6.0f, 12.0f));
                scriptEvents.addEvent(EVENT_CN_KAEL_SHADE_PHOENIXES, 45000);
                break;
            default:
                break;
        }
    }
};

class CastleNathriaRebornPhoenixAI : public CastleNathriaTimedAddAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CastleNathriaRebornPhoenixAI(c); }
    explicit CastleNathriaRebornPhoenixAI(Creature* pCreature) : CastleNathriaTimedAddAI(pCreature, timerSet(RebornPhoenixTimers), SPELL_PHOENIX_EMBERS) {}
};

class CastleNathriaRockboundVanquisherAI : public CastleNathriaTimedAddAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CastleNathriaRockboundVanquisherAI(c); }
    explicit CastleNathriaRockboundVanquisherAI(Creature* pCreature) : CastleNathriaTimedAddAI(pCreature, timerSet(VanquisherTimers)) {}
};

class CastleNathriaBleakwingAssassinAI : public CastleNathriaTimedAddAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CastleNathriaBleakwingAssassinAI(c); }
    explicit CastleNathriaBleakwingAssassinAI(Creature* pCreature) : CastleNathriaTimedAddAI(pCreature, timerSet(AssassinTimers)) {}
};

class CastleNathriaVileOccultistAI : public CastleNathriaTimedAddAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CastleNathriaVileOccultistAI(c); }
    explicit CastleNathriaVileOccultistAI(Creature* pCreature) : CastleNathriaTimedAddAI(pCreature, timerSet(OccultistTimers)) {}
};

class CastleNathriaPesteringFiendAI : public CastleNathriaTimedAddAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CastleNathriaPesteringFiendAI(c); }
    explicit CastleNathriaPesteringFiendAI(Creature* pCreature) : CastleNathriaTimedAddAI(pCreature, timerSet(FiendTimers)) {}
};

class CastleNathriaDarithosAI : public CastleNathriaTimedAddAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CastleNathriaDarithosAI(c); }
    explicit CastleNathriaDarithosAI(Creature* pCreature) : CastleNathriaTimedAddAI(pCreature, timerSet(DarithosTimers)) {}
};

// ignores threat, walks to Kael'thas and channels into him
class CastleNathriaSoulInfuserAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CastleNathriaSoulInfuserAI(c); }
    explicit CastleNathriaSoulInfuserAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        _setMeleeDisabled(true);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!isAlive())
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (mClockMs > diff)
        {
            mClockMs -= diff;
            return;
        }

        mClockMs = 1500;
        Creature* pKael = findNearestCreature(NPC_KAELTHAS_SUNSTRIDER, 200.0f);
        if (pKael == nullptr || !pKael->isAlive())
            return;

        if (getCreature()->getDistance(pKael) > SunKingInfuserReach)
        {
            if (isRooted())
                setRooted(false);

            moveTo(pKael->GetPositionX(), pKael->GetPositionY(), pKael->GetPositionZ());
            return;
        }

        if (!isRooted())
        {
            stopMovement();
            setRooted(true);
        }

        if (!_isCasting())
            castSpell(pKael, SPELL_INFUSER_SOUL_INFUSION);
    }

private:
    uint32_t mClockMs = 1000;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Artificer Xy'mox, a new relic at 70% and 40%

class CastleNathriaXymoxAI : public CastleNathriaBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CastleNathriaXymoxAI(c); }
    explicit CastleNathriaXymoxAI(Creature* pCreature) : CastleNathriaBossAI(pCreature, DATA_ARTIFICER_XYMOX) {}

protected:
    void onEngage() override
    {
        armBerserk(XymoxBerserkMs);
        armTimers(0, timerSet(XymoxTimers));
        openRelic(1);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        int32_t const health = _getHealthPercent();
        if (mStage == 1 && health <= XymoxExtinctionPct)
            openRelic(2);
        else if (mStage == 2 && health <= XymoxAnnihilationPct)
            openRelic(3);
    }

    void onTimerFired(uint32_t spellId) override
    {
        if (spellId == SPELL_XYMOX_CRYSTAL_OF_PHANTASMS)
        {
            for (uint8_t i = 0; i < 2; ++i)
                callAdd(NPC_FLEETING_SPIRIT, aroundSelf(8.0f, 15.0f));
        }
        else if (spellId == SPELL_XYMOX_ROOT_OF_EXTINCTION)
        {
            callAdd(NPC_SEED_OF_EXTINCTION, aroundSelf(10.0f, 20.0f));
        }
    }

private:
    // the active relic goes back into its case
    void openRelic(uint32_t stage)
    {
        mStage = stage;
        summons.despawnAll();
        if (stage > 1)
            warn("Artificer Xy'mox returns the relic to its case and brings out another!");

        switch (stage)
        {
            case 1:
                armTimers(1, timerSet(XymoxPhantasmTimers));
                break;
            case 2:
                armTimers(1, timerSet(XymoxExtinctionTimers));
                break;
            default:
                armTimers(1, timerSet(XymoxAnnihilationTimers));
                break;
        }
    }
};

class CastleNathriaFleetingSpiritAI : public CastleNathriaTimedAddAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CastleNathriaFleetingSpiritAI(c); }
    explicit CastleNathriaFleetingSpiritAI(Creature* pCreature) : CastleNathriaTimedAddAI(pCreature, timerSet(FleetingSpiritTimers)) {}
};

// stays where it was planted
class CastleNathriaSeedOfExtinctionAI : public CastleNathriaTimedAddAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CastleNathriaSeedOfExtinctionAI(c); }
    explicit CastleNathriaSeedOfExtinctionAI(Creature* pCreature) : CastleNathriaTimedAddAI(pCreature, timerSet(SeedOfExtinctionTimers)) {}

protected:
    void onEngage() override
    {
        setRooted(true);
        _setMeleeDisabled(true);
        CastleNathriaTimedAddAI::onEngage();
    }

    void onDisengage() override
    {
        setRooted(false);
        _setMeleeDisabled(false);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Lady Inerva Darkvein, her abilities grow with the anima containers

class CastleNathriaInervaAI : public CastleNathriaBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CastleNathriaInervaAI(c); }
    explicit CastleNathriaInervaAI(Creature* pCreature) : CastleNathriaBossAI(pCreature, DATA_LADY_INERVA_DARKVEIN) {}

protected:
    void onEngage() override
    {
        for (uint32_t& level : mContainers)
            level = 0;

        mFocused = static_cast<uint8_t>(Util::getRandomUInt(InervaContainerCount - 1));
        mContainerClockMs = 0;
        mEnergy.reset();
        armBerserk(InervaBerserkMs);

        scriptEvents.addEvent(EVENT_CN_INERVA_FOCUS_ANIMA, 5000);
        scriptEvents.addEvent(EVENT_CN_INERVA_BOTTLED_ANIMA, 10000);
        scriptEvents.addEvent(EVENT_CN_INERVA_SINS, 20000);
        scriptEvents.addEvent(EVENT_CN_INERVA_CONCENTRATED_ANIMA, 30000);
    }

    // Expose at full energy, powered by the Container of Desire
    void onCombatTick(uint32_t diff) override
    {
        fillContainers(diff);

        mEnergy.fill(diff, InervaEnergyPerSecond);
        if (!mEnergy.isFull() || _isCasting())
            return;

        mEnergy.reset();
        castSpellOnVictim(InervaExposeSpells[containerTier(InervaDesire)]);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_CN_INERVA_FOCUS_ANIMA:
                mFocused = static_cast<uint8_t>(Util::getRandomUInt(InervaContainerCount - 1));
                castSpellOnSelf(SPELL_INERVA_FOCUS_ANIMA, true);
                scriptEvents.addEvent(EVENT_CN_INERVA_FOCUS_ANIMA, 30000);
                break;
            case EVENT_CN_INERVA_BOTTLED_ANIMA:
                castAtPlayer(InervaBottledSpells[containerTier(InervaBottled)], true);
                scriptEvents.addEvent(EVENT_CN_INERVA_BOTTLED_ANIMA, 25000);
                break;
            case EVENT_CN_INERVA_SINS:
                castAtPlayer(InervaSinSpells[containerTier(InervaSin)], false);
                for (uint8_t i = 0; i < 2; ++i)
                    callAdd(NPC_SINS_OF_THE_PAST, aroundSelf(8.0f, 15.0f), TIMED_DESPAWN, 20000);
                scriptEvents.addEvent(EVENT_CN_INERVA_SINS, 40000);
                break;
            case EVENT_CN_INERVA_CONCENTRATED_ANIMA:
            {
                // one specter per cast, more manifestations as the container rises
                uint8_t const tier = containerTier(InervaConcentrated);
                castAtPlayer(InervaConcentratedSpells[tier], false);
                callAdd(NPC_HARNESSED_SPECTER, aroundSelf(10.0f, 18.0f));
                for (uint8_t i = 0; i < tier; ++i)
                    callAdd(NPC_CONJURED_MANIFESTATION, aroundSelf(10.0f, 18.0f));
                scriptEvents.addEvent(EVENT_CN_INERVA_CONCENTRATED_ANIMA, 45000);
                break;
            }
            default:
                break;
        }
    }

private:
    // the focused container fills fastest, a full one breaches
    void fillContainers(uint32_t diff)
    {
        mContainerClockMs += diff;
        while (mContainerClockMs >= 1000)
        {
            mContainerClockMs -= 1000;
            for (uint8_t i = 0; i < InervaContainerCount; ++i)
            {
                mContainers[i] += (i == mFocused) ? InervaFocusedGain : InervaIdleGain;
                if (mContainers[i] < InervaContainerFull)
                    continue;

                mContainers[i] = 0;
                warn("A Primal Anima Container breaches!");
                castSpellAOE(SPELL_INERVA_CONTAINER_BREACH, true);
            }
        }
    }

    // 0 below a third, 1 below two thirds, 2 above
    uint8_t containerTier(uint8_t index) const
    {
        uint32_t const tier = mContainers[index] * 3 / InervaContainerFull;
        return static_cast<uint8_t>(std::min<uint32_t>(tier, 2));
    }

    uint32_t mContainers[InervaContainerCount] = {};
    uint8_t mFocused = 0;
    uint32_t mContainerClockMs = 0;
    CastleNathriaGauge mEnergy;
};

class CastleNathriaSinsOfThePastAI : public CastleNathriaCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CastleNathriaSinsOfThePastAI(c); }
    explicit CastleNathriaSinsOfThePastAI(Creature* pCreature) : CastleNathriaCombatAI(pCreature) {}

protected:
    void onEngage() override { scriptEvents.addEvent(EVENT_CN_SINS_SHARED_SUFFERING, 2000); }

    // the nearest player suffers
    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_CN_SINS_SHARED_SUFFERING)
            return;

        if (Unit* pTarget = nearestPlayer(60.0f))
            castSpell(pTarget, SPELL_SINS_SHARED_SUFFERING, true);

        scriptEvents.addEvent(EVENT_CN_SINS_SHARED_SUFFERING, 6000);
    }
};

class CastleNathriaConjuredManifestationAI : public CastleNathriaTimedAddAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CastleNathriaConjuredManifestationAI(c); }
    explicit CastleNathriaConjuredManifestationAI(Creature* pCreature) : CastleNathriaTimedAddAI(pCreature, timerSet(ConjuredManifestationTimers)) {}
};

// sessile, condemns the raid when nobody is in melee
class CastleNathriaHarnessedSpecterAI : public CastleNathriaTimedAddAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CastleNathriaHarnessedSpecterAI(c); }
    explicit CastleNathriaHarnessedSpecterAI(Creature* pCreature) : CastleNathriaTimedAddAI(pCreature, timerSet(HarnessedSpecterTimers)) {}

protected:
    void onEngage() override
    {
        setRooted(true);
        CastleNathriaTimedAddAI::onEngage();
        scriptEvents.addEvent(EVENT_CN_SPECTER_CONDEMN, 4000);
    }

    void onDisengage() override { setRooted(false); }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_CN_SPECTER_CONDEMN)
            return;

        if (nearestPlayer(6.0f) == nullptr)
            castSpellAOE(SPELL_SPECTER_CONDEMN);

        scriptEvents.addEvent(EVENT_CN_SPECTER_CONDEMN, 4000);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Sire Denathrius: Sinners Be Cleansed, March of the Penitent at 70%, Crimson Chorus, Indignation at 40%

class CastleNathriaDenathriusAI : public CastleNathriaBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CastleNathriaDenathriusAI(c); }
    explicit CastleNathriaDenathriusAI(Creature* pCreature) : CastleNathriaBossAI(pCreature, DATA_SIRE_DENATHRIUS) {}

protected:
    void onEngage() override
    {
        mReflectionRavages = true;
        armBerserk(DenathriusBerserkMs);
        castSpellOnSelf(SPELL_DENATHRIUS_COMMANDING_PRESENCE, true);
        _castOnInrangePlayers(SPELL_DENATHRIUS_BURDEN_OF_SIN, true);
        beginSinners();
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        int32_t const health = _getHealthPercent();
        if (mStage == DenathriusStageSinners && health <= DenathriusChorusPct)
            beginMarch();
        else if (mStage == DenathriusStageChorus && health <= DenathriusIndignationPct)
            beginIndignation();
    }

    void onTimerFired(uint32_t spellId) override
    {
        switch (spellId)
        {
            case SPELL_DENATHRIUS_CLEANSING_PAIN:
                for (uint8_t i = 0; i < 2; ++i)
                    callAdd(NPC_ECHO_OF_SIN, aroundSelf(5.0f, 12.0f), TIMED_DESPAWN, 30000);
                break;
            case SPELL_DENATHRIUS_COMMAND_RAVAGE:
                warn("Sire Denathrius commands Remornia to Ravage!");
                scriptEvents.addEvent(EVENT_CN_DENATHRIUS_RAVAGE, 3000);
                break;
            case SPELL_DENATHRIUS_SINISTER_REFLECTION:
                scriptEvents.addEvent(EVENT_CN_DENATHRIUS_REFLECTION, 2000);
                break;
            default:
                break;
        }
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_CN_DENATHRIUS_HUNT:
            {
                // the night hunter marks on heroic and above
                uint32_t const spellId = isHeroic() ? SPELL_DENATHRIUS_NIGHT_HUNTER : SPELL_DENATHRIUS_FEEDING_TIME;
                for (Unit* pTarget : pickPlayers(2, true))
                    castSpell(pTarget, spellId, true);
                scriptEvents.addEvent(EVENT_CN_DENATHRIUS_HUNT, 45000);
                break;
            }
            case EVENT_CN_DENATHRIUS_RAVAGE:
                castAtPlayer(SPELL_DENATHRIUS_RAVAGE, false);
                break;
            case EVENT_CN_DENATHRIUS_CHORUS:
                beginChorus();
                break;
            case EVENT_CN_DENATHRIUS_CABALISTS:
                for (uint8_t i = 0; i < 2; ++i)
                    callAdd(NPC_CRIMSON_CABALIST, aroundSelf(15.0f, 25.0f));
                scriptEvents.addEvent(EVENT_CN_DENATHRIUS_CABALISTS, 55000);
                break;
            case EVENT_CN_DENATHRIUS_REFLECTION:
                // the reflections alternate between Ravage and Massacre
                if (mReflectionRavages)
                    castSpellAOE(SPELL_REFLECTION_RAVAGE, true);
                else
                    castSpellOnSelf(SPELL_REFLECTION_MASSACRE, true);
                mReflectionRavages = !mReflectionRavages;
                break;
            default:
                break;
        }
    }

    void clearStageFlags() override
    {
        if (!mMarching)
            return;

        mMarching = false;
        setUntouchable(false);
    }

private:
    void beginSinners()
    {
        enterStage(DenathriusStageSinners);
        armTimers(0, timerSet(DenathriusSinnersTimers));
        scriptEvents.addEvent(EVENT_CN_DENATHRIUS_HUNT, 15000);
    }

    void beginMarch()
    {
        enterStage(DenathriusStageMarch);
        mMarching = true;
        warn("Sire Denathrius begins the March of the Penitent!");
        _castOnInrangePlayers(SPELL_DENATHRIUS_MARCH_OF_THE_PENITENT, true);
        setUntouchable(true);
        scriptEvents.addEvent(EVENT_CN_DENATHRIUS_CHORUS, DenathriusMarchMs);
    }

    // Remornia fights at his side
    void beginChorus()
    {
        clearStageFlags();
        enterStage(DenathriusStageChorus);
        callAdd(NPC_REMORNIA, aroundSelf(4.0f, 8.0f));
        armTimers(0, timerSet(DenathriusChorusTimers));
        scriptEvents.addEvent(EVENT_CN_DENATHRIUS_CABALISTS, 10000);
    }

    // he wields Remornia himself
    void beginIndignation()
    {
        enterStage(DenathriusStageIndignation);
        summons.despawnEntry(NPC_REMORNIA);
        warn("Sire Denathrius takes up Remornia in his own hand!");
        castSpellOnSelf(SPELL_DENATHRIUS_INDIGNATION, true);
        castSpellOnSelf(SPELL_DENATHRIUS_WIELD_REMORNIA, true);
        armTimers(0, timerSet(DenathriusIndignationTimers));
    }

    bool mMarching = false;
    bool mReflectionRavages = true;
};

// bound to Denathrius, the damage she takes goes to him
class CastleNathriaRemorniaAI : public CastleNathriaTimedAddAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CastleNathriaRemorniaAI(c); }
    explicit CastleNathriaRemorniaAI(Creature* pCreature) : CastleNathriaTimedAddAI(pCreature, timerSet(RemorniaTimers)) {}

    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage == nullptr || *damage == 0)
            return;

        Creature* pSire = findSire();
        if (pSire == nullptr)
            return;

        uint32_t const health = pSire->getHealth();
        pSire->setHealth(health > *damage ? health - *damage : 1);
        *damage = 0;
    }

protected:
    void onEngage() override
    {
        CastleNathriaTimedAddAI::onEngage();
        if (Creature* pSire = findSire())
            castSpell(pSire, SPELL_REMORNIA_BLOODBOUND, true);
    }

private:
    Creature* findSire()
    {
        Creature* pSire = findNearestCreature(BOSS_SIRE_DENATHRIUS, 200.0f);
        return pSire != nullptr && pSire->isAlive() ? pSire : nullptr;
    }
};

class CastleNathriaEchoOfSinAI : public CastleNathriaTimedAddAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CastleNathriaEchoOfSinAI(c); }
    explicit CastleNathriaEchoOfSinAI(Creature* pCreature) : CastleNathriaTimedAddAI(pCreature, timerSet(EchoOfSinTimers), SPELL_ECHO_EPHEMERAL) {}
};

class CastleNathriaCrimsonCabalistAI : public CastleNathriaTimedAddAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CastleNathriaCrimsonCabalistAI(c); }
    explicit CastleNathriaCrimsonCabalistAI(Creature* pCreature) : CastleNathriaTimedAddAI(pCreature, timerSet(CrimsonCabalistTimers)) {}
};

//////////////////////////////////////////////////////////////////////////////////////////
// The Council of Blood: survivors heal up and gain abilities, Danse Macabre at 50%

class CastleNathriaCouncilMemberAI : public CastleNathriaBossAI
{
public:
    CastleNathriaCouncilMemberAI(Creature* pCreature, CastleNathriaTimerSet baseTimers, CastleNathriaTimerSet infusedTimers, CastleNathriaTimerSet empoweredTimers)
        : CastleNathriaBossAI(pCreature, DATA_COUNCIL_OF_BLOOD), mBaseTimers(baseTimers), mInfusedTimers(infusedTimers), mEmpoweredTimers(empoweredTimers) {}

    void DoAction(int32_t action) override
    {
        if (!isAlive() || !_isInCombat())
            return;

        if (action == ACTION_COUNCIL_ALLY_FELL)
        {
            ++mFallenAllies;
            getCreature()->setHealth(getCreature()->getMaxHealth());
            onAllyFell(mFallenAllies);
            if (!mDancing)
                armCouncilTimers();
        }
        else if (action == ACTION_COUNCIL_DANSE_BEGIN && !mDancing)
        {
            mDancing = true;
            mDanseLeftMs = CouncilDanseMs;
            disarmAllTimers();
            setUntouchable(true);
        }
    }

protected:
    void onEngage() override
    {
        mFallenAllies = 0;
        mDancing = false;
        mDanseLeftMs = 0;
        armBerserk(CouncilBerserkMs);
        armCouncilTimers();
    }

    void onCombatTick(uint32_t diff) override
    {
        if (mDancing)
        {
            if (mDanseLeftMs > diff)
            {
                mDanseLeftMs -= diff;
                return;
            }

            clearStageFlags();
            armCouncilTimers();
            return;
        }

        if (_getHealthPercent() > CouncilDansePct)
            return;

        CastleNathriaInstanceScript* pCastle = getCastle();
        if (pCastle != nullptr && pCastle->callDanseMacabre())
        {
            warn("The Council of Blood retreats to the balcony for the Danse Macabre!");
            castSpellAOE(SPELL_COUNCIL_DANSE_MACABRE, true);
        }
    }

    void clearStageFlags() override
    {
        if (!mDancing)
            return;

        mDancing = false;
        setUntouchable(false);
    }

    void beforeDefeatCheck() override
    {
        if (CastleNathriaInstanceScript* pCastle = getCastle())
            pCastle->sendActionToGroup(DATA_COUNCIL_OF_BLOOD, ACTION_COUNCIL_ALLY_FELL, getCreature());
    }

    virtual void onAllyFell(uint32_t /*fallenAllies*/) {}

private:
    // infused after the first loss, empowered after the second
    void armCouncilTimers()
    {
        armTimers(0, mBaseTimers);
        if (mFallenAllies >= 1)
            armTimers(1, mInfusedTimers);
        if (mFallenAllies >= 2)
            armTimers(2, mEmpoweredTimers);
    }

    CastleNathriaTimerSet mBaseTimers;
    CastleNathriaTimerSet mInfusedTimers;
    CastleNathriaTimerSet mEmpoweredTimers;
    uint32_t mFallenAllies = 0;
    bool mDancing = false;
    uint32_t mDanseLeftMs = 0;
};

class CastleNathriaNiklausAI : public CastleNathriaCouncilMemberAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CastleNathriaNiklausAI(c); }
    explicit CastleNathriaNiklausAI(Creature* pCreature)
        : CastleNathriaCouncilMemberAI(pCreature, timerSet(NiklausTimers), timerSet(NiklausInfusedTimers), timerSet(NiklausEmpoweredTimers)) {}

protected:
    void onEngage() override
    {
        CastleNathriaCouncilMemberAI::onEngage();
        castSpellAOE(SPELL_COUNCIL_OPPRESSIVE_ATMOSPHERE, true);
    }

    void onAllyFell(uint32_t fallenAllies) override
    {
        if (fallenAllies == 2)
            castSpellOnSelf(SPELL_NIKLAUS_CASTELLANS_FURY, true);
    }

    void onTimerFired(uint32_t spellId) override
    {
        switch (spellId)
        {
            case SPELL_NIKLAUS_SUMMON_DUTIFUL_ATTENDANT:
                callAdd(NPC_DUTIFUL_ATTENDANT, aroundSelf(4.0f, 8.0f));
                break;
            case SPELL_NIKLAUS_DREDGER_SERVANTS:
                for (uint8_t i = 0; i < 2; ++i)
                    callAdd(NPC_BEGRUDGING_WAITER, aroundSelf(10.0f, 18.0f));
                break;
            case SPELL_NIKLAUS_CASTELLANS_CADRE:
                for (uint8_t i = 0; i < 2; ++i)
                    callAdd(NPC_VETERAN_STONEGUARD, aroundSelf(10.0f, 18.0f));
                break;
            default:
                break;
        }
    }
};

class CastleNathriaFriedaAI : public CastleNathriaCouncilMemberAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CastleNathriaFriedaAI(c); }
    explicit CastleNathriaFriedaAI(Creature* pCreature)
        : CastleNathriaCouncilMemberAI(pCreature, timerSet(FriedaTimers), timerSet(FriedaInfusedTimers), timerSet(FriedaEmpoweredTimers)) {}
};

class CastleNathriaStavrosAI : public CastleNathriaCouncilMemberAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CastleNathriaStavrosAI(c); }
    explicit CastleNathriaStavrosAI(Creature* pCreature)
        : CastleNathriaCouncilMemberAI(pCreature, timerSet(StavrosTimers), timerSet(StavrosInfusedTimers), timerSet(StavrosEmpoweredTimers)) {}

protected:
    void onAllyFell(uint32_t fallenAllies) override
    {
        if (fallenAllies == 2)
            castSpellOnSelf(SPELL_STAVROS_TWO_LEFT_FEET, true);
    }

    void onTimerFired(uint32_t spellId) override
    {
        if (spellId != SPELL_STAVROS_DANCING_FOOLS)
            return;

        for (uint8_t i = 0; i < 2; ++i)
            callAdd(NPC_WALTZING_VENTHYR, aroundSelf(10.0f, 18.0f));
    }
};

// shields a council member until slain
class CastleNathriaDutifulAttendantAI : public CastleNathriaCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CastleNathriaDutifulAttendantAI(c); }
    explicit CastleNathriaDutifulAttendantAI(Creature* pCreature) : CastleNathriaCombatAI(pCreature) {}

    void OnDied(Unit* /*pKiller*/) override
    {
        if (CastleNathriaInstanceScript* pCastle = getCastle())
        {
            if (!mWardGuid.isEmpty())
            {
                if (Creature* pWard = pCastle->getCreatureByGuid(mWardGuid))
                    pWard->removeAllAurasById(SPELL_ATTENDANT_UNYIELDING_SHIELD);
            }
        }

        mWardGuid = WoWGuid();
    }

protected:
    void onEngage() override
    {
        CastleNathriaInstanceScript* pCastle = getCastle();
        if (pCastle == nullptr || !mWardGuid.isEmpty())
            return;

        std::vector<Creature*> council;
        for (Creature* pMember : pCastle->getGroup(DATA_COUNCIL_OF_BLOOD))
        {
            if (pMember->isAlive())
                council.push_back(pMember);
        }

        if (council.empty())
            return;

        Creature* pWard = council[Util::getRandomUInt(static_cast<uint32_t>(council.size() - 1))];
        mWardGuid = pWard->GetNewGUID();
        castSpell(pWard, SPELL_ATTENDANT_UNYIELDING_SHIELD, true);
    }

private:
    WoWGuid mWardGuid;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Stone Legion Generals: one fights while the other is airborne, stone form at 50%

namespace
{
    enum class CastleNathriaGeneralStance : uint8_t
    {
        Grounded,
        Airborne,
        Stone
    };
}

class CastleNathriaGeneralAI : public CastleNathriaBossAI
{
public:
    CastleNathriaGeneralAI(Creature* pCreature, uint32_t selfEntry, uint32_t partnerEntry, uint8_t groundedStage, uint32_t stoneSpell, CastleNathriaTimerSet groundTimers, CastleNathriaTimerSet airTimers)
        : CastleNathriaBossAI(pCreature, DATA_STONE_LEGION_GENERALS), mSelfEntry(selfEntry), mPartnerEntry(partnerEntry), mGroundedStage(groundedStage),
        mStoneSpell(stoneSpell), mGroundTimers(groundTimers), mAirTimers(airTimers) {}

    void DoAction(int32_t action) override
    {
        if (!isAlive() || !_isInCombat())
            return;

        switch (action)
        {
            case ACTION_GENERALS_TURN_TO_STONE:
                takeStance(CastleNathriaGeneralStance::Stone);
                break;
            case ACTION_GENERALS_CALL_REINFORCEMENTS:
                callReinforcements();
                break;
            case ACTION_GENERALS_NEXT_STAGE:
                mReinforceLeftMs = 0;
                takeStageStance();
                break;
            case ACTION_GENERALS_LAST_STANDING:
                castSpellOnSelf(SPELL_GENERALS_SOLDIERS_OATH, true);
                break;
            default:
                break;
        }
    }

protected:
    void onEngage() override
    {
        mReinforcements.clear();
        mReinforceLeftMs = 0;
        armBerserk(GeneralsBerserkMs);
        takeStageStance();
    }

    void onCombatTick(uint32_t diff) override
    {
        CastleNathriaInstanceScript* pCastle = getCastle();
        if (pCastle == nullptr)
            return;

        if (mStance == CastleNathriaGeneralStance::Grounded && pCastle->getGeneralsStage() == mGroundedStage && !pCastle->isGeneralsIntermission() && _getHealthPercent() <= GeneralsStonePct)
            pCastle->beginGeneralsIntermission(mSelfEntry, mPartnerEntry);

        // stone form does not outlast a missing partner
        if (mStance == CastleNathriaGeneralStance::Stone)
        {
            mStoneMs += diff;
            if (mStoneMs >= GeneralsStoneLimitMs)
                pCastle->endGeneralsIntermission();
        }

        if (mReinforceLeftMs == 0)
            return;

        mReinforceLeftMs = mReinforceLeftMs > diff ? mReinforceLeftMs - diff : 0;
        if (mReinforceLeftMs == 0 || areReinforcementsDown())
        {
            mReinforceLeftMs = 0;
            pCastle->endGeneralsIntermission();
        }
    }

    void clearStageFlags() override
    {
        if (mStance == CastleNathriaGeneralStance::Grounded)
            return;

        if (mStance == CastleNathriaGeneralStance::Stone)
            _removeAura(mStoneSpell);

        mStance = CastleNathriaGeneralStance::Grounded;
        setUntouchable(false);
    }

    void onDefeat() override
    {
        if (CastleNathriaInstanceScript* pCastle = getCastle())
            pCastle->sendAction(mPartnerEntry, ACTION_GENERALS_LAST_STANDING);
    }

private:
    // both fight from stage three on or when the partner is missing
    void takeStageStance()
    {
        CastleNathriaInstanceScript* pCastle = getCastle();
        uint8_t const stage = pCastle != nullptr ? pCastle->getGeneralsStage() : 3;
        bool const grounded = stage >= 3 || stage == mGroundedStage || pCastle == nullptr || pCastle->findLiving(mPartnerEntry) == nullptr;
        takeStance(grounded ? CastleNathriaGeneralStance::Grounded : CastleNathriaGeneralStance::Airborne);
    }

    void takeStance(CastleNathriaGeneralStance stance)
    {
        clearStageFlags();
        disarmAllTimers();
        mStance = stance;
        mStoneMs = 0;

        switch (stance)
        {
            case CastleNathriaGeneralStance::Grounded:
                armTimers(0, mGroundTimers);
                break;
            case CastleNathriaGeneralStance::Airborne:
                setUntouchable(true);
                armTimers(0, mAirTimers);
                break;
            case CastleNathriaGeneralStance::Stone:
                setUntouchable(true);
                castSpellOnSelf(mStoneSpell, true);
                break;
        }
    }

    void callReinforcements()
    {
        warn("Stone Legion reinforcements launch an aerial assault!");
        mReinforcements.clear();
        if (Creature* pGoliath = callAdd(NPC_STONE_LEGION_GOLIATH, aroundSelf(10.0f, 20.0f)))
            mReinforcements.push_back(pGoliath->GetNewGUID());

        for (uint8_t i = 0; i < 2; ++i)
        {
            if (Creature* pCommando = callAdd(NPC_STONE_LEGION_COMMANDO, aroundSelf(10.0f, 20.0f)))
                mReinforcements.push_back(pCommando->GetNewGUID());
        }

        mReinforceLeftMs = GeneralsReinforcementMs;
    }

    bool areReinforcementsDown()
    {
        if (getCreature()->getWorldMap() == nullptr)
            return true;

        for (WoWGuid const& guid : mReinforcements)
        {
            Creature* pAdd = getCreature()->getWorldMap()->getCreature(guid);
            if (pAdd != nullptr && pAdd->isAlive())
                return false;
        }

        return true;
    }

    uint32_t mSelfEntry;
    uint32_t mPartnerEntry;
    uint8_t mGroundedStage;
    uint32_t mStoneSpell;
    CastleNathriaTimerSet mGroundTimers;
    CastleNathriaTimerSet mAirTimers;

    CastleNathriaGeneralStance mStance = CastleNathriaGeneralStance::Grounded;
    uint32_t mStoneMs = 0;
    uint32_t mReinforceLeftMs = 0;
    std::vector<WoWGuid> mReinforcements;
};

class CastleNathriaKaalAI : public CastleNathriaGeneralAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CastleNathriaKaalAI(c); }
    explicit CastleNathriaKaalAI(Creature* pCreature)
        : CastleNathriaGeneralAI(pCreature, BOSS_GENERAL_KAAL, BOSS_GENERAL_GRASHAAL, 1, SPELL_KAAL_HARDENED_STONE_FORM, timerSet(KaalGroundTimers), timerSet(KaalAirTimers)) {}

protected:
    void onTimerFired(uint32_t spellId) override
    {
        if (spellId != SPELL_KAAL_CALL_SHADOW_FORCES)
            return;

        for (uint8_t i = 0; i < 2; ++i)
            callAdd(NPC_STONE_LEGION_SKIRMISHER, aroundSelf(8.0f, 15.0f));
    }
};

class CastleNathriaGrashaalAI : public CastleNathriaGeneralAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CastleNathriaGrashaalAI(c); }
    explicit CastleNathriaGrashaalAI(Creature* pCreature)
        : CastleNathriaGeneralAI(pCreature, BOSS_GENERAL_GRASHAAL, BOSS_GENERAL_KAAL, 2, SPELL_GRASHAAL_HARDENED_STONE_FORM, timerSet(GrashaalGroundTimers), timerSet(GrashaalAirTimers)) {}
};

class CastleNathriaStoneLegionSkirmisherAI : public CastleNathriaTimedAddAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CastleNathriaStoneLegionSkirmisherAI(c); }
    explicit CastleNathriaStoneLegionSkirmisherAI(Creature* pCreature) : CastleNathriaTimedAddAI(pCreature, timerSet(SkirmisherTimers)) {}
};

class CastleNathriaStoneLegionGoliathAI : public CastleNathriaTimedAddAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CastleNathriaStoneLegionGoliathAI(c); }
    explicit CastleNathriaStoneLegionGoliathAI(Creature* pCreature) : CastleNathriaTimedAddAI(pCreature, timerSet(GoliathTimers), SPELL_GOLIATH_ANIMA_ENGORGED) {}
};

class CastleNathriaStoneLegionCommandoAI : public CastleNathriaTimedAddAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CastleNathriaStoneLegionCommandoAI(c); }
    explicit CastleNathriaStoneLegionCommandoAI(Creature* pCreature) : CastleNathriaTimedAddAI(pCreature, timerSet(CommandoTimers), SPELL_COMMANDO_ANIMA_LADEN) {}
};

//////////////////////////////////////////////////////////////////////////////////////////
// Huntsman Altimor, a new gargon is released when the bound one falls

class CastleNathriaAltimorAI : public CastleNathriaBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CastleNathriaAltimorAI(c); }
    explicit CastleNathriaAltimorAI(Creature* pCreature) : CastleNathriaBossAI(pCreature, DATA_HUNTSMAN_ALTIMOR) {}

    void DoAction(int32_t action) override
    {
        if (!isAlive())
            return;

        if (action == ACTION_ALTIMOR_BIND_GARGON)
        {
            CastleNathriaInstanceScript* pCastle = getCastle();
            Creature* pGargon = pCastle != nullptr ? pCastle->getCurrentGargon() : nullptr;
            if (pGargon != nullptr && pGargon->isAlive())
                castSpell(pGargon, SPELL_ALTIMOR_HUNTSMANS_BOND, true);
        }
        else if (action == ACTION_ALTIMOR_GARGON_FELL && _isInCombat())
        {
            castSpellOnSelf(SPELL_ALTIMOR_BROKEN_BOND, true);
        }
    }

protected:
    void onEngage() override
    {
        armBerserk(AltimorBerserkMs);
        armTimers(0, timerSet(AltimorTimers));

        CastleNathriaInstanceScript* pCastle = getCastle();
        if (pCastle != nullptr && !pCastle->hasReleasedGargon())
            pCastle->releaseNextGargon(this);
    }
};

class CastleNathriaGargonAI : public CastleNathriaBossAI
{
public:
    CastleNathriaGargonAI(Creature* pCreature, CastleNathriaTimerSet timers, uint32_t engageAura = 0)
        : CastleNathriaBossAI(pCreature, DATA_HUNTSMAN_ALTIMOR), mTimers(timers), mEngageAura(engageAura) {}

protected:
    void onEngage() override
    {
        armBerserk(AltimorBerserkMs);
        if (mEngageAura != 0)
            castSpellOnSelf(mEngageAura, true);

        armTimers(0, mTimers);
    }

    // the next gargon joins before the encounter is judged
    void beforeDefeatCheck() override
    {
        CastleNathriaInstanceScript* pCastle = getCastle();
        if (pCastle == nullptr)
            return;

        pCastle->sendAction(BOSS_HUNTSMAN_ALTIMOR, ACTION_ALTIMOR_GARGON_FELL);
        if (pCastle->isCurrentGargon(getCreature()))
            pCastle->releaseNextGargon(this);
    }

private:
    CastleNathriaTimerSet mTimers;
    uint32_t mEngageAura;
};

class CastleNathriaMargoreAI : public CastleNathriaGargonAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CastleNathriaMargoreAI(c); }
    explicit CastleNathriaMargoreAI(Creature* pCreature) : CastleNathriaGargonAI(pCreature, timerSet(MargoreTimers)) {}
};

class CastleNathriaBargastAI : public CastleNathriaGargonAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CastleNathriaBargastAI(c); }
    explicit CastleNathriaBargastAI(Creature* pCreature) : CastleNathriaGargonAI(pCreature, timerSet(BargastTimers)) {}

protected:
    void onTimerFired(uint32_t spellId) override
    {
        if (spellId != SPELL_BARGAST_SHADES_OF_BARGAST)
            return;

        for (uint8_t i = 0; i < 2; ++i)
            callAdd(NPC_SHADE_OF_BARGAST, aroundSelf(5.0f, 10.0f));
    }
};

class CastleNathriaHecutisAI : public CastleNathriaGargonAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CastleNathriaHecutisAI(c); }
    explicit CastleNathriaHecutisAI(Creature* pCreature) : CastleNathriaGargonAI(pCreature, timerSet(HecutisTimers), SPELL_HECUTIS_CRUSHING_STONE) {}
};

class CastleNathriaShadeOfBargastAI : public CastleNathriaTimedAddAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CastleNathriaShadeOfBargastAI(c); }
    explicit CastleNathriaShadeOfBargastAI(Creature* pCreature) : CastleNathriaTimedAddAI(pCreature, timerSet(ShadeOfBargastTimers), SPELL_BARGAST_SHADE_UNYIELDING) {}
};

void SetupCastleNathria(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_CASTLE_NATHRIA, &CastleNathriaInstanceScript::Create);

    // Hungering Destroyer
    mgr->register_creature_script(BOSS_HUNGERING_DESTROYER, &CastleNathriaHungeringDestroyerAI::Create);

    // Shriekwing
    mgr->register_creature_script(BOSS_SHRIEKWING, &CastleNathriaShriekwingAI::Create);
    mgr->register_creature_script(BOSS_SHRIEKWING_ALT, &CastleNathriaShriekwingAI::Create);

    // Sludgefist
    mgr->register_creature_script(BOSS_SLUDGEFIST, &CastleNathriaSludgefistAI::Create);
    mgr->register_creature_script(BOSS_SLUDGEFIST_ALT, &CastleNathriaSludgefistAI::Create);

    // Sun King's Salvation
    mgr->register_creature_script(NPC_KAELTHAS_SUNSTRIDER, &CastleNathriaKaelthasAI::Create);
    mgr->register_creature_script(NPC_SHADE_OF_KAELTHAS, &CastleNathriaKaelShadeAI::Create);
    mgr->register_creature_script(NPC_REBORN_PHOENIX, &CastleNathriaRebornPhoenixAI::Create);
    mgr->register_creature_script(NPC_ROCKBOUND_VANQUISHER, &CastleNathriaRockboundVanquisherAI::Create);
    mgr->register_creature_script(NPC_BLEAKWING_ASSASSIN, &CastleNathriaBleakwingAssassinAI::Create);
    mgr->register_creature_script(NPC_VILE_OCCULTIST, &CastleNathriaVileOccultistAI::Create);
    mgr->register_creature_script(NPC_SOUL_INFUSER, &CastleNathriaSoulInfuserAI::Create);
    mgr->register_creature_script(NPC_PESTERING_FIEND, &CastleNathriaPesteringFiendAI::Create);
    mgr->register_creature_script(NPC_HIGH_TORTURER_DARITHOS, &CastleNathriaDarithosAI::Create);

    // Artificer Xy'mox
    mgr->register_creature_script(BOSS_ARTIFICER_XYMOX, &CastleNathriaXymoxAI::Create);
    mgr->register_creature_script(NPC_FLEETING_SPIRIT, &CastleNathriaFleetingSpiritAI::Create);
    mgr->register_creature_script(NPC_SEED_OF_EXTINCTION, &CastleNathriaSeedOfExtinctionAI::Create);

    // Lady Inerva Darkvein
    mgr->register_creature_script(BOSS_LADY_INERVA_DARKVEIN, &CastleNathriaInervaAI::Create);
    mgr->register_creature_script(BOSS_LADY_INERVA_DARKVEIN_ALT, &CastleNathriaInervaAI::Create);
    mgr->register_creature_script(NPC_SINS_OF_THE_PAST, &CastleNathriaSinsOfThePastAI::Create);
    mgr->register_creature_script(NPC_CONJURED_MANIFESTATION, &CastleNathriaConjuredManifestationAI::Create);
    mgr->register_creature_script(NPC_CONJURED_MANIFESTATION_ALT, &CastleNathriaConjuredManifestationAI::Create);
    mgr->register_creature_script(NPC_HARNESSED_SPECTER, &CastleNathriaHarnessedSpecterAI::Create);

    // Sire Denathrius
    mgr->register_creature_script(BOSS_SIRE_DENATHRIUS, &CastleNathriaDenathriusAI::Create);
    mgr->register_creature_script(NPC_REMORNIA, &CastleNathriaRemorniaAI::Create);
    mgr->register_creature_script(NPC_ECHO_OF_SIN, &CastleNathriaEchoOfSinAI::Create);
    mgr->register_creature_script(NPC_CRIMSON_CABALIST, &CastleNathriaCrimsonCabalistAI::Create);
    mgr->register_creature_script(NPC_CRIMSON_CABALIST_ALT, &CastleNathriaCrimsonCabalistAI::Create);

    // The Council of Blood
    mgr->register_creature_script(BOSS_CASTELLAN_NIKLAUS, &CastleNathriaNiklausAI::Create);
    mgr->register_creature_script(BOSS_BARONESS_FRIEDA, &CastleNathriaFriedaAI::Create);
    mgr->register_creature_script(BOSS_LORD_STAVROS, &CastleNathriaStavrosAI::Create);
    mgr->register_creature_script(NPC_DUTIFUL_ATTENDANT, &CastleNathriaDutifulAttendantAI::Create);
    mgr->register_creature_script(NPC_DUTIFUL_ATTENDANT_ALT, &CastleNathriaDutifulAttendantAI::Create);

    // Stone Legion Generals
    mgr->register_creature_script(BOSS_GENERAL_KAAL, &CastleNathriaKaalAI::Create);
    mgr->register_creature_script(BOSS_GENERAL_GRASHAAL, &CastleNathriaGrashaalAI::Create);
    mgr->register_creature_script(NPC_STONE_LEGION_SKIRMISHER, &CastleNathriaStoneLegionSkirmisherAI::Create);
    mgr->register_creature_script(NPC_STONE_LEGION_SKIRMISHER_ALT, &CastleNathriaStoneLegionSkirmisherAI::Create);
    mgr->register_creature_script(NPC_STONE_LEGION_GOLIATH, &CastleNathriaStoneLegionGoliathAI::Create);
    mgr->register_creature_script(NPC_STONE_LEGION_GOLIATH_ALT, &CastleNathriaStoneLegionGoliathAI::Create);
    mgr->register_creature_script(NPC_STONE_LEGION_COMMANDO, &CastleNathriaStoneLegionCommandoAI::Create);
    mgr->register_creature_script(NPC_STONE_LEGION_COMMANDO_ALT, &CastleNathriaStoneLegionCommandoAI::Create);

    // Huntsman Altimor
    mgr->register_creature_script(BOSS_HUNTSMAN_ALTIMOR, &CastleNathriaAltimorAI::Create);
    mgr->register_creature_script(BOSS_MARGORE, &CastleNathriaMargoreAI::Create);
    mgr->register_creature_script(BOSS_BARGAST, &CastleNathriaBargastAI::Create);
    mgr->register_creature_script(BOSS_HECUTIS, &CastleNathriaHecutisAI::Create);
    mgr->register_creature_script(NPC_SHADE_OF_BARGAST, &CastleNathriaShadeOfBargastAI::Create);
}
