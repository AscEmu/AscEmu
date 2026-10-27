/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Raid_Antorus_The_Burning_Throne.hpp"

#include "Map/Maps/WorldMap.hpp"
#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <cmath>
#include <map>
#include <vector>

enum AntorusEvents : uint32_t
{
    // Garothi Worldbreaker
    EVENT_GAROTHI_FEL_BOMBARDMENT = 1,
    EVENT_GAROTHI_WEAPON,
    EVENT_GAROTHI_DECIMATION_IMPACT,
    EVENT_GAROTHI_REACH_CHECK,
    EVENT_GAROTHI_DRIVE_END,
    EVENT_GAROTHI_ERADICATION,

    // Felhounds of Sargeras
    EVENT_FELHOUND_BLESSING_CHECK,
    EVENT_FELHOUND_CORRUPTION_IMPACT,
    EVENT_SHATUG_CORRUPTING_MAW,
    EVENT_FHARG_BURNING_MAW,
    EVENT_FHARG_DESOLATE_PATH,

    // Antoran High Command
    EVENT_COMMAND_EXPLOIT_WEAKNESS,
    EVENT_COMMAND_ASSUME_COMMAND,
    EVENT_COMMAND_HANDOVER,
    EVENT_COMMAND_FUSILLADE,
    EVENT_COMMAND_FUSILLADE_VOLLEY,
    EVENT_COMMAND_ENTROPIC_MINES,
    EVENT_COMMAND_REINFORCEMENTS,
    EVENT_MINE_SCAN,
    EVENT_PYROMANCER_PYROBLAST,
    EVENT_SHOCKTROOPER_DEMONIC_CHARGE,
    EVENT_SHOCKTROOPER_BLADESTORM,

    // Portal Keeper Hasabel
    EVENT_HASABEL_REALITY_TEAR,
    EVENT_HASABEL_FELSTORM_BARRAGE,
    EVENT_HASABEL_FELCRUSH_ROW,
    EVENT_HASABEL_TRANSPORT_PORTAL,
    EVENT_HASABEL_PORTAL_RELEASE,
    EVENT_IMP_FIERY_DETONATION,
    EVENT_SKITTERER_ACIDIC_WEB,
    EVENT_STALKER_HOWLING_SHADOWS,

    // The Defense of Eonar
    EVENT_EONAR_GROUND_WAVE,
    EVENT_EONAR_SKY_WAVE,
    EVENT_EONAR_RAIN_OF_FEL,
    EVENT_EONAR_RAIN_OF_FEL_IMPACT,
    EVENT_EONAR_DEFENDER_CHECK,
    EVENT_DEMON_ADVANCE,
    EVENT_DEMON_STRIKE,
    EVENT_PURIFIER_FEL_SWIPE,

    // Imonar the Soulhunter
    EVENT_IMONAR_SHOCK_LANCE,
    EVENT_IMONAR_SLEEP_CANISTER,
    EVENT_IMONAR_PULSE_GRENADE,
    EVENT_IMONAR_SEVER,
    EVENT_IMONAR_CHARGED_BLASTS,
    EVENT_IMONAR_EMPOWERED_SHOCK_LANCE,
    EVENT_IMONAR_EMPOWERED_PULSE,
    EVENT_IMONAR_EMPOWERED_SHRAPNEL,
    EVENT_IMONAR_ROCKET_CHECK,
    EVENT_IMONAR_FORCED_LANDING,
    EVENT_BOMBING_RUN_MARK,
    EVENT_BOMBING_RUN_FIRE,
    EVENT_ARTILLERY_STRIKE_FIRE,

    // Kin'garoth
    EVENT_KINGAROTH_FORGING_STRIKE,
    EVENT_KINGAROTH_REVERBERATING_STRIKE,
    EVENT_KINGAROTH_DIABOLIC_BOMB,
    EVENT_KINGAROTH_RUINER,
    EVENT_KINGAROTH_REACH_CHECK,
    EVENT_KINGAROTH_CONSTRUCTION,
    EVENT_KINGAROTH_CONSTRUCTION_END,
    EVENT_KINGAROTH_APOCALYPSE_BLAST,
    EVENT_RUINER_SWEEP,
    EVENT_REAVER_ACTIVATE,
    EVENT_REAVER_ANNIHILATION,
    EVENT_REAVER_DECIMATION,
    EVENT_REAVER_DEMOLISH,
    EVENT_REAVER_MARK_IMPACT,

    // Varimathras
    EVENT_VARIMATHRAS_TORMENT,
    EVENT_VARIMATHRAS_DARK_FISSURE,
    EVENT_VARIMATHRAS_DARK_ERUPTION,
    EVENT_VARIMATHRAS_MARKED_PREY,
    EVENT_VARIMATHRAS_SHADOW_HUNTER,
    EVENT_VARIMATHRAS_NECROTIC_EMBRACE,

    // The Coven of Shivarra
    EVENT_ASARA_TOUCH_OF_DARKNESS,
    EVENT_ASARA_SHADOW_BLADES,
    EVENT_NOURA_FIERY_STRIKE,
    EVENT_NOURA_FULMINATING_PULSE,
    EVENT_DIIMA_FLASHFREEZE,
    EVENT_DIIMA_CHILLED_BLOOD,
    EVENT_THURAYA_TOUCH_OF_THE_COSMOS,
    EVENT_SHADOW_BLADE_LAUNCH,
    EVENT_TORMENT_PULSE,

    // Aggramar
    EVENT_AGGRAMAR_TECHNIQUE_STRIKE,
    EVENT_AGGRAMAR_TEMPEST_WAVE,
    EVENT_AGGRAMAR_SCORCHING_BLAZE,
    EVENT_AGGRAMAR_WAKE_OF_FLAME,
    EVENT_AGGRAMAR_FLARE,
    EVENT_AGGRAMAR_RAISE_AEGIS,
    EVENT_EMBER_SEEK,

    // Argus the Unmaker
    EVENT_ARGUS_SWEEPING_SCYTHE,
    EVENT_ARGUS_TORTURED_RAGE,
    EVENT_ARGUS_SOULBLIGHT_ORB,
    EVENT_ARGUS_SKY_AND_SEA,
    EVENT_ARGUS_EDGE_OF_OBLITERATION,
    EVENT_ARGUS_AVATAR_OF_AGGRAMAR,
    EVENT_ARGUS_VOLATILE_SOUL,
    EVENT_ARGUS_TEMPORAL_BLAST,
    EVENT_ARGUS_CONSTELLARS,
    EVENT_ARGUS_COSMIC_BEACON,
    EVENT_ARGUS_REORIGINATION_MODULES,
    EVENT_EDGE_TARGET,
    EVENT_EDGE_SWEEP,
    EVENT_CONSTELLAR_STARBLAST,
    EVENT_CONSTELLAR_SMASH,
    EVENT_MODULE_INITIALIZE,
    EVENT_MODULE_PULSE
};

enum AntorusActions : int32_t
{
    ACTION_ANTORUS_CLEANUP = 1,

    ACTION_GAROTHI_ANNIHILATOR_LOST,
    ACTION_GAROTHI_DECIMATOR_LOST,

    ACTION_COMMAND_TAKE,
    ACTION_COMMAND_YIELD,

    ACTION_COVEN_BEGIN_DUTY,
    ACTION_COVEN_END_DUTY,
    ACTION_COVEN_TORMENT_GOLGANNETH,
    ACTION_COVEN_TORMENT_KHAZGOROTH,
    ACTION_COVEN_TORMENT_AMANTHUL,

    ACTION_KINGAROTH_REAVER_DOWN,
    ACTION_AGGRAMAR_FLAME_DOWN,
    ACTION_ARGUS_CONSTELLAR_DOWN,
    ACTION_CONSTELLAR_BEACON
};

enum AntorusCreatureData : uint32_t
{
    EONAR_DATA_DEMON_SLAIN = 1,
    PORTAL_DATA_STAGE      = 2
};

enum AntorusMovePoints : uint32_t
{
    POINT_DEMON_TO_EONAR     = 1,
    POINT_IMONAR_CROSSING    = 2,
    POINT_RUINER_STEP        = 3,
    POINT_COVEN_PEDESTAL     = 4,
    POINT_SHADOW_BLADE_PATH  = 5,
    POINT_AGGRAMAR_CENTRE    = 6,
    POINT_EMBER_TO_AGGRAMAR  = 7,
    POINT_ARGUS_CENTRE       = 8,
    POINT_EDGE_SWEEP         = 9
};

namespace
{
    float const AntorusTwoPi = 6.28318531f;
    float const AntorusHalfPi = 1.57079633f;

    // Garothi Worldbreaker: arena centre
    LocationVector const GarothiArenaCentre(-3300.20f, 9772.24f, -63.35f);

    // High Command: fusillade impact grid
    LocationVector const CommandFusilladeGrid[12] =
    {
        { -2787.93f, 10830.80f, 139.62f },
        { -2819.56f, 10822.47f, 139.62f },
        { -2796.53f, 10862.35f, 139.62f },
        { -2839.66f, 10834.07f, 139.62f },
        { -2828.15f, 10854.02f, 139.62f },
        { -2816.64f, 10873.96f, 139.62f },
        { -2805.12f, 10893.90f, 139.62f },
        { -2859.77f, 10845.68f, 139.62f },
        { -2848.26f, 10865.63f, 139.62f },
        { -2836.74f, 10885.57f, 139.62f },
        { -2879.88f, 10857.29f, 139.62f },
        { -2868.36f, 10877.23f, 139.62f }
    };

    // Hasabel: felcrush portal pairs on the nexus platform
    LocationVector const HasabelFelcrushSpots[3][2] =
    {
        { { -3738.13f, -1388.33f, 624.49f, 0.88f }, { -3666.90f, -1312.99f, 624.49f, 4.09f } },
        { { -3653.38f, -1359.35f, 624.49f, 2.85f }, { -3753.77f, -1341.14f, 624.48f, 5.94f } },
        { { -3725.36f, -1307.77f, 624.49f, 5.30f }, { -3688.55f, -1396.82f, 624.49f, 2.04f } }
    };

    // Eonar: lane entries mid, bottom, top and sky
    enum EonarLane : uint8_t
    {
        EONAR_LANE_MID = 0,
        EONAR_LANE_BOTTOM,
        EONAR_LANE_TOP,
        EONAR_LANE_SKY
    };

    LocationVector const EonarLaneEntries[4] =
    {
        { -3993.1f, -10939.0f, 721.8f, 0.8f },
        { -3880.1f, -10927.4f, 685.9f, 2.4f },
        { -3927.6f, -10668.0f, 751.6f, 4.0f },
        { -4003.0f, -10650.1f, 760.0f, 4.6f }
    };

    LocationVector const ParaxisAnchor(-4208.913f, -10699.95f, 1061.243f, 1.457f);

    struct EonarSpawnGroup
    {
        uint8_t wave;
        uint8_t lane;
        uint32_t entry;
        uint8_t count;
        uint32_t delay;
        uint32_t spacing;
        bool hardOnly;
    };

    EonarSpawnGroup const EonarGroundWaves[] =
    {
        { 1, EONAR_LANE_MID,    NPC_EONAR_FELGUARD,         6, 1000,  400,   false },
        { 1, EONAR_LANE_MID,    NPC_FEL_INFUSED_DESTRUCTOR, 1, 3000,  0,     false },
        { 2, EONAR_LANE_BOTTOM, NPC_EONAR_FEL_HOUND,        12, 20000, 700,  false },
        { 2, EONAR_LANE_BOTTOM, NPC_FEL_INFUSED_DESTRUCTOR, 1, 28000, 0,     false },
        { 3, EONAR_LANE_TOP,    NPC_EONAR_FEL_LORD,         2, 10000, 5000,  false },
        { 3, EONAR_LANE_TOP,    NPC_FEL_CHARGED_OBFUSCATOR, 1, 20000, 0,     false },
        { 3, EONAR_LANE_TOP,    NPC_FEL_INFUSED_DESTRUCTOR, 1, 25000, 0,     false },
        { 4, EONAR_LANE_MID,    NPC_EONAR_FELGUARD,         6, 21000, 400,   false },
        { 4, EONAR_LANE_MID,    NPC_FEL_POWERED_PURIFIER,   1, 23500, 0,     true  },
        { 5, EONAR_LANE_TOP,    NPC_EONAR_FELGUARD,         6, 25000, 400,   false },
        { 5, EONAR_LANE_BOTTOM, NPC_EONAR_FELGUARD,         5, 25000, 400,   false },
        { 5, EONAR_LANE_BOTTOM, NPC_FEL_INFUSED_DESTRUCTOR, 1, 20000, 0,     false },
        { 5, EONAR_LANE_BOTTOM, NPC_FEL_POWERED_PURIFIER,   1, 20000, 0,     true  },
        { 6, EONAR_LANE_MID,    NPC_EONAR_FEL_LORD,         2, 25000, 10000, false },
        { 6, EONAR_LANE_MID,    NPC_FEL_CHARGED_OBFUSCATOR, 1, 30000, 0,     false },
        { 6, EONAR_LANE_MID,    NPC_EONAR_FEL_HOUND,        6, 27000, 700,   false },
        { 6, EONAR_LANE_MID,    NPC_FEL_POWERED_PURIFIER,   1, 20000, 0,     true  },
        { 7, EONAR_LANE_MID,    NPC_EONAR_FEL_LORD,         1, 20000, 0,     false },
        { 7, EONAR_LANE_BOTTOM, NPC_EONAR_FEL_LORD,         1, 20000, 0,     false },
        { 7, EONAR_LANE_TOP,    NPC_EONAR_FEL_LORD,         1, 20000, 0,     false },
        { 7, EONAR_LANE_MID,    NPC_FEL_INFUSED_DESTRUCTOR, 1, 25000, 0,     false },
        { 7, EONAR_LANE_BOTTOM, NPC_FEL_CHARGED_OBFUSCATOR, 1, 25000, 0,     false },
        { 7, EONAR_LANE_TOP,    NPC_FEL_INFUSED_DESTRUCTOR, 1, 25000, 0,     false },
        { 8, EONAR_LANE_MID,    NPC_EONAR_FEL_LORD,         1, 20000, 0,     false },
        { 8, EONAR_LANE_BOTTOM, NPC_EONAR_FEL_LORD,         1, 20000, 0,     false },
        { 8, EONAR_LANE_TOP,    NPC_EONAR_FEL_LORD,         1, 20000, 0,     false },
        { 8, EONAR_LANE_MID,    NPC_FEL_INFUSED_DESTRUCTOR, 1, 25000, 0,     false },
        { 8, EONAR_LANE_BOTTOM, NPC_FEL_CHARGED_OBFUSCATOR, 1, 25000, 0,     false },
        { 8, EONAR_LANE_TOP,    NPC_FEL_INFUSED_DESTRUCTOR, 1, 25000, 0,     false }
    };

    // start to start gap after each ground wave, 0 ends the sequence
    uint32_t const EonarGroundWaveGaps[8] = { 5000, 44000, 35000, 71000, 30000, 100000, 100000, 0 };
    uint32_t const EonarFirstGroundWave = 10000;

    EonarSpawnGroup const EonarSkyWaves[] =
    {
        { 1, EONAR_LANE_SKY, NPC_VOLANT_KERAPTERON, 5, 9000,  1000, false },
        { 2, EONAR_LANE_SKY, NPC_VOLANT_KERAPTERON, 8, 21000, 1000, false },
        { 3, EONAR_LANE_SKY, NPC_VOLANT_KERAPTERON, 8, 18000, 1000, false }
    };

    uint32_t const EonarSkyWaveGaps[3] = { 114000, 81000, 0 };
    uint32_t const EonarFirstSkyWave = 146000;
    uint32_t const EonarLifeForcesToWin = 4;

    // Imonar: platform centres and ship berths
    LocationVector const ImonarPlatforms[2] =
    {
        { -10574.5f, 8723.3f, 1871.56f },
        { -10575.8f, 8538.3f, 1871.53f }
    };

    LocationVector const ImonarBomberBerths[5] =
    {
        { -10627.9f, 8495.83f, 1878.13f, 0.64f },
        { -10625.3f, 8535.34f, 1878.15f, 0.01f },
        { -10621.7f, 8571.31f, 1878.13f, 5.63f },
        { -10538.6f, 8499.77f, 1878.13f, 2.35f },
        { -10531.3f, 8555.84f, 1878.13f, 3.56f }
    };

    LocationVector const ImonarArtilleryBerths[2] =
    {
        { -10533.5f, 8757.92f, 1892.45f, 4.14f },
        { -10616.6f, 8708.64f, 1904.49f, 0.57f }
    };

    // Kin'garoth: reaver bays left, right and back
    LocationVector const KingarothReaverBays[3] =
    {
        { -10635.07f, 8142.42f, 1873.17f, 5.65f },
        { -10514.54f, 8142.44f, 1873.17f, 3.76f },
        { -10574.77f, 8024.12f, 1873.14f, 1.57f }
    };

    // reaver entries per round and bay, normal then harder difficulties
    uint32_t const KingarothNormalReavers[3][3] =
    {
        { NPC_GAROTHI_REAVER_DECIMATOR,   NPC_GAROTHI_REAVER_DEMOLISHER, 0 },
        { 0,                              NPC_GAROTHI_REAVER_ANNIHILATOR, NPC_GAROTHI_REAVER_DEMOLISHER },
        { NPC_GAROTHI_REAVER_DECIMATOR,   0,                              NPC_GAROTHI_REAVER_DEMOLISHER }
    };

    uint32_t const KingarothHardReavers[3][3] =
    {
        { NPC_GAROTHI_REAVER_DECIMATOR,   NPC_GAROTHI_REAVER_DEMOLISHER, NPC_GAROTHI_REAVER_ANNIHILATOR },
        { NPC_GAROTHI_REAVER_ANNIHILATOR, NPC_GAROTHI_REAVER_DEMOLISHER, NPC_GAROTHI_REAVER_ANNIHILATOR },
        { NPC_GAROTHI_REAVER_DECIMATOR,   NPC_GAROTHI_REAVER_DECIMATOR,  NPC_GAROTHI_REAVER_ANNIHILATOR }
    };

    // Coven: torment pedestals and torment soul spots
    LocationVector const CovenPedestals[3] =
    {
        { -12663.24f, -2791.05f, 2480.8f, 2.36f },
        { -12605.61f, -2791.02f, 2480.8f, 0.73f },
        { -12605.71f, -2848.49f, 2480.8f, 5.46f }
    };

    LocationVector const CovenAmanthulSouls[4] =
    {
        { -12667.2f, -2834.27f, 2475.08f, 0.42f },
        { -12649.5f, -2787.94f, 2475.15f, 5.16f },
        { -12602.6f, -2804.43f, 2475.37f, 3.58f },
        { -12618.8f, -2851.78f, 2475.34f, 2.01f }
    };

    LocationVector const CovenGolgannethSouls[4] =
    {
        { -12634.49f, -2780.56f, 2475.22f, 4.71f },
        { -12676.59f, -2820.14f, 2475.34f, 0.01f },
        { -12634.44f, -2860.37f, 2475.22f, 1.57f },
        { -12590.45f, -2820.04f, 2475.17f, 3.14f }
    };

    LocationVector const CovenKhazgorothSouls[8] =
    {
        { -12676.9f, -2820.13f, 2476.93f, 0.00f },
        { -12656.8f, -2842.03f, 2475.54f, 0.80f },
        { -12634.3f, -2859.33f, 2476.88f, 1.57f },
        { -12612.0f, -2842.17f, 2475.54f, 2.31f },
        { -12586.8f, -2819.73f, 2476.91f, 3.13f },
        { -12612.1f, -2797.80f, 2475.54f, 3.95f },
        { -12634.9f, -2781.53f, 2476.91f, 4.72f },
        { -12656.6f, -2797.45f, 2475.54f, 5.48f }
    };

    uint32_t const CovenTormentIntervalMs = 80000;
    uint32_t const CovenTormentsPerDuty = 2;

    enum CovenTorment : uint32_t
    {
        COVEN_TORMENT_GOLGANNETH = 0,
        COVEN_TORMENT_KHAZGOROTH,
        COVEN_TORMENT_AMANTHUL,
        COVEN_TORMENT_COUNT
    };

    // Aggramar: arena centre
    LocationVector const AggramarArenaCentre(-12634.27f, -2254.4f, 2514.26f);

    // Argus: arena centre and constellar stations
    LocationVector const ArgusArenaCentre(2867.12f, -4567.54f, 292.94f);

    LocationVector const ArgusConstellarStations[7] =
    {
        { 2828.07f, -4553.25f, 292.029f, 5.92f },
        { 2856.43f, -4533.67f, 292.027f, 5.00f },
        { 2828.72f, -4584.80f, 292.029f, 0.42f },
        { 2890.83f, -4538.69f, 292.028f, 4.02f },
        { 2852.88f, -4606.91f, 292.029f, 1.22f },
        { 2905.77f, -4570.62f, 292.028f, 3.06f },
        { 2888.44f, -4600.90f, 292.028f, 2.14f }
    };

    // point on a circle around origin, facing outwards
    LocationVector ringPoint(LocationVector const& origin, float range, float angle)
    {
        return LocationVector(origin.x + range * std::cos(angle), origin.y + range * std::sin(angle), origin.z, angle);
    }

    LocationVector scatterAround(LocationVector const& origin, float minRange, float maxRange)
    {
        return ringPoint(origin, Util::getRandomFloat(minRange, maxRange), Util::getRandomFloat(AntorusTwoPi));
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class AntorusInstanceScript : public InstanceScript
{
public:
    explicit AntorusInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(AntorusEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new AntorusInstanceScript(pMapMgr); }

    bool setBossState(uint32_t id, EncounterStates state) override
    {
        if (isGroupEncounter(id) && getBossState(id) == EncounterStates::InProgress)
        {
            // a credited member falling before its partners must not end the encounter
            if (state == EncounterStates::Performed && !isGroupDown(id, nullptr))
                return false;

            // a member stepping out while the others still fight
            if (state == EncounterStates::NotStarted && isGroupFighting(id, nullptr))
                return false;
        }

        return InstanceScript::setBossState(id, state);
    }

    void OnCreaturePushToWorld(Creature* pCreature) override
    {
        switch (pCreature->getEntry())
        {
            case BOSS_GAROTHI_WORLDBREAKER:
            case BOSS_FHARG:
            case BOSS_SHATUG:
            case BOSS_ADMIRAL_SVIRAX:
            case BOSS_CHIEF_ENGINEER_ISHKAR:
            case BOSS_GENERAL_ERODUS:
            case BOSS_PORTAL_KEEPER_HASABEL:
            case NPC_ESSENCE_OF_EONAR:
            case BOSS_IMONAR:
            case BOSS_KINGAROTH:
            case BOSS_VARIMATHRAS:
            case BOSS_ASARA:
            case BOSS_NOURA:
            case BOSS_DIIMA:
            case BOSS_THURAYA:
            case BOSS_AGGRAMAR:
            case BOSS_ARGUS_THE_UNMAKER:
                mTrackedGuids[pCreature->getEntry()] = pCreature->GetNewGUID();
                break;
            default:
                break;
        }
    }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        bool open = false;
        if (!doorRule(pGameObject->getEntry(), open))
            return;

        mDoorGuids.push_back(pGameObject->GetNewGUID());
        alignDoor(pGameObject, open);
    }

    void OnEncounterStateChange(uint32_t id, uint32_t state) override
    {
        refreshDoors();

        if (id == DATA_HIGH_COMMAND && (state == EncounterStates::InProgress || state == EncounterStates::Failed))
            mActiveCommander = BOSS_ADMIRAL_SVIRAX;

        if (id == DATA_COVEN)
        {
            if (state == EncounterStates::InProgress)
                beginTorments();
            else
                endTorments();
        }

        if (state != EncounterStates::Failed)
            return;

        // a wiped group fights again in full
        for (Creature* pMember : getGroup(id))
        {
            if (!pMember->isAlive())
            {
                pMember->despawn(1000, 5000);
                continue;
            }

            pMember->setHealth(pMember->getMaxHealth());
            if (pMember->GetScript() != nullptr)
                pMember->GetScript()->DoAction(ACTION_ANTORUS_CLEANUP);
        }
    }

    void UpdateEvent() override
    {
        if (mTormentTimer == 0 || !isTimerFinished(mTormentTimer))
            return;

        resetTimer(mTormentTimer, CovenTormentIntervalMs);
        unleashTorment();
    }

    Creature* getTracked(uint32_t entry)
    {
        auto const itr = mTrackedGuids.find(entry);
        return itr != mTrackedGuids.end() ? getCreatureByGuid(itr->second) : nullptr;
    }

    void sendAction(uint32_t entry, int32_t action)
    {
        Creature* pTarget = getTracked(entry);
        if (pTarget != nullptr && pTarget->isAlive() && pTarget->GetScript() != nullptr)
            pTarget->GetScript()->DoAction(action);
    }

    bool isGroupEncounter(uint32_t id) const { return !groupEntries(id).empty(); }

    std::vector<Creature*> getGroup(uint32_t id)
    {
        std::vector<Creature*> group;
        for (uint32_t entry : groupEntries(id))
        {
            if (Creature* pMember = getTracked(entry))
                group.push_back(pMember);
        }

        return group;
    }

    bool isGroupMember(uint32_t id, Unit* pUnit) const
    {
        if (pUnit == nullptr || !pUnit->isCreature())
            return false;

        for (uint32_t entry : groupEntries(id))
        {
            if (pUnit->getEntry() == entry)
                return true;
        }

        return false;
    }

    bool isGroupDown(uint32_t id, Creature* pExcept)
    {
        for (Creature* pMember : getGroup(id))
        {
            if (pMember != pExcept && pMember->isAlive())
                return false;
        }

        return true;
    }

    bool isGroupFighting(uint32_t id, Creature* pExcept)
    {
        for (Creature* pMember : getGroup(id))
        {
            if (pMember != pExcept && pMember->isAlive() && pMember->isInCombat())
                return true;
        }

        return false;
    }

    // shared health pool: the blow lands on every member
    void shareDamage(uint32_t id, Creature* pSource, uint32_t amount)
    {
        for (Creature* pMember : getGroup(id))
        {
            if (pMember == pSource || !pMember->isAlive())
                continue;

            uint32_t const health = pMember->getHealth();
            pMember->setHealth(health > amount ? health - amount : 1);
        }
    }

    // the last blow on one member ends the whole pool
    void collapseGroup(uint32_t id, Creature* pSource)
    {
        if (mCollapsing)
            return;

        mCollapsing = true;
        for (Creature* pMember : getGroup(id))
        {
            if (pMember != pSource && pMember->isAlive())
                pMember->dealDamage(pMember, pMember->getHealth(), 0);
        }
        mCollapsing = false;
    }

    // High Command: the commander on the field
    uint32_t activeCommander() const { return mActiveCommander; }

    void passCommand()
    {
        uint32_t const previous = mActiveCommander;
        uint32_t const next = nextCommander(previous);

        Creature* pNext = getTracked(next);
        if (pNext == nullptr || !pNext->isAlive())
            return;

        if (Creature* pPrevious = getTracked(previous))
        {
            uint64_t const scaled = static_cast<uint64_t>(pPrevious->getHealth()) * pNext->getMaxHealth() / (pPrevious->getMaxHealth() > 0 ? pPrevious->getMaxHealth() : 1);
            pNext->setHealth(scaled > 0 ? static_cast<uint32_t>(scaled) : 1);
        }

        // the new commander engages before the old one steps back
        mActiveCommander = next;
        sendAction(next, ACTION_COMMAND_TAKE);
        sendAction(previous, ACTION_COMMAND_YIELD);
    }

    // Coven: the sister performing the torments
    uint32_t covenDuty() const { return mCovenDuty; }

private:
    std::vector<uint32_t> groupEntries(uint32_t id) const
    {
        switch (id)
        {
            case DATA_HIGH_COMMAND:
                return { BOSS_ADMIRAL_SVIRAX, BOSS_CHIEF_ENGINEER_ISHKAR, BOSS_GENERAL_ERODUS };
            case DATA_COVEN:
                return { BOSS_ASARA, BOSS_NOURA, BOSS_DIIMA, BOSS_THURAYA };
            case DATA_FELHOUNDS:
                return { BOSS_FHARG, BOSS_SHATUG };
            default:
                return {};
        }
    }

    static uint32_t nextCommander(uint32_t entry)
    {
        switch (entry)
        {
            case BOSS_ADMIRAL_SVIRAX:
                return BOSS_CHIEF_ENGINEER_ISHKAR;
            case BOSS_CHIEF_ENGINEER_ISHKAR:
                return BOSS_GENERAL_ERODUS;
            default:
                return BOSS_ADMIRAL_SVIRAX;
        }
    }

    static uint32_t nextDutySister(uint32_t entry)
    {
        switch (entry)
        {
            case BOSS_DIIMA:
                return BOSS_NOURA;
            case BOSS_NOURA:
                return BOSS_ASARA;
            default:
                return BOSS_DIIMA;
        }
    }

    bool finished(uint32_t id) const { return getBossState(id) == EncounterStates::Performed; }
    bool fighting(uint32_t id) const { return getBossState(id) == EncounterStates::InProgress; }

    // false for objects that are no encounter door
    bool doorRule(uint32_t entry, bool& open) const
    {
        switch (entry)
        {
            case GO_GAROTHI_ROCK:
            case GO_GAROTHI_COLLISION:
                open = finished(DATA_GAROTHI);
                return true;
            case GO_HIGH_COMMAND_BARRIER:
                open = !fighting(DATA_HIGH_COMMAND);
                return true;
            case GO_HASABEL_BARRIER:
                open = !fighting(DATA_HASABEL);
                return true;
            case GO_EONAR_DOOR:
                open = !fighting(DATA_EONAR);
                return true;
            case GO_IMONAR_ARCANE_BARRIER:
                open = finished(DATA_IMONAR);
                return true;
            case GO_KINGAROTH_BARRIER:
                open = !fighting(DATA_KINGAROTH);
                return true;
            case GO_VARIMATHRAS_ENTRANCE:
                open = !fighting(DATA_VARIMATHRAS);
                return true;
            case GO_VARIMATHRAS_EXIT:
                open = finished(DATA_VARIMATHRAS);
                return true;
            case GO_COVEN_TEMPLE_DOOR:
            case GO_COVEN_TITAN_BARRIER:
                open = finished(DATA_VARIMATHRAS) && !fighting(DATA_COVEN);
                return true;
            case GO_AGGRAMAR_DOOR:
                open = finished(DATA_AGGRAMAR);
                return true;
            default:
                return false;
        }
    }

    // changes the state only when it differs
    void alignDoor(GameObject* pDoor, bool open)
    {
        bool const isOpen = pDoor->getState() != GO_STATE_CLOSED;
        if (isOpen != open)
            pDoor->setState(open ? GO_STATE_OPEN : GO_STATE_CLOSED);
    }

    void refreshDoors()
    {
        for (WoWGuid const& guid : mDoorGuids)
        {
            GameObject* pDoor = getGameObjectByGuid(guid);
            bool open = false;
            if (pDoor != nullptr && doorRule(pDoor->getEntry(), open))
                alignDoor(pDoor, open);
        }
    }

    void beginTorments()
    {
        mCovenDuty = BOSS_DIIMA;
        mTormentsThisDuty = 0;
        mLastTorment = COVEN_TORMENT_COUNT;

        if (mTormentTimer != 0)
            removeTimer(mTormentTimer);
        mTormentTimer = addTimer(CovenTormentIntervalMs);

        sendAction(BOSS_DIIMA, ACTION_COVEN_BEGIN_DUTY);
    }

    void endTorments()
    {
        if (mTormentTimer != 0)
            removeTimer(mTormentTimer);

        mCovenDuty = 0;
    }

    // a torment never repeats right away
    void unleashTorment()
    {
        uint32_t torment = Util::getRandomUInt(COVEN_TORMENT_COUNT - 1);
        if (torment == mLastTorment)
            torment = (torment + 1 + Util::getRandomUInt(COVEN_TORMENT_COUNT - 2)) % COVEN_TORMENT_COUNT;
        mLastTorment = torment;

        sendAction(mCovenDuty, ACTION_COVEN_TORMENT_GOLGANNETH + static_cast<int32_t>(torment));

        if (++mTormentsThisDuty < CovenTormentsPerDuty)
            return;

        // the next sister takes over the torments
        mTormentsThisDuty = 0;
        uint32_t const previous = mCovenDuty;
        mCovenDuty = nextDutySister(previous);
        sendAction(previous, ACTION_COVEN_END_DUTY);
        sendAction(mCovenDuty, ACTION_COVEN_BEGIN_DUTY);
    }

    std::map<uint32_t, WoWGuid> mTrackedGuids;
    std::vector<WoWGuid> mDoorGuids;
    bool mCollapsing = false;

    uint32_t mActiveCommander = BOSS_ADMIRAL_SVIRAX;

    uint32_t mCovenDuty = 0;
    uint32_t mTormentsThisDuty = 0;
    uint32_t mLastTorment = COVEN_TORMENT_COUNT;
    uint32_t mTormentTimer = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared base: event tick and target helpers

class AntorusAI : public CreatureAIScript
{
public:
    explicit AntorusAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override { onEngage(); }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        // helpers keep their own schedule outside of combat
        if (!mAlwaysActive)
        {
            scriptEvents.resetEvents();
            summons.despawnAll();
        }

        onDisengage();
    }

    void onSummonedCreature(Creature* pSummon) override { summons.summon(pSummon); }

    void DoAction(int32_t action) override
    {
        if (action == ACTION_ANTORUS_CLEANUP)
            summons.despawnAll();
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!mAlwaysActive && !_isInCombat())
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        scriptEvents.updateEvents(diff, 0);
        onTick(diff);

        if (_isCasting())
            return;

        if (uint32_t const eventId = scriptEvents.getFinishedEvent())
            onEvent(eventId);
    }

protected:
    virtual void onEngage() {}
    virtual void onDisengage() {}
    virtual void onTick(uint32_t /*diff*/) {}
    virtual void onEvent(uint32_t /*eventId*/) {}

    AntorusInstanceScript* antorus()
    {
        if (getCreature()->GetMapId() != MAP_ANTORUS_THE_BURNING_THRONE)
            return nullptr;

        return static_cast<AntorusInstanceScript*>(getInstanceScript());
    }

    Creature* tracked(uint32_t entry)
    {
        AntorusInstanceScript* pAntorus = antorus();
        return pAntorus != nullptr ? pAntorus->getTracked(entry) : nullptr;
    }

    bool hardMode() { return isHeroic(); }

    Unit* currentVictim() { return getCreature()->getAIInterface()->getCurrentTarget(); }

    Unit* anyPlayer() { return selectUnitTarget(FilterArgs(TargetFilter_Player)); }

    // prefers someone other than the tank
    Unit* sparePlayer()
    {
        if (Unit* pTarget = selectUnitTarget(FilterArgs(static_cast<TargetFilter>(TargetFilter_Player | TargetFilter_NotCurrent))))
            return pTarget;

        return anyPlayer();
    }

    // up to count different players
    std::vector<Unit*> pickPlayers(uint32_t count, bool spareTank)
    {
        std::vector<Unit*> picked;
        for (uint32_t attempt = 0; attempt < count * 3 && picked.size() < count; ++attempt)
        {
            Unit* pTarget = spareTank ? sparePlayer() : anyPlayer();
            if (pTarget == nullptr)
                break;

            bool known = false;
            for (Unit* pKnown : picked)
            {
                if (pKnown == pTarget)
                    known = true;
            }

            if (!known)
                picked.push_back(pTarget);
        }

        return picked;
    }

    // living players of the map around this creature, independent of hostility and visibility
    std::vector<Player*> playersWithin(float range)
    {
        std::vector<Player*> found;
        WorldMap* pMap = getCreature()->getWorldMap();
        if (pMap == nullptr)
            return found;

        LocationVector const centre = getCreature()->GetPosition();
        for (auto const& mapPlayer : pMap->getPlayers())
        {
            Player* pPlayer = mapPlayer.second;
            if (pPlayer != nullptr && pPlayer->isAlive() && pPlayer->GetPosition().getExactDist(centre) <= range)
                found.push_back(pPlayer);
        }

        return found;
    }

    void castOn(Unit* pTarget, uint32_t spellId, bool triggered = false)
    {
        if (pTarget != nullptr)
            castSpell(pTarget, spellId, triggered);
    }

    Unit* unitFor(WoWGuid const& guid)
    {
        WorldMap* pMap = getCreature()->getWorldMap();
        return pMap != nullptr ? pMap->getUnit(guid) : nullptr;
    }

    Creature* callAdd(uint32_t entry, LocationVector const& where, CreatureSummonDespawnType despawnType = CORPSE_DESPAWN, uint32_t duration = 0)
    {
        Creature* pAdd = summonCreature(entry, where, despawnType, duration);
        if (pAdd != nullptr)
            setZoneWideCombat(pAdd);

        return pAdd;
    }

    LocationVector here() { return getCreature()->GetPosition(); }

    void warn(char const* text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }

    bool mAlwaysActive = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame, boss state, group pull and berserk

class AntorusBossAI : public AntorusAI
{
public:
    AntorusBossAI(Creature* pCreature, uint32_t encounterId) : AntorusAI(pCreature), mEncounterId(encounterId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mEncounterId, EncounterStates::InProgress);
        }

        if (pullsGroup())
            rallyGroup();

        mBerserkLeft = mBerserkMs;
        AntorusAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        scriptEvents.resetEvents();

        if (isAlive())
        {
            InstanceScript* pInstance = getInstanceScript();
            if (pInstance != nullptr)
                pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

            // the last member leaving the fight ends the attempt
            AntorusInstanceScript* pAntorus = antorus();
            if (pAntorus == nullptr || !pAntorus->isGroupFighting(mEncounterId, getCreature()))
            {
                if (pInstance != nullptr)
                    pInstance->setBossState(mEncounterId, EncounterStates::Failed);

                summons.despawnAll();
            }
        }

        onDisengage();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        scriptEvents.resetEvents();
        summons.despawnAll();

        InstanceScript* pInstance = getInstanceScript();
        if (pInstance != nullptr)
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

        onDefeat();

        AntorusInstanceScript* pAntorus = antorus();
        if (pInstance != nullptr && (pAntorus == nullptr || pAntorus->isGroupDown(mEncounterId, getCreature())))
            pInstance->setBossState(mEncounterId, EncounterStates::Performed);
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
                castSpellOnSelf(SPELL_ANTORUS_BERSERK, true);
            }
        }

        AntorusAI::AIUpdate(time_passed);
    }

protected:
    virtual bool pullsGroup() const { return true; }
    virtual void onDefeat() {}

    void setBerserk(uint32_t durationMs) { mBerserkMs = durationMs; }

    void rallyGroup()
    {
        AntorusInstanceScript* pAntorus = antorus();
        if (pAntorus == nullptr)
            return;

        for (Creature* pMember : pAntorus->getGroup(mEncounterId))
        {
            if (pMember != getCreature() && pMember->isAlive() && !pMember->isInCombat())
                setZoneWideCombat(pMember);
        }
    }

    void collapseOwnGroup()
    {
        if (AntorusInstanceScript* pAntorus = antorus())
            pAntorus->collapseGroup(mEncounterId, getCreature());
    }

    uint32_t mEncounterId;

private:
    uint32_t mBerserkMs = 0;
    uint32_t mBerserkLeft = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Stationary helpers: portals, triggers, orbs and the Paraxis

class AntorusFixtureAI : public AntorusAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AntorusFixtureAI(c); }
    explicit AntorusFixtureAI(Creature* pCreature) : AntorusAI(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        setRooted(true);
        _setMeleeDisabled(true);
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);

        switch (getCreature()->getEntry())
        {
            // the cast bars are the warning
            case NPC_COLLAPSING_WORLD:
                castSpellOnSelf(SPELL_HASABEL_COLLAPSING_WORLD);
                break;
            case NPC_FELCRUSH_PORTAL:
                castSpellOnSelf(SPELL_HASABEL_FELSTORM_BARRAGE_DMG);
                break;
            case NPC_TRANSPORT_PORTAL:
                castSpellOnSelf(SPELL_HASABEL_TRANSPORT_PORTAL, true);
                break;
            case NPC_SOULBLIGHT_ORB:
                castSpellOnSelf(SPELL_SOULBLIGHT_ORB_AURA, true);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Garothi Worldbreaker

class AntorusGarothiAI : public AntorusBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AntorusGarothiAI(c); }
    explicit AntorusGarothiAI(Creature* pCreature) : AntorusBossAI(pCreature, DATA_GAROTHI) {}

    void OnLoad() override
    {
        // vehicle template: the core keeps it passive unless the AI is switched on
        getCreature()->setAItoUse(true);
        setCanEnterCombat(true);
        setRooted(true);
    }

    void DoAction(int32_t action) override
    {
        if (action == ACTION_GAROTHI_ANNIHILATOR_LOST || action == ACTION_GAROTHI_DECIMATOR_LOST)
            weaponDestroyed(action == ACTION_GAROTHI_ANNIHILATOR_LOST);

        AntorusBossAI::DoAction(action);
    }

protected:
    void onEngage() override
    {
        setRooted(true);
        mAnnihilatorIntact = true;
        mDecimatorIntact = true;
        mAnnihilatorNext = Util::getRandomUInt(1) == 0;
        mDriveCount = 0;
        mDriving = false;
        mDecimationTargets.clear();
        armWeapons(true);
    }

    void onDisengage() override { stopDrive(); }

    void onTick(uint32_t /*diff*/) override
    {
        if (mDriving || mDriveCount >= 2)
            return;

        if (_getHealthPercent() < driveThreshold())
            startDrive();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_GAROTHI_FEL_BOMBARDMENT:
                castSpellOnVictim(SPELL_GAROTHI_FEL_BOMBARDMENT, true);
                scriptEvents.addEvent(EVENT_GAROTHI_FEL_BOMBARDMENT, 20000);
                break;
            case EVENT_GAROTHI_WEAPON:
                fireWeapon();
                scriptEvents.addEvent(EVENT_GAROTHI_WEAPON, 16000);
                break;
            case EVENT_GAROTHI_DECIMATION_IMPACT:
                for (WoWGuid const& guid : mDecimationTargets)
                {
                    Unit* pTarget = unitFor(guid);
                    if (pTarget != nullptr && pTarget->isAlive())
                        getCreature()->castSpellLoc(pTarget->GetPosition(), SPELL_GAROTHI_DECIMATION_MISSILE, true);
                }
                mDecimationTargets.clear();
                break;
            case EVENT_GAROTHI_REACH_CHECK:
                checkReach();
                scriptEvents.addEvent(EVENT_GAROTHI_REACH_CHECK, 3000);
                break;
            case EVENT_GAROTHI_DRIVE_END:
                summons.despawnEntry(NPC_GAROTHI_ANNIHILATOR);
                summons.despawnEntry(NPC_GAROTHI_DECIMATOR);
                stopDrive();
                castSpellOnSelf(SPELL_GAROTHI_EMPOWERED, true);
                armWeapons(false);
                break;
            case EVENT_GAROTHI_ERADICATION:
                warn("Garothi Worldbreaker begins to cast Eradication! Get away!");
                castSpellOnSelf(SPELL_GAROTHI_EMPOWERED, true);
                castSpellOnSelf(SPELL_GAROTHI_ERADICATION);
                armWeapons(false);
                break;
            default:
                break;
        }
    }

private:
    // heroic and mythic drive earlier and twice as deep
    int32_t driveThreshold()
    {
        if (hardMode())
            return mDriveCount == 0 ? 66 : 35;

        return mDriveCount == 0 ? 61 : 20;
    }

    void armWeapons(bool opening)
    {
        scriptEvents.addEvent(EVENT_GAROTHI_FEL_BOMBARDMENT, opening ? 9500 : 14000);
        scriptEvents.addEvent(EVENT_GAROTHI_REACH_CHECK, 6000);
        if (mAnnihilatorIntact || mDecimatorIntact)
            scriptEvents.addEvent(EVENT_GAROTHI_WEAPON, opening ? 8500 : 12000);
    }

    // the two cannons take turns, a destroyed one stays silent
    void fireWeapon()
    {
        if (!mAnnihilatorIntact && !mDecimatorIntact)
            return;

        bool annihilate = mAnnihilatorNext;
        if (!mAnnihilatorIntact)
            annihilate = false;
        else if (!mDecimatorIntact)
            annihilate = true;
        mAnnihilatorNext = !annihilate;

        if (annihilate)
        {
            castSpellOnSelf(SPELL_GAROTHI_ANNIHILATION, true);
            uint32_t impacts = hardMode() ? 4 : 2;
            if (!mDecimatorIntact)
                impacts = impacts * 3 / 2;

            for (uint32_t i = 0; i < impacts; ++i)
                getCreature()->castSpellLoc(scatterAround(GarothiArenaCentre, 10.0f, 35.0f), SPELL_GAROTHI_ANNIHILATION_MISSILE, true);
            return;
        }

        uint32_t marks = hardMode() ? 3 : 2;
        if (!mAnnihilatorIntact)
            marks = marks * 3 / 2;

        mDecimationTargets.clear();
        for (Unit* pTarget : pickPlayers(marks, true))
        {
            castSpell(pTarget, SPELL_GAROTHI_DECIMATION_MARK, true);
            mDecimationTargets.push_back(pTarget->GetNewGUID());
        }
        scriptEvents.addEvent(EVENT_GAROTHI_DECIMATION_IMPACT, 5000);
    }

    // nobody in reach of the melee
    void checkReach()
    {
        if (hasAura(SPELL_GAROTHI_CARNAGE))
            return;

        Unit* pVictim = currentVictim();
        if (pVictim != nullptr && getRangeToObject(pVictim) <= 10.0f)
            return;

        Player* pNearest = getNearestPlayer();
        if (pNearest != nullptr && pNearest->isAlive() && getRangeToObject(pNearest) <= 10.0f)
            return;

        castSpellOnSelf(SPELL_GAROTHI_CARNAGE, true);
    }

    void startDrive()
    {
        ++mDriveCount;
        mDriving = true;
        scriptEvents.resetEvents();
        mDecimationTargets.clear();

        setReactState(REACT_PASSIVE);
        _setMeleeDisabled(true);
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);

        warn("Garothi Worldbreaker engages its Apocalypse Drive! Destroy one of its cannons!");
        castSpellOnSelf(SPELL_GAROTHI_APOCALYPSE_DRIVE_AURA, true);
        castSpellOnSelf(SPELL_GAROTHI_APOCALYPSE_DRIVE);

        // the remaining cannons become targets beside the body
        float const facing = getCreature()->GetOrientation();
        if (mAnnihilatorIntact)
            callAdd(NPC_GAROTHI_ANNIHILATOR, ringPoint(here(), 7.0f, facing + AntorusHalfPi), MANUAL_DESPAWN);
        if (mDecimatorIntact)
            callAdd(NPC_GAROTHI_DECIMATOR, ringPoint(here(), 7.0f, facing - AntorusHalfPi), MANUAL_DESPAWN);

        scriptEvents.addEvent(EVENT_GAROTHI_DRIVE_END, 20000);
    }

    void stopDrive()
    {
        mDriving = false;
        _removeAura(SPELL_GAROTHI_APOCALYPSE_DRIVE_AURA);
        removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        setReactState(REACT_AGGRESSIVE);
        _setMeleeDisabled(false);
    }

    // a cannon broken during the drive ends it with Eradication
    void weaponDestroyed(bool annihilator)
    {
        if (!mDriving)
            return;

        if (annihilator)
            mAnnihilatorIntact = false;
        else
            mDecimatorIntact = false;

        getCreature()->interruptSpell();
        scriptEvents.removeEvent(EVENT_GAROTHI_DRIVE_END);
        summons.despawnEntry(NPC_GAROTHI_ANNIHILATOR);
        summons.despawnEntry(NPC_GAROTHI_DECIMATOR);
        stopDrive();
        scriptEvents.addEvent(EVENT_GAROTHI_ERADICATION, 4000);
    }

    bool mAnnihilatorIntact = true;
    bool mDecimatorIntact = true;
    bool mAnnihilatorNext = true;
    bool mDriving = false;
    uint32_t mDriveCount = 0;
    std::vector<WoWGuid> mDecimationTargets;
};

class AntorusGarothiCannonAI : public AntorusAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AntorusGarothiCannonAI(c); }
    explicit AntorusGarothiCannonAI(Creature* pCreature) : AntorusAI(pCreature) {}

    void OnLoad() override
    {
        setRooted(true);
        _setMeleeDisabled(true);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (AntorusInstanceScript* pAntorus = antorus())
            pAntorus->sendAction(BOSS_GAROTHI_WORLDBREAKER, getCreature()->getEntry() == NPC_GAROTHI_ANNIHILATOR ? ACTION_GAROTHI_ANNIHILATOR_LOST : ACTION_GAROTHI_DECIMATOR_LOST);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Felhounds of Sargeras

class AntorusFelhoundAI : public AntorusBossAI
{
public:
    AntorusFelhoundAI(Creature* pCreature, uint32_t partner, uint32_t openingEnergy, bool quickened)
        : AntorusBossAI(pCreature, DATA_FELHOUNDS), mPartner(partner), mOpeningEnergy(openingEnergy), mQuickened(quickened) {}

protected:
    void onEngage() override
    {
        mEnergy = mOpeningEnergy;
        mEnergyClock = 0;
        mBonusTicks = 0;
        mBonusStride = 4;
        // the third ability waits for the first full bar
        mThirdArmed = false;
        mTwoThirdArmed = true;
        mCorruptionTargets.clear();

        scriptEvents.addEvent(EVENT_FELHOUND_BLESSING_CHECK, 1000);
        engageHound();
    }

    void onDisengage() override { _removeAura(SPELL_FELHOUND_SARGERAS_BLESSING); }

    void onTick(uint32_t diff) override
    {
        mEnergyClock += diff;
        while (mEnergyClock >= 1000)
        {
            mEnergyClock -= 1000;
            gainEnergy();
        }
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_FELHOUND_BLESSING_CHECK:
                checkBlessing();
                scriptEvents.addEvent(EVENT_FELHOUND_BLESSING_CHECK, 1000);
                break;
            case EVENT_FELHOUND_CORRUPTION_IMPACT:
                for (WoWGuid const& guid : mCorruptionTargets)
                {
                    Unit* pTarget = unitFor(guid);
                    if (pTarget != nullptr && pTarget->isAlive())
                        castSpell(pTarget, corruptionSpell(), true);
                }
                mCorruptionTargets.clear();
                break;
            default:
                onHoundEvent(eventId);
                break;
        }
    }

    virtual void engageHound() {}
    virtual void onHoundEvent(uint32_t /*eventId*/) {}
    virtual void onThirdEnergy() {}
    virtual void onFullEnergy() {}
    virtual uint32_t corruptionSpell() const = 0;

    // marked players are struck once the mark runs out
    void markForCorruption(uint32_t markSpell)
    {
        mCorruptionTargets.clear();
        for (Unit* pTarget : pickPlayers(2, true))
        {
            castSpell(pTarget, markSpell, true);
            mCorruptionTargets.push_back(pTarget->GetNewGUID());
        }
        scriptEvents.addEvent(EVENT_FELHOUND_CORRUPTION_IMPACT, 6000);
    }

private:
    // one point per second, Shatug gains an extra point every fourth or fifth tick
    void gainEnergy()
    {
        uint32_t gain = 1;
        if (mQuickened && ++mBonusTicks >= mBonusStride)
        {
            mBonusTicks = 0;
            mBonusStride = mBonusStride == 4 ? 5 : 4;
            ++gain;
        }

        mEnergy += gain;
        if (mEnergy >= 100)
        {
            mEnergy = 0;
            mThirdArmed = true;
            mTwoThirdArmed = true;
            onFullEnergy();
            return;
        }

        if (mThirdArmed && mEnergy >= 33)
        {
            mThirdArmed = false;
            onThirdEnergy();
        }

        // the two thirds abilities are heroic and mythic only and left out
        if (mTwoThirdArmed && mEnergy >= 66)
            mTwoThirdArmed = false;
    }

    // both hounds grow stronger while close to each other
    void checkBlessing()
    {
        Creature* pPartner = tracked(mPartner);
        bool const close = pPartner != nullptr && pPartner->isAlive() && getRangeToObject(pPartner) <= 40.0f;

        if (close && !hasAura(SPELL_FELHOUND_SARGERAS_BLESSING))
            castSpellOnSelf(SPELL_FELHOUND_SARGERAS_BLESSING, true);
        else if (!close && hasAura(SPELL_FELHOUND_SARGERAS_BLESSING))
            _removeAura(SPELL_FELHOUND_SARGERAS_BLESSING);
    }

    uint32_t mPartner;
    uint32_t mOpeningEnergy;
    bool mQuickened;

    uint32_t mEnergy = 0;
    uint32_t mEnergyClock = 0;
    uint32_t mBonusTicks = 0;
    uint32_t mBonusStride = 4;
    bool mThirdArmed = false;
    bool mTwoThirdArmed = true;
    std::vector<WoWGuid> mCorruptionTargets;
};

class AntorusShatugAI : public AntorusFelhoundAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AntorusShatugAI(c); }
    explicit AntorusShatugAI(Creature* pCreature) : AntorusFelhoundAI(pCreature, BOSS_FHARG, 86, true) {}

protected:
    void engageHound() override { scriptEvents.addEvent(EVENT_SHATUG_CORRUPTING_MAW, 11000); }

    void onHoundEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_SHATUG_CORRUPTING_MAW)
            return;

        castSpellOnVictim(SPELL_SHATUG_CORRUPTING_MAW, true);
        castSpellOnVictim(SPELL_SHATUG_DECAY, true);
        scriptEvents.addEvent(EVENT_SHATUG_CORRUPTING_MAW, 11000);
    }

    void onThirdEnergy() override { castSpellAOE(SPELL_SHATUG_CONSUMING_SPHERE, true); }

    void onFullEnergy() override
    {
        warn("Shatug begins to Siphon Corruption!");
        castSpellOnSelf(SPELL_SHATUG_SIPHON_CORRUPTION);
        markForCorruption(SPELL_SHATUG_SIPHONED_MARK);
    }

    uint32_t corruptionSpell() const override { return SPELL_SHATUG_SIPHONED; }
};

class AntorusFhargAI : public AntorusFelhoundAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AntorusFhargAI(c); }
    explicit AntorusFhargAI(Creature* pCreature) : AntorusFelhoundAI(pCreature, BOSS_SHATUG, 62, false) {}

protected:
    void engageHound() override
    {
        mGazeTargets.clear();
        scriptEvents.addEvent(EVENT_FHARG_BURNING_MAW, 9000);
    }

    void onHoundEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_FHARG_BURNING_MAW:
                castSpellOnVictim(SPELL_FHARG_BURNING_MAW, true);
                castSpellOnVictim(SPELL_FHARG_SMOULDERING, true);
                scriptEvents.addEvent(EVENT_FHARG_BURNING_MAW, 11000);
                break;
            case EVENT_FHARG_DESOLATE_PATH:
                for (WoWGuid const& guid : mGazeTargets)
                {
                    Unit* pTarget = unitFor(guid);
                    if (pTarget != nullptr && pTarget->isAlive())
                        getCreature()->castSpellLoc(pTarget->GetPosition(), SPELL_FHARG_DESOLATE_PATH, true);
                }
                mGazeTargets.clear();
                break;
            default:
                break;
        }
    }

    // the gazed players draw the burning path
    void onThirdEnergy() override
    {
        warn("F'harg fixes his Desolate Gaze!");
        mGazeTargets.clear();
        for (Unit* pTarget : pickPlayers(2, true))
        {
            castSpell(pTarget, SPELL_FHARG_DESOLATE_GAZE, true);
            mGazeTargets.push_back(pTarget->GetNewGUID());
        }
        scriptEvents.addEvent(EVENT_FHARG_DESOLATE_PATH, 4000);
    }

    void onFullEnergy() override
    {
        warn("F'harg begins to Enflame Corruption!");
        castSpellOnSelf(SPELL_FHARG_ENFLAME_CORRUPTION);
        markForCorruption(SPELL_FHARG_ENFLAMED_MARK);
    }

    uint32_t corruptionSpell() const override { return SPELL_FHARG_ENFLAMED; }

private:
    std::vector<WoWGuid> mGazeTargets;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Antoran High Command

class AntorusCommanderAI : public AntorusBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AntorusCommanderAI(c); }
    explicit AntorusCommanderAI(Creature* pCreature) : AntorusBossAI(pCreature, DATA_HIGH_COMMAND) {}

    void OnLoad() override { applyStandby(); }

    void AIUpdate(unsigned long time_passed) override
    {
        followCommanderHealth();
        AntorusBossAI::AIUpdate(time_passed);
    }

    void DoAction(int32_t action) override
    {
        switch (action)
        {
            case ACTION_COMMAND_TAKE:
                takeCommand();
                break;
            case ACTION_COMMAND_YIELD:
                yieldCommand();
                break;
            case ACTION_ANTORUS_CLEANUP:
                applyStandby();
                break;
            default:
                break;
        }

        AntorusBossAI::DoAction(action);
    }

protected:
    // only the commander on the field fights, the others wait in their pods
    bool pullsGroup() const override { return false; }

    void onEngage() override
    {
        if (inCommand())
            openCommand(mTakeover);

        mTakeover = false;
    }

    void onDefeat() override { collapseOwnGroup(); }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_COMMAND_EXPLOIT_WEAKNESS:
                castSpellOnVictim(SPELL_COMMAND_EXPLOIT_WEAKNESS);
                scriptEvents.addEvent(EVENT_COMMAND_EXPLOIT_WEAKNESS, 8000);
                break;
            case EVENT_COMMAND_ASSUME_COMMAND:
                castSpellOnSelf(SPELL_COMMAND_ASSUME_COMMAND);
                scriptEvents.addEvent(EVENT_COMMAND_HANDOVER, 3000);
                break;
            case EVENT_COMMAND_HANDOVER:
                if (AntorusInstanceScript* pAntorus = antorus())
                    pAntorus->passCommand();
                break;
            case EVENT_COMMAND_FUSILLADE:
                warn("Admiral Svirax orders a Fusillade from the Legion Cruiser!");
                if (Creature* pAdmiral = tracked(BOSS_ADMIRAL_SVIRAX))
                    pAdmiral->castSpell(pAdmiral, SPELL_COMMAND_FUSILLADE_CAST, true);
                scriptEvents.addEvent(EVENT_COMMAND_FUSILLADE_VOLLEY, 3000);
                scriptEvents.addEvent(EVENT_COMMAND_FUSILLADE, 30000);
                break;
            case EVENT_COMMAND_FUSILLADE_VOLLEY:
                for (LocationVector const& impact : CommandFusilladeGrid)
                    getCreature()->castSpellLoc(impact, SPELL_COMMAND_FUSILLADE_MISSILE, true);
                break;
            case EVENT_COMMAND_ENTROPIC_MINES:
                for (Unit* pTarget : pickPlayers(2, false))
                    summonCreature(NPC_ENTROPIC_MINE, scatterAround(pTarget->GetPosition(), 3.0f, 8.0f), TIMED_DESPAWN, 90000);
                scriptEvents.addEvent(EVENT_COMMAND_ENTROPIC_MINES, 10000);
                break;
            case EVENT_COMMAND_REINFORCEMENTS:
                warn("General Erodus calls in reinforcements!");
                if (Creature* pGeneral = tracked(BOSS_GENERAL_ERODUS))
                    pGeneral->castSpell(pGeneral, SPELL_COMMAND_SUMMON_REINFORCEMENTS, true);
                callAdd(NPC_FANATICAL_PYROMANCER, scatterAround(here(), 4.0f, 10.0f));
                callAdd(NPC_FELBLADE_SHOCKTROOPER, scatterAround(here(), 4.0f, 10.0f));
                callAdd(NPC_FELBLADE_SHOCKTROOPER, scatterAround(here(), 4.0f, 10.0f));
                scriptEvents.addEvent(EVENT_COMMAND_REINFORCEMENTS, 34000);
                break;
            default:
                break;
        }
    }

private:
    bool inCommand()
    {
        AntorusInstanceScript* pAntorus = antorus();
        uint32_t const commander = pAntorus != nullptr ? pAntorus->activeCommander() : static_cast<uint32_t>(BOSS_ADMIRAL_SVIRAX);
        return commander == getCreature()->getEntry();
    }

    // the absent commanders support the one on the field
    void openCommand(bool takeover)
    {
        uint32_t const entry = getCreature()->getEntry();

        scriptEvents.addEvent(EVENT_COMMAND_EXPLOIT_WEAKNESS, 8000);
        scriptEvents.addEvent(EVENT_COMMAND_ASSUME_COMMAND, 90000);

        if (entry != BOSS_ADMIRAL_SVIRAX)
            scriptEvents.addEvent(EVENT_COMMAND_FUSILLADE, 16000);
        if (entry != BOSS_CHIEF_ENGINEER_ISHKAR)
            scriptEvents.addEvent(EVENT_COMMAND_ENTROPIC_MINES, 15000);
        if (entry != BOSS_GENERAL_ERODUS)
            scriptEvents.addEvent(EVENT_COMMAND_REINFORCEMENTS, takeover ? 16000 : 8000);
    }

    void takeCommand()
    {
        removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        setReactState(REACT_AGGRESSIVE);
        getCreature()->m_canRegenerateHP = true;

        switch (getCreature()->getEntry())
        {
            case BOSS_ADMIRAL_SVIRAX:
                warn("Admiral Svirax assumes command!");
                break;
            case BOSS_CHIEF_ENGINEER_ISHKAR:
                warn("Chief Engineer Ishkar assumes command!");
                break;
            default:
                warn("General Erodus assumes command!");
                break;
        }

        mTakeover = true;
        openCommand(true);
        if (!_isInCombat())
            setZoneWideCombat(getCreature());
    }

    void yieldCommand()
    {
        scriptEvents.resetEvents();
        attackStop();
        setReactState(REACT_PASSIVE);
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        getCreature()->m_canRegenerateHP = false;
        moveToSpawn();
    }

    // look out of combat: Svirax leads, the others wait unselectable
    void applyStandby()
    {
        mTakeover = false;
        getCreature()->m_canRegenerateHP = true;

        if (inCommand())
        {
            removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            setReactState(REACT_AGGRESSIVE);
        }
        else
        {
            addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            setReactState(REACT_PASSIVE);
        }
    }

    // shared health: waiting commanders mirror the one on the field
    void followCommanderHealth()
    {
        AntorusInstanceScript* pAntorus = antorus();
        if (pAntorus == nullptr || !isAlive() || inCommand() || pAntorus->getBossState(DATA_HIGH_COMMAND) != EncounterStates::InProgress)
            return;

        Creature* pLead = pAntorus->getTracked(pAntorus->activeCommander());
        if (pLead == nullptr || !pLead->isAlive() || pLead->getMaxHealth() == 0)
            return;

        uint64_t const scaled = static_cast<uint64_t>(pLead->getHealth()) * getCreature()->getMaxHealth() / pLead->getMaxHealth();
        getCreature()->setHealth(scaled > 0 ? static_cast<uint32_t>(scaled) : 1);
    }

    bool mTakeover = false;
};

class AntorusEntropicMineAI : public AntorusAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AntorusEntropicMineAI(c); }
    explicit AntorusEntropicMineAI(Creature* pCreature) : AntorusAI(pCreature) { mAlwaysActive = true; }

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        setRooted(true);
        _setMeleeDisabled(true);
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        castSpellOnSelf(SPELL_COMMAND_MINE_COSMETIC, true);
        // arms after a short delay
        scriptEvents.addEvent(EVENT_MINE_SCAN, 2000);
    }

protected:
    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_MINE_SCAN)
            return;

        Player* pNearest = getNearestPlayer();
        if (pNearest != nullptr && pNearest->isAlive() && getRangeToObject(pNearest) <= 2.5f)
        {
            castSpellAOE(SPELL_COMMAND_ENTROPIC_BLAST, true);
            castSpellAOE(SPELL_COMMAND_ENTROPIC_BLAST_DOT, true);
            despawn(500, 0);
            return;
        }

        scriptEvents.addEvent(EVENT_MINE_SCAN, 300);
    }
};

class AntorusCommandReinforcementAI : public AntorusAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AntorusCommandReinforcementAI(c); }
    explicit AntorusCommandReinforcementAI(Creature* pCreature) : AntorusAI(pCreature) {}

protected:
    void onEngage() override
    {
        castSpellOnSelf(SPELL_COMMAND_ADD_SPAWN, true);
        if (getCreature()->getEntry() == NPC_FANATICAL_PYROMANCER)
            scriptEvents.addEvent(EVENT_PYROMANCER_PYROBLAST, 4200);
        else
            scriptEvents.addEvent(EVENT_SHOCKTROOPER_DEMONIC_CHARGE, 4500);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_PYROMANCER_PYROBLAST:
                castOn(sparePlayer(), SPELL_PYROMANCER_PYROBLAST);
                scriptEvents.addEvent(EVENT_PYROMANCER_PYROBLAST, 9000);
                break;
            case EVENT_SHOCKTROOPER_DEMONIC_CHARGE:
                castOn(sparePlayer(), SPELL_SHOCKTROOPER_DEMONIC_CHARGE, true);
                scriptEvents.addEvent(EVENT_SHOCKTROOPER_BLADESTORM, 1000);
                scriptEvents.addEvent(EVENT_SHOCKTROOPER_DEMONIC_CHARGE, 18000);
                break;
            case EVENT_SHOCKTROOPER_BLADESTORM:
                castSpellOnSelf(SPELL_SHOCKTROOPER_BLADESTORM, true);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Portal Keeper Hasabel

class AntorusHasabelAI : public AntorusBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AntorusHasabelAI(c); }
    explicit AntorusHasabelAI(Creature* pCreature) : AntorusBossAI(pCreature, DATA_HASABEL) { setBerserk(720000); }

protected:
    void onEngage() override
    {
        mEnergy = 0;
        mEnergyClock = 0;
        mEnergyTicks = 0;
        mCollapseLock = 0;
        mGateway = 0;
        mFelcrushRow = 0;
        mPortalReleases = 0;

        scriptEvents.addEvent(EVENT_HASABEL_REALITY_TEAR, 7000);
        scriptEvents.addEvent(EVENT_HASABEL_FELSTORM_BARRAGE, 26000);
    }

    void onTick(uint32_t diff) override
    {
        mCollapseLock = mCollapseLock > diff ? mCollapseLock - diff : 0;

        // three, three and four energy per second
        mEnergyClock += diff;
        while (mEnergyClock >= 1000)
        {
            mEnergyClock -= 1000;
            mEnergy += (++mEnergyTicks % 3 == 0) ? 4 : 3;
        }

        if (mEnergy >= 100 && mCollapseLock == 0 && !_isCasting())
            collapseWorld();

        int32_t const health = _getHealthPercent();
        if ((mGateway == 0 && health < 91) || (mGateway == 1 && health < 61) || (mGateway == 2 && health < 31))
            openGateway();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_HASABEL_REALITY_TEAR:
                castSpellOnVictim(SPELL_HASABEL_REALITY_TEAR);
                scriptEvents.addEvent(EVENT_HASABEL_REALITY_TEAR, 12000);
                break;
            case EVENT_HASABEL_FELSTORM_BARRAGE:
                warn("Portal Keeper Hasabel unleashes a Felstorm Barrage!");
                castSpellOnSelf(SPELL_HASABEL_FELSTORM_BARRAGE, true);
                if (hardMode())
                {
                    mFelcrushRow = 0;
                    openFelcrushRow();
                }
                else
                {
                    LocationVector const& spot = HasabelFelcrushSpots[Util::getRandomUInt(2)][Util::getRandomUInt(1)];
                    summonCreature(NPC_FELCRUSH_PORTAL, spot, TIMED_DESPAWN, 12000);
                }
                mCollapseLock = 10000;
                scriptEvents.addEvent(EVENT_HASABEL_TRANSPORT_PORTAL, 10000);
                scriptEvents.addEvent(EVENT_HASABEL_FELSTORM_BARRAGE, 32000);
                break;
            case EVENT_HASABEL_FELCRUSH_ROW:
                openFelcrushRow();
                break;
            case EVENT_HASABEL_TRANSPORT_PORTAL:
                openTransportPortal();
                scriptEvents.addEvent(EVENT_HASABEL_FELSTORM_BARRAGE, 10000);
                scriptEvents.addEvent(EVENT_HASABEL_TRANSPORT_PORTAL, 40000);
                break;
            case EVENT_HASABEL_PORTAL_RELEASE:
                releaseFromPortal();
                break;
            default:
                break;
        }
    }

private:
    // heroic and mythic open one portal of each pair two seconds apart
    void openFelcrushRow()
    {
        if (mFelcrushRow >= 3)
            return;

        summonCreature(NPC_FELCRUSH_PORTAL, HasabelFelcrushSpots[mFelcrushRow][Util::getRandomUInt(1)], TIMED_DESPAWN, 12000);
        if (++mFelcrushRow < 3)
            scriptEvents.addEvent(EVENT_HASABEL_FELCRUSH_ROW, 2000);
    }

    void collapseWorld()
    {
        warn("Portal Keeper Hasabel begins to cast Collapsing World!");
        mEnergy = 0;

        Unit* pTarget = sparePlayer();
        summonCreature(NPC_COLLAPSING_WORLD, pTarget != nullptr ? pTarget->GetPosition() : here(), TIMED_DESPAWN, 12000);

        scriptEvents.addEvent(EVENT_HASABEL_TRANSPORT_PORTAL, 10000);
        scriptEvents.addEvent(EVENT_HASABEL_FELSTORM_BARRAGE, 10000);
    }

    void openGateway()
    {
        ++mGateway;
        switch (mGateway)
        {
            case 1:
                warn("Portal Keeper Hasabel opens the Gateway: Xoroth!");
                scriptEvents.addEvent(EVENT_HASABEL_TRANSPORT_PORTAL, 2000);
                break;
            case 2:
                warn("Portal Keeper Hasabel opens the Gateway: Rancora!");
                break;
            default:
                warn("Portal Keeper Hasabel opens the Gateway: Nathreza!");
                break;
        }
    }

    void openTransportPortal()
    {
        Creature* pPortal = summonCreature(NPC_TRANSPORT_PORTAL, scatterAround(here(), 10.0f, 20.0f), TIMED_DESPAWN, 20000);
        if (pPortal == nullptr)
            return;

        mPortalGuid = pPortal->GetNewGUID();
        mPortalReleases = 0;
        scriptEvents.addEvent(EVENT_HASABEL_PORTAL_RELEASE, 3000);
    }

    // six demons per portal, the stalker and skitterers join later in the fight
    void releaseFromPortal()
    {
        InstanceScript* pInstance = getInstanceScript();
        Creature* pPortal = pInstance != nullptr ? pInstance->getCreatureByGuid(mPortalGuid) : nullptr;
        if (pPortal == nullptr || !pPortal->isAlive())
            return;

        ++mPortalReleases;
        uint32_t entry = NPC_BLAZING_IMP;
        if (mGateway >= 3 && mPortalReleases == 1)
            entry = NPC_HUNGERING_STALKER;
        else if (mGateway >= 2 && (mPortalReleases == 1 || mPortalReleases == 6))
            entry = NPC_FELTOUCHED_SKITTERER;

        callAdd(entry, scatterAround(pPortal->GetPosition(), 1.0f, 3.0f));

        if (mPortalReleases < 6)
            scriptEvents.addEvent(EVENT_HASABEL_PORTAL_RELEASE, 3000);
    }

    uint32_t mEnergy = 0;
    uint32_t mEnergyClock = 0;
    uint32_t mEnergyTicks = 0;
    uint32_t mCollapseLock = 0;
    uint32_t mGateway = 0;
    uint32_t mFelcrushRow = 0;
    uint32_t mPortalReleases = 0;
    WoWGuid mPortalGuid;
};

class AntorusHasabelMinionAI : public AntorusAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AntorusHasabelMinionAI(c); }
    explicit AntorusHasabelMinionAI(Creature* pCreature) : AntorusAI(pCreature) {}

protected:
    void onEngage() override
    {
        switch (getCreature()->getEntry())
        {
            case NPC_BLAZING_IMP:
                if (hardMode())
                    scriptEvents.addEvent(EVENT_IMP_FIERY_DETONATION, 500);
                break;
            case NPC_FELTOUCHED_SKITTERER:
                scriptEvents.addEvent(EVENT_SKITTERER_ACIDIC_WEB, 6000);
                break;
            default:
                scriptEvents.addEvent(EVENT_STALKER_HOWLING_SHADOWS, 2000);
                break;
        }
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_IMP_FIERY_DETONATION:
                castSpellOnSelf(SPELL_BLAZING_IMP_FIERY_DETONATION);
                scriptEvents.addEvent(EVENT_IMP_FIERY_DETONATION, 8000);
                break;
            case EVENT_SKITTERER_ACIDIC_WEB:
                castSpellOnVictim(SPELL_SKITTERER_ACIDIC_WEB);
                scriptEvents.addEvent(EVENT_SKITTERER_ACIDIC_WEB, 9000);
                break;
            case EVENT_STALKER_HOWLING_SHADOWS:
                castSpellOnSelf(SPELL_STALKER_HOWLING_DARK, true);
                castSpellOnSelf(SPELL_STALKER_HOWLING_SHADOWS);
                scriptEvents.addEvent(EVENT_STALKER_HOWLING_SHADOWS, 12000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// The Defense of Eonar

class AntorusEonarAI : public AntorusAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AntorusEonarAI(c); }
    explicit AntorusEonarAI(Creature* pCreature) : AntorusAI(pCreature) {}

    void OnLoad() override { setReactState(REACT_PASSIVE); }

    // demon blows may pull the essence into combat, the event ignores that
    void OnCombatStart(Unit* /*pTarget*/) override {}
    void OnCombatStop(Unit* /*pTarget*/) override {}

    // the event runs without the essence ever entering combat
    void AIUpdate(unsigned long time_passed) override
    {
        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (!mRunning)
        {
            watchForDefenders(diff);
            return;
        }

        scriptEvents.updateEvents(diff, 0);
        releaseQueued(diff);

        while (mRunning)
        {
            uint32_t const eventId = scriptEvents.getFinishedEvent();
            if (eventId == 0)
                break;

            onEvent(eventId);
        }
    }

    void SetCreatureData(uint32_t type, uint32_t data) override
    {
        if (type == EONAR_DATA_DEMON_SLAIN && mRunning)
            gatherLife(data);
    }

    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (!mRunning || damage == nullptr || *damage < getCreature()->getHealth())
            return;

        *damage = 0;
        fail();
    }

protected:
    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_EONAR_GROUND_WAVE:
                ++mGroundWave;
                warn("Legion ships deliver another wave of demons!");
                queueWave(EonarGroundWaves, sizeof(EonarGroundWaves) / sizeof(EonarGroundWaves[0]), mGroundWave);
                if (mGroundWave <= 8 && EonarGroundWaveGaps[mGroundWave - 1] != 0)
                    scriptEvents.addEvent(EVENT_EONAR_GROUND_WAVE, EonarGroundWaveGaps[mGroundWave - 1]);
                break;
            case EVENT_EONAR_SKY_WAVE:
                ++mSkyWave;
                warn("Volant Kerapterons pour out of a demonic portal!");
                queueWave(EonarSkyWaves, sizeof(EonarSkyWaves) / sizeof(EonarSkyWaves[0]), mSkyWave);
                if (mSkyWave <= 3 && EonarSkyWaveGaps[mSkyWave - 1] != 0)
                    scriptEvents.addEvent(EVENT_EONAR_SKY_WAVE, EonarSkyWaveGaps[mSkyWave - 1]);
                break;
            case EVENT_EONAR_RAIN_OF_FEL:
                rainOfFel();
                scriptEvents.addEvent(EVENT_EONAR_RAIN_OF_FEL, Util::getRandomUInt(20000, 45000));
                break;
            case EVENT_EONAR_RAIN_OF_FEL_IMPACT:
                if (Creature* pParaxis = paraxis())
                {
                    for (WoWGuid const& guid : mRainTargets)
                    {
                        Unit* pTarget = unitFor(guid);
                        if (pTarget != nullptr && pTarget->isAlive())
                            pParaxis->castSpell(pTarget, SPELL_PARAXIS_RAIN_OF_FEL, true);
                    }
                }
                mRainTargets.clear();
                break;
            case EVENT_EONAR_DEFENDER_CHECK:
                if (playersWithin(350.0f).empty())
                {
                    fail();
                    return;
                }
                scriptEvents.addEvent(EVENT_EONAR_DEFENDER_CHECK, 2000);
                break;
            default:
                break;
        }
    }

private:
    struct QueuedDemon
    {
        uint32_t dueIn;
        uint32_t entry;
        uint8_t lane;
    };

    // a defender reaching the essence starts the event
    void watchForDefenders(uint32_t diff)
    {
        InstanceScript* pInstance = getInstanceScript();
        if (pInstance == nullptr || pInstance->getBossState(DATA_EONAR) == EncounterStates::Performed)
            return;

        if (mRestartDelay > diff)
        {
            mRestartDelay -= diff;
            return;
        }
        mRestartDelay = 1000;

        if (!playersWithin(20.0f).empty())
            begin();
    }

    void begin()
    {
        mRunning = true;
        mLife = 0;
        mLifeForces = 0;
        mGroundWave = 0;
        mSkyWave = 0;
        mQueue.clear();
        mRainTargets.clear();
        scriptEvents.resetEvents();

        getCreature()->m_canRegenerateHP = false;
        getCreature()->setHealth(getCreature()->getMaxHealth());

        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->setBossState(DATA_EONAR, EncounterStates::InProgress);
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
        }

        warn("The Legion descends upon the Essence of Eonar! Defend her!");

        if (Creature* pParaxis = summonCreature(NPC_THE_PARAXIS, ParaxisAnchor, MANUAL_DESPAWN, 0))
        {
            mParaxisGuid = pParaxis->GetNewGUID();
            pParaxis->m_canRegenerateHP = false;
        }

        scriptEvents.addEvent(EVENT_EONAR_GROUND_WAVE, EonarFirstGroundWave);
        scriptEvents.addEvent(EVENT_EONAR_SKY_WAVE, EonarFirstSkyWave);
        scriptEvents.addEvent(EVENT_EONAR_RAIN_OF_FEL, 15000);
        scriptEvents.addEvent(EVENT_EONAR_DEFENDER_CHECK, 2000);
    }

    void finish(EncounterStates result)
    {
        mRunning = false;
        mQueue.clear();
        mRainTargets.clear();
        scriptEvents.resetEvents();
        summons.despawnAll();

        getCreature()->m_canRegenerateHP = true;
        getCreature()->setHealth(getCreature()->getMaxHealth());

        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
            pInstance->setBossState(DATA_EONAR, result);
        }
    }

    void fail()
    {
        warn("The Essence of Eonar fades. The Legion has overrun her defenders.");
        mRestartDelay = 30000;
        finish(EncounterStates::Failed);
    }

    void queueWave(EonarSpawnGroup const* groups, size_t groupCount, uint32_t wave)
    {
        bool const hard = hardMode();
        for (size_t i = 0; i < groupCount; ++i)
        {
            EonarSpawnGroup const& group = groups[i];
            if (group.wave != wave || (group.hardOnly && !hard))
                continue;

            for (uint8_t n = 0; n < group.count; ++n)
                mQueue.push_back({ group.delay + n * group.spacing, group.entry, group.lane });
        }
    }

    void releaseQueued(uint32_t diff)
    {
        for (size_t i = 0; i < mQueue.size();)
        {
            QueuedDemon& demon = mQueue[i];
            if (demon.dueIn > diff)
            {
                demon.dueIn -= diff;
                ++i;
                continue;
            }

            summonCreature(demon.entry, scatterAround(EonarLaneEntries[demon.lane], 0.0f, 4.0f), CORPSE_DESPAWN, 0);
            mQueue.erase(mQueue.begin() + static_cast<std::ptrdiff_t>(i));
        }
    }

    Creature* paraxis()
    {
        InstanceScript* pInstance = getInstanceScript();
        return pInstance != nullptr ? pInstance->getCreatureByGuid(mParaxisGuid) : nullptr;
    }

    void rainOfFel()
    {
        Creature* pParaxis = paraxis();
        if (pParaxis == nullptr)
            return;

        std::vector<Player*> defenders = playersWithin(350.0f);
        mRainTargets.clear();
        for (uint32_t i = 0; i < 2 && !defenders.empty(); ++i)
        {
            size_t const pick = Util::getRandomUInt(static_cast<uint32_t>(defenders.size() - 1));
            pParaxis->castSpell(defenders[pick], SPELL_PARAXIS_RAIN_OF_FEL_MARK, true);
            mRainTargets.push_back(defenders[pick]->GetNewGUID());
            defenders.erase(defenders.begin() + static_cast<std::ptrdiff_t>(pick));
        }

        if (!mRainTargets.empty())
        {
            warn("The Paraxis calls down a Rain of Fel!");
            scriptEvents.addEvent(EVENT_EONAR_RAIN_OF_FEL_IMPACT, 5000);
        }
    }

    // fallen demons feed the essence
    void gatherLife(uint32_t entry)
    {
        bool const hard = hardMode();
        switch (entry)
        {
            case NPC_EONAR_FELGUARD:
                mLife += 4;
                break;
            case NPC_EONAR_FEL_HOUND:
                mLife += hard ? 5 : 7;
                break;
            case NPC_VOLANT_KERAPTERON:
                mLife += hard ? 7 : 8;
                break;
            case NPC_EONAR_FEL_LORD:
                mLife += 9;
                break;
            default:
                mLife += 10;
                break;
        }

        if (mLife >= 100)
            unleashLifeForce();
    }

    // every Life Force tears a quarter off the Paraxis
    void unleashLifeForce()
    {
        mLife = 0;
        ++mLifeForces;
        warn("The Essence of Eonar unleashes her Life Force upon the Paraxis!");
        castSpellOnSelf(SPELL_EONAR_LIFE_FORCE, true);

        bool paraxisDown = mLifeForces >= EonarLifeForcesToWin;
        if (Creature* pParaxis = paraxis())
        {
            if (pParaxis->isAlive())
            {
                uint32_t const share = pParaxis->getMaxHealth() / 100 * 26;
                getCreature()->dealDamage(pParaxis, paraxisDown ? pParaxis->getHealth() : share, SPELL_EONAR_LIFE_FORCE_PARAXIS);
            }

            paraxisDown = paraxisDown || !pParaxis->isAlive();
        }

        if (!paraxisDown)
            return;

        warn("The Paraxis is destroyed! The Essence of Eonar is saved!");
        finish(EncounterStates::Performed);
    }

    bool mRunning = false;
    uint32_t mRestartDelay = 0;
    uint32_t mLife = 0;
    uint32_t mLifeForces = 0;
    uint32_t mGroundWave = 0;
    uint32_t mSkyWave = 0;
    WoWGuid mParaxisGuid;
    std::vector<QueuedDemon> mQueue;
    std::vector<WoWGuid> mRainTargets;
};

class AntorusEonarDemonAI : public AntorusAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AntorusEonarDemonAI(c); }
    explicit AntorusEonarDemonAI(Creature* pCreature) : AntorusAI(pCreature) { mAlwaysActive = true; }

    void OnLoad() override
    {
        uint32_t const entry = getCreature()->getEntry();

        // only the heavy constructs fight back
        if (entry != NPC_FEL_POWERED_PURIFIER && entry != NPC_FEL_INFUSED_DESTRUCTOR)
            setReactState(REACT_PASSIVE);

        switch (entry)
        {
            case NPC_EONAR_FELGUARD:
                castSpellOnSelf(SPELL_FELGUARD_WEAPONS, true);
                break;
            case NPC_EONAR_FEL_LORD:
                castSpellOnSelf(SPELL_FEL_LORD_WEAPONS, true);
                break;
            case NPC_FEL_CHARGED_OBFUSCATOR:
                castSpellOnSelf(SPELL_OBFUSCATOR_CLOAK, true);
                break;
            case NPC_VOLANT_KERAPTERON:
                setFlyMode(true);
                break;
            default:
                break;
        }

        if (entry != NPC_VOLANT_KERAPTERON)
            castSpellOnSelf(SPELL_EONAR_DEMON_SPAWN, true);

        scriptEvents.addEvent(EVENT_DEMON_ADVANCE, 3000);
        if (entry == NPC_FEL_POWERED_PURIFIER)
            scriptEvents.addEvent(EVENT_PURIFIER_FEL_SWIPE, 9000);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        Creature* pEonar = tracked(NPC_ESSENCE_OF_EONAR);
        if (pEonar != nullptr && pEonar->GetScript() != nullptr)
            pEonar->GetScript()->SetCreatureData(EONAR_DATA_DEMON_SLAIN, getCreature()->getEntry());
    }

protected:
    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_DEMON_ADVANCE:
                advance();
                break;
            case EVENT_DEMON_STRIKE:
                strike();
                break;
            case EVENT_PURIFIER_FEL_SWIPE:
                if (_isInCombat())
                    castSpellOnVictim(Util::getRandomUInt(1) == 0 ? SPELL_PURIFIER_FEL_SWIPE_RIGHT : SPELL_PURIFIER_FEL_SWIPE_LEFT);
                scriptEvents.addEvent(EVENT_PURIFIER_FEL_SWIPE, 9000);
                break;
            default:
                break;
        }
    }

private:
    // destructors shell her from range, the others strike in melee
    float strikeRange() { return getCreature()->getEntry() == NPC_FEL_INFUSED_DESTRUCTOR ? 40.0f : 6.0f; }

    void advance()
    {
        Creature* pEonar = tracked(NPC_ESSENCE_OF_EONAR);
        if (pEonar == nullptr || !pEonar->isAlive())
            return;

        if (getRangeToObject(pEonar) <= strikeRange())
        {
            stopMovement();
            if (getCreature()->getEntry() != NPC_FEL_INFUSED_DESTRUCTOR)
                castSpell(pEonar, SPELL_EONAR_DEMON_STRIKE, true);
            scriptEvents.addEvent(EVENT_DEMON_STRIKE, 1000);
            return;
        }

        if (!_isInCombat() || getReactState() == REACT_PASSIVE)
            movePoint(POINT_DEMON_TO_EONAR, scatterAround(pEonar->GetPosition(), 2.0f, 4.0f));

        scriptEvents.addEvent(EVENT_DEMON_ADVANCE, 2000);
    }

    // strike drains one percent per second, artillery two percent every eight seconds
    void strike()
    {
        Creature* pEonar = tracked(NPC_ESSENCE_OF_EONAR);
        if (pEonar == nullptr || !pEonar->isAlive())
            return;

        if (getRangeToObject(pEonar) > strikeRange() + 4.0f)
        {
            scriptEvents.addEvent(EVENT_DEMON_ADVANCE, 500);
            return;
        }

        if (getCreature()->getEntry() == NPC_FEL_INFUSED_DESTRUCTOR)
        {
            getCreature()->castSpellLoc(pEonar->GetPosition(), SPELL_DESTRUCTOR_ARTILLERY_STRIKE, true);
            getCreature()->dealDamage(pEonar, pEonar->getMaxHealth() / 50, SPELL_DESTRUCTOR_ARTILLERY_STRIKE);
            scriptEvents.addEvent(EVENT_DEMON_STRIKE, 8000);
            return;
        }

        uint32_t const drain = pEonar->getMaxHealth() / 100;
        getCreature()->dealDamage(pEonar, drain > 0 ? drain : 1, SPELL_EONAR_DEMON_STRIKE);
        scriptEvents.addEvent(EVENT_DEMON_STRIKE, 1000);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Imonar the Soulhunter

class AntorusImonarAI : public AntorusBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AntorusImonarAI(c); }
    explicit AntorusImonarAI(Creature* pCreature) : AntorusBossAI(pCreature, DATA_IMONAR) {}

    void OnReachWP(uint32_t type, uint32_t id) override
    {
        if (type == POINT_MOTION_TYPE && id == POINT_IMONAR_CROSSING && mCrossing)
            land();
    }

protected:
    void onEngage() override
    {
        mStage = 1;
        mPlatform = 0;
        mCrossing = false;
        openStage();
    }

    void onDisengage() override
    {
        mCrossing = false;
        setFlyMode(false);
        _removeAura(SPELL_IMONAR_JETPACKS_FIRST);
        _removeAura(SPELL_IMONAR_JETPACKS_SECOND);
        removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        setReactState(REACT_AGGRESSIVE);
    }

    void onTick(uint32_t /*diff*/) override
    {
        if (mCrossing || mStage >= 3)
            return;

        if (_getHealthPercent() < (mStage == 1 ? 67 : 34))
            takeOff();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_IMONAR_SHOCK_LANCE:
                castSpellOnVictim(SPELL_IMONAR_SHOCK_LANCE);
                scriptEvents.addEvent(EVENT_IMONAR_SHOCK_LANCE, 5000);
                break;
            case EVENT_IMONAR_SLEEP_CANISTER:
                castOn(sparePlayer(), SPELL_IMONAR_SLEEP_CANISTER);
                scriptEvents.addEvent(EVENT_IMONAR_SLEEP_CANISTER, 12000);
                break;
            case EVENT_IMONAR_PULSE_GRENADE:
                castOn(anyPlayer(), SPELL_IMONAR_PULSE_GRENADE, true);
                scriptEvents.addEvent(EVENT_IMONAR_PULSE_GRENADE, 17000);
                break;
            case EVENT_IMONAR_SEVER:
                castSpellOnVictim(SPELL_IMONAR_SEVER);
                scriptEvents.addEvent(EVENT_IMONAR_SEVER, 7000);
                break;
            case EVENT_IMONAR_CHARGED_BLASTS:
            {
                warn("Imonar calls in a bombing run of Charged Blasts!");
                castSpellOnSelf(SPELL_IMONAR_CHARGED_BLASTS, true);
                uint32_t const first = Util::getRandomUInt(4);
                summonCreature(NPC_IMONAR_BOMBING_RUN, ImonarBomberBerths[first], TIMED_DESPAWN, 15000);
                summonCreature(NPC_IMONAR_BOMBING_RUN, ImonarBomberBerths[(first + 1 + Util::getRandomUInt(3)) % 5], TIMED_DESPAWN, 15000);
                scriptEvents.addEvent(EVENT_IMONAR_CHARGED_BLASTS, 20000);
                break;
            }
            case EVENT_IMONAR_EMPOWERED_SHOCK_LANCE:
                castSpellOnVictim(SPELL_IMONAR_EMPOWERED_SHOCK_LANCE);
                scriptEvents.addEvent(EVENT_IMONAR_EMPOWERED_SHOCK_LANCE, 10000);
                break;
            case EVENT_IMONAR_EMPOWERED_PULSE:
                castSpellOnSelf(SPELL_IMONAR_EMPOWERED_PULSE, true);
                for (Unit* pTarget : pickPlayers(2, false))
                    castSpell(pTarget, SPELL_IMONAR_PULSE_GRENADE, true);
                scriptEvents.addEvent(EVENT_IMONAR_EMPOWERED_PULSE, 27000);
                break;
            case EVENT_IMONAR_EMPOWERED_SHRAPNEL:
                warn("Imonar calls in an Empowered Shrapnel Blast!");
                castSpellOnSelf(SPELL_IMONAR_EMPOWERED_SHRAPNEL, true);
                summonCreature(NPC_IMONAR_ARTILLERY_STRIKE, ImonarArtilleryBerths[Util::getRandomUInt(1)], TIMED_DESPAWN, 10000);
                scriptEvents.addEvent(EVENT_IMONAR_EMPOWERED_SHRAPNEL, 22000);
                break;
            case EVENT_IMONAR_ROCKET_CHECK:
                punishStragglers();
                scriptEvents.addEvent(EVENT_IMONAR_ROCKET_CHECK, 2000);
                break;
            case EVENT_IMONAR_FORCED_LANDING:
                // the flight path did not report, finish the crossing here
                if (mCrossing)
                {
                    MoveTeleport(ImonarPlatforms[1 - mPlatform]);
                    land();
                }
                break;
            default:
                break;
        }
    }

private:
    void openStage()
    {
        switch (mStage)
        {
            case 1:
                scriptEvents.addEvent(EVENT_IMONAR_SHOCK_LANCE, 4000);
                scriptEvents.addEvent(EVENT_IMONAR_SLEEP_CANISTER, 6000);
                scriptEvents.addEvent(EVENT_IMONAR_PULSE_GRENADE, 15000);
                break;
            case 2:
                scriptEvents.addEvent(EVENT_IMONAR_SEVER, 7000);
                scriptEvents.addEvent(EVENT_IMONAR_CHARGED_BLASTS, 8000);
                break;
            default:
                scriptEvents.addEvent(EVENT_IMONAR_EMPOWERED_SHOCK_LANCE, 5000);
                scriptEvents.addEvent(EVENT_IMONAR_EMPOWERED_PULSE, 6000);
                scriptEvents.addEvent(EVENT_IMONAR_EMPOWERED_SHRAPNEL, 16000);
                break;
        }

        scriptEvents.addEvent(EVENT_IMONAR_ROCKET_CHECK, 2000);
    }

    // jetpacks carry him to the other platform
    void takeOff()
    {
        mCrossing = true;
        scriptEvents.resetEvents();

        warn("Imonar fires his jetpacks and flees to the next platform!");
        attackStop();
        setReactState(REACT_PASSIVE);
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        castSpellOnSelf(mPlatform == 0 ? SPELL_IMONAR_JETPACKS_FIRST : SPELL_IMONAR_JETPACKS_SECOND, true);
        setFlyMode(true);
        movePoint(POINT_IMONAR_CROSSING, ImonarPlatforms[1 - mPlatform], false);
        scriptEvents.addEvent(EVENT_IMONAR_FORCED_LANDING, 25000);
    }

    void land()
    {
        mCrossing = false;
        mPlatform = 1 - mPlatform;
        ++mStage;
        scriptEvents.removeEvent(EVENT_IMONAR_FORCED_LANDING);

        setFlyMode(false);
        _removeAura(SPELL_IMONAR_JETPACKS_FIRST);
        _removeAura(SPELL_IMONAR_JETPACKS_SECOND);
        castSpellOnSelf(mPlatform == 1 ? SPELL_IMONAR_LANDING_FIRST : SPELL_IMONAR_LANDING_SECOND, true);
        removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        setReactState(REACT_AGGRESSIVE);
        openStage();
    }

    // Infernal Rockets for anyone off the current platform
    void punishStragglers()
    {
        LocationVector const& platform = ImonarPlatforms[mPlatform];
        for (Player* pPlayer : playersWithin(150.0f))
        {
            if (pPlayer->GetPosition().getExactDist(platform) > 35.0f)
            {
                castSpellAOE(mPlatform == 0 ? SPELL_IMONAR_INFERNAL_ROCKETS_HOME : SPELL_IMONAR_INFERNAL_ROCKETS_FAR, true);
                return;
            }
        }
    }

    uint32_t mStage = 1;
    uint32_t mPlatform = 0;
    bool mCrossing = false;
};

class AntorusImonarShipAI : public AntorusAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AntorusImonarShipAI(c); }
    explicit AntorusImonarShipAI(Creature* pCreature) : AntorusAI(pCreature) { mAlwaysActive = true; }

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        setRooted(true);
        _setMeleeDisabled(true);
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);

        if (getCreature()->getEntry() == NPC_IMONAR_BOMBING_RUN)
            scriptEvents.addEvent(EVENT_BOMBING_RUN_MARK, 1000);
        else
            scriptEvents.addEvent(EVENT_ARTILLERY_STRIKE_FIRE, 1500);
    }

protected:
    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_BOMBING_RUN_MARK:
                if (Unit* pTarget = anyPlayer())
                {
                    castSpell(pTarget, SPELL_IMONAR_CHARGED_BLASTS_MARK, true);
                    mTargetGuid = pTarget->GetNewGUID();
                    scriptEvents.addEvent(EVENT_BOMBING_RUN_FIRE, 3000);
                }
                break;
            case EVENT_BOMBING_RUN_FIRE:
            {
                Unit* pTarget = unitFor(mTargetGuid);
                if (pTarget != nullptr && pTarget->isAlive())
                    castSpell(pTarget, SPELL_IMONAR_CHARGED_BLASTS_DMG, true);
                break;
            }
            case EVENT_ARTILLERY_STRIKE_FIRE:
                for (Unit* pTarget : pickPlayers(3, false))
                    castSpell(pTarget, SPELL_IMONAR_SHRAPNEL_MISSILE, true);
                break;
            default:
                break;
        }
    }

private:
    WoWGuid mTargetGuid;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Kin'garoth

class AntorusKingarothAI : public AntorusBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AntorusKingarothAI(c); }
    explicit AntorusKingarothAI(Creature* pCreature) : AntorusBossAI(pCreature, DATA_KINGAROTH) {}

    void OnLoad() override { setRooted(true); }

    void DoAction(int32_t action) override
    {
        if (action == ACTION_KINGAROTH_REAVER_DOWN && isAlive() && _isInCombat())
            feedbackOverload();

        AntorusBossAI::DoAction(action);
    }

protected:
    void onEngage() override
    {
        setRooted(true);
        mConstructing = false;
        mReaverRound = 0;
        openDeployment(true);
        scriptEvents.addEvent(EVENT_KINGAROTH_CONSTRUCTION, 32000);
    }

    void onDisengage() override
    {
        mConstructing = false;
        _removeAura(SPELL_KINGAROTH_APOCALYPSE_PROTOCOL);
        setReactState(REACT_AGGRESSIVE);
        _setMeleeDisabled(false);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_KINGAROTH_FORGING_STRIKE:
                castSpellOnVictim(SPELL_KINGAROTH_FORGING_STRIKE);
                scriptEvents.addEvent(EVENT_KINGAROTH_FORGING_STRIKE, 14000);
                break;
            case EVENT_KINGAROTH_REVERBERATING_STRIKE:
                castOn(sparePlayer(), SPELL_KINGAROTH_REVERBERATING);
                scriptEvents.addEvent(EVENT_KINGAROTH_REVERBERATING_STRIKE, 30000);
                break;
            case EVENT_KINGAROTH_DIABOLIC_BOMB:
                castSpellAOE(SPELL_KINGAROTH_DIABOLIC_BOMB);
                scriptEvents.addEvent(EVENT_KINGAROTH_DIABOLIC_BOMB, 20000);
                break;
            case EVENT_KINGAROTH_RUINER:
                deployRuiner();
                scriptEvents.addEvent(EVENT_KINGAROTH_RUINER, 30000);
                scriptEvents.addEvent(EVENT_KINGAROTH_REACH_CHECK, 12000);
                break;
            case EVENT_KINGAROTH_REACH_CHECK:
                checkReach();
                scriptEvents.addEvent(EVENT_KINGAROTH_REACH_CHECK, 1500);
                break;
            case EVENT_KINGAROTH_CONSTRUCTION:
                beginConstruction();
                break;
            case EVENT_KINGAROTH_CONSTRUCTION_END:
                endConstruction();
                break;
            case EVENT_KINGAROTH_APOCALYPSE_BLAST:
                if (mConstructing)
                {
                    castOn(sparePlayer(), SPELL_KINGAROTH_APOCALYPSE_BLAST, true);
                    scriptEvents.addEvent(EVENT_KINGAROTH_APOCALYPSE_BLAST, 4000);
                }
                break;
            default:
                break;
        }
    }

private:
    void openDeployment(bool opening)
    {
        scriptEvents.addEvent(EVENT_KINGAROTH_FORGING_STRIKE, opening ? 6000 : 1000);
        scriptEvents.addEvent(EVENT_KINGAROTH_REVERBERATING_STRIKE, opening ? 14000 : 2000);
        scriptEvents.addEvent(EVENT_KINGAROTH_DIABOLIC_BOMB, opening ? 14000 : 20000);
        scriptEvents.addEvent(EVENT_KINGAROTH_RUINER, opening ? 26000 : 22000);
        scriptEvents.addEvent(EVENT_KINGAROTH_REACH_CHECK, 3000);
    }

    // the Ruiner sweeps an arc around him
    void deployRuiner()
    {
        warn("Kin'garoth deploys a Ruiner!");

        Unit* pTarget = sparePlayer();
        LocationVector const centre = here();
        float const angle = pTarget != nullptr ? centre.getAbsoluteAngle(pTarget->GetPositionX(), pTarget->GetPositionY()) : getCreature()->GetOrientation();

        Creature* pRuiner = summonCreature(NPC_KINGAROTH_RUINER, ringPoint(centre, 35.0f, angle), TIMED_DESPAWN, 15000);
        if (pRuiner != nullptr)
            castSpell(pRuiner, SPELL_KINGAROTH_RUINER_CHANNEL);
    }

    // nobody in melee reach
    void checkReach()
    {
        Unit* pVictim = currentVictim();
        if (pVictim == nullptr || getRangeToObject(pVictim) <= 10.0f)
            return;

        Player* pNearest = getNearestPlayer();
        if (pNearest != nullptr && pNearest->isAlive() && getRangeToObject(pNearest) <= 10.0f)
            return;

        castSpellOnVictim(SPELL_KINGAROTH_SHATTERING_STRIKE);
    }

    void beginConstruction()
    {
        mConstructing = true;
        scriptEvents.resetEvents();

        warn("Kin'garoth begins the Construction Stage! Destroy the Garothi before they activate!");
        attackStop();
        setReactState(REACT_PASSIVE);
        _setMeleeDisabled(true);
        castSpellOnSelf(SPELL_KINGAROTH_APOCALYPSE_PROTOCOL, true);

        uint32_t const (*rounds)[3] = hardMode() ? KingarothHardReavers : KingarothNormalReavers;
        for (uint32_t bay = 0; bay < 3; ++bay)
        {
            if (rounds[mReaverRound][bay] != 0)
                callAdd(rounds[mReaverRound][bay], KingarothReaverBays[bay]);
        }
        mReaverRound = (mReaverRound + 1) % 3;

        scriptEvents.addEvent(EVENT_KINGAROTH_APOCALYPSE_BLAST, 4000);
        scriptEvents.addEvent(EVENT_KINGAROTH_CONSTRUCTION_END, 43000);
    }

    void endConstruction()
    {
        mConstructing = false;
        scriptEvents.removeEvent(EVENT_KINGAROTH_APOCALYPSE_BLAST);
        _removeAura(SPELL_KINGAROTH_APOCALYPSE_PROTOCOL);
        setReactState(REACT_AGGRESSIVE);
        _setMeleeDisabled(false);
        openDeployment(false);
        scriptEvents.addEvent(EVENT_KINGAROTH_CONSTRUCTION, 77000);
    }

    // a destroyed reaver burns six percent of his health, four on heroic and mythic
    void feedbackOverload()
    {
        castSpellOnSelf(SPELL_KINGAROTH_FEEDBACK_OVERLOAD, true);
        uint32_t const share = getCreature()->getMaxHealth() / 100 * (hardMode() ? 4 : 6);
        getCreature()->dealDamage(getCreature(), share, SPELL_KINGAROTH_FEEDBACK_OVERLOAD);
    }

    bool mConstructing = false;
    uint32_t mReaverRound = 0;
};

class AntorusKingarothRuinerAI : public AntorusAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AntorusKingarothRuinerAI(c); }
    explicit AntorusKingarothRuinerAI(Creature* pCreature) : AntorusAI(pCreature) { mAlwaysActive = true; }

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        _setMeleeDisabled(true);
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        mSteps = 0;
        mDirection = Util::getRandomUInt(1) == 0 ? 0.3f : -0.3f;
        scriptEvents.addEvent(EVENT_RUINER_SWEEP, 1000);
    }

protected:
    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_RUINER_SWEEP)
            return;

        Creature* pKingaroth = tracked(BOSS_KINGAROTH);
        if (pKingaroth == nullptr || mSteps >= 6)
        {
            despawn(500, 0);
            return;
        }

        LocationVector const centre = pKingaroth->GetPosition();
        float const angle = centre.getAbsoluteAngle(getCreature()->GetPositionX(), getCreature()->GetPositionY()) + mDirection;
        movePoint(POINT_RUINER_STEP, ringPoint(centre, 35.0f, angle), false);

        ++mSteps;
        scriptEvents.addEvent(EVENT_RUINER_SWEEP, 1500);
    }

private:
    uint32_t mSteps = 0;
    float mDirection = 0.3f;
};

class AntorusGarothiReaverAI : public AntorusAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AntorusGarothiReaverAI(c); }
    explicit AntorusGarothiReaverAI(Creature* pCreature) : AntorusAI(pCreature) {}

    void OnDied(Unit* /*pKiller*/) override
    {
        if (AntorusInstanceScript* pAntorus = antorus())
            pAntorus->sendAction(BOSS_KINGAROTH, ACTION_KINGAROTH_REAVER_DOWN);
    }

protected:
    // thirty seconds of initialization before the weapons go live
    void onEngage() override
    {
        castSpellOnSelf(SPELL_REAVER_SPAWN, true);
        castSpellOnSelf(SPELL_REAVER_INITIALIZING, true);
        setReactState(REACT_PASSIVE);
        setRooted(true);
        scriptEvents.addEvent(EVENT_REAVER_ACTIVATE, 30000);

        switch (getCreature()->getEntry())
        {
            case NPC_GAROTHI_REAVER_ANNIHILATOR:
                scriptEvents.addEvent(EVENT_REAVER_ANNIHILATION, 45000);
                break;
            case NPC_GAROTHI_REAVER_DECIMATOR:
                scriptEvents.addEvent(EVENT_REAVER_DECIMATION, 45000);
                break;
            default:
                scriptEvents.addEvent(EVENT_REAVER_DEMOLISH, 46000);
                break;
        }
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_REAVER_ACTIVATE:
                _removeAura(SPELL_REAVER_INITIALIZING);
                setRooted(false);
                setReactState(REACT_AGGRESSIVE);
                break;
            case EVENT_REAVER_ANNIHILATION:
            {
                castSpellOnSelf(SPELL_REAVER_ANNIHILATION, true);
                uint32_t const impacts = hardMode() ? 4 : 3;
                for (uint32_t i = 0; i < impacts; ++i)
                    getCreature()->castSpellLoc(scatterAround(here(), 5.0f, 30.0f), SPELL_REAVER_ANNIHILATION_MISSILE, true);
                scriptEvents.addEvent(EVENT_REAVER_ANNIHILATION, 14000);
                break;
            }
            case EVENT_REAVER_DECIMATION:
                markTarget(SPELL_REAVER_DECIMATION_MARK);
                scriptEvents.addEvent(EVENT_REAVER_DECIMATION, 16000);
                break;
            case EVENT_REAVER_DEMOLISH:
                markTarget(SPELL_REAVER_DEMOLISH_MARK);
                scriptEvents.addEvent(EVENT_REAVER_DEMOLISH, 16000);
                break;
            case EVENT_REAVER_MARK_IMPACT:
            {
                Unit* pTarget = unitFor(mMarkedGuid);
                if (pTarget == nullptr || !pTarget->isAlive())
                    break;

                if (getCreature()->getEntry() == NPC_GAROTHI_REAVER_DECIMATOR)
                    castSpell(pTarget, SPELL_REAVER_DECIMATION_MISSILE, true);
                else
                    getCreature()->castSpellLoc(pTarget->GetPosition(), SPELL_REAVER_DEMOLISH, true);
                break;
            }
            default:
                break;
        }
    }

private:
    void markTarget(uint32_t markSpell)
    {
        Unit* pTarget = sparePlayer();
        if (pTarget == nullptr)
            return;

        castSpell(pTarget, markSpell, true);
        mMarkedGuid = pTarget->GetNewGUID();
        scriptEvents.addEvent(EVENT_REAVER_MARK_IMPACT, 5000);
    }

    WoWGuid mMarkedGuid;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Varimathras

class AntorusVarimathrasAI : public AntorusBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AntorusVarimathrasAI(c); }
    explicit AntorusVarimathrasAI(Creature* pCreature) : AntorusBossAI(pCreature, DATA_VARIMATHRAS) { setBerserk(310000); }

protected:
    void onEngage() override
    {
        mEnergy = 0;
        mEnergyClock = 0;
        mTormentStep = 0;
        mFissures.clear();

        scriptEvents.addEvent(EVENT_VARIMATHRAS_TORMENT, 5000);
        scriptEvents.addEvent(EVENT_VARIMATHRAS_DARK_FISSURE, 16000);
        if (hardMode())
            scriptEvents.addEvent(EVENT_VARIMATHRAS_NECROTIC_EMBRACE, 35500);
    }

    // Shadow Strike on a full bar, eleven energy every two seconds
    void onTick(uint32_t diff) override
    {
        mEnergyClock += diff;
        while (mEnergyClock >= 2000)
        {
            mEnergyClock -= 2000;
            mEnergy += 11;
        }

        if (mEnergy >= 100 && !_isCasting())
        {
            mEnergy = 0;
            castSpellOnVictim(SPELL_VARIMATHRAS_SHADOW_STRIKE);
        }
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_VARIMATHRAS_TORMENT:
                nextTorment();
                break;
            case EVENT_VARIMATHRAS_DARK_FISSURE:
                openFissures();
                scriptEvents.addEvent(EVENT_VARIMATHRAS_MARKED_PREY, Util::getRandomUInt(6000, 8000));
                scriptEvents.addEvent(EVENT_VARIMATHRAS_DARK_FISSURE, 30000);
                break;
            case EVENT_VARIMATHRAS_DARK_ERUPTION:
                for (LocationVector const& fissure : mFissures)
                    getCreature()->castSpellLoc(fissure, SPELL_VARIMATHRAS_DARK_ERUPTION, true);
                mFissures.clear();
                break;
            case EVENT_VARIMATHRAS_MARKED_PREY:
                if (Unit* pPrey = sparePlayer())
                {
                    castSpell(pPrey, SPELL_VARIMATHRAS_MARKED_PREY, true);
                    mPreyGuid = pPrey->GetNewGUID();
                    scriptEvents.addEvent(EVENT_VARIMATHRAS_SHADOW_HUNTER, 5000);
                }
                break;
            case EVENT_VARIMATHRAS_SHADOW_HUNTER:
            {
                Unit* pPrey = unitFor(mPreyGuid);
                if (pPrey == nullptr || !pPrey->isAlive())
                    break;

                summonCreature(NPC_SHADOW_OF_VARIMATHRAS, pPrey->GetPosition(), TIMED_DESPAWN, 2000);
                castSpell(pPrey, SPELL_VARIMATHRAS_SHADOW_HUNTER, true);
                castSpell(pPrey, SPELL_VARIMATHRAS_SHADOW_HUNTER_KB, true);
                break;
            }
            case EVENT_VARIMATHRAS_NECROTIC_EMBRACE:
                castOn(sparePlayer(), SPELL_VARIMATHRAS_NECROTIC_EMBRACE, true);
                scriptEvents.addEvent(EVENT_VARIMATHRAS_NECROTIC_EMBRACE, 30000);
                break;
            default:
                break;
        }
    }

private:
    // normal goes from Flames straight to Shadows, harder modes walk all four
    void nextTorment()
    {
        bool const hard = hardMode();
        switch (mTormentStep)
        {
            case 0:
                warn("Varimathras unleashes the Torment of Flames!");
                castSpellOnSelf(SPELL_VARIMATHRAS_TORMENT_FLAMES, true);
                mTormentStep = hard ? 1 : 3;
                scriptEvents.addEvent(EVENT_VARIMATHRAS_TORMENT, hard ? 100000 : 290000);
                break;
            case 1:
                warn("Varimathras unleashes the Torment of Frost!");
                castSpellOnSelf(SPELL_VARIMATHRAS_TORMENT_FROST, true);
                mTormentStep = 2;
                scriptEvents.addEvent(EVENT_VARIMATHRAS_TORMENT, 99000);
                break;
            case 2:
                warn("Varimathras unleashes the Torment of Fel!");
                castSpellOnSelf(SPELL_VARIMATHRAS_TORMENT_FEL, true);
                mTormentStep = 3;
                scriptEvents.addEvent(EVENT_VARIMATHRAS_TORMENT, 90000);
                break;
            default:
                warn("Varimathras unleashes the Torment of Shadows!");
                castSpellOnSelf(SPELL_VARIMATHRAS_TORMENT_SHADOWS, true);
                break;
        }
    }

    // one fissure per five players, the tank is spared
    void openFissures()
    {
        size_t const raid = playersWithin(100.0f).size();
        uint32_t const fissures = raid >= 5 ? static_cast<uint32_t>(raid / 5) : 1;

        mFissures.clear();
        for (Unit* pTarget : pickPlayers(fissures, true))
        {
            castSpell(pTarget, SPELL_VARIMATHRAS_DARK_FISSURE, true);
            mFissures.push_back(pTarget->GetPosition());
        }

        if (!mFissures.empty())
            scriptEvents.addEvent(EVENT_VARIMATHRAS_DARK_ERUPTION, 6000);
    }

    uint32_t mEnergy = 0;
    uint32_t mEnergyClock = 0;
    uint32_t mTormentStep = 0;
    std::vector<LocationVector> mFissures;
    WoWGuid mPreyGuid;
};

//////////////////////////////////////////////////////////////////////////////////////////
// The Coven of Shivarra

class AntorusCovenSisterAI : public AntorusBossAI
{
public:
    explicit AntorusCovenSisterAI(Creature* pCreature) : AntorusBossAI(pCreature, DATA_COVEN) {}

    void DamageTaken(Unit* pAttacker, uint32_t* damage) override
    {
        if (damage == nullptr || *damage == 0)
            return;

        AntorusInstanceScript* pAntorus = antorus();
        if (pAntorus != nullptr && !pAntorus->isGroupMember(DATA_COVEN, pAttacker))
            pAntorus->shareDamage(DATA_COVEN, getCreature(), *damage);
    }

    void DoAction(int32_t action) override
    {
        switch (action)
        {
            case ACTION_COVEN_BEGIN_DUTY:
                beginDuty();
                break;
            case ACTION_COVEN_END_DUTY:
                endDuty();
                break;
            case ACTION_COVEN_TORMENT_GOLGANNETH:
            case ACTION_COVEN_TORMENT_KHAZGOROTH:
            case ACTION_COVEN_TORMENT_AMANTHUL:
                performTorment(action);
                break;
            case ACTION_ANTORUS_CLEANUP:
                if (mOnDuty)
                    moveToSpawn();
                mOnDuty = false;
                removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                setReactState(REACT_AGGRESSIVE);
                getCreature()->m_canRegenerateHP = true;
                break;
            default:
                break;
        }

        AntorusBossAI::DoAction(action);
    }

protected:
    void onEngage() override
    {
        if (onDuty())
            beginDuty();
        else
            startAbilities();
    }

    void onDefeat() override { collapseOwnGroup(); }

    virtual void startAbilities() = 0;
    virtual char const* dutyWarning() const = 0;
    virtual uint32_t pedestal() const { return 0; }

private:
    bool onDuty()
    {
        AntorusInstanceScript* pAntorus = antorus();
        return pAntorus != nullptr && pAntorus->covenDuty() == getCreature()->getEntry();
    }

    // the sister on duty leaves the fight for the torment pedestal
    void beginDuty()
    {
        if (mOnDuty)
            return;

        mOnDuty = true;
        scriptEvents.resetEvents();
        attackStop();
        setReactState(REACT_PASSIVE);
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        getCreature()->m_canRegenerateHP = false;

        warn(dutyWarning());
        movePoint(POINT_COVEN_PEDESTAL, CovenPedestals[pedestal() % 3]);
    }

    void endDuty()
    {
        mOnDuty = false;
        removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        setReactState(REACT_AGGRESSIVE);
        getCreature()->m_canRegenerateHP = true;
        startAbilities();

        if (!_isInCombat())
            setZoneWideCombat(getCreature());
    }

    void performTorment(int32_t action)
    {
        switch (action)
        {
            case ACTION_COVEN_TORMENT_GOLGANNETH:
                warn("The Torment of Golganneth is unleashed!");
                castSpellOnSelf(SPELL_COVEN_TORMENT_GOLGANNETH, true);
                for (LocationVector const& spot : CovenGolgannethSouls)
                    callAdd(NPC_TORMENT_OF_GOLGANNETH, spot);
                break;
            case ACTION_COVEN_TORMENT_KHAZGOROTH:
                warn("The Torment of Khaz'goroth is unleashed!");
                castSpellOnSelf(SPELL_COVEN_TORMENT_KHAZGOROTH, true);
                for (LocationVector const& spot : CovenKhazgorothSouls)
                    callAdd(NPC_TORMENT_OF_KHAZGOROTH, spot);
                break;
            default:
                warn("The Torment of Aman'Thul is unleashed!");
                castSpellOnSelf(SPELL_COVEN_TORMENT_AMANTHUL, true);
                for (LocationVector const& spot : CovenAmanthulSouls)
                    callAdd(NPC_TORMENT_OF_AMANTHUL, spot);
                break;
        }
    }

    bool mOnDuty = false;
};

class AntorusAsaraAI : public AntorusCovenSisterAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AntorusAsaraAI(c); }
    explicit AntorusAsaraAI(Creature* pCreature) : AntorusCovenSisterAI(pCreature) {}

protected:
    // shadow bolts instead of melee
    void startAbilities() override
    {
        _setMeleeDisabled(true);
        scriptEvents.addEvent(EVENT_ASARA_TOUCH_OF_DARKNESS, 2500);
        scriptEvents.addEvent(EVENT_ASARA_SHADOW_BLADES, 12000);
    }

    char const* dutyWarning() const override { return "Asara, Mother of Night steps away to unleash the Torment of the Titans!"; }
    uint32_t pedestal() const override { return 1; }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ASARA_TOUCH_OF_DARKNESS:
                castSpellOnVictim(SPELL_ASARA_TOUCH_OF_DARKNESS);
                scriptEvents.addEvent(EVENT_ASARA_TOUCH_OF_DARKNESS, 2500);
                break;
            case EVENT_ASARA_SHADOW_BLADES:
                launchBlades();
                scriptEvents.addEvent(EVENT_ASARA_SHADOW_BLADES, 28000);
                break;
            default:
                break;
        }
    }

private:
    // three blades fan out towards a random player
    void launchBlades()
    {
        castSpellOnSelf(SPELL_ASARA_SHADOW_BLADES);

        LocationVector const origin = here();
        Unit* pTarget = anyPlayer();
        float const facing = pTarget != nullptr ? origin.getAbsoluteAngle(pTarget->GetPositionX(), pTarget->GetPositionY()) : getCreature()->GetOrientation();

        for (float const spread : { -0.15f, 0.0f, 0.15f })
            summonCreature(NPC_COVEN_SHADOW_BLADE, LocationVector(origin.x, origin.y, origin.z, facing + spread), TIMED_DESPAWN, 15000);
    }
};

class AntorusNouraAI : public AntorusCovenSisterAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AntorusNouraAI(c); }
    explicit AntorusNouraAI(Creature* pCreature) : AntorusCovenSisterAI(pCreature) {}

protected:
    void startAbilities() override
    {
        scriptEvents.addEvent(EVENT_NOURA_FIERY_STRIKE, 11000);
        if (hardMode())
            scriptEvents.addEvent(EVENT_NOURA_FULMINATING_PULSE, 20500);
    }

    char const* dutyWarning() const override { return "Noura, Mother of Flames steps away to unleash the Torment of the Titans!"; }
    uint32_t pedestal() const override { return 0; }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_NOURA_FIERY_STRIKE:
                castSpellOnVictim(SPELL_NOURA_FIERY_STRIKE);
                scriptEvents.addEvent(EVENT_NOURA_FIERY_STRIKE, 11000);
                break;
            case EVENT_NOURA_FULMINATING_PULSE:
                castOn(sparePlayer(), SPELL_NOURA_FULMINATING_PULSE, true);
                scriptEvents.addEvent(EVENT_NOURA_FULMINATING_PULSE, 40000);
                break;
            default:
                break;
        }
    }
};

class AntorusDiimaAI : public AntorusCovenSisterAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AntorusDiimaAI(c); }
    explicit AntorusDiimaAI(Creature* pCreature) : AntorusCovenSisterAI(pCreature) {}

protected:
    void startAbilities() override
    {
        scriptEvents.addEvent(EVENT_DIIMA_FLASHFREEZE, 11000);
        scriptEvents.addEvent(EVENT_DIIMA_CHILLED_BLOOD, 7000);
    }

    char const* dutyWarning() const override { return "Diima, Mother of Gloom steps away to unleash the Torment of the Titans!"; }
    uint32_t pedestal() const override { return 2; }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_DIIMA_FLASHFREEZE:
                castSpellOnVictim(SPELL_DIIMA_FLASHFREEZE);
                scriptEvents.addEvent(EVENT_DIIMA_FLASHFREEZE, 11000);
                break;
            case EVENT_DIIMA_CHILLED_BLOOD:
                for (Unit* pTarget : pickPlayers(3, true))
                    castSpell(pTarget, SPELL_DIIMA_CHILLED_BLOOD, true);
                scriptEvents.addEvent(EVENT_DIIMA_CHILLED_BLOOD, 26000);
                break;
            default:
                break;
        }
    }
};

class AntorusThurayaAI : public AntorusCovenSisterAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AntorusThurayaAI(c); }
    explicit AntorusThurayaAI(Creature* pCreature) : AntorusCovenSisterAI(pCreature) {}

protected:
    void startAbilities() override
    {
        _setMeleeDisabled(true);
        scriptEvents.addEvent(EVENT_THURAYA_TOUCH_OF_THE_COSMOS, 3000);
    }

    char const* dutyWarning() const override { return "Thu'raya, Mother of the Cosmos steps away to unleash the Torment of the Titans!"; }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_THURAYA_TOUCH_OF_THE_COSMOS)
            return;

        castSpellOnSelf(SPELL_THURAYA_TOUCH_OF_THE_COSMOS);
        scriptEvents.addEvent(EVENT_THURAYA_TOUCH_OF_THE_COSMOS, 3500);
    }
};

class AntorusCovenShadowBladeAI : public AntorusAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AntorusCovenShadowBladeAI(c); }
    explicit AntorusCovenShadowBladeAI(Creature* pCreature) : AntorusAI(pCreature) { mAlwaysActive = true; }

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        _setMeleeDisabled(true);
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        scriptEvents.addEvent(EVENT_SHADOW_BLADE_LAUNCH, 3000);
    }

    void OnReachWP(uint32_t type, uint32_t id) override
    {
        if (type == POINT_MOTION_TYPE && id == POINT_SHADOW_BLADE_PATH)
            despawn(200, 0);
    }

protected:
    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_SHADOW_BLADE_LAUNCH)
            return;

        castSpellOnSelf(SPELL_SHADOW_BLADE_AURA, true);
        movePoint(POINT_SHADOW_BLADE_PATH, ringPoint(here(), 80.0f, getCreature()->GetOrientation()), false);
    }
};

class AntorusCovenTormentAI : public AntorusAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AntorusCovenTormentAI(c); }
    explicit AntorusCovenTormentAI(Creature* pCreature) : AntorusAI(pCreature) {}

    void OnLoad() override
    {
        setRooted(true);
        _setMeleeDisabled(true);

        if (getCreature()->getEntry() == NPC_TORMENT_OF_KHAZGOROTH)
            castSpellOnSelf(SPELL_TORMENT_FLAMES_OF_KHAZGOROTH, true);
    }

protected:
    void onEngage() override
    {
        if (getCreature()->getEntry() != NPC_TORMENT_OF_KHAZGOROTH)
            scriptEvents.addEvent(EVENT_TORMENT_PULSE, 1000);
    }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_TORMENT_PULSE)
            return;

        // Aman'Thul's souls channel until interrupted, Golganneth's lash out at random
        if (getCreature()->getEntry() == NPC_TORMENT_OF_AMANTHUL)
        {
            castSpellOnSelf(SPELL_TORMENT_MACHINATIONS);
            scriptEvents.addEvent(EVENT_TORMENT_PULSE, 1000);
            return;
        }

        castOn(anyPlayer(), SPELL_TORMENT_FURY_OF_GOLGANNETH, true);
        scriptEvents.addEvent(EVENT_TORMENT_PULSE, 2500);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Aggramar

class AntorusAggramarAI : public AntorusBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AntorusAggramarAI(c); }
    explicit AntorusAggramarAI(Creature* pCreature) : AntorusBossAI(pCreature, DATA_AGGRAMAR) {}

    void DoAction(int32_t action) override
    {
        if (action == ACTION_AGGRAMAR_FLAME_DOWN && mAegisUp && mFlamesAlive > 0 && --mFlamesAlive == 0)
            lowerAegis();

        AntorusBossAI::DoAction(action);
    }

    void OnReachWP(uint32_t type, uint32_t id) override
    {
        if (type == POINT_MOTION_TYPE && id == POINT_AGGRAMAR_CENTRE && mIntermission)
            raiseAegis();
    }

protected:
    void onEngage() override
    {
        mStage = 1;
        mEnergy = 0;
        mEnergyClock = 0;
        mTechniqueStep = 0;
        mInTechnique = false;
        mIntermission = false;
        mAegisUp = false;
        mFlamesAlive = 0;
        openStage();
    }

    void onDisengage() override
    {
        mIntermission = false;
        mAegisUp = false;
        _removeAura(SPELL_AGGRAMAR_CORRUPT_AEGIS);
        _removeAura(SPELL_AGGRAMAR_TAESHALACH_TECHNIQUE);
        removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        setReactState(REACT_AGGRESSIVE);
    }

    void onTick(uint32_t diff) override
    {
        if (mIntermission)
            return;

        if (mStage < 3 && _getHealthPercent() < (mStage == 1 ? 81 : 41))
        {
            beginIntermission();
            return;
        }

        if (mInTechnique)
            return;

        // about three energy per second, Taeshalach's Technique on a full bar
        mEnergyClock += diff;
        while (mEnergyClock >= 1000)
        {
            mEnergyClock -= 1000;
            mEnergy += (mEnergy % 20 == 0) ? 2 : 3;
        }

        if (mEnergy >= 100 && !_isCasting())
            beginTechnique();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_AGGRAMAR_TECHNIQUE_STRIKE:
                techniqueStrike();
                break;
            case EVENT_AGGRAMAR_TEMPEST_WAVE:
                _castOnInrangePlayersWithinDist(0.0f, 20.0f * 20.0f, SPELL_AGGRAMAR_SEARING_TEMPEST_DMG, true);
                break;
            case EVENT_AGGRAMAR_SCORCHING_BLAZE:
                castOn(sparePlayer(), SPELL_AGGRAMAR_SCORCHING_BLAZE, true);
                scriptEvents.addEvent(EVENT_AGGRAMAR_SCORCHING_BLAZE, 6500);
                break;
            case EVENT_AGGRAMAR_WAKE_OF_FLAME:
                castSpellOnVictim(SPELL_AGGRAMAR_WAKE_OF_FLAME);
                scriptEvents.addEvent(EVENT_AGGRAMAR_WAKE_OF_FLAME, 24000);
                break;
            case EVENT_AGGRAMAR_FLARE:
                warn(mStage >= 3 ? "Aggramar calls down an Empowered Flare!" : "Aggramar calls down a Flare!");
                for (Unit* pTarget : pickPlayers(3, false))
                    castSpell(pTarget, mStage >= 3 ? SPELL_AGGRAMAR_EMPOWERED_FLARE : SPELL_AGGRAMAR_FLARE, true);
                scriptEvents.addEvent(EVENT_AGGRAMAR_FLARE, 15000);
                break;
            case EVENT_AGGRAMAR_RAISE_AEGIS:
                // the walk to the centre did not report, raise the aegis on the spot
                if (mIntermission && !mAegisUp)
                    raiseAegis();
                break;
            default:
                break;
        }
    }

private:
    void openStage()
    {
        scriptEvents.addEvent(EVENT_AGGRAMAR_SCORCHING_BLAZE, mStage == 1 ? 5500 : 6000);
        if (mStage == 1)
            scriptEvents.addEvent(EVENT_AGGRAMAR_WAKE_OF_FLAME, 6000);
        else
            scriptEvents.addEvent(EVENT_AGGRAMAR_FLARE, 10000);
    }

    void beginTechnique()
    {
        mInTechnique = true;
        mEnergy = 0;
        mTechniqueStep = 0;
        scriptEvents.resetEvents();

        warn("Aggramar begins Taeshalach's Technique!");
        castSpellOnSelf(SPELL_AGGRAMAR_TAESHALACH_TECHNIQUE, true);
        scriptEvents.addEvent(EVENT_AGGRAMAR_TECHNIQUE_STRIKE, 100);
    }

    // Foe Breaker, Flame Rend, Foe Breaker, Flame Rend, Searing Tempest
    void techniqueStrike()
    {
        switch (mTechniqueStep)
        {
            case 0:
            case 2:
                castSpellOnVictim(SPELL_AGGRAMAR_FOE_BREAKER);
                break;
            case 1:
            case 3:
                castSpellOnVictim(SPELL_AGGRAMAR_FLAME_REND);
                break;
            case 4:
                warn("Aggramar unleashes a Searing Tempest! Get away!");
                castSpellOnSelf(SPELL_AGGRAMAR_SEARING_TEMPEST);
                scriptEvents.addEvent(EVENT_AGGRAMAR_TEMPEST_WAVE, 2500);
                break;
            default:
                mInTechnique = false;
                _removeAura(SPELL_AGGRAMAR_TAESHALACH_TECHNIQUE);
                openStage();
                return;
        }

        ++mTechniqueStep;
        scriptEvents.addEvent(EVENT_AGGRAMAR_TECHNIQUE_STRIKE, 5000);
    }

    void beginIntermission()
    {
        mIntermission = true;
        mInTechnique = false;
        mEnergy = 0;
        scriptEvents.resetEvents();
        _removeAura(SPELL_AGGRAMAR_TAESHALACH_TECHNIQUE);

        attackStop();
        setReactState(REACT_PASSIVE);
        movePoint(POINT_AGGRAMAR_CENTRE, AggramarArenaCentre);
        scriptEvents.addEvent(EVENT_AGGRAMAR_RAISE_AEGIS, 8000);
    }

    // six embers creep in, two flames have to die
    void raiseAegis()
    {
        if (mAegisUp)
            return;

        mAegisUp = true;
        scriptEvents.removeEvent(EVENT_AGGRAMAR_RAISE_AEGIS);

        warn("Aggramar raises the Corrupt Aegis! Destroy the Flames of Taeshalach!");
        castSpellOnSelf(SPELL_AGGRAMAR_CORRUPT_AEGIS, true);
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);

        LocationVector const centre = here();
        for (uint32_t i = 0; i < 6; ++i)
            callAdd(NPC_EMBER_OF_TAESHALACH, ringPoint(centre, 50.0f, i * AntorusTwoPi / 6.0f));

        mFlamesAlive = 0;
        float const facing = getCreature()->GetOrientation();
        for (float const side : { -AntorusHalfPi, AntorusHalfPi })
        {
            if (callAdd(NPC_FLAME_OF_TAESHALACH, ringPoint(centre, 15.0f, facing + side)) != nullptr)
                ++mFlamesAlive;
        }

        if (mFlamesAlive == 0)
            lowerAegis();
    }

    void lowerAegis()
    {
        mAegisUp = false;
        mIntermission = false;
        ++mStage;

        summons.despawnEntry(NPC_EMBER_OF_TAESHALACH);
        _removeAura(SPELL_AGGRAMAR_CORRUPT_AEGIS);
        removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        setReactState(REACT_AGGRESSIVE);

        warn(mStage == 2 ? "The Corrupt Aegis shatters! Aggramar fights on with Flare!" : "The Corrupt Aegis shatters! Aggramar empowers his Flare!");
        openStage();
    }

    uint32_t mStage = 1;
    uint32_t mEnergy = 0;
    uint32_t mEnergyClock = 0;
    uint32_t mTechniqueStep = 0;
    bool mInTechnique = false;
    bool mIntermission = false;
    bool mAegisUp = false;
    uint32_t mFlamesAlive = 0;
};

class AntorusEmberOfTaeshalachAI : public AntorusAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AntorusEmberOfTaeshalachAI(c); }
    explicit AntorusEmberOfTaeshalachAI(Creature* pCreature) : AntorusAI(pCreature) { mAlwaysActive = true; }

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        _setMeleeDisabled(true);
        castSpellOnSelf(SPELL_EMBER_WROUGHT_IN_FLAME, true);
        scriptEvents.addEvent(EVENT_EMBER_SEEK, 1000);
    }

protected:
    // reaching Aggramar erupts in flames
    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_EMBER_SEEK)
            return;

        Creature* pAggramar = tracked(BOSS_AGGRAMAR);
        if (pAggramar == nullptr || !pAggramar->isAlive())
            return;

        if (getRangeToObject(pAggramar) <= 10.0f)
        {
            castSpellAOE(SPELL_EMBER_BLAZING_ERUPTION, true);
            despawn(500, 0);
            return;
        }

        movePoint(POINT_EMBER_TO_AGGRAMAR, pAggramar->GetPosition());
        scriptEvents.addEvent(EVENT_EMBER_SEEK, 500);
    }
};

class AntorusFlameOfTaeshalachAI : public AntorusAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AntorusFlameOfTaeshalachAI(c); }
    explicit AntorusFlameOfTaeshalachAI(Creature* pCreature) : AntorusAI(pCreature) {}

    void OnDied(Unit* /*pKiller*/) override
    {
        if (AntorusInstanceScript* pAntorus = antorus())
            pAntorus->sendAction(BOSS_AGGRAMAR, ACTION_AGGRAMAR_FLAME_DOWN);
    }

protected:
    void onEngage() override { castSpellOnSelf(SPELL_FLAME_UNCHECKED_FLAME, true); }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Argus the Unmaker

class AntorusArgusAI : public AntorusBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AntorusArgusAI(c); }
    explicit AntorusArgusAI(Creature* pCreature) : AntorusBossAI(pCreature, DATA_ARGUS) { setBerserk(720000); }

    void DoAction(int32_t action) override
    {
        if (action == ACTION_ARGUS_CONSTELLAR_DOWN && mStage == 3 && mConstellarsAlive > 0 && --mConstellarsAlive == 0)
            beginFinalStage();

        AntorusBossAI::DoAction(action);
    }

protected:
    void onEngage() override
    {
        mStage = 1;
        mScytheGap = 8200;
        mBombNext = true;
        mConstellarsAlive = 0;
        mModuleCount = 3;
        mConstellars.clear();
        openStage();
    }

    void onDisengage() override
    {
        removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        setReactState(REACT_AGGRESSIVE);
    }

    void onTick(uint32_t /*diff*/) override
    {
        if (mStage == 1 && _getHealthPercent() < 70)
            beginSecondStage();
        else if (mStage == 2 && _getHealthPercent() < 40)
            beginThirdStage();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ARGUS_SWEEPING_SCYTHE:
                castSpellOnVictim(SPELL_ARGUS_SWEEPING_SCYTHE);
                scriptEvents.addEvent(EVENT_ARGUS_SWEEPING_SCYTHE, mScytheGap);
                mScytheGap = mScytheGap > 6200 ? 6200 : 7200;
                break;
            case EVENT_ARGUS_TORTURED_RAGE:
                castSpellAOE(SPELL_ARGUS_TORTURED_RAGE);
                scriptEvents.addEvent(EVENT_ARGUS_TORTURED_RAGE, 13400);
                break;
            case EVENT_ARGUS_SOULBLIGHT_ORB:
            {
                Unit* pTarget = sparePlayer();
                summonCreature(NPC_SOULBLIGHT_ORB, pTarget != nullptr ? pTarget->GetPosition() : here(), TIMED_DESPAWN, 20000);
                scriptEvents.addEvent(EVENT_ARGUS_SOULBLIGHT_ORB, 25000);
                break;
            }
            case EVENT_ARGUS_SKY_AND_SEA:
                skyAndSea();
                scriptEvents.addEvent(EVENT_ARGUS_SKY_AND_SEA, 25000);
                break;
            case EVENT_ARGUS_EDGE_OF_OBLITERATION:
                warn("Argus the Unmaker hurls an Edge of Obliteration!");
                summonCreature(NPC_EDGE_OF_OBLITERATION, ringPoint(ArgusArenaCentre, 40.0f, Util::getRandomFloat(AntorusTwoPi)), TIMED_DESPAWN, 30000);
                scriptEvents.addEvent(EVENT_ARGUS_EDGE_OF_OBLITERATION, 34000);
                break;
            case EVENT_ARGUS_AVATAR_OF_AGGRAMAR:
                if (Unit* pTank = currentVictim())
                {
                    warn("Aggramar's avatar descends upon the tank!");
                    pTank->castSpell(pTank, SPELL_ARGUS_AVATAR_OF_AGGRAMAR, true);
                }
                break;
            case EVENT_ARGUS_VOLATILE_SOUL:
                volatileSoul();
                scriptEvents.addEvent(EVENT_ARGUS_VOLATILE_SOUL, mStage >= 4 ? 40000 : 20000);
                break;
            case EVENT_ARGUS_TEMPORAL_BLAST:
                castSpellOnSelf(SPELL_ARGUS_TEMPORAL_BLAST, true);
                scriptEvents.addEvent(EVENT_ARGUS_CONSTELLARS, 4000);
                break;
            case EVENT_ARGUS_CONSTELLARS:
                callConstellars();
                break;
            case EVENT_ARGUS_COSMIC_BEACON:
                orderBeacons();
                scriptEvents.addEvent(EVENT_ARGUS_COSMIC_BEACON, hardMode() ? 20000 : 30000);
                break;
            case EVENT_ARGUS_REORIGINATION_MODULES:
                warn("Argus the Unmaker deploys Reorigination Modules! Destroy them!");
                for (uint32_t i = 0; i < mModuleCount; ++i)
                {
                    LocationVector spot = ringPoint(ArgusArenaCentre, 40.0f, i * AntorusTwoPi / mModuleCount);
                    spot.z += 5.0f;
                    callAdd(NPC_REORIGINATION_MODULE, spot);
                }
                ++mModuleCount;
                scriptEvents.addEvent(EVENT_ARGUS_REORIGINATION_MODULES, 50000);
                break;
            default:
                break;
        }
    }

private:
    void openStage()
    {
        switch (mStage)
        {
            case 1:
                scriptEvents.addEvent(EVENT_ARGUS_SWEEPING_SCYTHE, 6000);
                scriptEvents.addEvent(EVENT_ARGUS_TORTURED_RAGE, 12000);
                scriptEvents.addEvent(EVENT_ARGUS_SOULBLIGHT_ORB, 35000);
                scriptEvents.addEvent(EVENT_ARGUS_SKY_AND_SEA, 11000);
                break;
            case 2:
                scriptEvents.addEvent(EVENT_ARGUS_SWEEPING_SCYTHE, 16500);
                scriptEvents.addEvent(EVENT_ARGUS_EDGE_OF_OBLITERATION, 21000);
                scriptEvents.addEvent(EVENT_ARGUS_AVATAR_OF_AGGRAMAR, 21000);
                scriptEvents.addEvent(EVENT_ARGUS_VOLATILE_SOUL, 28000);
                break;
            default:
                scriptEvents.addEvent(EVENT_ARGUS_SWEEPING_SCYTHE, 5200);
                scriptEvents.addEvent(EVENT_ARGUS_TORTURED_RAGE, 10000);
                scriptEvents.addEvent(EVENT_ARGUS_REORIGINATION_MODULES, 13000);
                scriptEvents.addEvent(EVENT_ARGUS_VOLATILE_SOUL, 20000);
                break;
        }
    }

    void beginSecondStage()
    {
        mStage = 2;
        mScytheGap = 8200;
        mBombNext = true;
        scriptEvents.resetEvents();
        warn("The might of the Pantheon fades. Argus the Unmaker grows stronger!");
        openStage();
    }

    // Argus withdraws while the constellar designates fight
    void beginThirdStage()
    {
        mStage = 3;
        scriptEvents.resetEvents();
        summons.despawnEntry(NPC_EDGE_OF_OBLITERATION);

        warn("Argus the Unmaker withdraws and calls upon the Constellar Designates!");
        attackStop();
        setReactState(REACT_PASSIVE);
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        movePoint(POINT_ARGUS_CENTRE, ArgusArenaCentre);
        scriptEvents.addEvent(EVENT_ARGUS_TEMPORAL_BLAST, 12000);
    }

    void callConstellars()
    {
        mConstellars.clear();
        mConstellarsAlive = 0;
        for (LocationVector const& station : ArgusConstellarStations)
        {
            if (Creature* pConstellar = callAdd(NPC_CONSTELLAR_DESIGNATE, station))
            {
                mConstellars.push_back(pConstellar->GetNewGUID());
                ++mConstellarsAlive;
            }
        }

        if (mConstellarsAlive == 0)
        {
            beginFinalStage();
            return;
        }

        scriptEvents.addEvent(EVENT_ARGUS_COSMIC_BEACON, 25000);
    }

    void orderBeacons()
    {
        InstanceScript* pInstance = getInstanceScript();
        if (pInstance == nullptr)
            return;

        uint32_t orders = hardMode() ? 3 : 2;
        for (WoWGuid const& guid : mConstellars)
        {
            Creature* pConstellar = pInstance->getCreatureByGuid(guid);
            if (orders == 0 || pConstellar == nullptr || !pConstellar->isAlive() || pConstellar->GetScript() == nullptr || Util::getRandomUInt(1) == 0)
                continue;

            pConstellar->GetScript()->DoAction(ACTION_CONSTELLAR_BEACON);
            --orders;
        }
    }

    void beginFinalStage()
    {
        mStage = 4;
        mScytheGap = 8200;
        mBombNext = true;
        scriptEvents.resetEvents();
        mConstellars.clear();

        warn("Argus the Unmaker returns to the battle!");
        removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        setReactState(REACT_AGGRESSIVE);
        openStage();
    }

    // the Titan gifts land on two players
    void skyAndSea()
    {
        std::vector<Unit*> chosen = pickPlayers(2, false);
        if (chosen.empty())
            return;

        chosen[0]->castSpell(chosen[0], SPELL_ARGUS_GIFT_OF_THE_SKY, true);
        if (chosen.size() > 1)
            chosen[1]->castSpell(chosen[1], SPELL_ARGUS_GIFT_OF_THE_SEA, true);
    }

    // two Soulbursts, every second wave also a Soulbomb
    void volatileSoul()
    {
        std::vector<Unit*> chosen = pickPlayers(mBombNext ? 3 : 2, true);
        for (size_t i = 0; i < chosen.size(); ++i)
            castSpell(chosen[i], (mBombNext && i == 2) ? SPELL_ARGUS_SOULBOMB : SPELL_ARGUS_SOULBURST, true);

        if (mBombNext)
            warn("Argus the Unmaker marks his victims with Soulburst and Soulbomb!");

        mBombNext = !mBombNext;
    }

    uint32_t mStage = 1;
    uint32_t mScytheGap = 8200;
    bool mBombNext = true;
    uint32_t mConstellarsAlive = 0;
    uint32_t mModuleCount = 3;
    std::vector<WoWGuid> mConstellars;
};

class AntorusEdgeOfObliterationAI : public AntorusAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AntorusEdgeOfObliterationAI(c); }
    explicit AntorusEdgeOfObliterationAI(Creature* pCreature) : AntorusAI(pCreature) { mAlwaysActive = true; }

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        _setMeleeDisabled(true);
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        castSpellOnSelf(SPELL_EDGE_OF_OBLITERATION_VISUAL, true);
        scriptEvents.addEvent(EVENT_EDGE_TARGET, 4500);
    }

    void OnReachWP(uint32_t type, uint32_t id) override
    {
        if (type == POINT_MOTION_TYPE && id == POINT_EDGE_SWEEP)
            castSpellAOE(SPELL_EDGE_OF_OBLITERATION, true);
    }

protected:
    // picks a player and sweeps through the spot a few seconds later
    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_EDGE_TARGET:
                if (Unit* pTarget = anyPlayer())
                {
                    mSweepEnd = pTarget->GetPosition();
                    if (mSweepEnd.getExactDist2d(ArgusArenaCentre) > 50.0f)
                        mSweepEnd = ArgusArenaCentre;
                    scriptEvents.addEvent(EVENT_EDGE_SWEEP, 4000);
                }
                scriptEvents.addEvent(EVENT_EDGE_TARGET, 10000);
                break;
            case EVENT_EDGE_SWEEP:
                movePoint(POINT_EDGE_SWEEP, mSweepEnd, false);
                break;
            default:
                break;
        }
    }

private:
    LocationVector mSweepEnd;
};

class AntorusConstellarDesignateAI : public AntorusAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AntorusConstellarDesignateAI(c); }
    explicit AntorusConstellarDesignateAI(Creature* pCreature) : AntorusAI(pCreature) {}

    void OnLoad() override
    {
        setRooted(true);
        _setMeleeDisabled(true);
    }

    void DoAction(int32_t action) override
    {
        if (action == ACTION_CONSTELLAR_BEACON && isAlive())
        {
            if (Unit* pTarget = sparePlayer())
            {
                castSpell(pTarget, SPELL_CONSTELLAR_COSMIC_BEACON, true);
                mBeaconGuid = pTarget->GetNewGUID();
                scriptEvents.addEvent(EVENT_CONSTELLAR_SMASH, 4000);
            }
        }

        AntorusAI::DoAction(action);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (AntorusInstanceScript* pAntorus = antorus())
            pAntorus->sendAction(BOSS_ARGUS_THE_UNMAKER, ACTION_ARGUS_CONSTELLAR_DOWN);
    }

protected:
    void onEngage() override { scriptEvents.addEvent(EVENT_CONSTELLAR_STARBLAST, 5000); }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_CONSTELLAR_STARBLAST:
                castOn(anyPlayer(), SPELL_CONSTELLAR_STARBLAST);
                scriptEvents.addEvent(EVENT_CONSTELLAR_STARBLAST, 2000);
                break;
            case EVENT_CONSTELLAR_SMASH:
            {
                Unit* pTarget = unitFor(mBeaconGuid);
                if (pTarget != nullptr && pTarget->isAlive())
                    getCreature()->castSpellLoc(pTarget->GetPosition(), SPELL_CONSTELLAR_COSMIC_SMASH, true);
                break;
            }
            default:
                break;
        }
    }

private:
    WoWGuid mBeaconGuid;
};

class AntorusReoriginationModuleAI : public AntorusAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AntorusReoriginationModuleAI(c); }
    explicit AntorusReoriginationModuleAI(Creature* pCreature) : AntorusAI(pCreature) { mAlwaysActive = true; }

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        setRooted(true);
        _setMeleeDisabled(true);
        scriptEvents.addEvent(EVENT_MODULE_INITIALIZE, 3500);
    }

protected:
    // a module left alive finishes its sequence and pulses the raid
    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_MODULE_INITIALIZE:
                castSpellOnSelf(SPELL_MODULE_INITIALIZATION, true);
                scriptEvents.addEvent(EVENT_MODULE_PULSE, 25000);
                break;
            case EVENT_MODULE_PULSE:
                castSpellAOE(SPELL_MODULE_REORIGINATION_PULSE, true);
                despawn(1000, 0);
                break;
            default:
                break;
        }
    }
};

void SetupAntorusTheBurningThrone(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_ANTORUS_THE_BURNING_THRONE, &AntorusInstanceScript::Create);

    // Garothi Worldbreaker
    mgr->register_creature_script(BOSS_GAROTHI_WORLDBREAKER, &AntorusGarothiAI::Create);
    mgr->register_creature_script(NPC_GAROTHI_ANNIHILATOR, &AntorusGarothiCannonAI::Create);
    mgr->register_creature_script(NPC_GAROTHI_DECIMATOR, &AntorusGarothiCannonAI::Create);

    // Felhounds of Sargeras
    mgr->register_creature_script(BOSS_SHATUG, &AntorusShatugAI::Create);
    mgr->register_creature_script(BOSS_FHARG, &AntorusFhargAI::Create);

    // Antoran High Command
    mgr->register_creature_script(BOSS_ADMIRAL_SVIRAX, &AntorusCommanderAI::Create);
    mgr->register_creature_script(BOSS_CHIEF_ENGINEER_ISHKAR, &AntorusCommanderAI::Create);
    mgr->register_creature_script(BOSS_GENERAL_ERODUS, &AntorusCommanderAI::Create);
    mgr->register_creature_script(NPC_ENTROPIC_MINE, &AntorusEntropicMineAI::Create);
    mgr->register_creature_script(NPC_FANATICAL_PYROMANCER, &AntorusCommandReinforcementAI::Create);
    mgr->register_creature_script(NPC_FELBLADE_SHOCKTROOPER, &AntorusCommandReinforcementAI::Create);

    // Portal Keeper Hasabel
    mgr->register_creature_script(BOSS_PORTAL_KEEPER_HASABEL, &AntorusHasabelAI::Create);
    mgr->register_creature_script(NPC_COLLAPSING_WORLD, &AntorusFixtureAI::Create);
    mgr->register_creature_script(NPC_FELCRUSH_PORTAL, &AntorusFixtureAI::Create);
    mgr->register_creature_script(NPC_TRANSPORT_PORTAL, &AntorusFixtureAI::Create);
    mgr->register_creature_script(NPC_BLAZING_IMP, &AntorusHasabelMinionAI::Create);
    mgr->register_creature_script(NPC_FELTOUCHED_SKITTERER, &AntorusHasabelMinionAI::Create);
    mgr->register_creature_script(NPC_HUNGERING_STALKER, &AntorusHasabelMinionAI::Create);

    // The Defense of Eonar
    mgr->register_creature_script(NPC_ESSENCE_OF_EONAR, &AntorusEonarAI::Create);
    mgr->register_creature_script(NPC_THE_PARAXIS, &AntorusFixtureAI::Create);
    mgr->register_creature_script(NPC_EONAR_FELGUARD, &AntorusEonarDemonAI::Create);
    mgr->register_creature_script(NPC_EONAR_FEL_LORD, &AntorusEonarDemonAI::Create);
    mgr->register_creature_script(NPC_EONAR_FEL_HOUND, &AntorusEonarDemonAI::Create);
    mgr->register_creature_script(NPC_VOLANT_KERAPTERON, &AntorusEonarDemonAI::Create);
    mgr->register_creature_script(NPC_FEL_INFUSED_DESTRUCTOR, &AntorusEonarDemonAI::Create);
    mgr->register_creature_script(NPC_FEL_CHARGED_OBFUSCATOR, &AntorusEonarDemonAI::Create);
    mgr->register_creature_script(NPC_FEL_POWERED_PURIFIER, &AntorusEonarDemonAI::Create);

    // Imonar the Soulhunter
    mgr->register_creature_script(BOSS_IMONAR, &AntorusImonarAI::Create);
    mgr->register_creature_script(NPC_IMONAR_BOMBING_RUN, &AntorusImonarShipAI::Create);
    mgr->register_creature_script(NPC_IMONAR_ARTILLERY_STRIKE, &AntorusImonarShipAI::Create);

    // Kin'garoth
    mgr->register_creature_script(BOSS_KINGAROTH, &AntorusKingarothAI::Create);
    mgr->register_creature_script(NPC_KINGAROTH_RUINER, &AntorusKingarothRuinerAI::Create);
    mgr->register_creature_script(NPC_GAROTHI_REAVER_ANNIHILATOR, &AntorusGarothiReaverAI::Create);
    mgr->register_creature_script(NPC_GAROTHI_REAVER_DECIMATOR, &AntorusGarothiReaverAI::Create);
    mgr->register_creature_script(NPC_GAROTHI_REAVER_DEMOLISHER, &AntorusGarothiReaverAI::Create);

    // Varimathras
    mgr->register_creature_script(BOSS_VARIMATHRAS, &AntorusVarimathrasAI::Create);
    mgr->register_creature_script(NPC_SHADOW_OF_VARIMATHRAS, &AntorusFixtureAI::Create);

    // The Coven of Shivarra
    mgr->register_creature_script(BOSS_ASARA, &AntorusAsaraAI::Create);
    mgr->register_creature_script(BOSS_NOURA, &AntorusNouraAI::Create);
    mgr->register_creature_script(BOSS_DIIMA, &AntorusDiimaAI::Create);
    mgr->register_creature_script(BOSS_THURAYA, &AntorusThurayaAI::Create);
    mgr->register_creature_script(NPC_COVEN_SHADOW_BLADE, &AntorusCovenShadowBladeAI::Create);
    mgr->register_creature_script(NPC_TORMENT_OF_GOLGANNETH, &AntorusCovenTormentAI::Create);
    mgr->register_creature_script(NPC_TORMENT_OF_KHAZGOROTH, &AntorusCovenTormentAI::Create);
    mgr->register_creature_script(NPC_TORMENT_OF_AMANTHUL, &AntorusCovenTormentAI::Create);

    // Aggramar
    mgr->register_creature_script(BOSS_AGGRAMAR, &AntorusAggramarAI::Create);
    mgr->register_creature_script(NPC_EMBER_OF_TAESHALACH, &AntorusEmberOfTaeshalachAI::Create);
    mgr->register_creature_script(NPC_FLAME_OF_TAESHALACH, &AntorusFlameOfTaeshalachAI::Create);

    // Argus the Unmaker
    mgr->register_creature_script(BOSS_ARGUS_THE_UNMAKER, &AntorusArgusAI::Create);
    mgr->register_creature_script(NPC_SOULBLIGHT_ORB, &AntorusFixtureAI::Create);
    mgr->register_creature_script(NPC_EDGE_OF_OBLITERATION, &AntorusEdgeOfObliterationAI::Create);
    mgr->register_creature_script(NPC_CONSTELLAR_DESIGNATE, &AntorusConstellarDesignateAI::Create);
    mgr->register_creature_script(NPC_REORIGINATION_MODULE, &AntorusReoriginationModuleAI::Create);
}
