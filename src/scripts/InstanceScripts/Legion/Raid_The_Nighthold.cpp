/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Raid_The_Nighthold.hpp"

#include "Map/Maps/WorldMap.hpp"
#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <algorithm>
#include <cmath>
#include <list>
#include <string>
#include <vector>

enum TheNightholdEvents : uint32_t
{
    // Krosus
    EVENT_KROSUS_SEARING_BRAND = 1,
    EVENT_KROSUS_ISOLATION_CHECK,
    EVENT_KROSUS_SLAM,
    EVENT_KROSUS_FEL_BEAM,
    EVENT_KROSUS_FEL_BEAM_SWEEP,
    EVENT_KROSUS_ORB_OF_DESTRUCTION,
    EVENT_KROSUS_BURNING_PITCH,
    EVENT_KROSUS_PITCH_FALL,
    EVENT_KROSUS_PITCH_LANDING,
    EVENT_EMBER_FELBLAST,

    // Skorpyron
    EVENT_SKORPYRON_ARCANOSLASH,
    EVENT_SKORPYRON_FOCUSED_BLAST,
    EVENT_SKORPYRON_FOCUSED_BLAST_RELEASE,
    EVENT_SKORPYRON_CALL_OF_THE_SCORPID,
    EVENT_SKORPYRON_SHOCKWAVE,
    EVENT_SKORPYRON_SHELL_REGROWTH,
    EVENT_SCORPID_ENERGY_SURGE,
    EVENT_SCORPID_BOON,

    // Tichondrius
    EVENT_TICHONDRIUS_CARRION_PLAGUE,
    EVENT_TICHONDRIUS_SEEKER_SWARM,
    EVENT_TICHONDRIUS_FEAST_OF_BLOOD,
    EVENT_TICHONDRIUS_ECHOES_OF_THE_VOID,
    EVENT_TICHONDRIUS_BRAND_OF_ARGUS,
    EVENT_TICHONDRIUS_GUARDS,
    EVENT_TICHONDRIUS_ILLUSIONARY_NIGHT,
    EVENT_TICHONDRIUS_NIGHT_FALLS,
    EVENT_TICHONDRIUS_NIGHT_BLOODFANGS,
    EVENT_TICHONDRIUS_NIGHT_NIGHTMARE,
    EVENT_TICHONDRIUS_NIGHT_ENDS,
    EVENT_NIGHTMARE_BEAM,
    EVENT_SPELLGUARD_VOLATILE_WOUND,
    EVENT_SPELLGUARD_RAPID_PURSUIT,
    EVENT_WATCHER_BURNING_SOUL,
    EVENT_WATCHER_RING_OF_SHADOWS,

    // Star Augur Etraeus
    EVENT_ETRAEUS_BURST,
    EVENT_ETRAEUS_GRAVITATIONAL_PULL,
    EVENT_ETRAEUS_CORONAL_EJECTION,
    EVENT_ETRAEUS_NOVA,
    EVENT_ETRAEUS_BIG_BANG,
    EVENT_ETRAEUS_PHASE_BEGIN,
    EVENT_EJECTION_ERUPT,
    EVENT_THING_WITNESS_THE_VOID,

    // Chronomatic Anomaly
    EVENT_ANOMALY_BURST_OF_TIME,
    EVENT_ANOMALY_CHRONOMETRIC_PARTICLES,
    EVENT_ANOMALY_TIME_RELEASE,
    EVENT_ANOMALY_TIME_BOMB,
    EVENT_ANOMALY_WANING_PARTICLE,
    EVENT_ANOMALY_TEMPORAL_ORBS,
    EVENT_ANOMALY_POWER_OVERWHELMING,
    EVENT_ANOMALY_TIME_RESET,
    EVENT_PARTICLE_WARP_NIGHTWELL,

    // Gul'dan
    EVENT_GULDAN_LIQUID_HELLFIRE,
    EVENT_GULDAN_HAND_OF_GULDAN,
    EVENT_GULDAN_EYE_OF_AMANTHUL,
    EVENT_GULDAN_FEL_EFFLUX,
    EVENT_GULDAN_SHIELD_FADES,
    EVENT_GULDAN_UNLEASHED,
    EVENT_GULDAN_BONDS_OF_FEL,
    EVENT_GULDAN_EYE_OF_GULDAN,
    EVENT_GULDAN_FEL_SCYTHE,
    EVENT_GULDAN_WELL_OF_SOULS,
    EVENT_GULDAN_BLACK_HARVEST,
    EVENT_GULDAN_FLAMES_OF_SARGERAS,
    EVENT_GULDAN_STORM_OF_THE_DESTROYER,
    EVENT_GULDAN_SOUL_SIPHON,
    EVENT_KURAZMAL_SHATTER_ESSENCE,
    EVENT_KURAZMAL_FEL_OBELISK,
    EVENT_VETHRIZ_DARK_BLAST,
    EVENT_VETHRIZ_GAZE,
    EVENT_DZORYKX_SOUL_VORTEX,
    EVENT_DZORYKX_ANGUISHED_SPIRITS,
    EVENT_EYE_PULSE,
    EVENT_EYE_DUPLICATE,

    // Trilliax
    EVENT_TRILLIAX_ARCANE_SLASH,
    EVENT_TRILLIAX_TOXIC_SLICE,
    EVENT_TRILLIAX_STERILIZE,
    EVENT_TRILLIAX_CLEANSING_RAGE,
    EVENT_TRILLIAX_MANA_RUPTURE,
    EVENT_TRILLIAX_ARCING_BONDS,
    EVENT_TRILLIAX_ANNIHILATION,
    EVENT_TRILLIAX_ANNIHILATION_BLAST,
    EVENT_TRILLIAX_SUCCULENT_FEAST,
    EVENT_TRILLIAX_TIDY_UP,
    EVENT_TRILLIAX_MODE_CHANGE,
    EVENT_SCRUBBER_SEEK,

    // Spellblade Aluriel
    EVENT_ALURIEL_ANNIHILATE,
    EVENT_ALURIEL_ANNIHILATE_FOLLOWUP,
    EVENT_ALURIEL_MARK,
    EVENT_ALURIEL_REPLICATE,
    EVENT_ALURIEL_REPLICATE_ECHO,
    EVENT_ALURIEL_DETONATE,
    EVENT_ALURIEL_ANIMATE,
    EVENT_ALURIEL_BRAND_BURN,
    EVENT_ALURIEL_NEXT_SCHOOL,
    EVENT_ENCHANTMENT_PYROBLAST,

    // Grand Magistrix Elisande
    EVENT_ELISANDE_SINGULARITY,
    EVENT_ELISANDE_RECURSIVE_ELEMENTAL,
    EVENT_ELISANDE_EXPEDIENT_ELEMENTAL,
    EVENT_ELISANDE_CASCADENT_STAR,
    EVENT_ELISANDE_ABLATION,
    EVENT_ELISANDE_ABLATING_EXPLOSION,
    EVENT_ELISANDE_ABLATIVE_PULSE,
    EVENT_ELISANDE_DELPHURIC_BEAM,
    EVENT_ELISANDE_DELPHURIC_BEAM_FIRE,
    EVENT_ELISANDE_EPOCHERIC_ORB,
    EVENT_ELISANDE_PERMELIATIVE_TORMENT,
    EVENT_ELISANDE_CONFLEXIVE_BURST,
    EVENT_ELISANDE_CONFLEXIVE_BURST_BLAST,
    EVENT_ELISANDE_LAYER_RESUME,
    EVENT_EPOCHERIC_ORB_IMPACT,
    EVENT_RECURSIVE_ELEMENTAL_BLAST,
    EVENT_EXPEDIENT_ELEMENTAL_EXOTHERMIC,

    // High Botanist Tel'arn
    EVENT_TELARN_SOLAR_COLLAPSE,
    EVENT_TELARN_PARASITIC_FETTER,
    EVENT_TELARN_CONTROLLED_CHAOS,
    EVENT_TELARN_CALL_OF_NIGHT,
    EVENT_SOLARIST_FLARE,
    EVENT_SOLARIST_SOLAR_COLLAPSE,
    EVENT_SOLARIST_PLASMA_SPHERES,
    EVENT_NATURALIST_TOXIC_SPORES,
    EVENT_NATURALIST_GRACE_OF_NATURE,
    EVENT_NATURALIST_PARASITIC_FETTER,
    EVENT_CHAOS_STALKER_RING,
    EVENT_COLLAPSE_STALKER_WAVE,
    EVENT_TOXIC_SPORE_CHECK
};

namespace
{
    float const NightholdTwoPi = 6.28318531f;

    // SetCreatureData slot for the coronal ejection spell
    uint32_t const NightholdEjectionKit = 1;

    LocationVector nightholdRing(LocationVector const& origin, float range, float angle)
    {
        return LocationVector(origin.x + range * std::cos(angle), origin.y + range * std::sin(angle), origin.z, angle);
    }

    LocationVector nightholdScatter(LocationVector const& origin, float minRange, float maxRange)
    {
        return nightholdRing(origin, Util::getRandomFloat(minRange, maxRange), Util::getRandomFloat(NightholdTwoPi));
    }

    // list of delays walked one step per use
    class NightholdCadence
    {
    public:
        enum class Mode : uint8_t
        {
            Once,
            HoldLast,
            Cycle
        };

        void start(std::vector<uint32_t> const& steps, Mode mode)
        {
            mSteps = steps;
            mMode = mode;
            mNext = 0;
        }

        // 0 once a Once list is used up
        uint32_t next()
        {
            if (mSteps.empty())
                return 0;

            if (mNext >= mSteps.size())
            {
                if (mMode == Mode::Once)
                    return 0;

                if (mMode == Mode::HoldLast)
                    return mSteps.back();

                mNext = 0;
            }

            return mSteps[mNext++];
        }

    private:
        std::vector<uint32_t> mSteps;
        Mode mMode = Mode::Once;
        size_t mNext = 0;
    };

    struct NightholdDoor
    {
        uint32_t entry;
        uint32_t encounterId;
        bool opensOnKill;
    };

    NightholdDoor const NightholdDoors[] =
    {
        { GO_SKORPYRON_ENTRANCE,       DATA_SKORPYRON,            false },
        { GO_SKORPYRON_EXIT,           DATA_SKORPYRON,            true  },
        { GO_ANOMALY_ENTRANCE,         DATA_CHRONOMATIC_ANOMALY,  false },
        { GO_ANOMALY_EXIT,             DATA_CHRONOMATIC_ANOMALY,  true  },
        { GO_TRILLIAX_ENTRANCE,        DATA_TRILLIAX,             false },
        { GO_TRILLIAX_EXIT,            DATA_TRILLIAX,             true  },
        { GO_ALURIEL_SEALED_DOOR,      DATA_SPELLBLADE_ALURIEL,   true  },
        { GO_ETRAEUS_ACADEMY_DOOR,     DATA_STAR_AUGUR_ETRAEUS,   false },
        { GO_BOTANIST_DOOR_LEFT,       DATA_HIGH_BOTANIST_TELARN, false },
        { GO_BOTANIST_DOOR_RIGHT,      DATA_HIGH_BOTANIST_TELARN, false },
        { GO_TICHONDRIUS_BALCONY_DOOR, DATA_TICHONDRIUS,          false },
        { GO_TICHONDRIUS_DOOR_1,       DATA_TICHONDRIUS,          false },
        { GO_TICHONDRIUS_DOOR_2,       DATA_TICHONDRIUS,          false },
        { GO_TICHONDRIUS_DOOR_3,       DATA_TICHONDRIUS,          false },
        { GO_TICHONDRIUS_DOOR_4,       DATA_TICHONDRIUS,          false },
        { GO_ELISANDE_DOOR_1,          DATA_ELISANDE,             false },
        { GO_ELISANDE_DOOR_2,          DATA_ELISANDE,             false },
        { GO_ELISANDE_DOOR_3,          DATA_ELISANDE,             false }
    };

    // Krosus: bridge as corner plus length and width axes
    LocationVector const KrosusBridgeCorner(-92.46f, 2780.96f, 3.17f);
    float const KrosusBridgeLengthX = 77.43f;
    float const KrosusBridgeLengthY = 44.01f;
    float const KrosusBridgeWidthX = -14.63f;
    float const KrosusBridgeWidthY = 23.46f;
    float const KrosusBeamSplitX = -110.0f;
    float const KrosusPitchSoakRange = 3.0f;
    uint32_t const KrosusMinimumPitches = 10;
    uint32_t const KrosusBridgeSlams = 12;

    std::vector<uint32_t> const KrosusFelBeamNormal = { 5000, 15000, 30000, 30000, 23000, 27000, 30000, 44000, 14000, 16000, 14000, 16000, 22000, 60000 };
    std::vector<uint32_t> const KrosusFelBeamHeroic = { 7000, 29000, 30000, 42000, 16000, 16000, 14000, 16000, 27000, 54000, 26000, 5000, 5000, 16000, 5000, 12000, 12000, 5000, 13000 };
    std::vector<uint32_t> const KrosusOrbNormal = { 70000, 40000, 60000, 25000, 60000, 37000, 15000, 15000, 30000 };
    std::vector<uint32_t> const KrosusOrbHeroic = { 19000, 60000, 23000, 62000, 27000, 25000, 15000, 15000, 14000, 30000, 55000 };
    std::vector<uint32_t> const KrosusPitchNormal = { 38000, 102000, 85000, 90000 };
    std::vector<uint32_t> const KrosusPitchHeroic = { 49000, 85000, 90000, 94000 };

    // Skorpyron: where the brood crawls out
    LocationVector const SkorpyronBroodSpot(121.33f, 3418.02f, -250.12f, 2.36f);
    uint32_t const SkorpyronScorpidsPerCall = 3;

    // Tichondrius
    LocationVector const TichondriusRoomCentre(241.41f, 3399.45f, 171.80f);
    LocationVector const TichondriusNightmareSpots[5] =
    {
        { 240.79f, 3430.17f, 171.8f, 5.09f },
        { 241.73f, 3354.39f, 171.8f, 1.45f },
        { 252.31f, 3430.54f, 171.8f, 4.58f },
        { 289.90f, 3398.10f, 171.8f, 3.05f },
        { 210.68f, 3390.30f, 171.8f, 6.28f }
    };
    LocationVector const TichondriusGuardSpots[3] =
    {
        { 289.26f, 3394.23f, 171.91f, 2.38f },
        { 246.59f, 3351.88f, 171.82f, 2.38f },
        { 265.80f, 3374.88f, 171.78f, 2.35f }
    };
    // bat waves: count and delay to the next wave, the last two alternate
    uint32_t const TichondriusBatWaveCount[5] = { 2, 3, 4, 2, 2 };
    uint32_t const TichondriusBatWaveDelay[5] = { 6000, 3000, 2000, 2000, 1000 };
    uint32_t const TichondriusNightMs = 30000;

    // Star Augur Etraeus
    std::vector<uint32_t> const EtraeusIceEjections = { 35200, 6600, 4800, 51200, 2100, 3000, 24200, 2100, 2200 };
    std::vector<uint32_t> const EtraeusFelEjections = { 3900, 2900, 2600, 9700, 2100, 1500, 32700, 1800, 2000, 13700, 3200, 1900, 21800, 7300, 11100, 2900, 2100, 23300, 2000, 2000 };
    std::vector<uint32_t> const EtraeusVoidEjections = { 10000, 10000, 42000 };

    // Chronomatic Anomaly: stalker spots around the Nightwell
    LocationVector const AnomalyParticleSpots[12] =
    {
        { 236.42f, 3108.52f, -236.93f, 0.66f },
        { 277.72f, 3087.88f, -236.93f, 1.58f },
        { 315.29f, 3163.98f, -236.95f, 3.68f },
        { 232.46f, 3156.74f, -236.95f, 5.98f },
        { 228.15f, 3132.27f, -236.95f, 0.19f },
        { 248.43f, 3175.77f, -236.95f, 5.46f },
        { 254.49f, 3093.32f, -236.93f, 1.12f },
        { 271.78f, 3184.27f, -236.95f, 4.89f },
        { 296.25f, 3179.95f, -236.95f, 4.29f },
        { 303.50f, 3097.13f, -236.95f, 2.05f },
        { 319.47f, 3116.16f, -236.95f, 2.55f },
        { 323.78f, 3140.63f, -236.93f, 3.09f }
    };

    // per speed: release, bomb, particle, orbs, overwhelming, release targets (0 = everyone)
    struct AnomalyTempo
    {
        uint32_t timeRelease;
        uint32_t timeBomb;
        uint32_t particle;
        uint32_t temporalOrbs;
        uint32_t powerOverwhelming;
        uint32_t releaseTargets;
    };

    AnomalyTempo const AnomalyTempoNormal[3] =
    {
        { 8000, 35000, 28000, 48000, 60000, 4 },
        { 5000, 20000, 38000, 30000, 60000, 0 },
        { 10000, 0, 28000, 15000, 60000, 2 }
    };

    AnomalyTempo const AnomalyTempoHeroic[3] =
    {
        { 8000, 28000, 23000, 38000, 53000, 4 },
        { 10000, 15000, 43000, 20000, 53000, 0 },
        { 5000, 17000, 38000, 0, 53000, 2 }
    };

    uint32_t const AnomalySpeedAuras[3] = { SPELL_ANOMALY_SPEED_NORMAL, SPELL_ANOMALY_SPEED_SLOW, SPELL_ANOMALY_SPEED_FAST };
    uint32_t const AnomalyBurstSpells[3] = { SPELL_ANOMALY_BURST_OF_TIME_NORMAL, SPELL_ANOMALY_BURST_OF_TIME_SLOW, SPELL_ANOMALY_BURST_OF_TIME_FAST };

    // Gul'dan
    LocationVector const GuldanRoomCentre(282.08f, 3135.37f, 465.93f);
    uint32_t const GuldanCouncil[3] = { NPC_FEL_LORD_KURAZMAL, NPC_INQUISITOR_VETHRIZ, NPC_DZORYKX_THE_TRAPPER };
    int32_t const GuldanEmpowerHealth[3] = { 85, 70, 55 };
    int32_t const GuldanSoulsHealth = 40;
    std::vector<uint32_t> const GuldanEffluxNormal = { 11000, 14000, 19900, 15600, 16800, 15900, 15800, 12000 };
    std::vector<uint32_t> const GuldanEffluxHeroic = { 11000, 14000, 18500, 12000 };
    std::vector<uint32_t> const GuldanHellfireNormal = { 63000, 41000, 41000, 84000, 41000, 84000, 36000, 41000 };
    std::vector<uint32_t> const GuldanHellfireHeroic = { 59000, 36000, 36000, 74000, 36000, 74000, 31600, 36000 };
    std::vector<uint32_t> const GuldanSoulEyesNormal = { 71500, 71400, 28600, 114000 };
    std::vector<uint32_t> const GuldanSoulEyesHeroic = { 62500, 62500, 25000, 100000 };
    std::vector<uint32_t> const GuldanHarvestNormal = { 82900, 100000, 100000 };
    std::vector<uint32_t> const GuldanHarvestHeroic = { 72500, 87500, 87500 };
    std::vector<uint32_t> const GuldanStormNormal = { 78600, 70000, 87000 };
    std::vector<uint32_t> const GuldanStormHeroic = { 68700, 61300, 76500 };

    // Trilliax
    LocationVector const TrilliaxRoomCentre(458.19f, 3309.75f, -224.126f);
    LocationVector const TrilliaxScrubberSpots[5] =
    {
        { 413.16f, 3285.52f, -225.67f, 3.64f },
        { 459.70f, 3360.98f, -225.67f, 1.52f },
        { 479.96f, 3355.19f, -225.69f, 1.11f },
        { 453.32f, 3259.95f, -225.69f, 4.62f },
        { 502.97f, 3331.67f, -225.69f, 0.44f }
    };
    uint32_t const TrilliaxModeMs[3] = { 45000, 30000, 20000 };
    uint32_t const TrilliaxToxicSlices = 5;

    // Spellblade Aluriel: frost, fire, arcane
    uint32_t const AlurielSchoolMs[3] = { 85000, 72000, 62000 };
    uint32_t const AlurielEnchantments[3] = { SPELL_ALURIEL_ICY_ENCHANTMENT, SPELL_ALURIEL_FIERY_ENCHANTMENT, SPELL_ALURIEL_MAGIC_ENCHANTMENT };
    uint32_t const AlurielAnimatedAdds[3] = { NPC_ICY_ENCHANTMENT, NPC_FIERY_ENCHANTMENT, NPC_ARCANE_ENCHANTMENT };
    uint32_t const AlurielReplications[3] = { SPELL_ALURIEL_REPLICATE_MARK_OF_FROST, SPELL_ALURIEL_REPLICATE_SEARING_BRAND, SPELL_ALURIEL_REPLICATE_ARCANE_ORB };
    uint32_t const AlurielDetonations[3] = { SPELL_ALURIEL_DETONATE_MARK_OF_FROST, SPELL_ALURIEL_DETONATE_SEARING_BRAND, SPELL_ALURIEL_DETONATE_ARCANE_ORB };
    uint32_t const AlurielAnimations[3] = { SPELL_ALURIEL_ANIMATE_MARK_OF_FROST, SPELL_ALURIEL_ANIMATE_SEARING_BRAND, SPELL_ALURIEL_ANIMATE_ARCANE_ORB };
    uint32_t const AlurielAnimatedPerCast = 3;

    // Grand Magistrix Elisande
    LocationVector const ElisandeRoomCentre(282.31f, 3135.51f, 226.84f);
    std::vector<uint32_t> const ElisandeSingularityNormal = { 23000, 36000, 46000, 65000 };
    std::vector<uint32_t> const ElisandeSingularityHeroic = { 25000, 36000, 57000, 65000 };
    std::vector<uint32_t> const ElisandeRecursiveNormal = { 5000, 49000, 41000, 60000 };
    std::vector<uint32_t> const ElisandeRecursiveHeroic = { 5000, 49000, 52000, 60000 };
    std::vector<uint32_t> const ElisandeExpedientNormal = { 8000, 71000, 101000 };
    std::vector<uint32_t> const ElisandeExpedientHeroic = { 8000, 88000, 95000, 20000 };
    std::vector<uint32_t> const ElisandeBeamNormal = { 59000, 26000, 40000, 110000 };
    std::vector<uint32_t> const ElisandeBeamHeroic = { 59000, 57000, 60000, 70000 };
    std::vector<uint32_t> const ElisandeOrbNormal = { 18000, 56000, 31000, 85000, 15000 };
    std::vector<uint32_t> const ElisandeOrbHeroic = { 18000, 76000, 37000, 70000, 15000, 15000, 30000, 15000 };
    std::vector<uint32_t> const ElisandeTormentNormal = { 23000, 41000, 106000 };
    std::vector<uint32_t> const ElisandeTormentHeroic = { 23000, 61000, 37000, 60000 };
    std::vector<uint32_t> const ElisandeBurstNormal = { 48000, 67000, 50000, 45000, 10000 };
    std::vector<uint32_t> const ElisandeBurstHeroic = { 48000, 52000, 56000, 65000, 10000 };
    uint32_t const ElisandeLayerShiftMs = 10000;
    uint32_t const ElisandeMarkMs = 6000;
    uint32_t const ElisandeOrbFallMs = 8000;
    float const ElisandeOrbSoakRange = 4.0f;

    // High Botanist Tel'arn
    LocationVector const TelarnSphereSpots[3] =
    {
        { 655.05f, 2777.77f, 125.39f },
        { 610.00f, 2787.85f, 125.40f },
        { 640.28f, 2817.88f, 125.39f }
    };
    uint32_t const TelarnCollapseWaves = 6;
    uint32_t const TelarnCollapseCount[6] = { 12, 12, 6, 6, 6, 1 };
    float const TelarnCollapseRadius[6] = { 25.0f, 20.0f, 15.0f, 10.0f, 5.0f, 0.0f };
    uint32_t const TelarnChaosRings[3] = { SPELL_TELARN_CONTROLLED_CHAOS_NEAR, SPELL_TELARN_CONTROLLED_CHAOS_MIDDLE, SPELL_TELARN_CONTROLLED_CHAOS_FAR };
    uint32_t const TelarnHealthShareMs = 500;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class NightholdInstanceScript : public InstanceScript
{
public:
    explicit NightholdInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(TheNightholdEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new NightholdInstanceScript(pMapMgr); }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        NightholdDoor const* pDoor = findDoor(pGameObject->getEntry());
        if (pDoor == nullptr)
            return;

        mDoors.push_back({ pGameObject->GetNewGUID(), pDoor });
        alignDoor(pGameObject, *pDoor);
    }

    void OnEncounterStateChange(uint32_t id, uint32_t /*state*/) override
    {
        for (TrackedDoor const& tracked : mDoors)
        {
            if (tracked.door->encounterId != id)
                continue;

            if (GameObject* pGameObject = getGameObjectByGuid(tracked.guid))
                alignDoor(pGameObject, *tracked.door);
        }
    }

private:
    struct TrackedDoor
    {
        WoWGuid guid;
        NightholdDoor const* door;
    };

    static NightholdDoor const* findDoor(uint32_t entry)
    {
        for (NightholdDoor const& door : NightholdDoors)
        {
            if (door.entry == entry)
                return &door;
        }

        return nullptr;
    }

    // room doors shut during the pull, passage doors open after the kill
    void alignDoor(GameObject* pGameObject, NightholdDoor const& door)
    {
        EncounterStates const state = getBossState(door.encounterId);
        bool const wantOpen = door.opensOnKill ? state == EncounterStates::Performed : state != EncounterStates::InProgress;
        bool const isOpen = pGameObject->getState() != GO_STATE_CLOSED;

        if (wantOpen != isOpen)
            pGameObject->setState(wantOpen ? GO_STATE_OPEN : GO_STATE_CLOSED);
    }

    std::vector<TrackedDoor> mDoors;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event tick and target helpers

class NightholdCreatureAI : public CreatureAIScript
{
public:
    explicit NightholdCreatureAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override { onEngage(); }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        scriptEvents.resetEvents();
        summons.despawnAll();
        onDisengage();
    }

    void onSummonedCreature(Creature* pSummon) override { summons.summon(pSummon); }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!mTicksIdle && !_isInCombat())
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        scriptEvents.updateEvents(diff, 0);
        onTick(diff);

        if (_isCasting())
            return;

        if (uint32_t const eventId = scriptEvents.getFinishedEvent())
            onEvent(eventId);
    }

    bool heroicPlus() { return isHeroic(); }

    uint32_t raidSize()
    {
        WorldMap* pMap = getCreature()->getWorldMap();
        return pMap != nullptr ? pMap->getPlayerCount() : 0;
    }

    void warn(std::string const& text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }

    // distinct random players, aura > 0 requires it, aura < 0 excludes it
    std::vector<Unit*> pickPlayers(size_t count, bool skipTank, int32_t aura = 0)
    {
        FilterArgs args(TargetFilter_Player);
        if (skipTank)
            args.addFilter(TargetFilter_NotCurrent);
        args.setAuraId(aura);

        std::vector<Unit*> pool;
        for (Object* pObject : getCreature()->getInRangePlayersSet())
        {
            if (pObject != nullptr && isValidUnitTarget(pObject, args.getTargetFilter(), args))
                pool.push_back(static_cast<Unit*>(pObject));
        }

        std::vector<Unit*> picked;
        while (!pool.empty() && picked.size() < count)
        {
            size_t const index = Util::getRandomUInt(static_cast<uint32_t>(pool.size() - 1));
            picked.push_back(pool[index]);
            pool[index] = pool.back();
            pool.pop_back();
        }

        // a stacked raid still gets its share
        if (picked.empty() && skipTank)
            return pickPlayers(count, false, aura);

        return picked;
    }

    Unit* pickPlayer(bool skipTank)
    {
        std::vector<Unit*> const picked = pickPlayers(1, skipTank);
        return picked.empty() ? nullptr : picked.front();
    }

    Unit* rangedPlayer(float minRange)
    {
        FilterArgs args(TargetFilter_Player, minRange, 150.0f);
        args.addFilter(TargetFilter_InRangeOnly);
        if (Unit* pTarget = selectUnitTarget(args))
            return pTarget;

        return pickPlayer(false);
    }

    Unit* closestPlayer()
    {
        FilterArgs args(TargetFilter_Player);
        args.addFilter(TargetFilter_Closest);
        return selectUnitTarget(args);
    }

    Unit* currentVictim() { return getCreature()->getAIInterface()->getCurrentTarget(); }

    bool playerNear(LocationVector const& where, float range, Unit** pFound = nullptr)
    {
        for (Object* pObject : getCreature()->getInRangePlayersSet())
        {
            if (pObject == nullptr || !static_cast<Unit*>(pObject)->isAlive())
                continue;

            if (pObject->GetPosition().getExactDist(where) <= range)
            {
                if (pFound != nullptr)
                    *pFound = static_cast<Unit*>(pObject);
                return true;
            }
        }

        return false;
    }

    void castOn(Unit* pTarget, uint32_t spellId, bool triggered = false)
    {
        if (pTarget != nullptr)
            castSpell(pTarget, spellId, triggered);
    }

    void castAt(LocationVector const& where, uint32_t spellId) { getCreature()->castSpellLoc(where, spellId, true); }

    Creature* callAdd(uint32_t entry, LocationVector const& position, CreatureSummonDespawnType despawnType = CORPSE_DESPAWN, uint32_t duration = 0, bool engage = true)
    {
        Creature* pAdd = summonCreature(entry, position, despawnType, duration);
        if (pAdd != nullptr && engage)
            setZoneWideCombat(pAdd);

        return pAdd;
    }

    // summons through another creature so its summon list owns the add
    Creature* callAddFor(Creature* pOwner, uint32_t entry, LocationVector const& position, CreatureSummonDespawnType despawnType = CORPSE_DESPAWN, uint32_t duration = 0, bool engage = true)
    {
        if (pOwner == nullptr || !pOwner->isAlive())
            return callAdd(entry, position, despawnType, duration, engage);

        Creature* pAdd = pOwner->summonCreature(entry, position, despawnType, duration);
        if (pAdd != nullptr && engage)
            setZoneWideCombat(pAdd);

        return pAdd;
    }

    // inert helper adds: passive, rooted, unselectable
    void becomeStalker()
    {
        setReactState(REACT_PASSIVE);
        setRooted(true);
        addUnitFlags(UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
    }

protected:
    virtual void onEngage() {}
    virtual void onDisengage() {}
    virtual void onTick(uint32_t /*diff*/) {}
    virtual void onEvent(uint32_t /*eventId*/) {}

    // events also tick out of combat
    bool mTicksIdle = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame, boss state and berserk

class NightholdBossAI : public NightholdCreatureAI
{
public:
    NightholdBossAI(Creature* pCreature, uint32_t encounterId) : NightholdCreatureAI(pCreature), mEncounterId(encounterId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mEncounterId, EncounterStates::InProgress);
        }

        mBerserkLeft = 0;
        NightholdCreatureAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        // a kill is closed in OnDied
        if (!isAlive())
            return;

        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
            pInstance->setBossState(mEncounterId, EncounterStates::Failed);
        }

        mBerserkLeft = 0;
        NightholdCreatureAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
            pInstance->setBossState(mEncounterId, EncounterStates::Performed);
        }

        mBerserkLeft = 0;
        scriptEvents.resetEvents();
        summons.despawnAll();
        onDefeat();
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (mBerserkLeft != 0 && _isInCombat())
        {
            uint32_t const diff = static_cast<uint32_t>(time_passed);
            if (mBerserkLeft > diff)
            {
                mBerserkLeft -= diff;
            }
            else
            {
                mBerserkLeft = 0;
                onBerserk();
            }
        }

        NightholdCreatureAI::AIUpdate(time_passed);
    }

protected:
    virtual void onDefeat() {}

    virtual void onBerserk() { castSpellOnSelf(SPELL_NIGHTHOLD_BERSERK, true); }

    // 0 disarms
    void armBerserk(uint32_t delayMs) { mBerserkLeft = delayMs; }

    uint32_t mEncounterId;
    uint32_t mBerserkLeft = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Krosus

class NightholdKrosusAI : public NightholdBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NightholdKrosusAI(c); }
    explicit NightholdKrosusAI(Creature* pCreature) : NightholdBossAI(pCreature, DATA_KROSUS) {}

    // stands beside the bridge
    void OnLoad() override { setRooted(true); }

protected:
    void onEngage() override
    {
        setRooted(true);
        mSlams = 0;
        mBeamLeftSide = Util::getRandomUInt(1) == 0;
        mPitchSpots.clear();

        bool const heroic = heroicPlus();
        mFelBeams.start(heroic ? KrosusFelBeamHeroic : KrosusFelBeamNormal, NightholdCadence::Mode::HoldLast);
        mOrbs.start(heroic ? KrosusOrbHeroic : KrosusOrbNormal, NightholdCadence::Mode::HoldLast);
        mPitches.start(heroic ? KrosusPitchHeroic : KrosusPitchNormal, NightholdCadence::Mode::HoldLast);

        scriptEvents.addEvent(EVENT_KROSUS_SEARING_BRAND, 2000);
        scriptEvents.addEvent(EVENT_KROSUS_ISOLATION_CHECK, 8000);
        scriptEvents.addEvent(EVENT_KROSUS_SLAM, 30000);
        scriptEvents.addEvent(EVENT_KROSUS_FEL_BEAM, mFelBeams.next());
        scriptEvents.addEvent(EVENT_KROSUS_ORB_OF_DESTRUCTION, mOrbs.next());
        scriptEvents.addEvent(EVENT_KROSUS_BURNING_PITCH, mPitches.next());
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_KROSUS_SEARING_BRAND:
                castSpellOnVictim(SPELL_KROSUS_SEARING_BRAND, true);
                scriptEvents.addEvent(EVENT_KROSUS_SEARING_BRAND, 4000);
                break;
            case EVENT_KROSUS_ISOLATION_CHECK:
                checkIsolation();
                break;
            case EVENT_KROSUS_SLAM:
                warn("Krosus raises his fists to Slam the bridge!");
                castSpellOnVictim(SPELL_KROSUS_SLAM);
                // the bridge would be gone after the last slam
                if (++mSlams == KrosusBridgeSlams)
                {
                    warn("Krosus goes berserk!");
                    castSpellOnSelf(SPELL_NIGHTHOLD_BERSERK, true);
                }
                scriptEvents.addEvent(EVENT_KROSUS_SLAM, 30000);
                break;
            case EVENT_KROSUS_FEL_BEAM:
                castSpellOnSelf(SPELL_KROSUS_FEL_BEAM_CONTROL, true);
                if (Creature* pAim = findNearestCreature(NPC_KROSUS_BEAM_TARGET, 120.0f))
                    castSpell(pAim, SPELL_KROSUS_FEL_BEAM);
                else
                    castSpellAOE(SPELL_KROSUS_FEL_BEAM);
                scriptEvents.addEvent(EVENT_KROSUS_FEL_BEAM_SWEEP, 100);
                scriptEvents.addEvent(EVENT_KROSUS_FEL_BEAM, mFelBeams.next());
                break;
            case EVENT_KROSUS_FEL_BEAM_SWEEP:
                sweepFelBeam();
                break;
            case EVENT_KROSUS_ORB_OF_DESTRUCTION:
                if (Unit* pTarget = rangedPlayer(15.0f))
                {
                    warn("Krosus hurls an Orb of Destruction!");
                    castSpell(pTarget, SPELL_KROSUS_ORB_OF_DESTRUCTION, true);
                    castSpellOnSelf(SPELL_KROSUS_ORB_OF_DESTRUCTION_CAST);
                }
                scriptEvents.addEvent(EVENT_KROSUS_ORB_OF_DESTRUCTION, mOrbs.next());
                break;
            case EVENT_KROSUS_BURNING_PITCH:
                warn("Krosus flings Burning Pitch across the bridge!");
                planPitch();
                castSpellOnSelf(SPELL_KROSUS_BURNING_PITCH);
                scriptEvents.addEvent(EVENT_KROSUS_PITCH_FALL, 100);
                scriptEvents.addEvent(EVENT_KROSUS_BURNING_PITCH, mPitches.next());
                break;
            case EVENT_KROSUS_PITCH_FALL:
                for (LocationVector const& spot : mPitchSpots)
                    castAt(spot, Util::getRandomUInt(1) == 0 ? SPELL_KROSUS_BURNING_PITCH_FALL : SPELL_KROSUS_BURNING_PITCH_FALL_ALT);
                scriptEvents.addEvent(EVENT_KROSUS_PITCH_LANDING, 3000);
                break;
            case EVENT_KROSUS_PITCH_LANDING:
                landPitch();
                break;
            default:
                break;
        }
    }

private:
    // nobody in melee reach means raid wide punishment
    void checkIsolation()
    {
        FilterArgs args(TargetFilter_Player);
        args.addFilter(TargetFilter_InMeleeRange);

        if (selectUnitTarget(args) == nullptr)
        {
            castSpellAOE(SPELL_KROSUS_ISOLATED_RAGE, true);
            castSpellAOE(SPELL_KROSUS_ISOLATED_RAGE_BURST, true);
            scriptEvents.addEvent(EVENT_KROSUS_ISOLATION_CHECK, 1000);
            return;
        }

        scriptEvents.addEvent(EVENT_KROSUS_ISOLATION_CHECK, 2000);
    }

    // the beam casters on one side of the bridge fire, sides alternate
    void sweepFelBeam()
    {
        std::list<Creature*> casters;
        GetCreatureListWithEntryInGrid(casters, NPC_KROSUS_BEAM_CASTER, 150.0f);

        for (Creature* pCaster : casters)
        {
            bool const leftSide = pCaster->GetPositionX() < KrosusBeamSplitX;
            if (leftSide == mBeamLeftSide)
                pCaster->castSpell(pCaster, SPELL_KROSUS_FEL_BEAM_DAMAGE, true);
        }

        mBeamLeftSide = !mBeamLeftSide;
    }

    // one pitch per player, never less than ten
    void planPitch()
    {
        mPitchSpots.clear();

        uint32_t const count = std::max(raidSize(), KrosusMinimumPitches);
        for (uint32_t i = 0; i < count; ++i)
        {
            float const along = Util::getRandomFloat(1.0f);
            float const across = Util::getRandomFloat(0.1f, 0.9f);
            mPitchSpots.emplace_back(KrosusBridgeCorner.x + along * KrosusBridgeLengthX + across * KrosusBridgeWidthX,
                KrosusBridgeCorner.y + along * KrosusBridgeLengthY + across * KrosusBridgeWidthY, KrosusBridgeCorner.z);
        }
    }

    // soaked pitch burns the soaker, unsoaked pitch becomes an ember
    void landPitch()
    {
        for (LocationVector const& spot : mPitchSpots)
        {
            Unit* pSoaker = nullptr;
            if (playerNear(spot, KrosusPitchSoakRange, &pSoaker))
                castSpell(pSoaker, SPELL_KROSUS_BURNING_PITCH_HIT, true);
            else
                callAdd(NPC_BURNING_EMBER, spot);
        }

        mPitchSpots.clear();
    }

    NightholdCadence mFelBeams;
    NightholdCadence mOrbs;
    NightholdCadence mPitches;
    std::vector<LocationVector> mPitchSpots;
    uint32_t mSlams = 0;
    bool mBeamLeftSide = true;
};

class NightholdBurningEmberAI : public NightholdCreatureAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NightholdBurningEmberAI(c); }
    explicit NightholdBurningEmberAI(Creature* pCreature) : NightholdCreatureAI(pCreature) {}

protected:
    void onEngage() override { scriptEvents.addEvent(EVENT_EMBER_FELBLAST, 2000); }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_EMBER_FELBLAST)
            return;

        castOn(pickPlayer(false), heroicPlus() ? SPELL_BURNING_EMBER_FELBURST : SPELL_BURNING_EMBER_FELBLAST);
        scriptEvents.addEvent(EVENT_EMBER_FELBLAST, 2000);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Skorpyron

class NightholdSkorpyronAI : public NightholdBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NightholdSkorpyronAI(c); }
    explicit NightholdSkorpyronAI(Creature* pCreature) : NightholdBossAI(pCreature, DATA_SKORPYRON) {}

protected:
    void onEngage() override
    {
        mEnergy = 0;
        mEnergyClock = 0;
        mEnergyStep = 0;
        mShellBreaks = 0;
        mCalls = 0;

        castSpellOnSelf(SPELL_SKORPYRON_CHITINOUS_EXOSKELETON, true);

        scriptEvents.addEvent(EVENT_SKORPYRON_ARCANOSLASH, 6000);
        scriptEvents.addEvent(EVENT_SKORPYRON_FOCUSED_BLAST, 13000);
        scriptEvents.addEvent(EVENT_SKORPYRON_CALL_OF_THE_SCORPID, 22000);
    }

    void onTick(uint32_t diff) override
    {
        // one energy on every fifth second, two otherwise
        mEnergyClock += diff;
        while (mEnergyClock >= 1000)
        {
            mEnergyClock -= 1000;
            mEnergy += mEnergyStep == 0 ? 1 : 2;
            mEnergyStep = (mEnergyStep + 1) % 5;
        }

        if (mEnergy >= 100)
        {
            mEnergy = 0;
            scriptEvents.addEvent(EVENT_SKORPYRON_SHOCKWAVE, 100);
        }

        // each lost quarter cracks the shell
        int32_t const health = _getHealthPercent();
        uint32_t const breaks = health <= 25 ? 3 : health <= 50 ? 2 : health <= 75 ? 1 : 0;
        if (breaks > mShellBreaks)
        {
            mShellBreaks = breaks;
            warn("Skorpyron's exoskeleton shatters!");
            _removeAura(SPELL_SKORPYRON_CHITINOUS_EXOSKELETON);
            castSpellOnSelf(SPELL_SKORPYRON_EXOSKELETAL_VULNERABILITY, true);
            scriptEvents.addEvent(EVENT_SKORPYRON_SHELL_REGROWTH, 15000);
        }
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SKORPYRON_ARCANOSLASH:
                castSpellOnVictim(SPELL_SKORPYRON_ARCANOSLASH);
                scriptEvents.addEvent(EVENT_SKORPYRON_ARCANOSLASH, 10000);
                break;
            case EVENT_SKORPYRON_FOCUSED_BLAST:
                if (Unit* pTarget = pickPlayer(true))
                {
                    mBlastTarget = pTarget->GetNewGUID();
                    getCreature()->setFacingToObject(pTarget);
                    warn("Skorpyron focuses a blast!");
                    castSpellOnSelf(SPELL_SKORPYRON_FOCUSED_BLAST);
                    scriptEvents.addEvent(EVENT_SKORPYRON_FOCUSED_BLAST_RELEASE, 100);
                }
                scriptEvents.addEvent(EVENT_SKORPYRON_FOCUSED_BLAST, 33000);
                break;
            case EVENT_SKORPYRON_FOCUSED_BLAST_RELEASE:
                if (Unit* pTarget = getCreature()->getWorldMap()->getUnit(mBlastTarget))
                {
                    if (pTarget->isAlive())
                        castSpell(pTarget, SPELL_SKORPYRON_FOCUSED_BLAST_HIT, true);
                }
                break;
            case EVENT_SKORPYRON_CALL_OF_THE_SCORPID:
                callBrood();
                scriptEvents.addEvent(EVENT_SKORPYRON_CALL_OF_THE_SCORPID, 20000);
                break;
            case EVENT_SKORPYRON_SHOCKWAVE:
                warn("Skorpyron unleashes a Shockwave!");
                castSpellOnSelf(SPELL_SKORPYRON_SHOCKWAVE);
                break;
            case EVENT_SKORPYRON_SHELL_REGROWTH:
                castSpellOnSelf(SPELL_SKORPYRON_CHITINOUS_EXOSKELETON, true);
                castSpellOnSelf(SPELL_SKORPYRON_INFUSED_EXOSKELETON, true);
                break;
            default:
                break;
        }
    }

private:
    // heroic adds a volatile scorpid on every second call
    void callBrood()
    {
        castSpellOnSelf(SPELL_SKORPYRON_CALL_OF_THE_SCORPID, true);

        for (uint32_t i = 0; i < SkorpyronScorpidsPerCall; ++i)
            callAdd(NPC_CRYSTALLINE_SCORPID, nightholdScatter(SkorpyronBroodSpot, 0.0f, 3.0f));

        if (heroicPlus() && (mCalls % 2) == 0)
            callAdd(NPC_VOLATILE_SCORPID, nightholdScatter(SkorpyronBroodSpot, 0.0f, 3.0f));

        ++mCalls;
    }

    WoWGuid mBlastTarget;
    uint32_t mEnergy = 0;
    uint32_t mEnergyClock = 0;
    uint32_t mEnergyStep = 0;
    uint32_t mShellBreaks = 0;
    uint32_t mCalls = 0;
};

class NightholdScorpidAI : public NightholdCreatureAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NightholdScorpidAI(c); }
    explicit NightholdScorpidAI(Creature* pCreature) : NightholdCreatureAI(pCreature) {}

    void OnLoad() override
    {
        mBurst = false;
        if (isVolatile())
            castSpellOnSelf(SPELL_VOLATILE_SCORPID_CHITIN, true);
    }

    // the volatile shell bursts on the killing blow
    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage == nullptr || mBurst || !isVolatile() || *damage < getCreature()->getHealth())
            return;

        mBurst = true;
        castSpellAOE(SPELL_VOLATILE_SCORPID_CHITIN_BLAST, true);
    }

protected:
    void onEngage() override
    {
        if (!isVolatile())
            scriptEvents.addEvent(EVENT_SCORPID_ENERGY_SURGE, 5000);

        scriptEvents.addEvent(EVENT_SCORPID_BOON, 12000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SCORPID_ENERGY_SURGE:
                castSpellOnVictim(SPELL_SCORPID_ENERGY_SURGE);
                scriptEvents.addEvent(EVENT_SCORPID_ENERGY_SURGE, 5000);
                break;
            case EVENT_SCORPID_BOON:
                castSpellOnSelf(SPELL_SCORPID_BOON_OF_THE_SCORPID, true);
                scriptEvents.addEvent(EVENT_SCORPID_BOON, 10000);
                break;
            default:
                break;
        }
    }

private:
    bool isVolatile() { return getCreature()->getEntry() == NPC_VOLATILE_SCORPID; }

    bool mBurst = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Tichondrius

class NightholdTichondriusAI : public NightholdBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NightholdTichondriusAI(c); }
    explicit NightholdTichondriusAI(Creature* pCreature) : NightholdBossAI(pCreature, DATA_TICHONDRIUS) {}

protected:
    void onEngage() override
    {
        mNights = 0;
        mInNight = false;
        mLongArgus = false;

        castSpellOnSelf(SPELL_TICHONDRIUS_VAMPIRIC_AURA, true);
        armBerserk(heroicPlus() ? 463000 : 523000);
        startRotation();
    }

    void onDisengage() override
    {
        if (mInNight)
            leaveNight();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_TICHONDRIUS_CARRION_PLAGUE:
                for (Unit* pTarget : pickPlayers(plagueTargets(), true))
                    castSpell(pTarget, SPELL_TICHONDRIUS_CARRION_PLAGUE, true);
                scriptEvents.addEvent(EVENT_TICHONDRIUS_CARRION_PLAGUE, 25000);
                break;
            case EVENT_TICHONDRIUS_SEEKER_SWARM:
            {
                // the swarm hunts the plagued
                std::vector<Unit*> plagued = pickPlayers(40, false, SPELL_TICHONDRIUS_CARRION_PLAGUE);
                if (plagued.empty())
                    plagued = pickPlayers(1, true);

                for (Unit* pTarget : plagued)
                    castSpell(pTarget, SPELL_TICHONDRIUS_SEEKER_SWARM, true);
                scriptEvents.addEvent(EVENT_TICHONDRIUS_SEEKER_SWARM, 25000);
                break;
            }
            case EVENT_TICHONDRIUS_FEAST_OF_BLOOD:
                feastOfBlood();
                scriptEvents.addEvent(EVENT_TICHONDRIUS_FEAST_OF_BLOOD, 25000);
                break;
            case EVENT_TICHONDRIUS_ECHOES_OF_THE_VOID:
                warn("Tichondrius calls forth the Echoes of the Void! Hide behind a Fel Spire!");
                castSpellOnSelf(SPELL_TICHONDRIUS_ECHOES_OF_THE_VOID);
                raiseSpires();
                scriptEvents.addEvent(EVENT_TICHONDRIUS_ECHOES_OF_THE_VOID, 65000);
                break;
            case EVENT_TICHONDRIUS_BRAND_OF_ARGUS:
                castSpellAOE(SPELL_TICHONDRIUS_BRAND_OF_ARGUS);
                scriptEvents.addEvent(EVENT_TICHONDRIUS_BRAND_OF_ARGUS, mLongArgus ? 35000 : 25000);
                mLongArgus = !mLongArgus;
                break;
            case EVENT_TICHONDRIUS_GUARDS:
                warn("Tichondrius calls his guards!");
                callAdd(NPC_FELSWORN_SPELLGUARD, TichondriusGuardSpots[0]);
                callAdd(NPC_FELSWORN_SPELLGUARD, TichondriusGuardSpots[1]);
                if (mNights == 2)
                    callAdd(NPC_SIGHTLESS_WATCHER, TichondriusGuardSpots[2]);
                scriptEvents.addEvent(EVENT_TICHONDRIUS_GUARDS, 52000);
                break;
            case EVENT_TICHONDRIUS_ILLUSIONARY_NIGHT:
                scriptEvents.resetEvents();
                warn("Tichondrius shrouds the room in Illusionary Night!");
                castSpellOnSelf(SPELL_TICHONDRIUS_ILLUSIONARY_NIGHT);
                scriptEvents.addEvent(EVENT_TICHONDRIUS_NIGHT_FALLS, 100);
                break;
            case EVENT_TICHONDRIUS_NIGHT_FALLS:
                enterNight();
                break;
            case EVENT_TICHONDRIUS_NIGHT_BLOODFANGS:
                releaseBats();
                break;
            case EVENT_TICHONDRIUS_NIGHT_NIGHTMARE:
                callAdd(NPC_CARRION_NIGHTMARE, TichondriusNightmareSpots[Util::getRandomUInt(4)], TIMED_DESPAWN, 6000, false);
                scriptEvents.addEvent(EVENT_TICHONDRIUS_NIGHT_NIGHTMARE, 8000);
                break;
            case EVENT_TICHONDRIUS_NIGHT_ENDS:
                leaveNight();
                ++mNights;
                startRotation();
                break;
            default:
                break;
        }
    }

private:
    // timers start three seconds earlier after a night
    void startRotation()
    {
        uint32_t const head = mNights > 0 ? 3000 : 0;

        scriptEvents.addEvent(EVENT_TICHONDRIUS_CARRION_PLAGUE, 5000 - head);
        scriptEvents.addEvent(EVENT_TICHONDRIUS_FEAST_OF_BLOOD, 20000 - head);
        scriptEvents.addEvent(EVENT_TICHONDRIUS_SEEKER_SWARM, 25000 - head);
        scriptEvents.addEvent(EVENT_TICHONDRIUS_ECHOES_OF_THE_VOID, 55000 - head);

        if (heroicPlus())
            scriptEvents.addEvent(EVENT_TICHONDRIUS_BRAND_OF_ARGUS, 15000 - head);

        if (mNights < 2)
            scriptEvents.addEvent(EVENT_TICHONDRIUS_ILLUSIONARY_NIGHT, 125000);

        if (mNights == 1 || mNights == 2)
            scriptEvents.addEvent(EVENT_TICHONDRIUS_GUARDS, 24000);
    }

    uint32_t plagueTargets()
    {
        uint32_t const players = raidSize();
        if (players <= 15)
            return 1;

        return players <= 20 ? 2 : 3;
    }

    // three blood clots chase the tank
    void feastOfBlood()
    {
        Unit* pVictim = currentVictim();
        if (pVictim == nullptr)
            return;

        castSpell(pVictim, SPELL_TICHONDRIUS_FEAST_OF_BLOOD);
        for (uint8_t i = 0; i < 3; ++i)
        {
            Creature* pBlood = callAdd(NPC_TAINTED_BLOOD, nightholdScatter(pVictim->GetPosition(), 3.0f, 5.0f));
            if (pBlood == nullptr)
                continue;

            pBlood->castSpell(pVictim, SPELL_TAINTED_BLOOD_FIXATE, true);
            if (CreatureAIScript* pBloodAI = pBlood->GetScript())
                pBloodAI->attackStart(pVictim);
        }
    }

    // four spires on a cross around the room centre
    void raiseSpires()
    {
        float angle = std::atan2(getCreature()->GetPositionY() - TichondriusRoomCentre.y, getCreature()->GetPositionX() - TichondriusRoomCentre.x) - 0.785f;
        for (uint8_t i = 0; i < 4; ++i)
        {
            callAdd(NPC_FEL_SPIRE, nightholdRing(TichondriusRoomCentre, 20.0f, angle), TIMED_DESPAWN, 15000, false);
            angle += NightholdTwoPi / 4.0f;
        }
    }

    void enterNight()
    {
        mInNight = true;
        mBatWave = 0;

        castSpellAOE(SPELL_TICHONDRIUS_ILLUSIONARY_NIGHT_FALL, true);
        attackStop();
        setReactState(REACT_PASSIVE);
        setRooted(true);
        addUnitFlags(UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);

        scriptEvents.addEvent(EVENT_TICHONDRIUS_NIGHT_BLOODFANGS, 1000);
        scriptEvents.addEvent(EVENT_TICHONDRIUS_NIGHT_NIGHTMARE, 7000);
        scriptEvents.addEvent(EVENT_TICHONDRIUS_NIGHT_ENDS, TichondriusNightMs);
    }

    void leaveNight()
    {
        mInNight = false;
        scriptEvents.removeEvent(EVENT_TICHONDRIUS_NIGHT_BLOODFANGS);
        scriptEvents.removeEvent(EVENT_TICHONDRIUS_NIGHT_NIGHTMARE);
        summons.despawnEntry(NPC_PHANTASMAL_BLOODFANG);
        summons.despawnEntry(NPC_CARRION_NIGHTMARE);

        removeUnitFlags(UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
        setRooted(false);
        setReactState(REACT_AGGRESSIVE);
    }

    void releaseBats()
    {
        for (uint32_t i = 0; i < TichondriusBatWaveCount[mBatWave]; ++i)
        {
            if (Creature* pBat = callAdd(NPC_PHANTASMAL_BLOODFANG, nightholdScatter(TichondriusRoomCentre, 5.0f, 40.0f)))
                pBat->castSpell(pBat, SPELL_PHANTASMAL_BLOODFANG_VISUAL, true);
        }

        scriptEvents.addEvent(EVENT_TICHONDRIUS_NIGHT_BLOODFANGS, TichondriusBatWaveDelay[mBatWave]);
        mBatWave = mBatWave == 4 ? 3 : mBatWave + 1;
    }

    uint32_t mNights = 0;
    uint32_t mBatWave = 0;
    bool mInNight = false;
    bool mLongArgus = false;
};

class NightholdFelSpireAI : public NightholdCreatureAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NightholdFelSpireAI(c); }
    explicit NightholdFelSpireAI(Creature* pCreature) : NightholdCreatureAI(pCreature) {}

    void OnLoad() override
    {
        becomeStalker();
        castSpellOnSelf(SPELL_FEL_SPIRE_SHELTER, true);
    }
};

class NightholdCarrionNightmareAI : public NightholdCreatureAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NightholdCarrionNightmareAI(c); }
    explicit NightholdCarrionNightmareAI(Creature* pCreature) : NightholdCreatureAI(pCreature) { mTicksIdle = true; }

    void OnLoad() override
    {
        becomeStalker();
        scriptEvents.addEvent(EVENT_NIGHTMARE_BEAM, 1000);
    }

protected:
    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_NIGHTMARE_BEAM)
            return;

        if (Unit* pTarget = closestPlayer())
        {
            getCreature()->setFacingToObject(pTarget);
            castSpell(pTarget, SPELL_CARRION_NIGHTMARE);
        }
    }
};

class NightholdPhantasmalBloodfangAI : public NightholdCreatureAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NightholdPhantasmalBloodfangAI(c); }
    explicit NightholdPhantasmalBloodfangAI(Creature* pCreature) : NightholdCreatureAI(pCreature) {}

    void OnLoad() override { castSpellOnSelf(SPELL_PHANTASMAL_BLOODFANG_VISUAL, true); }
};

class NightholdFelswornSpellguardAI : public NightholdCreatureAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NightholdFelswornSpellguardAI(c); }
    explicit NightholdFelswornSpellguardAI(Creature* pCreature) : NightholdCreatureAI(pCreature) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_SPELLGUARD_VOLATILE_WOUND, 4000);
        scriptEvents.addEvent(EVENT_SPELLGUARD_RAPID_PURSUIT, 4500);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SPELLGUARD_VOLATILE_WOUND:
                castSpellOnVictim(SPELL_SPELLGUARD_VOLATILE_WOUND, true);
                scriptEvents.addEvent(EVENT_SPELLGUARD_VOLATILE_WOUND, 4000);
                break;
            case EVENT_SPELLGUARD_RAPID_PURSUIT:
                castSpellOnSelf(SPELL_SPELLGUARD_RAPID_PURSUIT, true);
                scriptEvents.addEvent(EVENT_SPELLGUARD_RAPID_PURSUIT, 6000);
                break;
            default:
                break;
        }
    }
};

class NightholdSightlessWatcherAI : public NightholdCreatureAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NightholdSightlessWatcherAI(c); }
    explicit NightholdSightlessWatcherAI(Creature* pCreature) : NightholdCreatureAI(pCreature) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_WATCHER_BURNING_SOUL, 9000);
        if (heroicPlus())
            scriptEvents.addEvent(EVENT_WATCHER_RING_OF_SHADOWS, 11000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_WATCHER_BURNING_SOUL:
                castSpellAOE(SPELL_WATCHER_BURNING_SOUL);
                scriptEvents.addEvent(EVENT_WATCHER_BURNING_SOUL, 14000);
                break;
            case EVENT_WATCHER_RING_OF_SHADOWS:
                castSpellOnSelf(SPELL_WATCHER_RING_OF_SHADOWS);
                scriptEvents.addEvent(EVENT_WATCHER_RING_OF_SHADOWS, 24000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Star Augur Etraeus

enum class EtraeusSky : uint8_t
{
    Stars,
    Ice,
    Fel,
    Void
};

class NightholdEtraeusAI : public NightholdBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NightholdEtraeusAI(c); }
    explicit NightholdEtraeusAI(Creature* pCreature) : NightholdBossAI(pCreature, DATA_STAR_AUGUR_ETRAEUS) {}

    // floats over his platform
    void OnLoad() override
    {
        setRooted(true);
        castSpellOnSelf(SPELL_ETRAEUS_PRE_COMBAT_VISUAL, true);
    }

protected:
    void onEngage() override
    {
        _removeAura(SPELL_ETRAEUS_PRE_COMBAT_VISUAL);
        setRooted(true);
        mSky = EtraeusSky::Stars;
        mNextShift = 90;
        beginSky();
    }

    void onDisengage() override { castSpellOnSelf(SPELL_ETRAEUS_PRE_COMBAT_VISUAL, true); }

    void onTick(uint32_t /*diff*/) override
    {
        if (mNextShift == 0 || _getHealthPercent() > mNextShift)
            return;

        // a new sky every thirty percent
        mNextShift = mNextShift > 30 ? mNextShift - 30 : 0;
        mSky = static_cast<EtraeusSky>(static_cast<uint8_t>(mSky) + 1);

        scriptEvents.resetEvents();
        switch (mSky)
        {
            case EtraeusSky::Ice:
                warn("Star Augur Etraeus draws the cold of an icy world!");
                break;
            case EtraeusSky::Fel:
                warn("Star Augur Etraeus channels the fel of Argus!");
                break;
            default:
                warn("Star Augur Etraeus opens the way to the Void!");
                break;
        }

        castSpellOnSelf(SPELL_ETRAEUS_NETHER_TRAVERSAL);
        scriptEvents.addEvent(EVENT_ETRAEUS_PHASE_BEGIN, 100);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ETRAEUS_PHASE_BEGIN:
                beginSky();
                break;
            case EVENT_ETRAEUS_BURST:
                burst();
                break;
            case EVENT_ETRAEUS_GRAVITATIONAL_PULL:
                castSpellOnVictim(SPELL_ETRAEUS_GRAVITATIONAL_PULL);
                scriptEvents.addEvent(EVENT_ETRAEUS_GRAVITATIONAL_PULL, mSky == EtraeusSky::Ice ? 28000 : mSky == EtraeusSky::Fel ? 30000 : 65000);
                break;
            case EVENT_ETRAEUS_CORONAL_EJECTION:
                launchEjections();
                scriptEvents.addEvent(EVENT_ETRAEUS_CORONAL_EJECTION, mSky == EtraeusSky::Stars ? Util::getRandomUInt(14000, 18000) : mEjections.next());
                break;
            case EVENT_ETRAEUS_NOVA:
                nova();
                break;
            case EVENT_ETRAEUS_BIG_BANG:
                warn("Star Augur Etraeus unleashes the Big Bang!");
                castSpellOnSelf(SPELL_ETRAEUS_BIG_BANG);
                break;
            default:
                break;
        }
    }

private:
    void beginSky()
    {
        bool const heroic = heroicPlus();
        scriptEvents.addEvent(EVENT_ETRAEUS_BURST, 2000);

        switch (mSky)
        {
            case EtraeusSky::Stars:
                scriptEvents.addEvent(EVENT_ETRAEUS_CORONAL_EJECTION, 7000);
                break;
            case EtraeusSky::Ice:
                mEjections.start(EtraeusIceEjections, NightholdCadence::Mode::Cycle);
                scriptEvents.addEvent(EVENT_ETRAEUS_GRAVITATIONAL_PULL, 21000);
                scriptEvents.addEvent(EVENT_ETRAEUS_CORONAL_EJECTION, 12000);
                if (heroic)
                    scriptEvents.addEvent(EVENT_ETRAEUS_NOVA, 40000);
                break;
            case EtraeusSky::Fel:
                mEjections.start(EtraeusFelEjections, NightholdCadence::Mode::Cycle);
                scriptEvents.addEvent(EVENT_ETRAEUS_GRAVITATIONAL_PULL, 30000);
                scriptEvents.addEvent(EVENT_ETRAEUS_CORONAL_EJECTION, 7000);
                if (heroic)
                    scriptEvents.addEvent(EVENT_ETRAEUS_NOVA, 60000);
                break;
            case EtraeusSky::Void:
                mEjections.start(EtraeusVoidEjections, NightholdCadence::Mode::Cycle);
                scriptEvents.addEvent(EVENT_ETRAEUS_GRAVITATIONAL_PULL, 30000);
                scriptEvents.addEvent(EVENT_ETRAEUS_CORONAL_EJECTION, 6000);
                scriptEvents.addEvent(EVENT_ETRAEUS_BIG_BANG, 180000);
                if (heroic)
                    scriptEvents.addEvent(EVENT_ETRAEUS_NOVA, 60000);
                callAdd(NPC_THING_THAT_SHOULD_NOT_BE, nightholdScatter(getCreature()->GetPosition(), 8.0f, 12.0f));
                break;
        }
    }

    void burst()
    {
        switch (mSky)
        {
            case EtraeusSky::Stars:
                castSpellOnVictim(SPELL_ETRAEUS_STARBURST);
                scriptEvents.addEvent(EVENT_ETRAEUS_BURST, 2000);
                break;
            case EtraeusSky::Ice:
                castSpellOnVictim(SPELL_ETRAEUS_ICEBURST);
                scriptEvents.addEvent(EVENT_ETRAEUS_BURST, 2000);
                break;
            case EtraeusSky::Fel:
                castSpellOnVictim(SPELL_ETRAEUS_FELBURST);
                scriptEvents.addEvent(EVENT_ETRAEUS_BURST, 2000);
                break;
            case EtraeusSky::Void:
                castOn(pickPlayer(false), SPELL_ETRAEUS_VOIDBURST);
                scriptEvents.addEvent(EVENT_ETRAEUS_BURST, 2500);
                break;
        }
    }

    void nova()
    {
        switch (mSky)
        {
            case EtraeusSky::Ice:
                warn("Star Augur Etraeus casts Frigid Nova!");
                castSpellOnSelf(SPELL_ETRAEUS_FRIGID_NOVA);
                scriptEvents.addEvent(EVENT_ETRAEUS_NOVA, 60000);
                break;
            case EtraeusSky::Fel:
                warn("Star Augur Etraeus casts Fel Nova!");
                castSpellOnSelf(SPELL_ETRAEUS_FEL_NOVA);
                scriptEvents.addEvent(EVENT_ETRAEUS_NOVA, 50000);
                break;
            case EtraeusSky::Void:
                warn("Star Augur Etraeus casts Void Nova!");
                castSpellOnSelf(SPELL_ETRAEUS_VOID_NOVA);
                summons.despawnEntry(NPC_CORONAL_EJECTION);
                scriptEvents.addEvent(EVENT_ETRAEUS_NOVA, 50000);
                break;
            default:
                break;
        }
    }

    // ejections erupt under two players with the current sky
    void launchEjections()
    {
        uint32_t spellId = SPELL_CORONAL_EJECTION_BLAST;
        uint32_t lifetime = 4500;
        switch (mSky)
        {
            case EtraeusSky::Ice:
                spellId = SPELL_CORONAL_EJECTION_ICY;
                break;
            case EtraeusSky::Fel:
                spellId = SPELL_CORONAL_EJECTION_FEL;
                break;
            case EtraeusSky::Void:
                spellId = SPELL_CORONAL_EJECTION_VOID;
                lifetime = 30000;
                break;
            default:
                break;
        }

        for (Unit* pTarget : pickPlayers(2, true))
        {
            Creature* pEjection = callAdd(NPC_CORONAL_EJECTION, pTarget->GetPosition(), TIMED_DESPAWN, lifetime, false);
            if (pEjection != nullptr && pEjection->GetScript() != nullptr)
                pEjection->GetScript()->SetCreatureData(NightholdEjectionKit, spellId);
        }
    }

    NightholdCadence mEjections;
    EtraeusSky mSky = EtraeusSky::Stars;
    int32_t mNextShift = 90;
};

class NightholdCoronalEjectionAI : public NightholdCreatureAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NightholdCoronalEjectionAI(c); }
    explicit NightholdCoronalEjectionAI(Creature* pCreature) : NightholdCreatureAI(pCreature) { mTicksIdle = true; }

    void OnLoad() override { becomeStalker(); }

    void SetCreatureData(uint32_t type, uint32_t data) override
    {
        if (type != NightholdEjectionKit)
            return;

        mEruption = data;
        scriptEvents.addEvent(EVENT_EJECTION_ERUPT, 2000);
    }

protected:
    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_EJECTION_ERUPT || mEruption == 0)
            return;

        castSpellOnSelf(mEruption, true);

        // void ejections keep erupting
        if (mEruption == SPELL_CORONAL_EJECTION_VOID)
            scriptEvents.addEvent(EVENT_EJECTION_ERUPT, 11000);
    }

private:
    uint32_t mEruption = 0;
};

class NightholdThingThatShouldNotBeAI : public NightholdCreatureAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NightholdThingThatShouldNotBeAI(c); }
    explicit NightholdThingThatShouldNotBeAI(Creature* pCreature) : NightholdCreatureAI(pCreature) {}

    void OnLoad() override { castSpellOnSelf(SPELL_THING_VOID_SHIFT, true); }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
    }

protected:
    void onEngage() override
    {
        if (InstanceScript* pInstance = getInstanceScript())
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());

        scriptEvents.addEvent(EVENT_THING_WITNESS_THE_VOID, 10000);
    }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_THING_WITNESS_THE_VOID)
            return;

        warn("The Thing That Should Not Be forces the raid to Witness the Void! Look away!");
        castSpellAOE(SPELL_THING_WITNESS_THE_VOID);
        scriptEvents.addEvent(EVENT_THING_WITNESS_THE_VOID, 10000);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Chronomatic Anomaly

class NightholdChronomaticAnomalyAI : public NightholdBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NightholdChronomaticAnomalyAI(c); }
    explicit NightholdChronomaticAnomalyAI(Creature* pCreature) : NightholdBossAI(pCreature, DATA_CHRONOMATIC_ANOMALY) {}

    // a slain particle leaves a rift that cuts Power Overwhelming short
    void OnSummonDies(Creature* pSummon, Unit* /*pKiller*/) override
    {
        if (pSummon == nullptr || pSummon->getEntry() != NPC_WANING_TIME_PARTICLE || !_isInCombat())
            return;

        mRiftOpen = true;
        warn("The Waning Time Particle collapses into a Temporal Rift!");

        if (mOverwhelming)
            scriptEvents.addEvent(EVENT_ANOMALY_TIME_RESET, 3000);
    }

protected:
    void onEngage() override
    {
        mSpeed = 0;
        mRiftOpen = false;
        mOverwhelming = false;

        castSpellOnSelf(SPELL_ANOMALY_PASSAGE_OF_TIME, true);
        applySpeed();
        scriptEvents.addEvent(EVENT_ANOMALY_BURST_OF_TIME, heroicPlus() ? 8000 : 14000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ANOMALY_BURST_OF_TIME:
                castOn(pickPlayer(false), AnomalyBurstSpells[mSpeed], true);
                scriptEvents.addEvent(EVENT_ANOMALY_BURST_OF_TIME, heroicPlus() ? 4000 : 5000);
                break;
            case EVENT_ANOMALY_CHRONOMETRIC_PARTICLES:
                castSpellOnVictim(SPELL_ANOMALY_CHRONOMETRIC_PARTICLES);
                scriptEvents.addEvent(EVENT_ANOMALY_CHRONOMETRIC_PARTICLES, 6000);
                break;
            case EVENT_ANOMALY_TIME_RELEASE:
            {
                uint32_t const targets = tempo().releaseTargets;
                for (Unit* pTarget : pickPlayers(targets != 0 ? targets : 40, false))
                    castSpell(pTarget, SPELL_ANOMALY_TIME_RELEASE, true);
                break;
            }
            case EVENT_ANOMALY_TIME_BOMB:
                castOn(pickPlayer(true), SPELL_ANOMALY_TIME_BOMB, true);
                break;
            case EVENT_ANOMALY_WANING_PARTICLE:
                warn("A Waning Time Particle drifts toward the Nightwell!");
                callAdd(NPC_WANING_TIME_PARTICLE, AnomalyParticleSpots[Util::getRandomUInt(11)]);
                break;
            case EVENT_ANOMALY_TEMPORAL_ORBS:
                castSpellOnSelf(SPELL_ANOMALY_TEMPORAL_ORBS);
                break;
            case EVENT_ANOMALY_POWER_OVERWHELMING:
                stopTempo();
                mOverwhelming = true;
                warn("Chronomatic Anomaly begins to channel Power Overwhelming!");
                castSpellOnSelf(SPELL_ANOMALY_POWER_OVERWHELMING);
                scriptEvents.addEvent(EVENT_ANOMALY_TIME_RESET, mRiftOpen ? 5000 : 20000);
                break;
            case EVENT_ANOMALY_TIME_RESET:
                _removeAura(SPELL_ANOMALY_POWER_OVERWHELMING);
                mOverwhelming = false;
                mRiftOpen = false;
                mSpeed = (mSpeed + 1) % 3;
                applySpeed();
                break;
            default:
                break;
        }
    }

private:
    AnomalyTempo const& tempo() { return heroicPlus() ? AnomalyTempoHeroic[mSpeed] : AnomalyTempoNormal[mSpeed]; }

    // normal, slow and fast in turn
    void applySpeed()
    {
        for (uint32_t aura : AnomalySpeedAuras)
            _removeAura(aura);

        castSpellOnSelf(AnomalySpeedAuras[mSpeed], true);

        switch (mSpeed)
        {
            case 0:
                warn("Time returns to its normal pace.");
                break;
            case 1:
                warn("Time slows down.");
                break;
            default:
                warn("Time speeds up!");
                break;
        }

        AnomalyTempo const& pace = tempo();
        scriptEvents.addEvent(EVENT_ANOMALY_CHRONOMETRIC_PARTICLES, 10000);
        if (pace.timeRelease != 0)
            scriptEvents.addEvent(EVENT_ANOMALY_TIME_RELEASE, pace.timeRelease);
        if (pace.timeBomb != 0)
            scriptEvents.addEvent(EVENT_ANOMALY_TIME_BOMB, pace.timeBomb);
        if (pace.particle != 0)
            scriptEvents.addEvent(EVENT_ANOMALY_WANING_PARTICLE, pace.particle);
        if (pace.temporalOrbs != 0)
            scriptEvents.addEvent(EVENT_ANOMALY_TEMPORAL_ORBS, pace.temporalOrbs);
        scriptEvents.addEvent(EVENT_ANOMALY_POWER_OVERWHELMING, pace.powerOverwhelming);
    }

    void stopTempo()
    {
        scriptEvents.removeEvent(EVENT_ANOMALY_CHRONOMETRIC_PARTICLES);
        scriptEvents.removeEvent(EVENT_ANOMALY_TIME_RELEASE);
        scriptEvents.removeEvent(EVENT_ANOMALY_TIME_BOMB);
        scriptEvents.removeEvent(EVENT_ANOMALY_WANING_PARTICLE);
        scriptEvents.removeEvent(EVENT_ANOMALY_TEMPORAL_ORBS);
    }

    uint32_t mSpeed = 0;
    bool mRiftOpen = false;
    bool mOverwhelming = false;
};

class NightholdWaningTimeParticleAI : public NightholdCreatureAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NightholdWaningTimeParticleAI(c); }
    explicit NightholdWaningTimeParticleAI(Creature* pCreature) : NightholdCreatureAI(pCreature) {}

    void OnLoad() override { setRooted(true); }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
    }

protected:
    void onEngage() override
    {
        if (InstanceScript* pInstance = getInstanceScript())
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());

        scriptEvents.addEvent(EVENT_PARTICLE_WARP_NIGHTWELL, 2000);
    }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_PARTICLE_WARP_NIGHTWELL)
            return;

        castSpellAOE(SPELL_WANING_PARTICLE_WARP_NIGHTWELL);
        scriptEvents.addEvent(EVENT_PARTICLE_WARP_NIGHTWELL, 5000);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Gul'dan

enum class GuldanStage : uint8_t
{
    Council,
    Shielded,
    Empowered,
    Souls
};

class NightholdGuldanAI : public NightholdBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NightholdGuldanAI(c); }
    explicit NightholdGuldanAI(Creature* pCreature) : NightholdBossAI(pCreature, DATA_GULDAN) {}

    // the Eye of Aman'thul holds him until the council falls
    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage != nullptr && mShielded)
            *damage = 0;
    }

    void OnSummonDies(Creature* pSummon, Unit* /*pKiller*/) override
    {
        if (pSummon == nullptr || !_isInCombat() || (mStage != GuldanStage::Council && mStage != GuldanStage::Shielded))
            return;

        uint32_t const entry = pSummon->getEntry();
        if (entry != NPC_FEL_LORD_KURAZMAL && entry != NPC_INQUISITOR_VETHRIZ && entry != NPC_DZORYKX_THE_TRAPPER)
            return;

        if (++mCouncilDown >= 3)
            beginEmpowered();
    }

    // splitting eyes are owned by their master
    void spawnEye(uint32_t entry, LocationVector const& where, uint32_t health)
    {
        Creature* pEye = callAdd(entry, where);
        if (pEye != nullptr && health != 0 && health < pEye->getMaxHealth())
            pEye->setHealth(health);
    }

protected:
    void onEngage() override
    {
        bool const heroic = heroicPlus();

        mStage = GuldanStage::Council;
        mCouncilDown = 0;
        mHands = 0;
        mShielded = false;
        mScythe = false;
        mScytheEnergy = 0;
        mScytheClock = 0;
        mEyeCasts = 0;
        mFlames = 0;
        for (bool& empowered : mEmpowered)
            empowered = false;

        setRooted(true);
        armBerserk(720000);

        mEfflux.start(heroic ? GuldanEffluxHeroic : GuldanEffluxNormal, NightholdCadence::Mode::HoldLast);
        mHellfire.start(heroic ? GuldanHellfireHeroic : GuldanHellfireNormal, NightholdCadence::Mode::HoldLast);

        scriptEvents.addEvent(EVENT_GULDAN_LIQUID_HELLFIRE, 2000);
        scriptEvents.addEvent(EVENT_GULDAN_HAND_OF_GULDAN, 6000);
        scriptEvents.addEvent(EVENT_GULDAN_FEL_EFFLUX, mEfflux.next());
    }

    // free to walk home, the pull roots him again
    void onDisengage() override
    {
        mShielded = false;
        mScythe = false;
        setRooted(false);
    }

    void onBerserk() override
    {
        warn("Gul'dan goes berserk!");
        castSpellOnSelf(SPELL_GULDAN_BERSERK, true);
    }

    void onTick(uint32_t diff) override
    {
        // Fel Scythe fills two or three energy a second
        if (mScythe)
        {
            mScytheClock += diff;
            while (mScytheClock >= 1000)
            {
                mScytheClock -= 1000;
                mScytheEnergy += Util::getRandomUInt(2, 3);
            }

            if (mScytheEnergy >= 100)
            {
                mScytheEnergy = 0;
                scriptEvents.addEvent(EVENT_GULDAN_FEL_SCYTHE, 100);
            }
        }

        if (mStage != GuldanStage::Empowered && mStage != GuldanStage::Souls)
            return;

        int32_t const health = _getHealthPercent();
        for (uint8_t i = 0; i < 3; ++i)
        {
            if (mEmpowered[i] || health > GuldanEmpowerHealth[i])
                continue;

            mEmpowered[i] = true;
            switch (i)
            {
                case 0:
                    warn("Gul'dan empowers his Bonds of Fel!");
                    break;
                case 1:
                    warn("Gul'dan empowers his Liquid Hellfire!");
                    break;
                default:
                    warn("Gul'dan empowers his Eyes!");
                    break;
            }
        }

        if (mStage == GuldanStage::Empowered && !mShielded && health <= GuldanSoulsHealth)
            beginSouls();
    }

    void onEvent(uint32_t eventId) override
    {
        bool const heroic = heroicPlus();

        switch (eventId)
        {
            case EVENT_GULDAN_LIQUID_HELLFIRE:
                liquidHellfire();
                break;
            case EVENT_GULDAN_HAND_OF_GULDAN:
                handOfGuldan();
                break;
            case EVENT_GULDAN_EYE_OF_AMANTHUL:
                warn("The Eye of Aman'thul shields Gul'dan! Defeat his council!");
                castSpellOnSelf(SPELL_GULDAN_EYE_OF_AMANTHUL, true);
                mShielded = true;
                mStage = GuldanStage::Shielded;
                scriptEvents.addEvent(EVENT_GULDAN_LIQUID_HELLFIRE, 5000);
                scriptEvents.addEvent(EVENT_GULDAN_FEL_EFFLUX, 11000);
                break;
            case EVENT_GULDAN_FEL_EFFLUX:
                if (mStage != GuldanStage::Council && mStage != GuldanStage::Shielded)
                    break;
                if (Unit* pTarget = pickPlayer(false))
                    getCreature()->setFacingToObject(pTarget);
                castSpellOnSelf(SPELL_GULDAN_FEL_EFFLUX);
                scriptEvents.addEvent(EVENT_GULDAN_FEL_EFFLUX, mEfflux.next());
                break;
            case EVENT_GULDAN_SHIELD_FADES:
                castSpellOnSelf(SPELL_GULDAN_EYE_OF_AMANTHUL_FADE, true);
                _removeAura(SPELL_GULDAN_EYE_OF_AMANTHUL);
                mShielded = false;
                scriptEvents.addEvent(EVENT_GULDAN_UNLEASHED, 6000);
                break;
            case EVENT_GULDAN_UNLEASHED:
                warn("Gul'dan steps down to face the raid!");
                setRooted(false);
                mScythe = true;
                break;
            case EVENT_GULDAN_BONDS_OF_FEL:
                bondsOfFel();
                scriptEvents.addEvent(EVENT_GULDAN_BONDS_OF_FEL, heroic ? 43000 : 49000);
                break;
            case EVENT_GULDAN_EYE_OF_GULDAN:
                eyeOfGuldan();
                break;
            case EVENT_GULDAN_FEL_SCYTHE:
                castSpellOnVictim(SPELL_GULDAN_FEL_SCYTHE);
                castSpellOnSelf(SPELL_GULDAN_FURY_OF_THE_FEL, true);
                break;
            case EVENT_GULDAN_WELL_OF_SOULS:
                castAt(GuldanRoomCentre, SPELL_GULDAN_WELL_OF_SOULS);
                setRooted(false);
                break;
            case EVENT_GULDAN_BLACK_HARVEST:
                warn("Gul'dan begins to reap a Black Harvest!");
                castSpellOnSelf(SPELL_GULDAN_BLACK_HARVEST);
                scriptEvents.addEvent(EVENT_GULDAN_SOUL_SIPHON, Util::getRandomUInt(25000, 30000));
                scriptEvents.addEvent(EVENT_GULDAN_BLACK_HARVEST, mHarvest.next());
                break;
            case EVENT_GULDAN_FLAMES_OF_SARGERAS:
            {
                // one, two (three on heroic) marks in turn
                castSpellOnSelf(SPELL_GULDAN_FLAMES_OF_SARGERAS);
                uint32_t const marks = (mFlames % (heroic ? 3 : 2)) + 1;
                for (Unit* pTarget : pickPlayers(marks, false))
                    castSpell(pTarget, SPELL_GULDAN_FLAMES_OF_SARGERAS_MARK, true);
                ++mFlames;
                scriptEvents.addEvent(EVENT_GULDAN_FLAMES_OF_SARGERAS, heroic ? 8000 : 19000);
                break;
            }
            case EVENT_GULDAN_STORM_OF_THE_DESTROYER:
                warn("Gul'dan calls the Storm of the Destroyer!");
                castSpellOnSelf(SPELL_GULDAN_STORM_OF_THE_DESTROYER);
                scriptEvents.addEvent(EVENT_GULDAN_STORM_OF_THE_DESTROYER, mStorm.next());
                break;
            case EVENT_GULDAN_SOUL_SIPHON:
                for (Unit* pTarget : pickPlayers(Util::getRandomUInt(3, 4), false))
                {
                    castSpell(pTarget, SPELL_GULDAN_SOUL_SIPHON, true);
                    castSpell(pTarget, SPELL_GULDAN_SOUL_SIPHON_DRAIN, true);
                }
                break;
            default:
                break;
        }
    }

private:
    void liquidHellfire()
    {
        warn("Gul'dan hurls Liquid Hellfire!");
        bool const empowered = mEmpowered[1];
        castSpellOnSelf(empowered ? SPELL_GULDAN_EMPOWERED_LIQUID_HELLFIRE : SPELL_GULDAN_LIQUID_HELLFIRE);
        if (Unit* pTarget = rangedPlayer(15.0f))
            castAt(pTarget->GetPosition(), empowered ? SPELL_GULDAN_EMPOWERED_HELLFIRE_MISSILE : SPELL_GULDAN_LIQUID_HELLFIRE_MISSILE);

        switch (mStage)
        {
            case GuldanStage::Council:
                scriptEvents.addEvent(EVENT_GULDAN_LIQUID_HELLFIRE, 15000);
                break;
            case GuldanStage::Shielded:
                scriptEvents.addEvent(EVENT_GULDAN_LIQUID_HELLFIRE, heroicPlus() ? 23800 : 32000);
                break;
            case GuldanStage::Empowered:
                scriptEvents.addEvent(EVENT_GULDAN_LIQUID_HELLFIRE, mHellfire.next());
                break;
            default:
                break;
        }
    }

    // the council arrives one by one, heroic calls it back later on
    void handOfGuldan()
    {
        uint32_t const entry = GuldanCouncil[mHands % 3];
        ++mHands;

        warn("Gul'dan summons a demon with the Hand of Gul'dan!");
        castSpellOnSelf(SPELL_GULDAN_HAND_OF_GULDAN, true);
        callAdd(entry, nightholdScatter(getCreature()->GetPosition(), 12.0f, 18.0f));

        if (mStage == GuldanStage::Council)
        {
            if (mHands == 1)
                scriptEvents.addEvent(EVENT_GULDAN_HAND_OF_GULDAN, 15000);
            else if (mHands == 2)
                scriptEvents.addEvent(EVENT_GULDAN_HAND_OF_GULDAN, 10000);
            else
                scriptEvents.addEvent(EVENT_GULDAN_EYE_OF_AMANTHUL, 3000);
            return;
        }

        if (mStage == GuldanStage::Empowered)
        {
            if (mHands == 4)
                scriptEvents.addEvent(EVENT_GULDAN_HAND_OF_GULDAN, 48000);
            else if (mHands == 5)
                scriptEvents.addEvent(EVENT_GULDAN_HAND_OF_GULDAN, 138800);
        }
    }

    void bondsOfFel()
    {
        bool const empowered = mEmpowered[0];
        castSpellOnSelf(empowered ? SPELL_GULDAN_EMPOWERED_BONDS_OF_FEL : SPELL_GULDAN_BONDS_OF_FEL);
        for (Unit* pTarget : pickPlayers(heroicPlus() ? 3 : 2, true))
            castSpell(pTarget, empowered ? SPELL_GULDAN_EMPOWERED_BONDS_OF_FEL_HIT : SPELL_GULDAN_BONDS_OF_FEL_HIT, true);
    }

    // three eyes, or one empowered eye, open next to the tank
    void eyeOfGuldan()
    {
        bool const empowered = mEmpowered[2];
        castSpellOnSelf(empowered ? SPELL_GULDAN_EMPOWERED_EYE_OF_GULDAN : SPELL_GULDAN_EYE_OF_GULDAN);

        Unit* pAnchor = currentVictim();
        LocationVector const origin = pAnchor != nullptr ? pAnchor->GetPosition() : getCreature()->GetPosition();
        uint32_t const count = empowered ? 1 : 3;
        for (uint32_t i = 0; i < count; ++i)
            spawnEye(empowered ? NPC_EMPOWERED_EYE_OF_GULDAN : NPC_EYE_OF_GULDAN, nightholdScatter(origin, 2.0f, 7.0f), 0);

        if (mStage == GuldanStage::Souls)
        {
            scriptEvents.addEvent(EVENT_GULDAN_EYE_OF_GULDAN, mSoulEyes.next());
            return;
        }

        bool const heroic = heroicPlus();
        ++mEyeCasts;
        uint32_t const longWait = heroic ? 80000 : 75000;
        uint32_t const shortWait = heroic ? 48000 : 60000;
        scriptEvents.addEvent(EVENT_GULDAN_EYE_OF_GULDAN, mEyeCasts == 5 ? longWait : shortWait);
    }

    void beginEmpowered()
    {
        bool const heroic = heroicPlus();

        mStage = GuldanStage::Empowered;
        mEyeCasts = 0;
        scriptEvents.resetEvents();
        warn("The council has fallen. The Eye of Aman'thul begins to crack!");

        scriptEvents.addEvent(EVENT_GULDAN_SHIELD_FADES, 13000);
        scriptEvents.addEvent(EVENT_GULDAN_BONDS_OF_FEL, 25000);
        scriptEvents.addEvent(EVENT_GULDAN_EYE_OF_GULDAN, heroic ? 48000 : 50600);
        scriptEvents.addEvent(EVENT_GULDAN_LIQUID_HELLFIRE, mHellfire.next());
        if (heroic)
            scriptEvents.addEvent(EVENT_GULDAN_HAND_OF_GULDAN, 33000);
    }

    void beginSouls()
    {
        bool const heroic = heroicPlus();

        mStage = GuldanStage::Souls;
        scriptEvents.resetEvents();
        setRooted(true);
        warn("Gul'dan shatters the Eye of Aman'thul!");
        castSpellOnSelf(SPELL_GULDAN_EYE_OF_AMANTHUL_SHATTER, true);

        mSoulEyes.start(heroic ? GuldanSoulEyesHeroic : GuldanSoulEyesNormal, NightholdCadence::Mode::HoldLast);
        mHarvest.start(heroic ? GuldanHarvestHeroic : GuldanHarvestNormal, NightholdCadence::Mode::HoldLast);
        mStorm.start(heroic ? GuldanStormHeroic : GuldanStormNormal, NightholdCadence::Mode::HoldLast);

        scriptEvents.addEvent(EVENT_GULDAN_WELL_OF_SOULS, 14000);
        scriptEvents.addEvent(EVENT_GULDAN_SOUL_SIPHON, Util::getRandomUInt(25000, 30000));
        scriptEvents.addEvent(EVENT_GULDAN_FLAMES_OF_SARGERAS, heroic ? 27500 : 29000);
        scriptEvents.addEvent(EVENT_GULDAN_EYE_OF_GULDAN, heroic ? 39100 : 42500);
        scriptEvents.addEvent(EVENT_GULDAN_BLACK_HARVEST, heroic ? 64100 : 63000);
        scriptEvents.addEvent(EVENT_GULDAN_STORM_OF_THE_DESTROYER, heroic ? 84100 : 94000);
    }

    NightholdCadence mEfflux;
    NightholdCadence mHellfire;
    NightholdCadence mSoulEyes;
    NightholdCadence mHarvest;
    NightholdCadence mStorm;
    GuldanStage mStage = GuldanStage::Council;
    bool mEmpowered[3] = { false, false, false };
    uint32_t mCouncilDown = 0;
    uint32_t mHands = 0;
    uint32_t mScytheEnergy = 0;
    uint32_t mScytheClock = 0;
    uint32_t mEyeCasts = 0;
    uint32_t mFlames = 0;
    bool mShielded = false;
    bool mScythe = false;
};

class NightholdKurazmalAI : public NightholdCreatureAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NightholdKurazmalAI(c); }
    explicit NightholdKurazmalAI(Creature* pCreature) : NightholdCreatureAI(pCreature) {}

protected:
    void onEngage() override
    {
        castSpellOnSelf(SPELL_KURAZMAL_SHATTER_ESSENCE_AURA, true);
        scriptEvents.addEvent(EVENT_KURAZMAL_SHATTER_ESSENCE, 19000);
        if (heroicPlus())
            scriptEvents.addEvent(EVENT_KURAZMAL_FEL_OBELISK, 23000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_KURAZMAL_SHATTER_ESSENCE:
                warn("Fel Lord Kuraz'mal prepares to Shatter Essence!");
                castSpellOnVictim(SPELL_KURAZMAL_SHATTER_ESSENCE);
                scriptEvents.addEvent(EVENT_KURAZMAL_SHATTER_ESSENCE, 54000);
                break;
            case EVENT_KURAZMAL_FEL_OBELISK:
                for (Unit* pTarget : pickPlayers(3, false))
                    castAt(pTarget->GetPosition(), SPELL_KURAZMAL_FEL_OBELISK);
                scriptEvents.addEvent(EVENT_KURAZMAL_FEL_OBELISK, 23000);
                break;
            default:
                break;
        }
    }
};

class NightholdVethrizAI : public NightholdCreatureAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NightholdVethrizAI(c); }
    explicit NightholdVethrizAI(Creature* pCreature) : NightholdCreatureAI(pCreature) {}

    // blinks instead of walking
    void OnLoad() override { setRooted(true); }

protected:
    void onEngage() override
    {
        mEnergy = 0;
        mEnergyClock = 0;
        mBlasts = 0;
        scriptEvents.addEvent(EVENT_VETHRIZ_DARK_BLAST, 3000);
        scriptEvents.addEvent(EVENT_VETHRIZ_GAZE, 30000);
    }

    void onTick(uint32_t diff) override
    {
        mEnergyClock += diff;
        while (mEnergyClock >= 1000)
        {
            mEnergyClock -= 1000;
            mEnergy = std::min<uint32_t>(100, mEnergy + Util::getRandomUInt(3, 5));
        }
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_VETHRIZ_DARK_BLAST:
                if (mEnergy >= 100)
                {
                    mEnergy = 0;
                    castAt(nightholdScatter(GuldanRoomCentre, 0.0f, 20.0f), SPELL_VETHRIZ_SHADOWBLINK);
                }
                else if (heroicPlus() && mBlasts > 5)
                {
                    castSpellOnVictim(SPELL_VETHRIZ_DRAIN);
                }
                else
                {
                    castSpellOnVictim(SPELL_VETHRIZ_DARK_BLAST);
                    ++mBlasts;
                }
                scriptEvents.addEvent(EVENT_VETHRIZ_DARK_BLAST, 3000);
                break;
            case EVENT_VETHRIZ_GAZE:
                warn("Inquisitor Vethriz turns his Gaze on the raid!");
                castSpellOnSelf(SPELL_VETHRIZ_GAZE_OF_VETHRIZ);
                scriptEvents.addEvent(EVENT_VETHRIZ_GAZE, 30000);
                break;
            default:
                break;
        }
    }

private:
    uint32_t mEnergy = 0;
    uint32_t mEnergyClock = 0;
    uint32_t mBlasts = 0;
};

class NightholdDzorykxAI : public NightholdCreatureAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NightholdDzorykxAI(c); }
    explicit NightholdDzorykxAI(Creature* pCreature) : NightholdCreatureAI(pCreature) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_DZORYKX_SOUL_VORTEX, 35000);
        if (heroicPlus())
            scriptEvents.addEvent(EVENT_DZORYKX_ANGUISHED_SPIRITS, 19000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_DZORYKX_SOUL_VORTEX:
                warn("D'zorykx the Trapper opens a Soul Vortex!");
                castSpellOnSelf(SPELL_DZORYKX_SOUL_VORTEX_AURA, true);
                castSpellOnVictim(SPELL_DZORYKX_SOUL_VORTEX);
                scriptEvents.addEvent(EVENT_DZORYKX_SOUL_VORTEX, 21000);
                break;
            case EVENT_DZORYKX_ANGUISHED_SPIRITS:
                for (Unit* pTarget : pickPlayers(2, false))
                    castAt(pTarget->GetPosition(), SPELL_DZORYKX_ANGUISHED_SPIRITS);
                scriptEvents.addEvent(EVENT_DZORYKX_ANGUISHED_SPIRITS, 26000);
                break;
            default:
                break;
        }
    }
};

class NightholdEyeOfGuldanAI : public NightholdCreatureAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NightholdEyeOfGuldanAI(c); }
    explicit NightholdEyeOfGuldanAI(Creature* pCreature) : NightholdCreatureAI(pCreature) {}

    void OnLoad() override { setRooted(true); }

protected:
    void onEngage() override
    {
        mEnergy = 0;
        mEnergyClock = 0;
        scriptEvents.addEvent(EVENT_EYE_PULSE, 1000);
    }

    void onTick(uint32_t diff) override
    {
        mEnergyClock += diff;
        while (mEnergyClock >= 1000)
        {
            mEnergyClock -= 1000;
            mEnergy += Util::getRandomUInt(2, 3);
        }

        // a full eye splits in two
        if (mEnergy >= 100)
        {
            mEnergy = 0;
            scriptEvents.addEvent(EVENT_EYE_DUPLICATE, 100);
        }
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_EYE_PULSE:
                castOn(pickPlayer(false), isEmpowered() ? SPELL_EMPOWERED_EYE_OF_GULDAN_PULSE : SPELL_EYE_OF_GULDAN_PULSE);
                scriptEvents.addEvent(EVENT_EYE_PULSE, 6000);
                break;
            case EVENT_EYE_DUPLICATE:
            {
                castSpellOnSelf(SPELL_EYE_OF_GULDAN_DUPLICATE, true);
                Creature* pGuldan = findNearestCreature(BOSS_GULDAN, 200.0f);
                if (pGuldan == nullptr || !pGuldan->isAlive())
                    break;

                if (NightholdGuldanAI* pGuldanAI = dynamic_cast<NightholdGuldanAI*>(pGuldan->GetScript()))
                    pGuldanAI->spawnEye(getCreature()->getEntry(), nightholdScatter(getCreature()->GetPosition(), 4.0f, 8.0f), getCreature()->getHealth());
                break;
            }
            default:
                break;
        }
    }

private:
    bool isEmpowered() { return getCreature()->getEntry() == NPC_EMPOWERED_EYE_OF_GULDAN; }

    uint32_t mEnergy = 0;
    uint32_t mEnergyClock = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Trilliax

enum class TrilliaxMode : uint8_t
{
    Cleaner,
    Maniac,
    Caretaker
};

class NightholdTrilliaxAI : public NightholdBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NightholdTrilliaxAI(c); }
    explicit NightholdTrilliaxAI(Creature* pCreature) : NightholdBossAI(pCreature, DATA_TRILLIAX) {}

protected:
    void onEngage() override
    {
        mMode = TrilliaxMode::Cleaner;
        mFirstCleaner = true;

        castSpellOnSelf(SPELL_TRILLIAX_ARCANE_SEEPAGE, true);
        castSpellOnSelf(SPELL_TRILLIAX_CLEANER, true);
        scheduleMode();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_TRILLIAX_ARCANE_SLASH:
                castSpellOnVictim(SPELL_TRILLIAX_ARCANE_SLASH);
                scriptEvents.addEvent(EVENT_TRILLIAX_ARCANE_SLASH, mMode == TrilliaxMode::Maniac ? 8000 : 11000);
                break;
            case EVENT_TRILLIAX_TOXIC_SLICE:
                castSpellOnSelf(SPELL_TRILLIAX_TOXIC_SLICE);
                for (uint32_t i = 0; i < TrilliaxToxicSlices; ++i)
                    callAdd(NPC_TOXIC_SLICE, nightholdScatter(TrilliaxRoomCentre, 20.0f, 43.0f), MANUAL_DESPAWN, 0, false);
                scriptEvents.addEvent(EVENT_TRILLIAX_TOXIC_SLICE, 26000);
                break;
            case EVENT_TRILLIAX_STERILIZE:
                for (Unit* pTarget : pickPlayers(heroicPlus() ? 3 : 2, false))
                    castSpell(pTarget, SPELL_TRILLIAX_STERILIZE, true);
                break;
            case EVENT_TRILLIAX_CLEANSING_RAGE:
                warn("Trilliax erupts in Cleansing Rage!");
                castSpell(getCreature(), SPELL_TRILLIAX_CLEANSING_RAGE);
                break;
            case EVENT_TRILLIAX_MANA_RUPTURE:
                castSpellOnSelf(SPELL_TRILLIAX_MANA_RUPTURE, true);
                break;
            case EVENT_TRILLIAX_ARCING_BONDS:
                for (Unit* pTarget : pickPlayers(2, false))
                    castSpell(pTarget, SPELL_TRILLIAX_ARCING_BONDS, true);
                break;
            case EVENT_TRILLIAX_ANNIHILATION:
                warn("Trilliax leaps to the centre of the room! Annihilation is coming!");
                moveJump(TrilliaxRoomCentre, 20.0f, 10.0f);
                scriptEvents.addEvent(EVENT_TRILLIAX_ANNIHILATION_BLAST, 2500);
                break;
            case EVENT_TRILLIAX_ANNIHILATION_BLAST:
                castSpellOnSelf(SPELL_TRILLIAX_ANNIHILATION);
                break;
            case EVENT_TRILLIAX_SUCCULENT_FEAST:
            {
                castSpellOnSelf(SPELL_TRILLIAX_SUCCULENT_FEAST);
                uint32_t const cakes = heroicPlus() ? 3 : 2;
                for (uint32_t i = 0; i < cakes; ++i)
                    callAdd(NPC_SUCCULENT_FEAST, nightholdScatter(getCreature()->GetPosition(), 25.0f, 25.0f), MANUAL_DESPAWN, 0, false);
                break;
            }
            case EVENT_TRILLIAX_TIDY_UP:
                warn("Trilliax sends the Scrubbers to tidy up!");
                castSpellOnSelf(SPELL_TRILLIAX_TIDY_UP, true);
                for (LocationVector const& spot : TrilliaxScrubberSpots)
                    callAdd(NPC_SCRUBBER, spot, CORPSE_DESPAWN, 0, false);
                break;
            case EVENT_TRILLIAX_MODE_CHANGE:
                changeMode();
                break;
            default:
                break;
        }
    }

private:
    // cleaner, maniac and caretaker in turn
    void changeMode()
    {
        scriptEvents.resetEvents();

        switch (mMode)
        {
            case TrilliaxMode::Cleaner:
                _removeAura(SPELL_TRILLIAX_CLEANER);
                _removeAura(SPELL_TRILLIAX_THE_CLEANER);
                castSpellOnSelf(SPELL_TRILLIAX_THE_MANIAC, true);
                warn("Trilliax switches to Maniac Mode!");
                mMode = TrilliaxMode::Maniac;
                break;
            case TrilliaxMode::Maniac:
                _removeAura(SPELL_TRILLIAX_THE_MANIAC);
                _removeAura(SPELL_TRILLIAX_MANA_RUPTURE);
                castSpellOnSelf(SPELL_TRILLIAX_THE_CARETAKER, true);
                warn("Trilliax switches to Caretaker Mode!");
                mMode = TrilliaxMode::Caretaker;
                break;
            case TrilliaxMode::Caretaker:
                _removeAura(SPELL_TRILLIAX_THE_CARETAKER);
                castSpellOnSelf(SPELL_TRILLIAX_THE_CLEANER, true);
                warn("Trilliax switches to Cleaner Mode!");
                mMode = TrilliaxMode::Cleaner;
                mFirstCleaner = false;
                break;
        }

        scheduleMode();
    }

    void scheduleMode()
    {
        switch (mMode)
        {
            case TrilliaxMode::Cleaner:
                scriptEvents.addEvent(EVENT_TRILLIAX_ARCANE_SLASH, 7000);
                scriptEvents.addEvent(EVENT_TRILLIAX_TOXIC_SLICE, 10000);
                if (!mFirstCleaner)
                {
                    scriptEvents.addEvent(EVENT_TRILLIAX_STERILIZE, 1000);
                    scriptEvents.addEvent(EVENT_TRILLIAX_CLEANSING_RAGE, 10000);
                }
                break;
            case TrilliaxMode::Maniac:
                scriptEvents.addEvent(EVENT_TRILLIAX_MANA_RUPTURE, 2000);
                scriptEvents.addEvent(EVENT_TRILLIAX_ARCING_BONDS, 5000);
                scriptEvents.addEvent(EVENT_TRILLIAX_ARCANE_SLASH, 7000);
                scriptEvents.addEvent(EVENT_TRILLIAX_ANNIHILATION, 20000);
                break;
            case TrilliaxMode::Caretaker:
                scriptEvents.addEvent(EVENT_TRILLIAX_SUCCULENT_FEAST, 5000);
                scriptEvents.addEvent(EVENT_TRILLIAX_TIDY_UP, 12000);
                break;
        }

        scriptEvents.addEvent(EVENT_TRILLIAX_MODE_CHANGE, TrilliaxModeMs[static_cast<uint8_t>(mMode)]);
    }

    TrilliaxMode mMode = TrilliaxMode::Cleaner;
    bool mFirstCleaner = true;
};

// toxic slices and succulent feasts lying on the floor
class NightholdTrilliaxLeftoverAI : public NightholdCreatureAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NightholdTrilliaxLeftoverAI(c); }
    explicit NightholdTrilliaxLeftoverAI(Creature* pCreature) : NightholdCreatureAI(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        setRooted(true);
        castSpellOnSelf(getCreature()->getEntry() == NPC_TOXIC_SLICE ? SPELL_TOXIC_SLICE_AURA : SPELL_SUCCULENT_FEAST_AURA, true);
    }
};

class NightholdScrubberAI : public NightholdCreatureAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NightholdScrubberAI(c); }
    explicit NightholdScrubberAI(Creature* pCreature) : NightholdCreatureAI(pCreature) { mTicksIdle = true; }

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        scriptEvents.addEvent(EVENT_SCRUBBER_SEEK, 2000);
    }

protected:
    // a fight with it must not end the cleaning
    void onDisengage() override { scriptEvents.addEvent(EVENT_SCRUBBER_SEEK, 2000); }

    // walks to the nearest slice and scrubs it away
    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_SCRUBBER_SEEK)
            return;

        Creature* pSlice = findNearestCreature(NPC_TOXIC_SLICE, 100.0f);
        if (pSlice == nullptr || !pSlice->isAlive())
        {
            scriptEvents.addEvent(EVENT_SCRUBBER_SEEK, Util::getRandomUInt(2000, 4000));
            return;
        }

        if (getRangeToObject(pSlice) <= 4.0f)
        {
            castSpellOnSelf(SPELL_SCRUBBER_SCRUBBING, true);
            pSlice->despawn(100, 0);
            scriptEvents.addEvent(EVENT_SCRUBBER_SEEK, 2000);
            return;
        }

        moveTo(pSlice->GetPositionX(), pSlice->GetPositionY(), pSlice->GetPositionZ());
        scriptEvents.addEvent(EVENT_SCRUBBER_SEEK, 1000);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Spellblade Aluriel

enum class AlurielSchool : uint8_t
{
    Frost,
    Fire,
    Arcane
};

class NightholdAlurielAI : public NightholdBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NightholdAlurielAI(c); }
    explicit NightholdAlurielAI(Creature* pCreature) : NightholdBossAI(pCreature, DATA_SPELLBLADE_ALURIEL) {}

protected:
    void onEngage() override
    {
        armBerserk(heroicPlus() ? 480000 : 600000);
        mSchool = AlurielSchool::Frost;
        beginSchool();
    }

    void onEvent(uint32_t eventId) override
    {
        uint8_t const school = static_cast<uint8_t>(mSchool);

        switch (eventId)
        {
            case EVENT_ALURIEL_ANNIHILATE:
                warn("Spellblade Aluriel prepares to Annihilate her target!");
                castSpellOnSelf(SPELL_ALURIEL_ANNIHILATE);
                scriptEvents.addEvent(EVENT_ALURIEL_ANNIHILATE_FOLLOWUP, 100);
                scriptEvents.addEvent(EVENT_ALURIEL_ANNIHILATE, mSchool == AlurielSchool::Arcane ? 35000 : 45000);
                break;
            case EVENT_ALURIEL_ANNIHILATE_FOLLOWUP:
                castSpellOnVictim(SPELL_ALURIEL_ANNIHILATED, true);
                break;
            case EVENT_ALURIEL_MARK:
                if (mSchool == AlurielSchool::Frost)
                    markOfFrost(2);
                else if (mSchool == AlurielSchool::Fire)
                    searingBrand(2);
                else
                    castSpellOnSelf(SPELL_ALURIEL_ARCANE_ORB);
                break;
            case EVENT_ALURIEL_REPLICATE:
                warn("Spellblade Aluriel replicates her marks!");
                castSpellOnSelf(AlurielReplications[school], mSchool == AlurielSchool::Fire);
                if (mSchool == AlurielSchool::Frost)
                    markOfFrost(2);
                else if (mSchool == AlurielSchool::Fire)
                    searingBrand(2);
                else
                {
                    mOrbEchoes = 2;
                    scriptEvents.addEvent(EVENT_ALURIEL_REPLICATE_ECHO, 2100);
                }
                break;
            case EVENT_ALURIEL_REPLICATE_ECHO:
                castSpellOnSelf(SPELL_ALURIEL_REPLICATE_ARCANE_ORB, true);
                if (mOrbEchoes > 0 && --mOrbEchoes > 0)
                    scriptEvents.addEvent(EVENT_ALURIEL_REPLICATE_ECHO, 2100);
                break;
            case EVENT_ALURIEL_DETONATE:
                warn("Spellblade Aluriel detonates her marks!");
                castSpellOnSelf(AlurielDetonations[school]);
                break;
            case EVENT_ALURIEL_ANIMATE:
                warn("Spellblade Aluriel animates her enchantments!");
                castSpellOnSelf(AlurielAnimations[school]);
                for (Unit* pTarget : pickPlayers(AlurielAnimatedPerCast, false))
                    callAdd(AlurielAnimatedAdds[school], nightholdScatter(pTarget->GetPosition(), 2.0f, 4.0f));
                break;
            case EVENT_ALURIEL_BRAND_BURN:
                for (Unit* pTarget : pickPlayers(40, false, SPELL_ALURIEL_SEARING_BRAND))
                    castSpell(pTarget, SPELL_ALURIEL_SEARING_BRAND_BURN, true);
                break;
            case EVENT_ALURIEL_NEXT_SCHOOL:
                mSchool = static_cast<AlurielSchool>((school + 1) % 3);
                beginSchool();
                break;
            default:
                break;
        }
    }

private:
    // frost, fire and arcane in turn
    void beginSchool()
    {
        scriptEvents.resetEvents();
        for (uint32_t enchantment : AlurielEnchantments)
            _removeAura(enchantment);

        uint8_t const school = static_cast<uint8_t>(mSchool);
        castSpellOnSelf(AlurielEnchantments[school], true);

        scriptEvents.addEvent(EVENT_ALURIEL_ANNIHILATE, 7000);
        switch (mSchool)
        {
            case AlurielSchool::Frost:
                scriptEvents.addEvent(EVENT_ALURIEL_MARK, 18000);
                scriptEvents.addEvent(EVENT_ALURIEL_REPLICATE, 38000);
                scriptEvents.addEvent(EVENT_ALURIEL_DETONATE, 68000);
                scriptEvents.addEvent(EVENT_ALURIEL_ANIMATE, 75000);
                break;
            case AlurielSchool::Fire:
                scriptEvents.addEvent(EVENT_ALURIEL_MARK, 18000);
                scriptEvents.addEvent(EVENT_ALURIEL_REPLICATE, 26000);
                scriptEvents.addEvent(EVENT_ALURIEL_DETONATE, 45000);
                scriptEvents.addEvent(EVENT_ALURIEL_ANIMATE, 62000);
                break;
            case AlurielSchool::Arcane:
                scriptEvents.addEvent(EVENT_ALURIEL_MARK, 1000);
                scriptEvents.addEvent(EVENT_ALURIEL_REPLICATE, 15000);
                scriptEvents.addEvent(EVENT_ALURIEL_DETONATE, 35000);
                scriptEvents.addEvent(EVENT_ALURIEL_ANIMATE, 52000);
                break;
        }

        scriptEvents.addEvent(EVENT_ALURIEL_NEXT_SCHOOL, AlurielSchoolMs[school]);
    }

    // the mark is applied by the player to itself
    void markOfFrost(size_t count)
    {
        for (Unit* pTarget : pickPlayers(count, false, -static_cast<int32_t>(SPELL_ALURIEL_MARK_OF_FROST)))
            pTarget->castSpell(pTarget, SPELL_ALURIEL_MARK_OF_FROST, true);
    }

    // branded players are charged a few seconds later
    void searingBrand(size_t count)
    {
        for (Unit* pTarget : pickPlayers(count, true, -static_cast<int32_t>(SPELL_ALURIEL_SEARING_BRAND)))
            castSpell(pTarget, SPELL_ALURIEL_SEARING_BRAND, true);

        scriptEvents.addEvent(EVENT_ALURIEL_BRAND_BURN, 5000);
    }

    AlurielSchool mSchool = AlurielSchool::Frost;
    uint32_t mOrbEchoes = 0;
};

class NightholdAlurielEnchantmentAI : public NightholdCreatureAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NightholdAlurielEnchantmentAI(c); }
    explicit NightholdAlurielEnchantmentAI(Creature* pCreature) : NightholdCreatureAI(pCreature) {}

    // heroic fire and arcane enchantments stay where they rise
    void OnLoad() override
    {
        if (getCreature()->getEntry() != NPC_ICY_ENCHANTMENT && heroicPlus())
        {
            setRooted(true);
            _setMeleeDisabled(true);
        }
    }

protected:
    void onEngage() override
    {
        bool const heroic = heroicPlus();
        switch (getCreature()->getEntry())
        {
            case NPC_ICY_ENCHANTMENT:
                castSpellOnSelf(SPELL_ICY_ENCHANTMENT_BURST_OF_ICE, true);
                break;
            case NPC_FIERY_ENCHANTMENT:
                castSpellOnSelf(SPELL_FIERY_ENCHANTMENT_BURST_OF_FLAME, true);
                if (heroic)
                    scriptEvents.addEvent(EVENT_ENCHANTMENT_PYROBLAST, Util::getRandomUInt(5000, 7000));
                break;
            default:
                castSpellOnSelf(heroic ? SPELL_ARCANE_ENCHANTMENT_ARMAGEDDON : SPELL_ARCANE_ENCHANTMENT_BURST_OF_MAGIC, true);
                break;
        }
    }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_ENCHANTMENT_PYROBLAST)
            return;

        castOn(pickPlayer(false), SPELL_FIERY_ENCHANTMENT_PYROBLAST);
        scriptEvents.addEvent(EVENT_ENCHANTMENT_PYROBLAST, 7000);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Grand Magistrix Elisande

class NightholdElisandeAI : public NightholdBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NightholdElisandeAI(c); }
    explicit NightholdElisandeAI(Creature* pCreature) : NightholdBossAI(pCreature, DATA_ELISANDE) {}

    // the first two time layers end at a tenth of her health
    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage == nullptr)
            return;

        if (mShifting)
        {
            *damage = 0;
            return;
        }

        if (mLayer >= 3)
            return;

        uint32_t const health = getCreature()->getHealth();
        uint32_t const floor = getCreature()->getMaxHealth() / 10;
        if (health > floor && *damage < health - floor)
            return;

        *damage = health > floor ? health - floor : 0;
        mShifting = true;
        mShiftPending = true;
    }

protected:
    void onEngage() override
    {
        mLayer = 1;
        mShifting = false;
        mShiftPending = false;
        beginLayer();
    }

    void onDisengage() override
    {
        mShifting = false;
        mShiftPending = false;
        removeUnitFlags(UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
        setReactState(REACT_AGGRESSIVE);
    }

    void onBerserk() override
    {
        warn("Grand Magistrix Elisande Terminates the encounter!");
        scriptEvents.resetEvents();
        castSpellOnSelf(SPELL_ELISANDE_TERMINATE, true);
    }

    void onTick(uint32_t /*diff*/) override
    {
        if (!mShiftPending)
            return;

        mShiftPending = false;
        shiftLayer();
    }

    void onEvent(uint32_t eventId) override
    {
        bool const heroic = heroicPlus();

        switch (eventId)
        {
            case EVENT_ELISANDE_LAYER_RESUME:
                getCreature()->setHealth(getCreature()->getMaxHealth());
                _removeAura(SPELL_ELISANDE_ETCHED_IN_TIME);
                removeUnitFlags(UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                setReactState(REACT_AGGRESSIVE);
                mShifting = false;
                beginLayer();
                break;
            case EVENT_ELISANDE_SINGULARITY:
            {
                uint32_t const spellId = mLayer == 1 ? (heroic ? SPELL_ELISANDE_SINGULARITY_HEROIC : SPELL_ELISANDE_SINGULARITY)
                    : (heroic ? SPELL_ELISANDE_SINGULARITY_ECHO_HEROIC : SPELL_ELISANDE_SINGULARITY_ECHO);
                castOn(pickPlayer(false), spellId);
                rearm(EVENT_ELISANDE_SINGULARITY, mSingularity);
                break;
            }
            case EVENT_ELISANDE_RECURSIVE_ELEMENTAL:
                callAdd(NPC_RECURSIVE_ELEMENTAL, nightholdScatter(ElisandeRoomCentre, 30.0f, 40.0f));
                rearm(EVENT_ELISANDE_RECURSIVE_ELEMENTAL, mRecursive);
                break;
            case EVENT_ELISANDE_EXPEDIENT_ELEMENTAL:
                callAdd(NPC_EXPEDIENT_ELEMENTAL, nightholdScatter(ElisandeRoomCentre, 20.0f, 35.0f));
                rearm(EVENT_ELISANDE_EXPEDIENT_ELEMENTAL, mExpedient);
                break;
            case EVENT_ELISANDE_CASCADENT_STAR:
                castSpellOnVictim(SPELL_ELISANDE_CASCADENT_STAR);
                scriptEvents.addEvent(EVENT_ELISANDE_CASCADENT_STAR, 5000);
                break;
            case EVENT_ELISANDE_ABLATION:
                castSpellOnVictim(SPELL_ELISANDE_ABLATION);
                scriptEvents.addEvent(EVENT_ELISANDE_ABLATION, 5000);
                break;
            case EVENT_ELISANDE_ABLATING_EXPLOSION:
                castSpellOnVictim(SPELL_ELISANDE_ABLATING_EXPLOSION);
                scriptEvents.addEvent(EVENT_ELISANDE_ABLATING_EXPLOSION, 21000);
                break;
            case EVENT_ELISANDE_ABLATIVE_PULSE:
                castSpellOnVictim(SPELL_ELISANDE_ABLATIVE_PULSE);
                scriptEvents.addEvent(EVENT_ELISANDE_ABLATIVE_PULSE, 6000);
                break;
            case EVENT_ELISANDE_DELPHURIC_BEAM:
                warn("Grand Magistrix Elisande targets the raid with Delphuric Beam!");
                castSpellOnSelf(SPELL_ELISANDE_DELPHURIC_BEAM);
                for (Unit* pTarget : pickPlayers(heroic ? 3 : 2, false, -static_cast<int32_t>(SPELL_ELISANDE_DELPHURIC_BEAM_MARK)))
                    castSpell(pTarget, SPELL_ELISANDE_DELPHURIC_BEAM_MARK, true);
                scriptEvents.addEvent(EVENT_ELISANDE_DELPHURIC_BEAM_FIRE, ElisandeMarkMs);
                rearm(EVENT_ELISANDE_DELPHURIC_BEAM, mBeam);
                break;
            case EVENT_ELISANDE_DELPHURIC_BEAM_FIRE:
                for (Unit* pTarget : pickPlayers(40, false, SPELL_ELISANDE_DELPHURIC_BEAM_MARK))
                    castSpell(pTarget, SPELL_ELISANDE_DELPHURIC_BEAM_HIT, true);
                break;
            case EVENT_ELISANDE_EPOCHERIC_ORB:
                warn("Epocheric Orbs fall from the sky! Intercept them!");
                dropOrbs();
                rearm(EVENT_ELISANDE_EPOCHERIC_ORB, mOrb);
                break;
            case EVENT_ELISANDE_PERMELIATIVE_TORMENT:
                for (Unit* pTarget : pickPlayers(2, false))
                    castSpell(pTarget, SPELL_ELISANDE_PERMELIATIVE_TORMENT, true);
                rearm(EVENT_ELISANDE_PERMELIATIVE_TORMENT, mTorment);
                break;
            case EVENT_ELISANDE_CONFLEXIVE_BURST:
                warn("Grand Magistrix Elisande casts Conflexive Burst!");
                for (Unit* pTarget : pickPlayers(2, false, -static_cast<int32_t>(SPELL_ELISANDE_CONFLEXIVE_BURST)))
                    castSpell(pTarget, SPELL_ELISANDE_CONFLEXIVE_BURST, true);
                scriptEvents.addEvent(EVENT_ELISANDE_CONFLEXIVE_BURST_BLAST, ElisandeMarkMs);
                rearm(EVENT_ELISANDE_CONFLEXIVE_BURST, mBurst);
                break;
            case EVENT_ELISANDE_CONFLEXIVE_BURST_BLAST:
                for (Unit* pTarget : pickPlayers(40, false, SPELL_ELISANDE_CONFLEXIVE_BURST))
                {
                    castAt(pTarget->GetPosition(), SPELL_ELISANDE_CONFLEXIVE_BURST_BLAST);
                    pTarget->removeAllAurasById(SPELL_ELISANDE_CONFLEXIVE_BURST);
                }
                break;
            default:
                break;
        }
    }

private:
    void rearm(uint32_t eventId, NightholdCadence& cadence)
    {
        if (uint32_t const delay = cadence.next())
            scriptEvents.addEvent(eventId, delay);
    }

    // every layer restarts the timers, elementals come ten seconds later after the first
    void beginLayer()
    {
        bool const heroic = heroicPlus();
        uint32_t const lead = mLayer > 1 ? 10000 : 0;

        armBerserk((heroic ? (mLayer < 3 ? 229 : 224) : (mLayer < 3 ? 259 : 254)) * 1000);

        mSingularity.start(heroic ? ElisandeSingularityHeroic : ElisandeSingularityNormal, NightholdCadence::Mode::Once);
        mRecursive.start(heroic ? ElisandeRecursiveHeroic : ElisandeRecursiveNormal, NightholdCadence::Mode::Once);
        mExpedient.start(heroic ? ElisandeExpedientHeroic : ElisandeExpedientNormal, NightholdCadence::Mode::Once);

        rearm(EVENT_ELISANDE_SINGULARITY, mSingularity);
        scriptEvents.addEvent(EVENT_ELISANDE_RECURSIVE_ELEMENTAL, mRecursive.next() + lead);
        scriptEvents.addEvent(EVENT_ELISANDE_EXPEDIENT_ELEMENTAL, mExpedient.next() + lead);

        if (mLayer > 1)
        {
            mBeam.start(heroic ? ElisandeBeamHeroic : ElisandeBeamNormal, NightholdCadence::Mode::Once);
            mOrb.start(heroic ? ElisandeOrbHeroic : ElisandeOrbNormal, heroic ? NightholdCadence::Mode::Once : NightholdCadence::Mode::HoldLast);
            rearm(EVENT_ELISANDE_DELPHURIC_BEAM, mBeam);
            rearm(EVENT_ELISANDE_EPOCHERIC_ORB, mOrb);
        }

        switch (mLayer)
        {
            case 1:
                scriptEvents.addEvent(EVENT_ELISANDE_CASCADENT_STAR, 12000);
                scriptEvents.addEvent(EVENT_ELISANDE_ABLATION, 13000);
                break;
            case 2:
                scriptEvents.addEvent(EVENT_ELISANDE_ABLATING_EXPLOSION, 24000);
                break;
            default:
                mTorment.start(heroic ? ElisandeTormentHeroic : ElisandeTormentNormal, NightholdCadence::Mode::Once);
                mBurst.start(heroic ? ElisandeBurstHeroic : ElisandeBurstNormal, heroic ? NightholdCadence::Mode::Once : NightholdCadence::Mode::HoldLast);
                scriptEvents.addEvent(EVENT_ELISANDE_ABLATIVE_PULSE, 24000);
                rearm(EVENT_ELISANDE_PERMELIATIVE_TORMENT, mTorment);
                rearm(EVENT_ELISANDE_CONFLEXIVE_BURST, mBurst);
                break;
        }
    }

    // time stops, the room is cleared and she returns at full health
    void shiftLayer()
    {
        ++mLayer;
        armBerserk(0);
        scriptEvents.resetEvents();

        summons.despawnEntry(NPC_RECURSIVE_ELEMENTAL);
        summons.despawnEntry(NPC_EXPEDIENT_ELEMENTAL);
        summons.despawnEntry(NPC_EPOCHERIC_ORB);

        warn(mLayer == 2 ? "Grand Magistrix Elisande stops time and steps into another time layer!" : "Grand Magistrix Elisande stops time once more!");
        attackStop();
        setReactState(REACT_PASSIVE);
        addUnitFlags(UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
        castSpellOnSelf(SPELL_ELISANDE_TIME_STOP, true);
        castSpellOnSelf(SPELL_ELISANDE_ETCHED_IN_TIME, true);

        scriptEvents.addEvent(EVENT_ELISANDE_LAYER_RESUME, ElisandeLayerShiftMs);
    }

    // more orbs for bigger raids, spread evenly around the room
    void dropOrbs()
    {
        uint32_t const players = raidSize();
        uint32_t const count = players <= 15 ? 2 : players <= 20 ? 3 : players <= 25 ? 4 : 5;

        float angle = Util::getRandomFloat(NightholdTwoPi);
        for (uint32_t i = 0; i < count; ++i)
        {
            callAdd(NPC_EPOCHERIC_ORB, nightholdRing(ElisandeRoomCentre, Util::getRandomFloat(25.0f, 35.0f), angle), TIMED_DESPAWN, ElisandeOrbFallMs + 2000, false);
            angle += NightholdTwoPi / static_cast<float>(count);
        }
    }

    NightholdCadence mSingularity;
    NightholdCadence mRecursive;
    NightholdCadence mExpedient;
    NightholdCadence mBeam;
    NightholdCadence mOrb;
    NightholdCadence mTorment;
    NightholdCadence mBurst;
    uint32_t mLayer = 1;
    bool mShifting = false;
    bool mShiftPending = false;
};

class NightholdEpochericOrbAI : public NightholdCreatureAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NightholdEpochericOrbAI(c); }
    explicit NightholdEpochericOrbAI(Creature* pCreature) : NightholdCreatureAI(pCreature) { mTicksIdle = true; }

    void OnLoad() override
    {
        becomeStalker();
        castSpellOnSelf(SPELL_EPOCHERIC_ORB_VISUAL, true);
        scriptEvents.addEvent(EVENT_EPOCHERIC_ORB_IMPACT, ElisandeOrbFallMs);
    }

protected:
    // a player under the orb takes the small hit, otherwise the raid takes the falloff
    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_EPOCHERIC_ORB_IMPACT)
            return;

        LocationVector const impact = getCreature()->GetPosition();
        castAt(impact, playerNear(impact, ElisandeOrbSoakRange) ? SPELL_EPOCHERIC_ORB_SOAKED : SPELL_EPOCHERIC_ORB_FALLOFF);
        despawn(1000, 0);
    }
};

class NightholdRecursiveElementalAI : public NightholdCreatureAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NightholdRecursiveElementalAI(c); }
    explicit NightholdRecursiveElementalAI(Creature* pCreature) : NightholdCreatureAI(pCreature) {}

    void OnLoad() override { setRooted(true); }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
    }

protected:
    void onEngage() override
    {
        if (InstanceScript* pInstance = getInstanceScript())
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());

        castSpellOnSelf(SPELL_RECURSIVE_ELEMENTAL_MODE, true);
        scriptEvents.addEvent(EVENT_RECURSIVE_ELEMENTAL_BLAST, 1000);
    }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_RECURSIVE_ELEMENTAL_BLAST)
            return;

        castSpellOnVictim(SPELL_RECURSIVE_ELEMENTAL_BLAST);
        scriptEvents.addEvent(EVENT_RECURSIVE_ELEMENTAL_BLAST, 4000);
    }
};

class NightholdExpedientElementalAI : public NightholdCreatureAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NightholdExpedientElementalAI(c); }
    explicit NightholdExpedientElementalAI(Creature* pCreature) : NightholdCreatureAI(pCreature) {}

    void OnDied(Unit* /*pKiller*/) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
    }

protected:
    void onEngage() override
    {
        if (InstanceScript* pInstance = getInstanceScript())
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());

        castSpellOnSelf(SPELL_EXPEDIENT_ELEMENTAL_MODE, true);
        scriptEvents.addEvent(EVENT_EXPEDIENT_ELEMENTAL_EXOTHERMIC, 1000);
    }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_EXPEDIENT_ELEMENTAL_EXOTHERMIC)
            return;

        castSpellAOE(SPELL_EXPEDIENT_ELEMENTAL_EXOTHERMIC);
        scriptEvents.addEvent(EVENT_EXPEDIENT_ELEMENTAL_EXOTHERMIC, 4500);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// High Botanist Tel'arn

namespace
{
    // collapse stalker under a random player
    void telarnSolarCollapse(NightholdCreatureAI& ai, Creature* pOwner)
    {
        if (Unit* pTarget = ai.pickPlayer(false))
            ai.callAddFor(pOwner, NPC_SOLAR_COLLAPSE_STALKER, pTarget->GetPosition(), TIMED_DESPAWN, 15000, false);
    }

    // a fettered player sprouts a lasher
    void telarnParasiticFetter(NightholdCreatureAI& ai, Creature* pOwner)
    {
        Unit* pTarget = ai.pickPlayer(true);
        if (pTarget == nullptr)
            return;

        ai.castSpell(pTarget, SPELL_TELARN_PARASITIC_FETTER, true);
        ai.callAddFor(pOwner, NPC_PARASITIC_LASHER, nightholdScatter(pTarget->GetPosition(), 2.0f, 4.0f));
    }
}

class NightholdTelarnAI : public NightholdBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NightholdTelarnAI(c); }
    explicit NightholdTelarnAI(Creature* pCreature) : NightholdBossAI(pCreature, DATA_HIGH_BOTANIST_TELARN) {}

protected:
    void onEngage() override
    {
        mStage = 1;
        mPool = 1.0f;
        mShareClock = 0;
        mImages.clear();

        armBerserk(540000);
        castSpellOnSelf(SPELL_TELARN_RECURSIVE_STRIKES, true);

        scriptEvents.addEvent(EVENT_TELARN_SOLAR_COLLAPSE, 10000);
        scriptEvents.addEvent(EVENT_TELARN_PARASITIC_FETTER, 20000);
        scriptEvents.addEvent(EVENT_TELARN_CONTROLLED_CHAOS, 35000);
    }

    void onDisengage() override { mImages.clear(); }

    void onDefeat() override { mImages.clear(); }

    void onTick(uint32_t diff) override
    {
        mShareClock += diff;
        if (mShareClock >= TelarnHealthShareMs)
        {
            mShareClock = 0;
            shareHealth();
        }

        int32_t const health = _getHealthPercent();
        if (mStage == 1 && health <= 75)
            splitSolarist();
        else if (mStage == 2 && health <= 50)
            splitNaturalist();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_TELARN_SOLAR_COLLAPSE:
                warn("High Botanist Tel'arn calls down a Solar Collapse!");
                telarnSolarCollapse(*this, getCreature());
                scriptEvents.addEvent(EVENT_TELARN_SOLAR_COLLAPSE, 35000);
                break;
            case EVENT_TELARN_PARASITIC_FETTER:
                telarnParasiticFetter(*this, getCreature());
                scriptEvents.addEvent(EVENT_TELARN_PARASITIC_FETTER, mStage == 2 ? 40000 : 35000);
                break;
            case EVENT_TELARN_CONTROLLED_CHAOS:
                warn("High Botanist Tel'arn unleashes Controlled Chaos!");
                if (Unit* pTarget = pickPlayer(false))
                    callAdd(NPC_CONTROLLED_CHAOS_STALKER, pTarget->GetPosition(), TIMED_DESPAWN, 15000, false);
                scriptEvents.addEvent(EVENT_TELARN_CONTROLLED_CHAOS, mStage == 1 ? 35000 : mStage == 2 ? 40000 : 50000);
                break;
            case EVENT_TELARN_CALL_OF_NIGHT:
                warn("High Botanist Tel'arn casts Call of Night!");
                for (Unit* pTarget : pickPlayers(2, false))
                    castSpell(pTarget, SPELL_TELARN_CALL_OF_NIGHT, true);
                scriptEvents.addEvent(EVENT_TELARN_CALL_OF_NIGHT, 50000);
                break;
            default:
                break;
        }
    }

private:
    void splitSolarist()
    {
        mStage = 2;
        scriptEvents.resetEvents();
        warn("High Botanist Tel'arn splits off a Solarist!");
        castSpellOnSelf(SPELL_TELARN_NIGHTOSIS_NATURALIST, true);
        spawnImage(NPC_SOLARIST_TELARN);

        scriptEvents.addEvent(EVENT_TELARN_PARASITIC_FETTER, 22000);
        scriptEvents.addEvent(EVENT_TELARN_CONTROLLED_CHAOS, 42000);
    }

    void splitNaturalist()
    {
        mStage = 3;
        scriptEvents.resetEvents();
        warn("High Botanist Tel'arn splits off a Naturalist!");
        castSpellOnSelf(SPELL_TELARN_NIGHTOSIS_ARCANIST, true);
        spawnImage(NPC_NATURALIST_TELARN);

        scriptEvents.addEvent(EVENT_TELARN_CALL_OF_NIGHT, 20000);
        scriptEvents.addEvent(EVENT_TELARN_CONTROLLED_CHAOS, 52000);
    }

    void spawnImage(uint32_t entry)
    {
        Creature* pImage = callAdd(entry, nightholdScatter(getCreature()->GetPosition(), 3.0f, 5.0f));
        if (pImage == nullptr)
            return;

        pImage->setHealth(std::max<uint32_t>(1, static_cast<uint32_t>(pImage->getMaxHealth() * mPool)));
        mImages.push_back(pImage->GetNewGUID());
    }

    // all copies share one pool: every copy's loss is taken from all of them
    void shareHealth()
    {
        std::vector<Creature*> copies = { getCreature() };
        for (WoWGuid const& guid : mImages)
        {
            Creature* pImage = getCreature()->getWorldMap()->getCreature(guid);
            if (pImage != nullptr && pImage->isAlive())
                copies.push_back(pImage);
        }

        if (copies.size() < 2)
        {
            mPool = static_cast<float>(getCreature()->getHealth()) / static_cast<float>(getCreature()->getMaxHealth());
            return;
        }

        float lost = 0.0f;
        for (Creature* pCopy : copies)
        {
            float const share = static_cast<float>(pCopy->getHealth()) / static_cast<float>(pCopy->getMaxHealth());
            if (share < mPool)
                lost += mPool - share;
        }

        mPool = std::max(mPool - lost, 0.0f);
        for (Creature* pCopy : copies)
            pCopy->setHealth(std::max<uint32_t>(1, static_cast<uint32_t>(pCopy->getMaxHealth() * mPool)));
    }

    std::vector<WoWGuid> mImages;
    float mPool = 1.0f;
    uint32_t mShareClock = 0;
    uint32_t mStage = 1;
};

// split copies can not fall on their own
class NightholdTelarnImageAI : public NightholdCreatureAI
{
public:
    explicit NightholdTelarnImageAI(Creature* pCreature) : NightholdCreatureAI(pCreature) {}

    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        uint32_t const health = getCreature()->getHealth();
        if (damage != nullptr && *damage >= health)
            *damage = health > 1 ? health - 1 : 0;
    }

protected:
    Creature* botanist() { return findNearestCreature(BOSS_HIGH_BOTANIST_TELARN, 200.0f); }
};

class NightholdSolaristTelarnAI : public NightholdTelarnImageAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NightholdSolaristTelarnAI(c); }
    explicit NightholdSolaristTelarnAI(Creature* pCreature) : NightholdTelarnImageAI(pCreature) {}

protected:
    void onEngage() override
    {
        castSpellOnSelf(SPELL_TELARN_ARCANE_DISSOLVE_IN, true);
        scriptEvents.addEvent(EVENT_SOLARIST_FLARE, 7000);
        scriptEvents.addEvent(EVENT_SOLARIST_PLASMA_SPHERES, 10000);
        scriptEvents.addEvent(EVENT_SOLARIST_SOLAR_COLLAPSE, 32000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SOLARIST_FLARE:
                castSpellOnVictim(SPELL_SOLARIST_FLARE);
                scriptEvents.addEvent(EVENT_SOLARIST_FLARE, 9000);
                break;
            case EVENT_SOLARIST_PLASMA_SPHERES:
            {
                warn("Solarist Tel'arn summons Plasma Spheres!");
                castSpellOnSelf(SPELL_SOLARIST_SUMMON_PLASMA_SPHERES);
                Creature* pOwner = botanist();
                for (LocationVector const& spot : TelarnSphereSpots)
                    callAddFor(pOwner, NPC_PLASMA_SPHERE, spot);
                scriptEvents.addEvent(EVENT_SOLARIST_PLASMA_SPHERES, 40000);
                break;
            }
            case EVENT_SOLARIST_SOLAR_COLLAPSE:
                warn("Solarist Tel'arn calls down a Solar Collapse!");
                telarnSolarCollapse(*this, botanist());
                scriptEvents.addEvent(EVENT_SOLARIST_SOLAR_COLLAPSE, 40000);
                break;
            default:
                break;
        }
    }
};

class NightholdNaturalistTelarnAI : public NightholdTelarnImageAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NightholdNaturalistTelarnAI(c); }
    explicit NightholdNaturalistTelarnAI(Creature* pCreature) : NightholdTelarnImageAI(pCreature) {}

protected:
    void onEngage() override
    {
        castSpellOnSelf(SPELL_TELARN_ARCANE_DISSOLVE_IN, true);
        scriptEvents.addEvent(EVENT_NATURALIST_TOXIC_SPORES, 8000);
        scriptEvents.addEvent(EVENT_NATURALIST_GRACE_OF_NATURE, 9000);
        scriptEvents.addEvent(EVENT_NATURALIST_PARASITIC_FETTER, 35000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_NATURALIST_TOXIC_SPORES:
            {
                Creature* pOwner = botanist();
                for (Unit* pTarget : pickPlayers(2, false))
                    callAddFor(pOwner, NPC_TOXIC_SPORE, pTarget->GetPosition(), TIMED_DESPAWN, 20000, false);
                scriptEvents.addEvent(EVENT_NATURALIST_TOXIC_SPORES, 8000);
                break;
            }
            case EVENT_NATURALIST_GRACE_OF_NATURE:
                warn("Naturalist Tel'arn calls on the Grace of Nature!");
                castSpellOnSelf(SPELL_NATURALIST_GRACE_OF_NATURE);
                scriptEvents.addEvent(EVENT_NATURALIST_GRACE_OF_NATURE, 64000);
                break;
            case EVENT_NATURALIST_PARASITIC_FETTER:
                telarnParasiticFetter(*this, botanist());
                scriptEvents.addEvent(EVENT_NATURALIST_PARASITIC_FETTER, 64000);
                break;
            default:
                break;
        }
    }
};

class NightholdParasiticLasherAI : public NightholdCreatureAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NightholdParasiticLasherAI(c); }
    explicit NightholdParasiticLasherAI(Creature* pCreature) : NightholdCreatureAI(pCreature) {}

protected:
    void onEngage() override
    {
        if (Unit* pTarget = closestPlayer())
        {
            castSpell(pTarget, SPELL_PARASITIC_LASHER_FIXATE, true);
            attackStart(pTarget);
        }
    }
};

// three rings of chaos one after another
class NightholdControlledChaosAI : public NightholdCreatureAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NightholdControlledChaosAI(c); }
    explicit NightholdControlledChaosAI(Creature* pCreature) : NightholdCreatureAI(pCreature) { mTicksIdle = true; }

    void OnLoad() override
    {
        becomeStalker();
        mRing = 0;
        scriptEvents.addEvent(EVENT_CHAOS_STALKER_RING, 100);
    }

protected:
    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_CHAOS_STALKER_RING || mRing >= 3)
            return;

        castSpellOnSelf(TelarnChaosRings[mRing]);
        if (++mRing < 3)
            scriptEvents.addEvent(EVENT_CHAOS_STALKER_RING, 100);
    }

private:
    uint32_t mRing = 0;
};

// shrinking rings of falling suns
class NightholdSolarCollapseAI : public NightholdCreatureAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NightholdSolarCollapseAI(c); }
    explicit NightholdSolarCollapseAI(Creature* pCreature) : NightholdCreatureAI(pCreature) { mTicksIdle = true; }

    void OnLoad() override
    {
        becomeStalker();
        mWave = 0;
        scriptEvents.addEvent(EVENT_COLLAPSE_STALKER_WAVE, 1500);
    }

protected:
    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_COLLAPSE_STALKER_WAVE || mWave >= TelarnCollapseWaves)
            return;

        LocationVector const centre = getCreature()->GetPosition();
        uint32_t const count = TelarnCollapseCount[mWave];
        for (uint32_t i = 0; i < count; ++i)
            castAt(nightholdRing(centre, TelarnCollapseRadius[mWave], NightholdTwoPi * static_cast<float>(i) / static_cast<float>(count)), SPELL_TELARN_SOLAR_COLLAPSE);

        if (++mWave < TelarnCollapseWaves)
            scriptEvents.addEvent(EVENT_COLLAPSE_STALKER_WAVE, 1000);
    }

private:
    uint32_t mWave = 0;
};

class NightholdPlasmaSphereAI : public NightholdCreatureAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NightholdPlasmaSphereAI(c); }
    explicit NightholdPlasmaSphereAI(Creature* pCreature) : NightholdCreatureAI(pCreature) {}

    void OnLoad() override
    {
        mExploded = false;
        setReactState(REACT_PASSIVE);
        setRooted(true);
        castSpellOnSelf(SPELL_PLASMA_SPHERE, true);
    }

    // explodes instead of dying
    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage == nullptr || *damage < getCreature()->getHealth())
            return;

        *damage = 0;
        if (mExploded)
            return;

        mExploded = true;
        castSpellAOE(SPELL_PLASMA_EXPLOSION, true);
        addUnitFlags(UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
        despawn(500, 0);
    }

private:
    bool mExploded = false;
};

class NightholdToxicSporeAI : public NightholdCreatureAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NightholdToxicSporeAI(c); }
    explicit NightholdToxicSporeAI(Creature* pCreature) : NightholdCreatureAI(pCreature) { mTicksIdle = true; }

    void OnLoad() override
    {
        becomeStalker();
        castSpellOnSelf(SPELL_TOXIC_SPORE, true);
        scriptEvents.addEvent(EVENT_TOXIC_SPORE_CHECK, 2000);
    }

protected:
    // bursts on the first player stepping on it
    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_TOXIC_SPORE_CHECK)
            return;

        Unit* pVictim = nullptr;
        if (playerNear(getCreature()->GetPosition(), 2.5f, &pVictim))
        {
            castSpell(pVictim, SPELL_TOXIC_SPORE_BURST, true);
            despawn(100, 0);
            return;
        }

        scriptEvents.addEvent(EVENT_TOXIC_SPORE_CHECK, 1000);
    }
};

void SetupTheNighthold(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_THE_NIGHTHOLD, &NightholdInstanceScript::Create);

    // Krosus
    mgr->register_creature_script(BOSS_KROSUS, &NightholdKrosusAI::Create);
    mgr->register_creature_script(NPC_BURNING_EMBER, &NightholdBurningEmberAI::Create);

    // Skorpyron
    mgr->register_creature_script(BOSS_SKORPYRON, &NightholdSkorpyronAI::Create);
    mgr->register_creature_script(NPC_CRYSTALLINE_SCORPID, &NightholdScorpidAI::Create);
    mgr->register_creature_script(NPC_VOLATILE_SCORPID, &NightholdScorpidAI::Create);

    // Tichondrius
    mgr->register_creature_script(BOSS_TICHONDRIUS, &NightholdTichondriusAI::Create);
    mgr->register_creature_script(NPC_FEL_SPIRE, &NightholdFelSpireAI::Create);
    mgr->register_creature_script(NPC_CARRION_NIGHTMARE, &NightholdCarrionNightmareAI::Create);
    mgr->register_creature_script(NPC_PHANTASMAL_BLOODFANG, &NightholdPhantasmalBloodfangAI::Create);
    mgr->register_creature_script(NPC_FELSWORN_SPELLGUARD, &NightholdFelswornSpellguardAI::Create);
    mgr->register_creature_script(NPC_SIGHTLESS_WATCHER, &NightholdSightlessWatcherAI::Create);

    // Star Augur Etraeus
    mgr->register_creature_script(BOSS_STAR_AUGUR_ETRAEUS, &NightholdEtraeusAI::Create);
    mgr->register_creature_script(NPC_CORONAL_EJECTION, &NightholdCoronalEjectionAI::Create);
    mgr->register_creature_script(NPC_THING_THAT_SHOULD_NOT_BE, &NightholdThingThatShouldNotBeAI::Create);

    // Chronomatic Anomaly
    mgr->register_creature_script(BOSS_CHRONOMATIC_ANOMALY, &NightholdChronomaticAnomalyAI::Create);
    mgr->register_creature_script(NPC_WANING_TIME_PARTICLE, &NightholdWaningTimeParticleAI::Create);

    // Gul'dan
    mgr->register_creature_script(BOSS_GULDAN, &NightholdGuldanAI::Create);
    mgr->register_creature_script(NPC_FEL_LORD_KURAZMAL, &NightholdKurazmalAI::Create);
    mgr->register_creature_script(NPC_INQUISITOR_VETHRIZ, &NightholdVethrizAI::Create);
    mgr->register_creature_script(NPC_DZORYKX_THE_TRAPPER, &NightholdDzorykxAI::Create);
    mgr->register_creature_script(NPC_EYE_OF_GULDAN, &NightholdEyeOfGuldanAI::Create);
    mgr->register_creature_script(NPC_EMPOWERED_EYE_OF_GULDAN, &NightholdEyeOfGuldanAI::Create);

    // Trilliax
    mgr->register_creature_script(BOSS_TRILLIAX, &NightholdTrilliaxAI::Create);
    mgr->register_creature_script(NPC_TOXIC_SLICE, &NightholdTrilliaxLeftoverAI::Create);
    mgr->register_creature_script(NPC_SUCCULENT_FEAST, &NightholdTrilliaxLeftoverAI::Create);
    mgr->register_creature_script(NPC_SCRUBBER, &NightholdScrubberAI::Create);

    // Spellblade Aluriel
    mgr->register_creature_script(BOSS_SPELLBLADE_ALURIEL, &NightholdAlurielAI::Create);
    mgr->register_creature_script(NPC_ICY_ENCHANTMENT, &NightholdAlurielEnchantmentAI::Create);
    mgr->register_creature_script(NPC_FIERY_ENCHANTMENT, &NightholdAlurielEnchantmentAI::Create);
    mgr->register_creature_script(NPC_ARCANE_ENCHANTMENT, &NightholdAlurielEnchantmentAI::Create);

    // Grand Magistrix Elisande
    mgr->register_creature_script(BOSS_ELISANDE, &NightholdElisandeAI::Create);
    mgr->register_creature_script(NPC_EPOCHERIC_ORB, &NightholdEpochericOrbAI::Create);
    mgr->register_creature_script(NPC_RECURSIVE_ELEMENTAL, &NightholdRecursiveElementalAI::Create);
    mgr->register_creature_script(NPC_EXPEDIENT_ELEMENTAL, &NightholdExpedientElementalAI::Create);

    // High Botanist Tel'arn
    mgr->register_creature_script(BOSS_HIGH_BOTANIST_TELARN, &NightholdTelarnAI::Create);
    mgr->register_creature_script(NPC_SOLARIST_TELARN, &NightholdSolaristTelarnAI::Create);
    mgr->register_creature_script(NPC_NATURALIST_TELARN, &NightholdNaturalistTelarnAI::Create);
    mgr->register_creature_script(NPC_PARASITIC_LASHER, &NightholdParasiticLasherAI::Create);
    mgr->register_creature_script(NPC_CONTROLLED_CHAOS_STALKER, &NightholdControlledChaosAI::Create);
    mgr->register_creature_script(NPC_SOLAR_COLLAPSE_STALKER, &NightholdSolarCollapseAI::Create);
    mgr->register_creature_script(NPC_PLASMA_SPHERE, &NightholdPlasmaSphereAI::Create);
    mgr->register_creature_script(NPC_TOXIC_SPORE, &NightholdToxicSporeAI::Create);
}
