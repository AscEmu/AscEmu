/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Raid_Tomb_Of_Sargeras.hpp"

#include "Map/Maps/WorldMap.hpp"
#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

enum TombOfSargerasEvents
{
    // shared helpers
    EVENT_TOMB_SPELL_VOLLEY = 1,

    // Goroth
    EVENT_GOROTH_BURNING_ARMOR,
    EVENT_GOROTH_CRASHING_COMET,
    EVENT_GOROTH_INFERNAL_SPIKES,
    EVENT_GOROTH_SHATTERING_STAR,
    EVENT_GOROTH_INFERNAL_BURNING,
    EVENT_GOROTH_SPIKES_SHATTER,

    // Harjatan
    EVENT_HARJATAN_UNCHECKED_RAGE,
    EVENT_HARJATAN_COMMANDING_ROAR,
    EVENT_HARJATAN_DRAW_IN,
    EVENT_HARJATAN_DRAW_IN_ENDS,
    EVENT_GLADIATOR_DRIVEN_ASSAULT,
    EVENT_GLADIATOR_SPLASHY_CLEAVE,
    EVENT_WAVEMENDER_AQUEOUS_BURST,

    // Mistress Sassz'ine
    EVENT_SASSZINE_BURDEN_OF_PAIN,
    EVENT_SASSZINE_ABYSS_CALL,
    EVENT_SASSZINE_HYDRA_SHOT,
    EVENT_SASSZINE_HYDRA_IMPACT,
    EVENT_SASSZINE_THUNDERING_SHOCK,
    EVENT_SASSZINE_CONSUMING_HUNGER,
    EVENT_SASSZINE_BECKON_SARUKEL,
    EVENT_SASSZINE_SUMMON_OSSUNET,
    EVENT_SASSZINE_CALL_VELLIUS,

    // Fallen Avatar
    EVENT_AVATAR_PYLON,
    EVENT_AVATAR_UNBOUND_CHAOS,
    EVENT_AVATAR_DESOLATE,
    EVENT_AVATAR_SHADOWY_BLADES,
    EVENT_AVATAR_RUPTURE_REALITIES,
    EVENT_AVATAR_REACH_CHECK,
    EVENT_AVATAR_FULL_ENERGY,
    EVENT_AVATAR_CONSUMED,
    EVENT_AVATAR_ANNIHILATED,
    EVENT_AVATAR_SEAR,
    EVENT_AVATAR_DARK_MARK,
    EVENT_AVATAR_RUPTURE_AWAKENED,
    EVENT_VALOR_CLEANSING_PROTOCOL,

    // Demonic Inquisition
    EVENT_ATRIGAN_SCYTHE_SWEEP,
    EVENT_ATRIGAN_BONE_SAW,
    EVENT_ATRIGAN_TORMENTED_SOUL,
    EVENT_BELAC_PANGS_OF_GUILT,
    EVENT_BELAC_ECHOING_ANGUISH,
    EVENT_BELAC_TORMENTING_BURST,
    EVENT_BELAC_FEL_SQUALL,
    EVENT_TORMENTED_SOUL_CORRUPTION,
    EVENT_TORMENTED_SOUL_ERUPTION,

    // Sisters of the Moon
    EVENT_KASPARIAN_MOON_GLAIVE,
    EVENT_KASPARIAN_TWILIGHT_GLAIVE,
    EVENT_KASPARIAN_SPECTRAL_GLAIVE,
    EVENT_YATHAE_TWILIGHT_VOLLEY,
    EVENT_YATHAE_SHADOW_SHOT,
    EVENT_YATHAE_INCORPOREAL_RELEASE,
    EVENT_YATHAE_CALL_MOONTALON,
    EVENT_YATHAE_RAPID_SHOT,
    EVENT_LUNASPYRE_MOON_BURN,
    EVENT_LUNASPYRE_LUNAR_STRIKE,
    EVENT_LUNASPYRE_LUNAR_BEACON,
    EVENT_LUNASPYRE_LUNAR_FIRE,
    EVENT_MOONTALON_DEATHLY_SCREECH,

    // Kil'jaeden
    EVENT_KILJAEDEN_FELCLAWS,
    EVENT_KILJAEDEN_ARMAGEDDON,
    EVENT_KILJAEDEN_SINGULARITY,
    EVENT_KILJAEDEN_BURSTING_DREADFLAME,
    EVENT_KILJAEDEN_FOCUSED_DREADFLAME,
    EVENT_KILJAEDEN_SHADOW_REFLECTION,
    EVENT_KILJAEDEN_REFLECTIONS_EMERGE,
    EVENT_KILJAEDEN_FLIGHT_STEP,
    EVENT_KILJAEDEN_SHADOWSOULS,
    EVENT_KILJAEDEN_DARKNESS,
    EVENT_KILJAEDEN_OBELISKS,
    EVENT_KILJAEDEN_TEAR_RIFT,
    EVENT_SHADOWSOUL_DESTABILIZE,

    // Maiden of Vigilance
    EVENT_MAIDEN_INFUSION,
    EVENT_MAIDEN_HAMMER_OF_CREATION,
    EVENT_MAIDEN_CREATION_LANDS,
    EVENT_MAIDEN_HAMMER_OF_OBLITERATION,
    EVENT_MAIDEN_OBLITERATION_LANDS,
    EVENT_MAIDEN_MASS_INSTABILITY,
    EVENT_MAIDEN_INSTABILITY_LANDS,
    EVENT_MAIDEN_WRATH,
    EVENT_MAIDEN_WRATH_BEGIN,

    // The Desolate Host
    EVENT_ENGINE_BONE_SHARDS,
    EVENT_ENGINE_COLLAPSING_FISSURE,
    EVENT_ENGINE_TORMENTED_CRIES,
    EVENT_ENGINE_CORPOREAL_WAVE,
    EVENT_DEJAHNA_CRUSH_MIND,
    EVENT_DEJAHNA_SOULBIND,
    EVENT_DEJAHNA_WAILING_SOULS,
    EVENT_DEJAHNA_SPIRIT_WAVE,
    EVENT_HOST_TORMENT,
    EVENT_HOST_SUNDERING_DOOM,
    EVENT_HOST_DOOMED_SUNDERING,
    EVENT_TEMPLAR_RUPTURING_SLAM,
    EVENT_BONEWARDEN_GRASPING_DARKNESS,
    EVENT_PRIESTESS_SHATTERING_SCREAM,
    EVENT_PRIESTESS_SPIRIT_CHAINS
};

enum TombOfSargerasActions
{
    ACTION_TOMB_HOST_AWAKEN = 1
};

namespace
{
    float const TombTwoPi = 6.28318531f;

    // energy bars are kept as fill times
    uint32_t const GorothEnergyFillMs = 53000;
    uint32_t const HarjatanRageFillMs = 20000;
    uint32_t const InquisitionEnergyFillMs = 60000;
    uint32_t const AvatarDormantEnergyFillMs = 450000;
    uint32_t const AvatarAwakenedEnergyFillMs = 34000;
    uint32_t const ValorEnergyFillMs = 60000;

    uint8_t const GorothSpikesPerVolley = 3;
    uint8_t const SasszineStalkersPerBurden = 3;
    uint8_t const SasszineInkTargets = 2;
    uint8_t const KiljaedenArmageddonStalkers = 5;
    uint8_t const KiljaedenShadowsoulCount = 5;
    uint8_t const KiljaedenReflectionTargets = 2;

    uint32_t const SistersFirstMoonMs = 48000;
    uint32_t const SistersMoonCycleMs = 54000;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Positions

    LocationVector const HarjatanAddSpots[4] =
    {
        { 5877.17f, -906.50f, 2920.25f, 6.20f },
        { 5880.17f, -896.95f, 2920.35f, 5.73f },
        { 5889.86f, -886.83f, 2920.35f, 5.42f },
        { 5946.16f, -919.47f, 2919.52f, 2.62f }
    };

    LocationVector const SasszinePiranhadoSpots[8] =
    {
        { 5813.55f, -1211.53f, 2785.97f, 4.35f },
        { 5833.01f, -1227.46f, 2785.97f, 3.70f },
        { 5781.11f, -1212.40f, 2786.33f, 5.19f },
        { 5761.98f, -1231.07f, 2786.33f, 5.84f },
        { 5837.62f, -1261.57f, 2786.33f, 2.81f },
        { 5818.79f, -1283.59f, 2786.33f, 2.06f },
        { 5763.95f, -1267.11f, 2786.33f, 0.47f },
        { 5784.16f, -1286.02f, 2786.33f, 1.17f }
    };

    LocationVector const AvatarValorSpot(6583.50f, -771.65f, 1663.67f, 3.04f);

    LocationVector const AvatarPylonSpots[5] =
    {
        { 6547.43f, -760.646f, 1662.98f, 5.75959f },
        { 6538.19f, -795.174f, 1662.98f, 0.0f },
        { 6547.38f, -829.529f, 1662.98f, 0.523599f },
        { 6572.73f, -854.656f, 1662.98f, 1.0472f },
        { 6572.56f, -735.365f, 1662.98f, 5.23599f }
    };

    LocationVector const AvatarBladeSpots[3] =
    {
        { 6594.51f, -806.92f, 1663.58f, 0.0f },
        { 6589.36f, -794.80f, 1663.58f, 0.0f },
        { 6597.37f, -779.48f, 1663.58f, 0.0f }
    };

    LocationVector const InquisitionSoulSpot(6403.00f, -935.00f, 3037.16f, 0.0f);

    LocationVector const MaidenFarSide(6310.30f, -795.36f, 2887.84f, 0.0f);

    LocationVector const HostRoomCentre(6438.45f, -1089.73f, 2881.52f, 0.0f);

    LocationVector const KiljaedenPlatformCentre(4500.39f, -1510.12f, 5385.64f, 0.0f);

    LocationVector const KiljaedenSingularitySpots[4] =
    {
        { 4538.57f, -1548.27f, 5385.64f, 0.0f },
        { 4462.66f, -1547.71f, 5385.64f, 0.0f },
        { 4462.46f, -1472.97f, 5385.64f, 0.0f },
        { 4538.33f, -1472.74f, 5385.64f, 0.0f }
    };

    LocationVector const KiljaedenRiftSpots[4] =
    {
        { 4472.49f, -1449.58f, 5385.99f, 0.0f },
        { 4528.95f, -1450.75f, 5385.99f, 0.0f },
        { 4532.03f, -1568.90f, 5385.99f, 0.0f },
        { 4469.95f, -1570.39f, 5385.99f, 0.0f }
    };

    struct TombPlacement
    {
        uint32_t entry;
        LocationVector spot;
    };

    TombPlacement const SistersPlacements[3] =
    {
        { BOSS_HUNTRESS_KASPARIAN,  { 6208.61f, -1026.84f, 2899.56f, 2.77f } },
        { BOSS_PRIESTESS_LUNASPYRE, { 6208.68f, -1037.31f, 2899.55f, 2.97f } },
        { BOSS_CAPTAIN_YATHAE,      { 6215.13f, -1019.96f, 2899.55f, 2.66f } }
    };

    TombPlacement const HostPlacements[3] =
    {
        { BOSS_ENGINE_OF_SOULS,     { 6456.64f, -1105.83f, 2881.60f, 2.50f } },
        { BOSS_SOUL_QUEEN_DEJAHNA,  { 6461.07f, -1088.02f, 2881.60f, 2.73f } },
        { BOSS_DESOLATE_HOST,       { 6447.46f, -1099.68f, 2881.60f, 2.49f } }
    };

    //////////////////////////////////////////////////////////////////////////////////////////
    // Cooldown lists, the first value is the opener

    std::vector<uint32_t> const GorothStarSteps = { 24000, 60000, 60000, 60000, 50000 };

    std::vector<uint32_t> const SasszineShockSteps = { 10000, 44000, 32000 };
    std::vector<uint32_t> const SasszineHungerFirstSteps = { 64000, 34000 };
    std::vector<uint32_t> const SasszineOssunetSecondSteps = { 10000, 50000, 38000, 42000 };
    std::vector<uint32_t> const SasszineVelliusSecondSteps = { 36000, 42000, 44000 };
    std::vector<uint32_t> const SasszineOssunetThirdSteps = { 10000, 34000, 34000, 32000, 32000, 34000 };
    std::vector<uint32_t> const SasszineHungerThirdSteps = { 52000, 32000 };
    std::vector<uint32_t> const SasszineVelliusThirdSteps = { 36000, 56000, 76000, 44000, 32000 };

    std::vector<uint32_t> const AvatarChaosSteps = { 7000, 46000, 35000, 42000, 62000, 35000, 50000 };
    std::vector<uint32_t> const AvatarBladeSteps = { 30000, 45000, 61000, 37000, 42000, 38000 };
    std::vector<uint32_t> const AvatarRuptureSteps = { 38000, 60000 };

    std::vector<uint32_t> const KiljaedenArmageddonFirstSteps = { 10000, 64000 };
    std::vector<uint32_t> const KiljaedenSingularityFirstSteps = { 58000, 56000 };
    std::vector<uint32_t> const KiljaedenArmageddonSecondSteps = { 50400, 45000, 31000, 35000, 31000, 78000 };
    std::vector<uint32_t> const KiljaedenSingularitySecondSteps = { 74000, 81000 };
    std::vector<uint32_t> const KiljaedenBurstingSecondSteps = { 52400, 48000, 55000, 50000 };
    std::vector<uint32_t> const KiljaedenFocusedSecondSteps = { 76000, 99000 };
    // the last two values alternate
    std::vector<uint32_t> const KiljaedenBurstingFinalSteps = { 42000, 25000, 70000 };
    std::vector<uint32_t> const KiljaedenFocusedFinalSteps = { 80000, 95000, 80000 };

    // walks a cooldown list, the tail repeats from loopFrom
    class TombCooldowns
    {
    public:
        void begin(std::vector<uint32_t> const& steps)
        {
            beginLooped(steps, steps.empty() ? 0 : steps.size() - 1);
        }

        void beginLooped(std::vector<uint32_t> const& steps, size_t loopFrom)
        {
            mSteps = &steps;
            mCursor = 0;
            mLoopFrom = loopFrom < steps.size() ? loopFrom : 0;
        }

        uint32_t next()
        {
            if (mSteps == nullptr || mSteps->empty())
                return 60000;

            uint32_t const value = (*mSteps)[mCursor];
            if (++mCursor >= mSteps->size())
                mCursor = mLoopFrom;

            return value;
        }

    private:
        std::vector<uint32_t> const* mSteps = nullptr;
        size_t mCursor = 0;
        size_t mLoopFrom = 0;
    };

    // Kil'jaeden's flight above the platform
    enum class KiljaedenFlightMove : uint8_t
    {
        Armageddon,
        Bursting,
        Focused,
        Land
    };

    struct KiljaedenFlightStep
    {
        uint32_t delayMs;
        KiljaedenFlightMove move;
    };

    KiljaedenFlightStep const KiljaedenFlightPlan[] =
    {
        {  9000, KiljaedenFlightMove::Armageddon },
        {  1000, KiljaedenFlightMove::Bursting },
        { 16000, KiljaedenFlightMove::Focused },
        { 13000, KiljaedenFlightMove::Armageddon },
        {  1000, KiljaedenFlightMove::Focused },
        { 19000, KiljaedenFlightMove::Land }
    };

    size_t const KiljaedenFlightStepCount = sizeof(KiljaedenFlightPlan) / sizeof(KiljaedenFlightPlan[0]);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Doors and encounter groups

    enum class TombDoorKind : uint8_t
    {
        Room,       // shut while the encounter runs
        Passage,    // opens after the encounter
        Altar       // opens after the six wing encounters
    };

    struct TombDoorRule
    {
        uint32_t entry;
        uint32_t dataId;
        TombDoorKind kind;
    };

    TombDoorRule const TombDoorRules[] =
    {
        { GO_GOROTH_EXIT_GATE,          DATA_GOROTH,                TombDoorKind::Passage },
        { GO_GOROTH_UPPER_GATE,         DATA_GOROTH,                TombDoorKind::Passage },
        { GO_GOROTH_PILLAR_GATE,        DATA_GOROTH,                TombDoorKind::Passage },
        { GO_GOROTH_FIRE_DOOR,          DATA_GOROTH,                TombDoorKind::Room },
        { GO_GOROTH_FIREWALL_WEST,      DATA_GOROTH,                TombDoorKind::Room },
        { GO_GOROTH_FIREWALL_EAST,      DATA_GOROTH,                TombDoorKind::Room },
        { GO_HARJATAN_MISTRESS_GATE,    DATA_HARJATAN,              TombDoorKind::Passage },
        { GO_HARJATAN_HALLWAY_GATE,     DATA_HARJATAN,              TombDoorKind::Room },
        { GO_SASSZINE_WATER_GATE,       DATA_MISTRESS_SASSZINE,     TombDoorKind::Room },
        { GO_INQUISITION_ROOM_GATE,     DATA_DEMONIC_INQUISITION,   TombDoorKind::Room },
        { GO_SISTERS_GHOST_BARRIER,     DATA_SISTERS_OF_THE_MOON,   TombDoorKind::Room },
        { GO_HOST_ROOM_GATE,            DATA_THE_DESOLATE_HOST,     TombDoorKind::Room },
        { GO_HOST_GHOSTLY_BARRIER,      DATA_THE_DESOLATE_HOST,     TombDoorKind::Room },
        { GO_MAIDEN_ALTAR,              DATA_MAIDEN_OF_VIGILANCE,   TombDoorKind::Altar },
        { GO_MAIDEN_FELSTORM_BARRIER,   DATA_MAIDEN_OF_VIGILANCE,   TombDoorKind::Room },
        { GO_AVATAR_VAULT_GATE,         DATA_FALLEN_AVATAR,         TombDoorKind::Room }
    };

    uint32_t const TombAltarSeals[] =
    {
        DATA_GOROTH, DATA_HARJATAN, DATA_MISTRESS_SASSZINE, DATA_DEMONIC_INQUISITION, DATA_SISTERS_OF_THE_MOON, DATA_THE_DESOLATE_HOST
    };

    // members of the shared health encounters
    struct TombGroupSlot
    {
        uint32_t dataId;
        uint32_t entry;
        bool joinsOnPull;
    };

    TombGroupSlot const TombGroupSlots[] =
    {
        { DATA_DEMONIC_INQUISITION, BOSS_ATRIGAN,               true },
        { DATA_DEMONIC_INQUISITION, BOSS_BELAC,                 true },
        { DATA_SISTERS_OF_THE_MOON, BOSS_HUNTRESS_KASPARIAN,    true },
        { DATA_SISTERS_OF_THE_MOON, BOSS_CAPTAIN_YATHAE,        true },
        { DATA_SISTERS_OF_THE_MOON, BOSS_PRIESTESS_LUNASPYRE,   true },
        { DATA_THE_DESOLATE_HOST,   BOSS_ENGINE_OF_SOULS,       true },
        { DATA_THE_DESOLATE_HOST,   BOSS_SOUL_QUEEN_DEJAHNA,    true },
        // rises only at the second stage
        { DATA_THE_DESOLATE_HOST,   BOSS_DESOLATE_HOST,         false }
    };

    // random point on a ring around origin
    LocationVector tombRingPoint(LocationVector const& origin, float minRange, float maxRange)
    {
        float const angle = Util::getRandomFloat(TombTwoPi);
        float const range = Util::getRandomFloat(minRange, maxRange);
        return LocationVector(origin.x + range * std::cos(angle), origin.y + range * std::sin(angle), origin.z, angle);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class TombOfSargerasInstanceScript : public InstanceScript
{
public:
    explicit TombOfSargerasInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(TombOfSargerasEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new TombOfSargerasInstanceScript(pMapMgr); }

    // a credited member falling before its partners must not end the encounter
    bool setBossState(uint32_t id, EncounterStates state) override
    {
        if (state == EncounterStates::Performed && getBossState(id) == EncounterStates::InProgress && !isGroupDown(id, nullptr))
            return false;

        return InstanceScript::setBossState(id, state);
    }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        if (findDoorRule(pGameObject->getEntry()) == nullptr)
            return;

        mDoorGuids.push_back(pGameObject->GetNewGUID());
        alignDoor(pGameObject);
    }

    void OnEncounterStateChange(uint32_t id, uint32_t state) override
    {
        for (WoWGuid const& guid : mDoorGuids)
        {
            if (GameObject* pDoor = getGameObjectByGuid(guid))
                alignDoor(pDoor);
        }

        if (state != EncounterStates::Failed)
            return;

        if (id == DATA_THE_DESOLATE_HOST)
            mHostAwakened = false;

        // a wiped group fights again in full
        for (Creature* pMember : getGroup(id))
        {
            if (!pMember->isAlive())
                pMember->despawn(1000, 5000);
        }
    }

    std::vector<Creature*> getGroup(uint32_t dataId)
    {
        std::vector<Creature*> group;
        for (TombGroupSlot const& slot : TombGroupSlots)
        {
            if (slot.dataId != dataId)
                continue;

            for (Creature* pMember : getCreatureSetForEntry(slot.entry))
                group.push_back(pMember);
        }

        return group;
    }

    bool isGroupDown(uint32_t dataId, Creature* pExcept)
    {
        for (Creature* pMember : getGroup(dataId))
        {
            if (pMember != pExcept && pMember->isAlive())
                return false;
        }

        return true;
    }

    // pulling one member pulls the rest
    void rallyGroup(uint32_t dataId, Creature* pPuller)
    {
        if (mRallying)
            return;

        mRallying = true;
        for (TombGroupSlot const& slot : TombGroupSlots)
        {
            if (slot.dataId != dataId || !slot.joinsOnPull)
                continue;

            for (Creature* pMember : getCreatureSetForEntry(slot.entry))
            {
                if (pMember == pPuller || !pMember->isAlive() || pMember->isInCombat())
                    continue;

                if (CreatureAIScript* pScript = pMember->GetScript())
                    pScript->setZoneWideCombat();
            }
        }
        mRallying = false;
    }

    // shared health: a hit on one fighting member lands on all of them
    void shareDamage(uint32_t dataId, Creature* pHit, Unit* pAttacker, uint32_t damage)
    {
        if (mSharing || damage == 0 || pAttacker == nullptr)
            return;

        mSharing = true;
        for (Creature* pMember : getGroup(dataId))
        {
            if (pMember == pHit || pMember == pAttacker || !pMember->isAlive() || !pMember->isInCombat())
                continue;

            if (damage >= pMember->getHealth())
                pAttacker->dealDamage(pMember, pMember->getHealth(), 0);
            else
                pMember->setHealth(pMember->getHealth() - damage);
        }
        mSharing = false;
    }

    void awakenDesolateHost()
    {
        if (mHostAwakened)
            return;

        mHostAwakened = true;
        for (Creature* pHost : getCreatureSetForEntry(BOSS_DESOLATE_HOST))
        {
            if (pHost->isAlive() && pHost->GetScript() != nullptr)
                pHost->GetScript()->DoAction(ACTION_TOMB_HOST_AWAKEN);
        }
    }

    bool isHostAwakened() const { return mHostAwakened; }

private:
    TombDoorRule const* findDoorRule(uint32_t entry) const
    {
        for (TombDoorRule const& rule : TombDoorRules)
        {
            if (rule.entry == entry)
                return &rule;
        }

        return nullptr;
    }

    bool isDoorOpen(TombDoorRule const& rule) const
    {
        switch (rule.kind)
        {
            case TombDoorKind::Room:
                return getBossState(rule.dataId) != EncounterStates::InProgress;
            case TombDoorKind::Passage:
                return getBossState(rule.dataId) == EncounterStates::Performed;
            case TombDoorKind::Altar:
            {
                for (uint32_t dataId : TombAltarSeals)
                {
                    if (getBossState(dataId) != EncounterStates::Performed)
                        return false;
                }
                return true;
            }
        }

        return false;
    }

    // changes the state only when it differs
    void alignDoor(GameObject* pDoor)
    {
        TombDoorRule const* pRule = findDoorRule(pDoor->getEntry());
        if (pRule == nullptr)
            return;

        bool const isOpen = pDoor->getState() != GO_STATE_CLOSED;
        if (isOpen != isDoorOpen(*pRule))
            pDoor->setState(isDoorOpen(*pRule) ? GO_STATE_OPEN : GO_STATE_CLOSED);
    }

    std::vector<WoWGuid> mDoorGuids;
    bool mRallying = false;
    bool mSharing = false;
    bool mHostAwakened = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event tick, berserk and target helpers

class TombOfSargerasCombatAI : public CreatureAIScript
{
public:
    explicit TombOfSargerasCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

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

        if (_isCasting())
            return;

        if (uint32_t const eventId = scriptEvents.getFinishedEvent())
            onEvent(eventId);
    }

protected:
    virtual void onEngage() {}
    virtual void onDisengage() {}
    virtual void onCombatTick(uint32_t /*diff*/) {}
    virtual void onEvent(uint32_t /*eventId*/) {}

    TombOfSargerasInstanceScript* getTomb()
    {
        if (getCreature()->GetMapId() != MAP_TOMB_OF_SARGERAS)
            return nullptr;

        return static_cast<TombOfSargerasInstanceScript*>(getInstanceScript());
    }

    // random player, the tank only when nobody else is in range
    Unit* pickPlayer(bool skipTank = false)
    {
        if (skipTank)
        {
            FilterArgs args(TargetFilter_Player);
            args.addFilter(TargetFilter_NotCurrent);
            if (Unit* pTarget = selectUnitTarget(args))
                return pTarget;
        }

        return selectUnitTarget(FilterArgs(TargetFilter_Player));
    }

    Unit* castAtPlayer(uint32_t spellId, bool skipTank = false, bool triggered = false)
    {
        Unit* pTarget = pickPlayer(skipTank);
        if (pTarget != nullptr)
            castSpell(pTarget, spellId, triggered);

        return pTarget;
    }

    Unit* findUnit(WoWGuid const& guid)
    {
        if (guid.isEmpty() || getCreature()->getWorldMap() == nullptr)
            return nullptr;

        return getCreature()->getWorldMap()->getUnit(guid);
    }

    Creature* findCreature(WoWGuid const& guid)
    {
        if (guid.isEmpty() || getCreature()->getWorldMap() == nullptr)
            return nullptr;

        return getCreature()->getWorldMap()->getCreature(guid);
    }

    // summoned add that joins the fight at once
    Creature* callAdd(uint32_t entry, LocationVector const& spot, CreatureSummonDespawnType despawnType = CORPSE_DESPAWN, uint32_t durationMs = 0)
    {
        Creature* pAdd = summonCreature(entry, spot, despawnType, durationMs);
        if (pAdd != nullptr)
            setZoneWideCombat(pAdd);

        return pAdd;
    }

    // summoned helper that stays out of the fight, lifetime 0 keeps it until despawned
    Creature* placeHelper(uint32_t entry, LocationVector const& spot, uint32_t lifetimeMs)
    {
        return summonCreature(entry, spot, lifetimeMs != 0 ? TIMED_DESPAWN : MANUAL_DESPAWN, lifetimeMs);
    }

    LocationVector aroundSelf(float minRange, float maxRange)
    {
        return tombRingPoint(getCreature()->GetPosition(), minRange, maxRange);
    }

    void warn(char const* text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }

    void armBerserk(uint32_t spellId, uint32_t delayMs)
    {
        mBerserkSpell = spellId;
        mBerserkLeftMs = delayMs;
    }

    // a new stage drops every pending event
    void enterStage(uint32_t stage)
    {
        mStage = stage;
        scriptEvents.resetEvents();
    }

    // out of reach: no movement, no melee, not selectable
    void setDormant(bool dormant)
    {
        setRooted(dormant);
        _setMeleeDisabled(dormant);

        if (dormant)
            getCreature()->addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        else
            getCreature()->removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
    }

    uint32_t mStage = 0;

private:
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

    uint32_t mBerserkSpell = 0;
    uint32_t mBerserkLeftMs = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame, boss state, group pull and shared health

class TombOfSargerasBossAI : public TombOfSargerasCombatAI
{
public:
    TombOfSargerasBossAI(Creature* pCreature, uint32_t dataId) : TombOfSargerasCombatAI(pCreature), mDataId(dataId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mDataId, EncounterStates::InProgress);
        }

        if (TombOfSargerasInstanceScript* pTomb = getTomb())
            pTomb->rallyGroup(mDataId, getCreature());

        TombOfSargerasCombatAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        if (getCreature()->isAlive())
        {
            if (InstanceScript* pInstance = getInstanceScript())
            {
                pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
                pInstance->setBossState(mDataId, EncounterStates::Failed);
            }
        }

        TombOfSargerasCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* pKiller) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

            TombOfSargerasInstanceScript* pTomb = getTomb();
            if (pTomb == nullptr || pTomb->isGroupDown(mDataId, getCreature()))
                pInstance->setBossState(mDataId, EncounterStates::Performed);
        }

        scriptEvents.resetEvents();
        summons.despawnAll();
        onDefeat(pKiller);
    }

    void DamageTaken(Unit* pAttacker, uint32_t* damage) override
    {
        if (damage == nullptr)
            return;

        if (TombOfSargerasInstanceScript* pTomb = getTomb())
            pTomb->shareDamage(mDataId, getCreature(), pAttacker, *damage);
    }

protected:
    virtual void onDefeat(Unit* /*pKiller*/) {}

    uint32_t mDataId;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Helpers outside the fight

// immobile helper that never fights
class TombOfSargerasPassiveAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TombOfSargerasPassiveAI(c); }
    explicit TombOfSargerasPassiveAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        setCanEnterCombat(false);
        setRooted(true);
        getCreature()->addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
    }

protected:
    // counts down once, only summoned helpers act
    bool countdown(unsigned long time_passed, uint32_t& leftMs)
    {
        if (!getCreature()->isSummon() || leftMs == 0)
            return false;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (leftMs > diff)
        {
            leftMs -= diff;
            return false;
        }

        leftMs = 0;
        return true;
    }
};

// releases one spell after a delay
class TombOfSargerasDelayedCastAI : public TombOfSargerasPassiveAI
{
public:
    TombOfSargerasDelayedCastAI(Creature* pCreature, uint32_t spellId, uint32_t delayMs, bool triggered)
        : TombOfSargerasPassiveAI(pCreature), mSpellId(spellId), mLeftMs(delayMs), mTriggered(triggered) {}

    void AIUpdate(unsigned long time_passed) override
    {
        if (countdown(time_passed, mLeftMs))
            castSpellAOE(mSpellId, mTriggered);
    }

private:
    uint32_t mSpellId;
    uint32_t mLeftMs;
    bool mTriggered;
};

class TombOfSargerasJellyfishAI : public TombOfSargerasDelayedCastAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TombOfSargerasJellyfishAI(c); }
    explicit TombOfSargerasJellyfishAI(Creature* pCreature) : TombOfSargerasDelayedCastAI(pCreature, SPELL_JELLYFISH_THUNDERING_SHOCK, 500, false) {}
};

class TombOfSargerasPiranhadoAI : public TombOfSargerasDelayedCastAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TombOfSargerasPiranhadoAI(c); }
    explicit TombOfSargerasPiranhadoAI(Creature* pCreature) : TombOfSargerasDelayedCastAI(pCreature, SPELL_PIRANHADO_CRASHING_WAVE, 500, false) {}
};

class TombOfSargerasNetherRiftAI : public TombOfSargerasDelayedCastAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TombOfSargerasNetherRiftAI(c); }
    explicit TombOfSargerasNetherRiftAI(Creature* pCreature) : TombOfSargerasDelayedCastAI(pCreature, SPELL_NETHER_RIFT_VISUAL, 5000, true) {}
};

// rain when someone stands in it, a platform wide blast otherwise
class TombOfSargerasArmageddonStalkerAI : public TombOfSargerasPassiveAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TombOfSargerasArmageddonStalkerAI(c); }
    explicit TombOfSargerasArmageddonStalkerAI(Creature* pCreature) : TombOfSargerasPassiveAI(pCreature) {}

    void AIUpdate(unsigned long time_passed) override
    {
        if (!countdown(time_passed, mLeftMs))
            return;

        Player* pSoaker = getNearestPlayer();
        bool const soaked = pSoaker != nullptr && pSoaker->isAlive() && getRangeToObject(pSoaker) <= 4.0f;
        castSpellAOE(soaked ? SPELL_ARMAGEDDON_RAIN : SPELL_ARMAGEDDON_BLAST, true);
    }

private:
    uint32_t mLeftMs = 7000;
};

// charges up, then blasts and crumbles
class TombOfSargerasDemonicObeliskAI : public TombOfSargerasPassiveAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TombOfSargerasDemonicObeliskAI(c); }
    explicit TombOfSargerasDemonicObeliskAI(Creature* pCreature) : TombOfSargerasPassiveAI(pCreature) {}

    void AIUpdate(unsigned long time_passed) override
    {
        if (countdown(time_passed, mChargeLeftMs))
            castSpellOnSelf(SPELL_OBELISK_CHARGE, true);

        if (countdown(time_passed, mBlastLeftMs))
        {
            castSpellAOE(SPELL_OBELISK_BLAST, true);
            despawn(500, 0);
        }
    }

private:
    uint32_t mChargeLeftMs = 9000;
    uint32_t mBlastLeftMs = 14000;
};

// encounter anchor: places missing members once after loading
class TombOfSargerasAnchorAI : public TombOfSargerasPassiveAI
{
public:
    TombOfSargerasAnchorAI(Creature* pCreature, uint32_t dataId, TombPlacement const* placements, size_t count)
        : TombOfSargerasPassiveAI(pCreature), mDataId(dataId), mPlacements(placements), mCount(count) {}

    void AIUpdate(unsigned long time_passed) override
    {
        if (mPlaced)
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (mWaitMs > diff)
        {
            mWaitMs -= diff;
            return;
        }

        mPlaced = true;

        InstanceScript* pInstance = getInstanceScript();
        if (pInstance == nullptr || pInstance->getBossState(mDataId) == EncounterStates::Performed)
            return;

        for (size_t i = 0; i < mCount; ++i)
        {
            if (pInstance->getCreatureSetForEntry(mPlacements[i].entry).empty())
                spawnCreature(mPlacements[i].entry, mPlacements[i].spot);
        }
    }

private:
    uint32_t mDataId;
    TombPlacement const* mPlacements;
    size_t mCount;
    uint32_t mWaitMs = 5000;
    bool mPlaced = false;
};

class TombOfSargerasMoonStalkerAI : public TombOfSargerasAnchorAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TombOfSargerasMoonStalkerAI(c); }
    explicit TombOfSargerasMoonStalkerAI(Creature* pCreature) : TombOfSargerasAnchorAI(pCreature, DATA_SISTERS_OF_THE_MOON, SistersPlacements, 3) {}
};

class TombOfSargerasSwirlingSoulsAI : public TombOfSargerasAnchorAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TombOfSargerasSwirlingSoulsAI(c); }
    explicit TombOfSargerasSwirlingSoulsAI(Creature* pCreature) : TombOfSargerasAnchorAI(pCreature, DATA_THE_DESOLATE_HOST, HostPlacements, 3) {}
};

// add that fires a spell at its victim instead of meleeing
class TombOfSargerasSpellSlingerAI : public TombOfSargerasCombatAI
{
public:
    TombOfSargerasSpellSlingerAI(Creature* pCreature, uint32_t spellId, uint32_t intervalMs)
        : TombOfSargerasCombatAI(pCreature), mSpellId(spellId), mIntervalMs(intervalMs) {}

protected:
    void onEngage() override
    {
        _setMeleeDisabled(true);
        scriptEvents.addEvent(EVENT_TOMB_SPELL_VOLLEY, 1000);
    }

    void onDisengage() override { _setMeleeDisabled(false); }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_TOMB_SPELL_VOLLEY)
            return;

        castSpellOnVictim(mSpellId);
        scriptEvents.addEvent(EVENT_TOMB_SPELL_VOLLEY, mIntervalMs);
    }

private:
    uint32_t mSpellId;
    uint32_t mIntervalMs;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Goroth

class TombOfSargerasGorothAI : public TombOfSargerasBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TombOfSargerasGorothAI(c); }
    explicit TombOfSargerasGorothAI(Creature* pCreature) : TombOfSargerasBossAI(pCreature, DATA_GOROTH) {}

protected:
    void onEngage() override
    {
        mStars.begin(GorothStarSteps);
        scriptEvents.addEvent(EVENT_GOROTH_INFERNAL_SPIKES, 4000);
        scriptEvents.addEvent(EVENT_GOROTH_CRASHING_COMET, 8000);
        scriptEvents.addEvent(EVENT_GOROTH_BURNING_ARMOR, 10000);
        scriptEvents.addEvent(EVENT_GOROTH_SHATTERING_STAR, mStars.next());
        scriptEvents.addEvent(EVENT_GOROTH_INFERNAL_BURNING, GorothEnergyFillMs);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_GOROTH_BURNING_ARMOR:
                castSpellOnVictim(SPELL_GOROTH_BURNING_ARMOR);
                scriptEvents.addEvent(EVENT_GOROTH_BURNING_ARMOR, 24000);
                break;
            case EVENT_GOROTH_CRASHING_COMET:
                castAtPlayer(SPELL_GOROTH_CRASHING_COMET, true, true);
                scriptEvents.addEvent(EVENT_GOROTH_CRASHING_COMET, 18000);
                break;
            case EVENT_GOROTH_INFERNAL_SPIKES:
                raiseSpikes();
                scriptEvents.addEvent(EVENT_GOROTH_INFERNAL_SPIKES, 16000);
                break;
            case EVENT_GOROTH_SHATTERING_STAR:
                if (castAtPlayer(SPELL_GOROTH_SHATTERING_STAR, true) != nullptr)
                    warn("Goroth marks a target with Shattering Star!");
                scriptEvents.addEvent(EVENT_GOROTH_SHATTERING_STAR, mStars.next());
                break;
            case EVENT_GOROTH_INFERNAL_BURNING:
                warn("Goroth begins to cast Infernal Burning! Hide behind an Infernal Spike!");
                castSpellAOE(SPELL_GOROTH_INFERNAL_BURNING);
                scriptEvents.addEvent(EVENT_GOROTH_SPIKES_SHATTER, 100);
                break;
            case EVENT_GOROTH_SPIKES_SHATTER:
                // the blast breaks every spike and the energy starts over
                summons.despawnEntry(NPC_GOROTH_INFERNAL_SPIKE);
                scriptEvents.addEvent(EVENT_GOROTH_INFERNAL_BURNING, GorothEnergyFillMs);
                break;
            default:
                break;
        }
    }

private:
    void raiseSpikes()
    {
        for (uint8_t i = 0; i < GorothSpikesPerVolley; ++i)
        {
            if (Unit* pTarget = pickPlayer(true))
                placeHelper(NPC_GOROTH_INFERNAL_SPIKE, pTarget->GetPosition(), 0);
        }
    }

    TombCooldowns mStars;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Harjatan

class TombOfSargerasHarjatanAI : public TombOfSargerasBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TombOfSargerasHarjatanAI(c); }
    explicit TombOfSargerasHarjatanAI(Creature* pCreature) : TombOfSargerasBossAI(pCreature, DATA_HARJATAN) {}

protected:
    void onEngage() override
    {
        mAwaitingDischarge = false;
        armBerserk(SPELL_HARJATAN_BERSERK, 480000);
        scriptEvents.addEvent(EVENT_HARJATAN_UNCHECKED_RAGE, HarjatanRageFillMs);
        scriptEvents.addEvent(EVENT_HARJATAN_COMMANDING_ROAR, 17000);
        scriptEvents.addEvent(EVENT_HARJATAN_DRAW_IN, 58000);
    }

    void onCombatTick(uint32_t diff) override
    {
        if (!mAwaitingDischarge)
            return;

        // frost charges are spent or run out
        if (mDischargeLeftMs > diff && hasAura(SPELL_HARJATAN_FRIGID_BLOWS))
        {
            mDischargeLeftMs -= diff;
            return;
        }

        mAwaitingDischarge = false;
        castSpellAOE(SPELL_HARJATAN_FROSTY_DISCHARGE, true);
        scriptEvents.addEvent(EVENT_HARJATAN_UNCHECKED_RAGE, HarjatanRageFillMs);
        scriptEvents.addEvent(EVENT_HARJATAN_COMMANDING_ROAR, 17000);
        scriptEvents.addEvent(EVENT_HARJATAN_DRAW_IN, 59000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_HARJATAN_UNCHECKED_RAGE:
                castSpellOnVictim(SPELL_HARJATAN_UNCHECKED_RAGE);
                scriptEvents.addEvent(EVENT_HARJATAN_UNCHECKED_RAGE, HarjatanRageFillMs);
                break;
            case EVENT_HARJATAN_COMMANDING_ROAR:
                warn("Harjatan calls Razorjaw reinforcements!");
                castSpellAOE(SPELL_HARJATAN_COMMANDING_ROAR, true);
                for (uint8_t i = 0; i < 4; ++i)
                    callAdd(i < 2 ? NPC_RAZORJAW_GLADIATOR : NPC_RAZORJAW_WAVEMENDER, HarjatanAddSpots[i]);
                scriptEvents.addEvent(EVENT_HARJATAN_COMMANDING_ROAR, 36000);
                break;
            case EVENT_HARJATAN_DRAW_IN:
                warn("Harjatan begins to Draw In!");
                castSpellAOE(SPELL_HARJATAN_DRAW_IN);
                scriptEvents.addEvent(EVENT_HARJATAN_DRAW_IN_ENDS, 100);
                break;
            case EVENT_HARJATAN_DRAW_IN_ENDS:
                castSpellOnSelf(SPELL_HARJATAN_FRIGID_BLOWS, true);
                mAwaitingDischarge = true;
                mDischargeLeftMs = 20000;
                break;
            default:
                break;
        }
    }

private:
    bool mAwaitingDischarge = false;
    uint32_t mDischargeLeftMs = 0;
};

class TombOfSargerasGladiatorAI : public TombOfSargerasCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TombOfSargerasGladiatorAI(c); }
    explicit TombOfSargerasGladiatorAI(Creature* pCreature) : TombOfSargerasCombatAI(pCreature) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_GLADIATOR_DRIVEN_ASSAULT, 1000);
        scriptEvents.addEvent(EVENT_GLADIATOR_SPLASHY_CLEAVE, 6000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_GLADIATOR_DRIVEN_ASSAULT:
                castAtPlayer(SPELL_GLADIATOR_DRIVEN_ASSAULT, true, true);
                scriptEvents.addEvent(EVENT_GLADIATOR_DRIVEN_ASSAULT, 20000);
                break;
            case EVENT_GLADIATOR_SPLASHY_CLEAVE:
                castSpellOnVictim(SPELL_GLADIATOR_SPLASHY_CLEAVE);
                scriptEvents.addEvent(EVENT_GLADIATOR_SPLASHY_CLEAVE, 10000);
                break;
            default:
                break;
        }
    }
};

class TombOfSargerasWavemenderAI : public TombOfSargerasCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TombOfSargerasWavemenderAI(c); }
    explicit TombOfSargerasWavemenderAI(Creature* pCreature) : TombOfSargerasCombatAI(pCreature) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_WAVEMENDER_AQUEOUS_BURST, 4000);
    }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_WAVEMENDER_AQUEOUS_BURST)
            return;

        castAtPlayer(SPELL_WAVEMENDER_AQUEOUS_BURST);
        scriptEvents.addEvent(EVENT_WAVEMENDER_AQUEOUS_BURST, 8000);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Mistress Sassz'ine

class TombOfSargerasSasszineAI : public TombOfSargerasBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TombOfSargerasSasszineAI(c); }
    explicit TombOfSargerasSasszineAI(Creature* pCreature) : TombOfSargerasBossAI(pCreature, DATA_MISTRESS_SASSZINE) {}

protected:
    void onEngage() override
    {
        armBerserk(SPELL_SASSZINE_BERSERK, 480000);

        enterStage(1);
        mShock.begin(SasszineShockSteps);
        mHunger.begin(SasszineHungerFirstSteps);
        scriptEvents.addEvent(EVENT_SASSZINE_THUNDERING_SHOCK, mShock.next());
        scriptEvents.addEvent(EVENT_SASSZINE_BURDEN_OF_PAIN, 19000);
        scriptEvents.addEvent(EVENT_SASSZINE_HYDRA_SHOT, 26000);
        scriptEvents.addEvent(EVENT_SASSZINE_CONSUMING_HUNGER, mHunger.next());
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mStage == 1 && _getHealthPercent() <= 70)
        {
            enterStage(2);
            warn("Mistress Sassz'ine calls the horrors of the deep!");
            mOssunet.begin(SasszineOssunetSecondSteps);
            mVellius.begin(SasszineVelliusSecondSteps);
            scriptEvents.addEvent(EVENT_SASSZINE_SUMMON_OSSUNET, mOssunet.next());
            scriptEvents.addEvent(EVENT_SASSZINE_HYDRA_SHOT, 18000);
            scriptEvents.addEvent(EVENT_SASSZINE_BURDEN_OF_PAIN, 28000);
            scriptEvents.addEvent(EVENT_SASSZINE_CALL_VELLIUS, mVellius.next());
            scriptEvents.addEvent(EVENT_SASSZINE_BECKON_SARUKEL, 42000);
        }
        else if (mStage == 2 && _getHealthPercent() <= 40)
        {
            enterStage(3);
            warn("Mistress Sassz'ine rises from the depths with all her might!");
            mOssunet.begin(SasszineOssunetThirdSteps);
            mHunger.begin(SasszineHungerThirdSteps);
            mVellius.begin(SasszineVelliusThirdSteps);
            scriptEvents.addEvent(EVENT_SASSZINE_SUMMON_OSSUNET, mOssunet.next());
            scriptEvents.addEvent(EVENT_SASSZINE_HYDRA_SHOT, 18000);
            scriptEvents.addEvent(EVENT_SASSZINE_BURDEN_OF_PAIN, 28000);
            scriptEvents.addEvent(EVENT_SASSZINE_CALL_VELLIUS, mVellius.next());
            scriptEvents.addEvent(EVENT_SASSZINE_CONSUMING_HUNGER, mHunger.next());
        }
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SASSZINE_BURDEN_OF_PAIN:
                castSpellOnVictim(SPELL_SASSZINE_BURDEN_OF_PAIN);
                scriptEvents.addEvent(EVENT_SASSZINE_ABYSS_CALL, 100);
                scriptEvents.addEvent(EVENT_SASSZINE_BURDEN_OF_PAIN, mStage == 1 ? 28000 : 40000);
                break;
            case EVENT_SASSZINE_ABYSS_CALL:
                for (uint8_t i = 0; i < SasszineStalkersPerBurden; ++i)
                    callAdd(NPC_SASSZINE_ABYSS_STALKER, aroundSelf(5.0f, 10.0f));
                break;
            case EVENT_SASSZINE_HYDRA_SHOT:
                if (Unit* pTarget = castAtPlayer(SPELL_SASSZINE_HYDRA_SHOT_MARK, true, true))
                {
                    mHydraTarget = pTarget->GetNewGUID();
                    warn("Mistress Sassz'ine takes aim with Hydra Shot!");
                    scriptEvents.addEvent(EVENT_SASSZINE_HYDRA_IMPACT, 6000);
                }
                scriptEvents.addEvent(EVENT_SASSZINE_HYDRA_SHOT, 40000);
                break;
            case EVENT_SASSZINE_HYDRA_IMPACT:
            {
                Unit* pTarget = findUnit(mHydraTarget);
                if (pTarget != nullptr && pTarget->isAlive())
                    castSpell(pTarget, SPELL_SASSZINE_HYDRA_SHOT, true);
                break;
            }
            case EVENT_SASSZINE_THUNDERING_SHOCK:
                if (Unit* pTarget = pickPlayer())
                    placeHelper(NPC_SASSZINE_JELLYFISH, pTarget->GetPosition(), 8000);
                scriptEvents.addEvent(EVENT_SASSZINE_THUNDERING_SHOCK, mShock.next());
                break;
            case EVENT_SASSZINE_CONSUMING_HUNGER:
                if (Unit* pTarget = pickPlayer(true))
                    callAdd(NPC_SASSZINE_WAVERUNNER, pTarget->GetPosition());
                scriptEvents.addEvent(EVENT_SASSZINE_CONSUMING_HUNGER, mHunger.next());
                break;
            case EVENT_SASSZINE_BECKON_SARUKEL:
                warn("Mistress Sassz'ine beckons Sarukel!");
                castSpellAOE(SPELL_SASSZINE_BECKON_SARUKEL);
                scriptEvents.addEvent(EVENT_SASSZINE_BECKON_SARUKEL, 42000);
                break;
            case EVENT_SASSZINE_SUMMON_OSSUNET:
                castSpellAOE(SPELL_SASSZINE_SUMMON_OSSUNET, true);
                for (uint8_t i = 0; i < SasszineInkTargets; ++i)
                    castAtPlayer(SPELL_SASSZINE_BEFOULING_INK, false, true);
                scriptEvents.addEvent(EVENT_SASSZINE_SUMMON_OSSUNET, mOssunet.next());
                break;
            case EVENT_SASSZINE_CALL_VELLIUS:
                warn("Mistress Sassz'ine calls Vellius! A Crashing Wave approaches!");
                castSpellAOE(SPELL_SASSZINE_CALL_VELLIUS);
                placeHelper(NPC_SASSZINE_PIRANHADO, SasszinePiranhadoSpots[Util::getRandomUInt(7)], 10000);
                scriptEvents.addEvent(EVENT_SASSZINE_CALL_VELLIUS, mVellius.next());
                break;
            default:
                break;
        }
    }

private:
    TombCooldowns mShock;
    TombCooldowns mHunger;
    TombCooldowns mOssunet;
    TombCooldowns mVellius;
    WoWGuid mHydraTarget;
};

class TombOfSargerasWaverunnerAI : public TombOfSargerasSpellSlingerAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TombOfSargerasWaverunnerAI(c); }
    explicit TombOfSargerasWaverunnerAI(Creature* pCreature) : TombOfSargerasSpellSlingerAI(pCreature, SPELL_WAVERUNNER_WATER_BLAST, 2500) {}
};

//////////////////////////////////////////////////////////////////////////////////////////
// Fallen Avatar, stage one bound by the pylons, stage two awakened

class TombOfSargerasFallenAvatarAI : public TombOfSargerasBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TombOfSargerasFallenAvatarAI(c); }
    explicit TombOfSargerasFallenAvatarAI(Creature* pCreature) : TombOfSargerasBossAI(pCreature, DATA_FALLEN_AVATAR) {}

    // losing the Maiden of Valor wakes the Avatar early
    void OnSummonDies(Creature* pSummon, Unit* pKiller) override
    {
        if (pSummon->getEntry() == NPC_AVATAR_MAIDEN_OF_VALOR && pKiller != getCreature() && mStage == 1 && _isInCombat())
            awaken();
    }

protected:
    void onEngage() override
    {
        setRooted(true);
        mPylonGuids.clear();
        mLastPylon = 0;
        mValorGuid = WoWGuid();

        if (Creature* pValor = callAdd(NPC_AVATAR_MAIDEN_OF_VALOR, AvatarValorSpot, MANUAL_DESPAWN))
            mValorGuid = pValor->GetNewGUID();

        for (LocationVector const& spot : AvatarPylonSpots)
        {
            if (Creature* pPylon = placeHelper(NPC_AVATAR_CONTAINMENT_PYLON, spot, 0))
                mPylonGuids.push_back(pPylon->GetNewGUID());
        }

        mChaos.begin(AvatarChaosSteps);
        mBlades.begin(AvatarBladeSteps);
        mRupture.begin(AvatarRuptureSteps);
        scriptEvents.addEvent(EVENT_AVATAR_PYLON, 1000);
        scriptEvents.addEvent(EVENT_AVATAR_REACH_CHECK, 4500);
        scriptEvents.addEvent(EVENT_AVATAR_UNBOUND_CHAOS, mChaos.next());
        scriptEvents.addEvent(EVENT_AVATAR_DESOLATE, 13000);
        scriptEvents.addEvent(EVENT_AVATAR_SHADOWY_BLADES, mBlades.next());
        scriptEvents.addEvent(EVENT_AVATAR_RUPTURE_REALITIES, mRupture.next());
        scriptEvents.addEvent(EVENT_AVATAR_FULL_ENERGY, AvatarDormantEnergyFillMs);
    }

    void onDisengage() override { setRooted(false); }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mStage == 1 && _getHealthPercent() <= 20)
            awaken();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_AVATAR_PYLON:
                castSpellAOE(SPELL_AVATAR_ACTIVATE_PYLONS, true);
                beamFromNextPylon();
                scriptEvents.addEvent(EVENT_AVATAR_PYLON, 50000);
                break;
            case EVENT_AVATAR_REACH_CHECK:
            {
                // nobody in reach: the Avatar lashes out at the whole room
                Unit* pVictim = getCreature()->getAIInterface()->getCurrentTarget();
                if (pVictim != nullptr && getRangeToObject(pVictim) > 10.0f)
                    castSpellAOE(SPELL_AVATAR_RIPPLE_OF_DARKNESS);
                scriptEvents.addEvent(EVENT_AVATAR_REACH_CHECK, Util::getRandomUInt(4000, 5000));
                break;
            }
            case EVENT_AVATAR_UNBOUND_CHAOS:
                castSpellAOE(SPELL_AVATAR_UNBOUND_CHAOS);
                scriptEvents.addEvent(EVENT_AVATAR_UNBOUND_CHAOS, mChaos.next());
                break;
            case EVENT_AVATAR_DESOLATE:
                castSpellOnVictim(SPELL_AVATAR_DESOLATE);
                scriptEvents.addEvent(EVENT_AVATAR_DESOLATE, 13000);
                break;
            case EVENT_AVATAR_SHADOWY_BLADES:
                warn("The Fallen Avatar calls forth Shadowy Blades!");
                castSpellAOE(SPELL_AVATAR_SHADOWY_BLADES, true);
                for (LocationVector const& spot : AvatarBladeSpots)
                {
                    Unit* pTarget = pickPlayer();
                    Creature* pBlade = placeHelper(NPC_AVATAR_CORRUPTED_BLADE, spot, 11000);
                    if (pTarget != nullptr && pBlade != nullptr)
                        pBlade->castSpell(pTarget, SPELL_BLADE_SHADOWY_BLADES, false);
                }
                scriptEvents.addEvent(EVENT_AVATAR_SHADOWY_BLADES, mBlades.next());
                break;
            case EVENT_AVATAR_RUPTURE_REALITIES:
                warn("The Fallen Avatar begins to cast Rupture Realities!");
                castSpellAOE(SPELL_AVATAR_RUPTURE_REALITIES);
                scriptEvents.addEvent(EVENT_AVATAR_RUPTURE_REALITIES, mRupture.next());
                break;
            case EVENT_AVATAR_FULL_ENERGY:
                awaken();
                break;
            case EVENT_AVATAR_CONSUMED:
                consumeValor();
                castSpellAOE(SPELL_AVATAR_ANNIHILATION);
                scriptEvents.addEvent(EVENT_AVATAR_ANNIHILATED, 100);
                break;
            case EVENT_AVATAR_ANNIHILATED:
                setRooted(false);
                scriptEvents.addEvent(EVENT_AVATAR_SEAR, 2000);
                scriptEvents.addEvent(EVENT_AVATAR_DESOLATE, 19000);
                scriptEvents.addEvent(EVENT_AVATAR_DARK_MARK, 21000);
                scriptEvents.addEvent(EVENT_AVATAR_RUPTURE_AWAKENED, AvatarAwakenedEnergyFillMs);
                break;
            case EVENT_AVATAR_SEAR:
                castSpellOnSelf(SPELL_AVATAR_SEAR, true);
                break;
            case EVENT_AVATAR_DARK_MARK:
                castAtPlayer(SPELL_AVATAR_DARK_MARK, true);
                scriptEvents.addEvent(EVENT_AVATAR_DARK_MARK, 34000);
                break;
            case EVENT_AVATAR_RUPTURE_AWAKENED:
                warn("The Fallen Avatar begins to cast Rupture Realities!");
                castSpellAOE(SPELL_AVATAR_RUPTURE_AWAKENED);
                scriptEvents.addEvent(EVENT_AVATAR_RUPTURE_AWAKENED, AvatarAwakenedEnergyFillMs);
                break;
            default:
                break;
        }
    }

private:
    // full energy, the Maiden's death or low health end the first stage
    void awaken()
    {
        if (mStage != 1)
            return;

        enterStage(2);
        getCreature()->interruptSpell();
        warn("The Fallen Avatar consumes the Maiden of Valor and awakens!");
        castSpellOnSelf(SPELL_AVATAR_CLEAN_UP, true);
        castSpellAOE(SPELL_AVATAR_CONSUME);
        scriptEvents.addEvent(EVENT_AVATAR_CONSUMED, 100);
    }

    // the Maiden's remaining health heals the Avatar
    void consumeValor()
    {
        Creature* pValor = findCreature(mValorGuid);
        if (pValor != nullptr && pValor->isAlive())
        {
            uint32_t const maxHealth = getCreature()->getMaxHealth();
            uint32_t const healed = getCreature()->getHealth() + pValor->getHealth();
            getCreature()->setHealth(std::min(maxHealth, healed));
            getCreature()->dealDamage(pValor, pValor->getHealth(), 0);
        }

        summons.despawnEntry(NPC_AVATAR_CONTAINMENT_PYLON);
        mPylonGuids.clear();
    }

    // one pylon at a time, never the same twice in a row
    void beamFromNextPylon()
    {
        if (mPylonGuids.empty())
            return;

        size_t const count = mPylonGuids.size();
        size_t const pick = (mLastPylon + 1 + Util::getRandomUInt(static_cast<uint32_t>(count > 1 ? count - 2 : 0))) % count;
        mLastPylon = pick;

        if (Creature* pPylon = findCreature(mPylonGuids[pick]))
        {
            pPylon->castSpell(pPylon, SPELL_PYLON_CORRUPTED_MATRIX, true);
            pPylon->castSpell(getCreature(), SPELL_PYLON_BEAM, true);
        }
    }

    TombCooldowns mChaos;
    TombCooldowns mBlades;
    TombCooldowns mRupture;
    std::vector<WoWGuid> mPylonGuids;
    size_t mLastPylon = 0;
    WoWGuid mValorGuid;
};

// Maiden of Valor: shield up while cleansing, a broken shield costs her a quarter of her health
class TombOfSargerasMaidenOfValorAI : public TombOfSargerasCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TombOfSargerasMaidenOfValorAI(c); }
    explicit TombOfSargerasMaidenOfValorAI(Creature* pCreature) : TombOfSargerasCombatAI(pCreature) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());

        TombOfSargerasCombatAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

        TombOfSargerasCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
    }

protected:
    void onEngage() override
    {
        mWatchingShield = false;
        scriptEvents.addEvent(EVENT_VALOR_CLEANSING_PROTOCOL, ValorEnergyFillMs);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (!mWatchingShield)
            return;

        // protocol finished with the shield intact
        if (!_isCasting())
        {
            mWatchingShield = false;
            return;
        }

        if (hasAura(SPELL_VALOR_CLEANSING_SHIELD))
        {
            mShieldSeen = true;
            return;
        }

        if (!mShieldSeen)
            return;

        mWatchingShield = false;
        getCreature()->interruptSpell();
        castSpellOnSelf(SPELL_VALOR_MALFUNCTION, true);

        uint32_t const loss = getCreature()->getMaxHealth() / 4;
        uint32_t const health = getCreature()->getHealth();
        getCreature()->setHealth(health > loss ? health - loss : 1);
    }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_VALOR_CLEANSING_PROTOCOL)
            return;

        warn("The Maiden of Valor begins Cleansing Protocol! Break her shield!");
        castSpellOnSelf(SPELL_VALOR_CLEANSING_SHIELD, true);
        castSpellAOE(SPELL_VALOR_CLEANSING_PROTOCOL);
        mWatchingShield = true;
        mShieldSeen = false;
        scriptEvents.addEvent(EVENT_VALOR_CLEANSING_PROTOCOL, ValorEnergyFillMs);
    }

private:
    bool mWatchingShield = false;
    bool mShieldSeen = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Demonic Inquisition

class TombOfSargerasAtriganAI : public TombOfSargerasBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TombOfSargerasAtriganAI(c); }
    explicit TombOfSargerasAtriganAI(Creature* pCreature) : TombOfSargerasBossAI(pCreature, DATA_DEMONIC_INQUISITION) {}

protected:
    void onEngage() override
    {
        armBerserk(SPELL_INQUISITION_UNBRIDLED_TORMENT, 720000);
        scriptEvents.addEvent(EVENT_ATRIGAN_TORMENTED_SOUL, 1000);
        scriptEvents.addEvent(EVENT_ATRIGAN_SCYTHE_SWEEP, 5200);
        scriptEvents.addEvent(EVENT_ATRIGAN_BONE_SAW, InquisitionEnergyFillMs);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ATRIGAN_TORMENTED_SOUL:
                callAdd(NPC_INQUISITION_TORMENTED_SOUL, InquisitionSoulSpot);
                break;
            case EVENT_ATRIGAN_SCYTHE_SWEEP:
                castSpellAOE(SPELL_ATRIGAN_SCYTHE_SWEEP);
                scriptEvents.addEvent(EVENT_ATRIGAN_SCYTHE_SWEEP, 24500);
                break;
            case EVENT_ATRIGAN_BONE_SAW:
                warn("Atrigan begins to cast Bone Saw!");
                castSpellAOE(SPELL_ATRIGAN_BONE_SAW);
                scriptEvents.addEvent(EVENT_ATRIGAN_BONE_SAW, InquisitionEnergyFillMs);
                break;
            default:
                break;
        }
    }
};

class TombOfSargerasBelacAI : public TombOfSargerasBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TombOfSargerasBelacAI(c); }
    explicit TombOfSargerasBelacAI(Creature* pCreature) : TombOfSargerasBossAI(pCreature, DATA_DEMONIC_INQUISITION) {}

protected:
    void onEngage() override
    {
        // a stationary caster
        setRooted(true);
        _setMeleeDisabled(true);
        armBerserk(SPELL_INQUISITION_UNBRIDLED_TORMENT, 720000);
        scriptEvents.addEvent(EVENT_BELAC_PANGS_OF_GUILT, 100);
        scriptEvents.addEvent(EVENT_BELAC_ECHOING_ANGUISH, 5000);
        scriptEvents.addEvent(EVENT_BELAC_TORMENTING_BURST, 9000);
        scriptEvents.addEvent(EVENT_BELAC_FEL_SQUALL, InquisitionEnergyFillMs);
    }

    void onDisengage() override
    {
        setRooted(false);
        _setMeleeDisabled(false);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_BELAC_PANGS_OF_GUILT:
                castSpellOnVictim(SPELL_BELAC_PANGS_OF_GUILT);
                scriptEvents.addEvent(EVENT_BELAC_PANGS_OF_GUILT, 10000);
                break;
            case EVENT_BELAC_ECHOING_ANGUISH:
                castAtPlayer(SPELL_BELAC_ECHOING_ANGUISH, true);
                scriptEvents.addEvent(EVENT_BELAC_ECHOING_ANGUISH, 22000);
                break;
            case EVENT_BELAC_TORMENTING_BURST:
                castSpellAOE(SPELL_BELAC_TORMENTING_BURST);
                scriptEvents.addEvent(EVENT_BELAC_TORMENTING_BURST, 17000);
                break;
            case EVENT_BELAC_FEL_SQUALL:
                warn("Belac begins to cast Fel Squall!");
                castSpellAOE(SPELL_BELAC_FEL_SQUALL);
                scriptEvents.addEvent(EVENT_BELAC_FEL_SQUALL, InquisitionEnergyFillMs);
                break;
            default:
                break;
        }
    }
};

class TombOfSargerasTormentedSoulAI : public TombOfSargerasCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TombOfSargerasTormentedSoulAI(c); }
    explicit TombOfSargerasTormentedSoulAI(Creature* pCreature) : TombOfSargerasCombatAI(pCreature) {}

protected:
    void onEngage() override
    {
        setRooted(true);
        scriptEvents.addEvent(EVENT_TORMENTED_SOUL_ERUPTION, 5000);
        scriptEvents.addEvent(EVENT_TORMENTED_SOUL_CORRUPTION, 6000);
    }

    void onDisengage() override { setRooted(false); }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_TORMENTED_SOUL_CORRUPTION:
                castSpellOnVictim(SPELL_TORMENTED_SOUL_CORRUPTION);
                scriptEvents.addEvent(EVENT_TORMENTED_SOUL_CORRUPTION, 6000);
                break;
            case EVENT_TORMENTED_SOUL_ERUPTION:
                castSpellAOE(SPELL_TORMENTED_SOUL_ERUPTION);
                scriptEvents.addEvent(EVENT_TORMENTED_SOUL_ERUPTION, Util::getRandomUInt(16000, 19000));
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Sisters of the Moon: one sister fights, the other two act as ghosts

class TombOfSargerasSisterAI : public TombOfSargerasBossAI
{
public:
    explicit TombOfSargerasSisterAI(Creature* pCreature) : TombOfSargerasBossAI(pCreature, DATA_SISTERS_OF_THE_MOON) {}

protected:
    void onEngage() override
    {
        mMoonLeftMs = SistersFirstMoonMs;
        shiftStage(1);
    }

    void onDisengage() override { setDormant(false); }

    void onCombatTick(uint32_t diff) override
    {
        // shared health: every sister sees the same thresholds
        if (mStage == 1 && _getHealthPercent() <= 70)
            shiftStage(2);
        else if (mStage == 2 && _getHealthPercent() <= 40)
            shiftStage(3);

        if (mMoonLeftMs > diff)
        {
            mMoonLeftMs -= diff;
            return;
        }

        mMoonLeftMs = SistersMoonCycleMs;
        onMoonTurn();
    }

    virtual bool leadsStage(uint32_t stage) const = 0;
    virtual void scheduleStage(uint32_t stage) = 0;
    virtual void onMoonTurn() {}

private:
    void shiftStage(uint32_t stage)
    {
        enterStage(stage);
        setDormant(!leadsStage(stage));
        scheduleStage(stage);
    }

    uint32_t mMoonLeftMs = 0;
};

class TombOfSargerasKasparianAI : public TombOfSargerasSisterAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TombOfSargerasKasparianAI(c); }
    explicit TombOfSargerasKasparianAI(Creature* pCreature) : TombOfSargerasSisterAI(pCreature) {}

protected:
    bool leadsStage(uint32_t stage) const override { return stage == 1; }

    void scheduleStage(uint32_t stage) override
    {
        if (stage == 1)
        {
            scriptEvents.addEvent(EVENT_KASPARIAN_MOON_GLAIVE, 14000);
            scriptEvents.addEvent(EVENT_KASPARIAN_TWILIGHT_GLAIVE, 20000);
            return;
        }

        scriptEvents.addEvent(EVENT_KASPARIAN_SPECTRAL_GLAIVE, 5000);
        scriptEvents.addEvent(EVENT_KASPARIAN_TWILIGHT_GLAIVE, 8000);
    }

    void onMoonTurn() override
    {
        if (mStage != 3)
            return;

        warn("Huntress Kasparian unleashes a Glaive Storm!");
        castSpellAOE(SPELL_KASPARIAN_GLAIVE_STORM, true);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_KASPARIAN_MOON_GLAIVE:
                castSpellOnVictim(SPELL_KASPARIAN_MOON_GLAIVE);
                scriptEvents.addEvent(EVENT_KASPARIAN_MOON_GLAIVE, 18000);
                break;
            case EVENT_KASPARIAN_TWILIGHT_GLAIVE:
                castAtPlayer(SPELL_KASPARIAN_TWILIGHT_GLAIVE, true);
                scriptEvents.addEvent(EVENT_KASPARIAN_TWILIGHT_GLAIVE, 18000);
                break;
            case EVENT_KASPARIAN_SPECTRAL_GLAIVE:
                castAtPlayer(SPELL_KASPARIAN_SPECTRAL_GLAIVE);
                scriptEvents.addEvent(EVENT_KASPARIAN_SPECTRAL_GLAIVE, 5000);
                break;
            default:
                break;
        }
    }
};

class TombOfSargerasYathaeAI : public TombOfSargerasSisterAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TombOfSargerasYathaeAI(c); }
    explicit TombOfSargerasYathaeAI(Creature* pCreature) : TombOfSargerasSisterAI(pCreature) {}

protected:
    bool leadsStage(uint32_t stage) const override { return stage == 2; }

    void scheduleStage(uint32_t stage) override
    {
        switch (stage)
        {
            case 1:
                scriptEvents.addEvent(EVENT_YATHAE_SHADOW_SHOT, 5000);
                scriptEvents.addEvent(EVENT_YATHAE_TWILIGHT_VOLLEY, 16000);
                break;
            case 2:
                scriptEvents.addEvent(EVENT_YATHAE_CALL_MOONTALON, 6000);
                scriptEvents.addEvent(EVENT_YATHAE_TWILIGHT_VOLLEY, 12000);
                scriptEvents.addEvent(EVENT_YATHAE_RAPID_SHOT, 18000);
                break;
            default:
                scriptEvents.addEvent(EVENT_YATHAE_SHADOW_SHOT, 5000);
                scriptEvents.addEvent(EVENT_YATHAE_TWILIGHT_VOLLEY, 28000);
                break;
        }
    }

    void onMoonTurn() override
    {
        if (mStage != 1)
            return;

        if (Unit* pTarget = castAtPlayer(SPELL_YATHAE_INCORPOREAL_SHOT_MARK, true, true))
        {
            mShotTarget = pTarget->GetNewGUID();
            warn("Captain Yathae Moonstrike takes aim with Incorporeal Shot!");
            scriptEvents.addEvent(EVENT_YATHAE_INCORPOREAL_RELEASE, 6000);
        }
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_YATHAE_TWILIGHT_VOLLEY:
                castAtPlayer(SPELL_YATHAE_TWILIGHT_VOLLEY, true);
                scriptEvents.addEvent(EVENT_YATHAE_TWILIGHT_VOLLEY, mStage == 2 ? 15000 : 20000);
                break;
            case EVENT_YATHAE_SHADOW_SHOT:
                castAtPlayer(SPELL_YATHAE_SHADOW_SHOT);
                scriptEvents.addEvent(EVENT_YATHAE_SHADOW_SHOT, 5500);
                break;
            case EVENT_YATHAE_INCORPOREAL_RELEASE:
            {
                Unit* pTarget = findUnit(mShotTarget);
                if (pTarget != nullptr && pTarget->isAlive())
                    castSpell(pTarget, SPELL_YATHAE_INCORPOREAL_SHOT, true);
                break;
            }
            case EVENT_YATHAE_CALL_MOONTALON:
                warn("Captain Yathae Moonstrike calls her Moontalon!");
                callAdd(NPC_SISTERS_MOONTALON, aroundSelf(3.0f, 6.0f));
                break;
            case EVENT_YATHAE_RAPID_SHOT:
                castAtPlayer(SPELL_YATHAE_RAPID_SHOT, true);
                scriptEvents.addEvent(EVENT_YATHAE_RAPID_SHOT, 18000);
                break;
            default:
                break;
        }
    }

private:
    WoWGuid mShotTarget;
};

class TombOfSargerasLunaspyreAI : public TombOfSargerasSisterAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TombOfSargerasLunaspyreAI(c); }
    explicit TombOfSargerasLunaspyreAI(Creature* pCreature) : TombOfSargerasSisterAI(pCreature) {}

protected:
    bool leadsStage(uint32_t stage) const override { return stage == 3; }

    void scheduleStage(uint32_t stage) override
    {
        if (stage < 3)
        {
            scriptEvents.addEvent(EVENT_LUNASPYRE_LUNAR_STRIKE, 500);
            scriptEvents.addEvent(EVENT_LUNASPYRE_MOON_BURN, 10000);
            return;
        }

        scriptEvents.addEvent(EVENT_LUNASPYRE_LUNAR_FIRE, 7000);
        scriptEvents.addEvent(EVENT_LUNASPYRE_MOON_BURN, 10000);
        scriptEvents.addEvent(EVENT_LUNASPYRE_LUNAR_BEACON, 18000);
    }

    void onMoonTurn() override
    {
        if (mStage != 2)
            return;

        warn("Priestess Lunaspyre casts Embrace of the Eclipse!");
        castSpellOnSelf(SPELL_LUNASPYRE_EMBRACE_ECLIPSE, true);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_LUNASPYRE_MOON_BURN:
                castAtPlayer(SPELL_LUNASPYRE_MOON_BURN, true);
                scriptEvents.addEvent(EVENT_LUNASPYRE_MOON_BURN, 24000);
                break;
            case EVENT_LUNASPYRE_LUNAR_STRIKE:
                castAtPlayer(SPELL_LUNASPYRE_LUNAR_STRIKE);
                scriptEvents.addEvent(EVENT_LUNASPYRE_LUNAR_STRIKE, 5500);
                break;
            case EVENT_LUNASPYRE_LUNAR_BEACON:
                castAtPlayer(SPELL_LUNASPYRE_LUNAR_BEACON, true);
                scriptEvents.addEvent(EVENT_LUNASPYRE_LUNAR_BEACON, 22000);
                break;
            case EVENT_LUNASPYRE_LUNAR_FIRE:
                castSpellOnVictim(SPELL_LUNASPYRE_LUNAR_FIRE);
                scriptEvents.addEvent(EVENT_LUNASPYRE_LUNAR_FIRE, 11000);
                break;
            default:
                break;
        }
    }
};

class TombOfSargerasMoontalonAI : public TombOfSargerasCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TombOfSargerasMoontalonAI(c); }
    explicit TombOfSargerasMoontalonAI(Creature* pCreature) : TombOfSargerasCombatAI(pCreature) {}

protected:
    void onEngage() override { mScreeching = false; }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mScreeching || _getHealthPercent() > 25)
            return;

        mScreeching = true;
        scriptEvents.addEvent(EVENT_MOONTALON_DEATHLY_SCREECH, 3000);
    }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_MOONTALON_DEATHLY_SCREECH)
            return;

        castSpellOnSelf(SPELL_MOONTALON_DEATHLY_SCREECH, true);
        scriptEvents.addEvent(EVENT_MOONTALON_DEATHLY_SCREECH, 6000);
    }

private:
    bool mScreeching = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Kil'jaeden: stage 1, flight, stage 2, Deceiver's Veil, final stage

class TombOfSargerasKiljaedenAI : public TombOfSargerasBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TombOfSargerasKiljaedenAI(c); }
    explicit TombOfSargerasKiljaedenAI(Creature* pCreature) : TombOfSargerasBossAI(pCreature, DATA_KILJAEDEN) {}

    void OnSummonDies(Creature* pSummon, Unit* /*pKiller*/) override
    {
        if (mStage != KJ_STAGE_VEIL || pSummon->getEntry() != NPC_KILJAEDEN_SHADOWSOUL)
            return;

        if (++mShadowsoulsDown >= mShadowsoulsCalled)
            beginFinalStage();
    }

protected:
    enum KiljaedenStage : uint32_t
    {
        KJ_STAGE_ONE = 1,
        KJ_STAGE_FLIGHT,
        KJ_STAGE_TWO,
        KJ_STAGE_VEIL,
        KJ_STAGE_FINAL
    };

    void onEngage() override
    {
        armBerserk(SPELL_KILJAEDEN_BERSERK, 600000);
        mLastSingularity = Util::getRandomUInt(3);
        mObeliskCount = 1;
        mObeliskWaves = 0;
        mReflectionTargets.clear();

        enterStage(KJ_STAGE_ONE);
        mArmageddon.begin(KiljaedenArmageddonFirstSteps);
        mSingularity.begin(KiljaedenSingularityFirstSteps);
        scriptEvents.addEvent(EVENT_KILJAEDEN_ARMAGEDDON, mArmageddon.next());
        scriptEvents.addEvent(EVENT_KILJAEDEN_FELCLAWS, 25000);
        scriptEvents.addEvent(EVENT_KILJAEDEN_SINGULARITY, mSingularity.next());
    }

    void onDisengage() override
    {
        setDormant(false);
        _removeAura(SPELL_KILJAEDEN_NETHER_GALE);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mStage == KJ_STAGE_ONE && _getHealthPercent() <= 80)
            beginFlight();
        else if (mStage == KJ_STAGE_TWO && _getHealthPercent() <= 40)
            beginVeil();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_KILJAEDEN_FELCLAWS:
                castSpellOnVictim(SPELL_KILJAEDEN_FELCLAWS);
                scriptEvents.addEvent(EVENT_KILJAEDEN_FELCLAWS, 25000);
                break;
            case EVENT_KILJAEDEN_ARMAGEDDON:
                releaseArmageddon();
                scriptEvents.addEvent(EVENT_KILJAEDEN_ARMAGEDDON, mArmageddon.next());
                break;
            case EVENT_KILJAEDEN_SINGULARITY:
                openSingularity();
                scriptEvents.addEvent(EVENT_KILJAEDEN_SINGULARITY, mSingularity.next());
                break;
            case EVENT_KILJAEDEN_BURSTING_DREADFLAME:
                castSpellAOE(SPELL_KILJAEDEN_BURSTING_DREADFLAME);
                scriptEvents.addEvent(EVENT_KILJAEDEN_BURSTING_DREADFLAME, mBursting.next());
                break;
            case EVENT_KILJAEDEN_FOCUSED_DREADFLAME:
                focusDreadflame();
                scriptEvents.addEvent(EVENT_KILJAEDEN_FOCUSED_DREADFLAME, mFocused.next());
                break;
            case EVENT_KILJAEDEN_SHADOW_REFLECTION:
                markReflections();
                scriptEvents.addEvent(EVENT_KILJAEDEN_SHADOW_REFLECTION, 111000);
                break;
            case EVENT_KILJAEDEN_REFLECTIONS_EMERGE:
                for (WoWGuid const& guid : mReflectionTargets)
                {
                    Unit* pTarget = findUnit(guid);
                    if (pTarget != nullptr && pTarget->isAlive())
                        callAdd(NPC_KILJAEDEN_ERUPTING_REFLECTION, pTarget->GetPosition());
                }
                mReflectionTargets.clear();
                break;
            case EVENT_KILJAEDEN_FLIGHT_STEP:
                runFlightStep();
                break;
            case EVENT_KILJAEDEN_SHADOWSOULS:
                mShadowsoulsCalled = 0;
                for (uint8_t i = 0; i < KiljaedenShadowsoulCount; ++i)
                {
                    if (callAdd(NPC_KILJAEDEN_SHADOWSOUL, tombRingPoint(KiljaedenPlatformCentre, 15.0f, 35.0f)) != nullptr)
                        ++mShadowsoulsCalled;
                }

                // no Shadowsoul to break the veil
                if (mShadowsoulsCalled == 0)
                    beginFinalStage();
                break;
            case EVENT_KILJAEDEN_DARKNESS:
                warn("Kil'jaeden begins to cast Darkness of a Thousand Souls!");
                castSpellAOE(SPELL_KILJAEDEN_DARKNESS_OF_SOULS);
                scriptEvents.addEvent(EVENT_KILJAEDEN_DARKNESS, 90000);
                break;
            case EVENT_KILJAEDEN_OBELISKS:
                raiseObelisks();
                break;
            case EVENT_KILJAEDEN_TEAR_RIFT:
            {
                summons.despawnEntry(NPC_KILJAEDEN_NETHER_RIFT);
                LocationVector const& spot = KiljaedenRiftSpots[Util::getRandomUInt(3)];
                warn("Kil'jaeden tears open a Nether Rift!");
                getCreature()->castSpellLoc(spot, SPELL_KILJAEDEN_TEAR_RIFT, false);
                placeHelper(NPC_KILJAEDEN_NETHER_RIFT, spot, 0);
                scriptEvents.addEvent(EVENT_KILJAEDEN_TEAR_RIFT, 94000);
                break;
            }
            default:
                break;
        }
    }

private:
    void releaseArmageddon()
    {
        warn("Kil'jaeden calls down Armageddon! Stand in the rain!");
        castSpellAOE(SPELL_KILJAEDEN_ARMAGEDDON);
        for (uint8_t i = 0; i < KiljaedenArmageddonStalkers; ++i)
            placeHelper(NPC_KILJAEDEN_ARMAGEDDON_STALKER, tombRingPoint(KiljaedenPlatformCentre, 15.0f, 38.0f), 9000);
    }

    // never the same corner twice in a row
    void openSingularity()
    {
        mLastSingularity = (mLastSingularity + 1 + Util::getRandomUInt(2)) % 4;
        warn("Kil'jaeden forms a Rupturing Singularity!");
        getCreature()->castSpellLoc(KiljaedenSingularitySpots[mLastSingularity], SPELL_KILJAEDEN_RUPTURING_SINGULARITY, true);
    }

    void focusDreadflame()
    {
        if (Unit* pTarget = pickPlayer(true))
        {
            warn("Kil'jaeden focuses his Dreadflame! Share the damage!");
            castSpell(pTarget, SPELL_KILJAEDEN_FOCUSED_DREADFLAME);
        }
    }

    void markReflections()
    {
        mReflectionTargets.clear();
        for (uint8_t i = 0; i < KiljaedenReflectionTargets; ++i)
        {
            if (Unit* pTarget = castAtPlayer(SPELL_KILJAEDEN_SHADOW_REFLECTION, true, true))
                mReflectionTargets.push_back(pTarget->GetNewGUID());
        }

        if (!mReflectionTargets.empty())
            scriptEvents.addEvent(EVENT_KILJAEDEN_REFLECTIONS_EMERGE, 8000);
    }

    // the obelisk count grows after every second wave
    void raiseObelisks()
    {
        for (uint32_t i = 0; i < mObeliskCount; ++i)
            placeHelper(NPC_KILJAEDEN_DEMONIC_OBELISK, tombRingPoint(KiljaedenPlatformCentre, 25.0f, 35.0f), 15000);

        uint32_t pause = 36000;
        if (++mObeliskWaves == 2)
        {
            mObeliskWaves = 0;
            ++mObeliskCount;
            pause = 60000;
        }

        scriptEvents.addEvent(EVENT_KILJAEDEN_OBELISKS, pause);
    }

    void beginFlight()
    {
        enterStage(KJ_STAGE_FLIGHT);
        getCreature()->interruptSpell();
        warn("Kil'jaeden takes to the sky above the platform!");
        setDormant(true);
        castSpellOnSelf(SPELL_KILJAEDEN_NETHER_GALE, true);
        mFlightStep = 0;
        scriptEvents.addEvent(EVENT_KILJAEDEN_FLIGHT_STEP, KiljaedenFlightPlan[0].delayMs);
    }

    void runFlightStep()
    {
        if (mFlightStep >= KiljaedenFlightStepCount)
            return;

        switch (KiljaedenFlightPlan[mFlightStep].move)
        {
            case KiljaedenFlightMove::Armageddon:
                releaseArmageddon();
                break;
            case KiljaedenFlightMove::Bursting:
                castSpellAOE(SPELL_KILJAEDEN_BURSTING_DREADFLAME);
                break;
            case KiljaedenFlightMove::Focused:
                focusDreadflame();
                break;
            case KiljaedenFlightMove::Land:
                _removeAura(SPELL_KILJAEDEN_NETHER_GALE);
                setDormant(false);
                beginStageTwo();
                return;
        }

        if (++mFlightStep < KiljaedenFlightStepCount)
            scriptEvents.addEvent(EVENT_KILJAEDEN_FLIGHT_STEP, KiljaedenFlightPlan[mFlightStep].delayMs);
    }

    void beginStageTwo()
    {
        enterStage(KJ_STAGE_TWO);
        warn("Kil'jaeden lands on the platform!");
        mArmageddon.begin(KiljaedenArmageddonSecondSteps);
        mSingularity.begin(KiljaedenSingularitySecondSteps);
        mBursting.begin(KiljaedenBurstingSecondSteps);
        mFocused.begin(KiljaedenFocusedSecondSteps);
        scriptEvents.addEvent(EVENT_KILJAEDEN_FELCLAWS, 10000);
        scriptEvents.addEvent(EVENT_KILJAEDEN_SHADOW_REFLECTION, 12000);
        scriptEvents.addEvent(EVENT_KILJAEDEN_ARMAGEDDON, mArmageddon.next());
        scriptEvents.addEvent(EVENT_KILJAEDEN_BURSTING_DREADFLAME, mBursting.next());
        scriptEvents.addEvent(EVENT_KILJAEDEN_SINGULARITY, mSingularity.next());
        scriptEvents.addEvent(EVENT_KILJAEDEN_FOCUSED_DREADFLAME, mFocused.next());
    }

    void beginVeil()
    {
        enterStage(KJ_STAGE_VEIL);
        getCreature()->interruptSpell();
        warn("Kil'jaeden shrouds himself in the Deceiver's Veil! Destroy the Shadowsouls!");
        setDormant(true);
        castSpellAOE(SPELL_KILJAEDEN_DECEIVERS_VEIL);
        mShadowsoulsDown = 0;
        mShadowsoulsCalled = KiljaedenShadowsoulCount;
        scriptEvents.addEvent(EVENT_KILJAEDEN_SHADOWSOULS, 3000);
    }

    void beginFinalStage()
    {
        enterStage(KJ_STAGE_FINAL);
        getCreature()->interruptSpell();
        warn("Kil'jaeden emerges from the veil!");
        setDormant(false);
        mBursting.beginLooped(KiljaedenBurstingFinalSteps, 1);
        mFocused.beginLooped(KiljaedenFocusedFinalSteps, 1);
        scriptEvents.addEvent(EVENT_KILJAEDEN_DARKNESS, 1000);
        scriptEvents.addEvent(EVENT_KILJAEDEN_FELCLAWS, 10000);
        scriptEvents.addEvent(EVENT_KILJAEDEN_TEAR_RIFT, 14000);
        scriptEvents.addEvent(EVENT_KILJAEDEN_OBELISKS, 25000);
        scriptEvents.addEvent(EVENT_KILJAEDEN_BURSTING_DREADFLAME, mBursting.next());
        scriptEvents.addEvent(EVENT_KILJAEDEN_FOCUSED_DREADFLAME, mFocused.next());
    }

    TombCooldowns mArmageddon;
    TombCooldowns mSingularity;
    TombCooldowns mBursting;
    TombCooldowns mFocused;
    std::vector<WoWGuid> mReflectionTargets;
    size_t mFlightStep = 0;
    uint32_t mLastSingularity = 0;
    uint32_t mObeliskCount = 1;
    uint32_t mObeliskWaves = 0;
    uint32_t mShadowsoulsDown = 0;
    uint32_t mShadowsoulsCalled = KiljaedenShadowsoulCount;
};

class TombOfSargerasEruptingReflectionAI : public TombOfSargerasSpellSlingerAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TombOfSargerasEruptingReflectionAI(c); }
    explicit TombOfSargerasEruptingReflectionAI(Creature* pCreature) : TombOfSargerasSpellSlingerAI(pCreature, SPELL_REFLECTION_ERUPTING_DREADFLAME, 2500) {}
};

class TombOfSargerasShadowsoulAI : public TombOfSargerasCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TombOfSargerasShadowsoulAI(c); }
    explicit TombOfSargerasShadowsoulAI(Creature* pCreature) : TombOfSargerasCombatAI(pCreature) {}

protected:
    void onEngage() override
    {
        setRooted(true);
        scriptEvents.addEvent(EVENT_SHADOWSOUL_DESTABILIZE, 3000);
    }

    void onDisengage() override { setRooted(false); }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_SHADOWSOUL_DESTABILIZE)
            return;

        castSpellAOE(SPELL_SHADOWSOUL_DESTABILIZED);
        scriptEvents.addEvent(EVENT_SHADOWSOUL_DESTABILIZE, 5000);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Maiden of Vigilance: hammer rotation, then Wrath of the Creators behind Titanic Bulwark

class TombOfSargerasMaidenOfVigilanceAI : public TombOfSargerasBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TombOfSargerasMaidenOfVigilanceAI(c); }
    explicit TombOfSargerasMaidenOfVigilanceAI(Creature* pCreature) : TombOfSargerasBossAI(pCreature, DATA_MAIDEN_OF_VIGILANCE) {}

protected:
    enum MaidenStage : uint32_t
    {
        MAIDEN_STAGE_ROTATION = 1,
        MAIDEN_STAGE_WRATH,
        MAIDEN_STAGE_EMPOWERED
    };

    void onEngage() override
    {
        armBerserk(SPELL_MAIDEN_BERSERK, 480000);
        mWrathCount = 0;
        startRotation(false);
    }

    void onDisengage() override
    {
        mWatchingWrath = false;
        setRooted(false);
        _setMeleeDisabled(false);
    }

    void onCombatTick(uint32_t diff) override
    {
        if (mStage != MAIDEN_STAGE_WRATH || !mWatchingWrath)
            return;

        mWrathWatchMs += diff;
        bool const shielded = hasAura(SPELL_MAIDEN_TITANIC_BULWARK);
        if (shielded)
            mBulwarkSeen = true;
        if (_isCasting())
            mWrathSeen = true;

        if (mBulwarkSeen && !shielded)
        {
            // the bulwark broke and stops the channel
            mWatchingWrath = false;
            getCreature()->interruptSpell();
            castSpellAOE(SPELL_MAIDEN_BLOWBACK, true);
            startRotation(true);
            return;
        }

        bool const channelOver = mWrathSeen && !_isCasting();
        bool const neverStarted = !mWrathSeen && !mBulwarkSeen && mWrathWatchMs > 5000;
        if (!channelOver && !neverStarted)
            return;

        // the channel ran its course with the bulwark intact
        mWatchingWrath = false;
        if (channelOver)
            castSpellAOE(SPELL_MAIDEN_EXTINGUISH_LIFE, true);
        _removeAura(SPELL_MAIDEN_TITANIC_BULWARK);
        startRotation(true);
    }

    void onEvent(uint32_t eventId) override
    {
        bool const empowered = mStage == MAIDEN_STAGE_EMPOWERED;

        switch (eventId)
        {
            case EVENT_MAIDEN_INFUSION:
                castSpellAOE(SPELL_MAIDEN_INFUSION);
                if (empowered)
                    scriptEvents.addEvent(EVENT_MAIDEN_INFUSION, 38000);
                break;
            case EVENT_MAIDEN_HAMMER_OF_CREATION:
                castSpellOnVictim(SPELL_MAIDEN_HAMMER_OF_CREATION);
                scriptEvents.addEvent(EVENT_MAIDEN_CREATION_LANDS, 100);
                scriptEvents.addEvent(EVENT_MAIDEN_HAMMER_OF_OBLITERATION, 19000);
                break;
            case EVENT_MAIDEN_CREATION_LANDS:
                castSpellOnVictim(SPELL_MAIDEN_CREATION_TANK_HIT, true);
                castSpellAOE(SPELL_MAIDEN_CREATION_SPLASH, true);
                break;
            case EVENT_MAIDEN_HAMMER_OF_OBLITERATION:
                castSpellOnVictim(SPELL_MAIDEN_HAMMER_OF_OBLITERATION);
                scriptEvents.addEvent(EVENT_MAIDEN_OBLITERATION_LANDS, 100);
                if (empowered)
                    scriptEvents.addEvent(EVENT_MAIDEN_HAMMER_OF_CREATION, 19000);
                break;
            case EVENT_MAIDEN_OBLITERATION_LANDS:
                castSpellOnVictim(SPELL_MAIDEN_OBLITERATION_TANK_HIT, true);
                castSpellAOE(SPELL_MAIDEN_OBLITERATION_SPLASH, true);
                break;
            case EVENT_MAIDEN_MASS_INSTABILITY:
                castSpellAOE(SPELL_MAIDEN_MASS_INSTABILITY);
                scriptEvents.addEvent(EVENT_MAIDEN_INSTABILITY_LANDS, 100);
                if (empowered)
                    scriptEvents.addEvent(EVENT_MAIDEN_MASS_INSTABILITY, 36000);
                break;
            case EVENT_MAIDEN_INSTABILITY_LANDS:
                castSpellOnVictim(SPELL_MAIDEN_UNSTABLE_SOUL, true);
                castAtPlayer(SPELL_MAIDEN_UNSTABLE_SOUL, true, true);
                castAtPlayer(SPELL_MAIDEN_UNSTABLE_SOUL, true, true);
                break;
            case EVENT_MAIDEN_WRATH:
                beginWrath();
                break;
            case EVENT_MAIDEN_WRATH_BEGIN:
                setRooted(true);
                castSpellOnSelf(SPELL_MAIDEN_TITANIC_BULWARK, true);
                castSpellAOE(SPELL_MAIDEN_WRATH_OF_THE_CREATORS);
                mWatchingWrath = true;
                mBulwarkSeen = false;
                mWrathSeen = false;
                mWrathWatchMs = 0;
                break;
            default:
                break;
        }
    }

private:
    void startRotation(bool empowered)
    {
        enterStage(empowered ? MAIDEN_STAGE_EMPOWERED : MAIDEN_STAGE_ROTATION);
        setRooted(false);
        _setMeleeDisabled(false);
        scriptEvents.addEvent(EVENT_MAIDEN_INFUSION, empowered ? 3000 : 2000);
        scriptEvents.addEvent(EVENT_MAIDEN_HAMMER_OF_CREATION, 12000);
        scriptEvents.addEvent(EVENT_MAIDEN_MASS_INSTABILITY, 21000);
        scriptEvents.addEvent(EVENT_MAIDEN_WRATH, empowered ? 82000 : 40000);
    }

    // every other Wrath is channelled from the far side of the platform
    void beginWrath()
    {
        enterStage(MAIDEN_STAGE_WRATH);
        warn("The Maiden of Vigilance channels Wrath of the Creators! Break her Titanic Bulwark!");
        _setMeleeDisabled(true);
        LocationVector const destination = (++mWrathCount % 2) != 0 ? MaidenFarSide : getCreature()->GetSpawnPosition();
        getCreature()->castSpellLoc(destination, SPELL_MAIDEN_TRANSLOCATE, true);
        scriptEvents.addEvent(EVENT_MAIDEN_WRATH_BEGIN, 1500);
    }

    uint32_t mWrathCount = 0;
    uint32_t mWrathWatchMs = 0;
    bool mWatchingWrath = false;
    bool mBulwarkSeen = false;
    bool mWrathSeen = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// The Desolate Host: Engine of Souls and Soul Queen Dejahna, the Host rises at 30 percent

class TombOfSargerasEngineOfSoulsAI : public TombOfSargerasBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TombOfSargerasEngineOfSoulsAI(c); }
    explicit TombOfSargerasEngineOfSoulsAI(Creature* pCreature) : TombOfSargerasBossAI(pCreature, DATA_THE_DESOLATE_HOST) {}

protected:
    void onEngage() override
    {
        mWakeSent = false;
        setRooted(true);
        _setMeleeDisabled(true);
        castSpellOnSelf(SPELL_HOST_QUIETUS, true);
        scriptEvents.addEvent(EVENT_ENGINE_BONE_SHARDS, 2000);
        scriptEvents.addEvent(EVENT_ENGINE_COLLAPSING_FISSURE, 7000);
        scriptEvents.addEvent(EVENT_ENGINE_CORPOREAL_WAVE, 60000);
        scriptEvents.addEvent(EVENT_ENGINE_TORMENTED_CRIES, 120000);
    }

    void onDisengage() override
    {
        setRooted(false);
        _setMeleeDisabled(false);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mWakeSent || _getHealthPercent() > 30)
            return;

        mWakeSent = true;
        if (TombOfSargerasInstanceScript* pTomb = getTomb())
            pTomb->awakenDesolateHost();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ENGINE_BONE_SHARDS:
                castSpellOnVictim(SPELL_ENGINE_BONE_SHARDS);
                scriptEvents.addEvent(EVENT_ENGINE_BONE_SHARDS, 2500);
                break;
            case EVENT_ENGINE_COLLAPSING_FISSURE:
                castAtPlayer(SPELL_ENGINE_COLLAPSING_FISSURE, false, true);
                scriptEvents.addEvent(EVENT_ENGINE_COLLAPSING_FISSURE, 32000);
                break;
            case EVENT_ENGINE_TORMENTED_CRIES:
                warn("The Engine of Souls unleashes Tormented Cries!");
                castSpellAOE(SPELL_ENGINE_TORMENTED_CRIES);
                scriptEvents.addEvent(EVENT_ENGINE_TORMENTED_CRIES, 120000);
                break;
            case EVENT_ENGINE_CORPOREAL_WAVE:
            {
                // reinforcements stop once the Host has risen
                TombOfSargerasInstanceScript* pTomb = getTomb();
                if (pTomb != nullptr && pTomb->isHostAwakened())
                    break;

                callAdd(NPC_HOST_REANIMATED_TEMPLAR, tombRingPoint(HostRoomCentre, 15.0f, 25.0f));
                callAdd(NPC_HOST_GHASTLY_BONEWARDEN, tombRingPoint(HostRoomCentre, 15.0f, 25.0f));
                callAdd(NPC_HOST_GHASTLY_BONEWARDEN, tombRingPoint(HostRoomCentre, 15.0f, 25.0f));
                scriptEvents.addEvent(EVENT_ENGINE_CORPOREAL_WAVE, 60000);
                break;
            }
            default:
                break;
        }
    }

private:
    bool mWakeSent = false;
};

class TombOfSargerasSoulQueenDejahnaAI : public TombOfSargerasBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TombOfSargerasSoulQueenDejahnaAI(c); }
    explicit TombOfSargerasSoulQueenDejahnaAI(Creature* pCreature) : TombOfSargerasBossAI(pCreature, DATA_THE_DESOLATE_HOST) {}

protected:
    void onEngage() override
    {
        mWakeSent = false;
        setRooted(true);
        _setMeleeDisabled(true);
        castSpellOnSelf(SPELL_HOST_QUIETUS, true);
        scriptEvents.addEvent(EVENT_DEJAHNA_CRUSH_MIND, 2000);
        scriptEvents.addEvent(EVENT_DEJAHNA_SOULBIND, 12000);
        scriptEvents.addEvent(EVENT_DEJAHNA_WAILING_SOULS, 60000);
        scriptEvents.addEvent(EVENT_DEJAHNA_SPIRIT_WAVE, 120000);
    }

    void onDisengage() override
    {
        setRooted(false);
        _setMeleeDisabled(false);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mWakeSent || _getHealthPercent() > 30)
            return;

        mWakeSent = true;
        if (TombOfSargerasInstanceScript* pTomb = getTomb())
            pTomb->awakenDesolateHost();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_DEJAHNA_CRUSH_MIND:
                castSpellOnVictim(SPELL_DEJAHNA_CRUSH_MIND);
                scriptEvents.addEvent(EVENT_DEJAHNA_CRUSH_MIND, 2500);
                break;
            case EVENT_DEJAHNA_SOULBIND:
                castAtPlayer(SPELL_DEJAHNA_SOULBIND, true);
                scriptEvents.addEvent(EVENT_DEJAHNA_SOULBIND, 24000);
                break;
            case EVENT_DEJAHNA_WAILING_SOULS:
                warn("Soul Queen Dejahna begins to cast Wailing Souls!");
                castSpellAOE(SPELL_DEJAHNA_WAILING_SOULS);
                scriptEvents.addEvent(EVENT_DEJAHNA_WAILING_SOULS, 120000);
                break;
            case EVENT_DEJAHNA_SPIRIT_WAVE:
            {
                TombOfSargerasInstanceScript* pTomb = getTomb();
                if (pTomb != nullptr && pTomb->isHostAwakened())
                    break;

                for (uint8_t i = 0; i < 2; ++i)
                    callAdd(NPC_HOST_FALLEN_PRIESTESS, tombRingPoint(HostRoomCentre, 15.0f, 25.0f));
                for (uint8_t i = 0; i < 4; ++i)
                    callAdd(NPC_HOST_SOUL_RESIDUE, tombRingPoint(HostRoomCentre, 15.0f, 25.0f));
                scriptEvents.addEvent(EVENT_DEJAHNA_SPIRIT_WAVE, 60000);
                break;
            }
            default:
                break;
        }
    }

private:
    bool mWakeSent = false;
};

class TombOfSargerasDesolateHostAI : public TombOfSargerasBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TombOfSargerasDesolateHostAI(c); }
    explicit TombOfSargerasDesolateHostAI(Creature* pCreature) : TombOfSargerasBossAI(pCreature, DATA_THE_DESOLATE_HOST) {}

    void OnLoad() override { conceal(); }

    void DoAction(int32_t action) override
    {
        if (action != ACTION_TOMB_HOST_AWAKEN || mRevealPending || _isInCombat())
            return;

        mRevealPending = true;
        mRevealLeftMs = 4000;
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (mRevealPending)
        {
            uint32_t const diff = static_cast<uint32_t>(time_passed);
            if (mRevealLeftMs > diff)
            {
                mRevealLeftMs -= diff;
            }
            else
            {
                mRevealPending = false;
                reveal();
            }
        }

        TombOfSargerasBossAI::AIUpdate(time_passed);
    }

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_HOST_SUNDERING_DOOM, 14000);
        scriptEvents.addEvent(EVENT_HOST_DOOMED_SUNDERING, 24000);
        scriptEvents.addEvent(EVENT_HOST_TORMENT, 30000);
    }

    void onDisengage() override
    {
        if (getCreature()->isAlive())
            conceal();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_HOST_TORMENT:
                castSpellOnSelf(SPELL_HOST_TORMENT, true);
                scriptEvents.addEvent(EVENT_HOST_TORMENT, 20000);
                break;
            case EVENT_HOST_SUNDERING_DOOM:
                castSpellOnSelf(SPELL_HOST_SUNDERING_DOOM);
                scriptEvents.addEvent(EVENT_HOST_SUNDERING_DOOM, 26000);
                break;
            case EVENT_HOST_DOOMED_SUNDERING:
                castSpellOnSelf(SPELL_HOST_DOOMED_SUNDERING);
                scriptEvents.addEvent(EVENT_HOST_DOOMED_SUNDERING, 26000);
                break;
            default:
                break;
        }
    }

private:
    // hidden and out of combat until the second stage
    void conceal()
    {
        mRevealPending = false;
        getCreature()->setVisible(false);
        getCreature()->addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        setCanEnterCombat(false);
    }

    void reveal()
    {
        TombOfSargerasInstanceScript* pTomb = getTomb();
        if (pTomb == nullptr || !getCreature()->isAlive() || !pTomb->isHostAwakened())
            return;

        if (pTomb->getBossState(DATA_THE_DESOLATE_HOST) != EncounterStates::InProgress)
            return;

        // joins at the health the other two share
        for (Creature* pMember : pTomb->getGroup(DATA_THE_DESOLATE_HOST))
        {
            if (pMember == getCreature() || !pMember->isAlive() || pMember->getMaxHealth() == 0)
                continue;

            uint64_t const share = static_cast<uint64_t>(getCreature()->getMaxHealth()) * pMember->getHealth() / pMember->getMaxHealth();
            getCreature()->setHealth(std::max<uint32_t>(1, static_cast<uint32_t>(share)));
            break;
        }

        getCreature()->setVisible(true);
        getCreature()->removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        setCanEnterCombat(true);
        warn("The Desolate Host rises!");
        setZoneWideCombat();
    }

    bool mRevealPending = false;
    uint32_t mRevealLeftMs = 0;
};

class TombOfSargerasReanimatedTemplarAI : public TombOfSargerasCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TombOfSargerasReanimatedTemplarAI(c); }
    explicit TombOfSargerasReanimatedTemplarAI(Creature* pCreature) : TombOfSargerasCombatAI(pCreature) {}

protected:
    void onEngage() override { scriptEvents.addEvent(EVENT_TEMPLAR_RUPTURING_SLAM, 12000); }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_TEMPLAR_RUPTURING_SLAM)
            return;

        castSpellAOE(SPELL_TEMPLAR_RUPTURING_SLAM);
        scriptEvents.addEvent(EVENT_TEMPLAR_RUPTURING_SLAM, 22000);
    }
};

class TombOfSargerasGhastlyBonewardenAI : public TombOfSargerasCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TombOfSargerasGhastlyBonewardenAI(c); }
    explicit TombOfSargerasGhastlyBonewardenAI(Creature* pCreature) : TombOfSargerasCombatAI(pCreature) {}

protected:
    void onEngage() override { scriptEvents.addEvent(EVENT_BONEWARDEN_GRASPING_DARKNESS, 4000); }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_BONEWARDEN_GRASPING_DARKNESS)
            return;

        castAtPlayer(SPELL_BONEWARDEN_GRASPING_DARKNESS);
        scriptEvents.addEvent(EVENT_BONEWARDEN_GRASPING_DARKNESS, Util::getRandomUInt(4000, 6000));
    }
};

class TombOfSargerasFallenPriestessAI : public TombOfSargerasCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TombOfSargerasFallenPriestessAI(c); }
    explicit TombOfSargerasFallenPriestessAI(Creature* pCreature) : TombOfSargerasCombatAI(pCreature) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_PRIESTESS_SHATTERING_SCREAM, 4000);
        scriptEvents.addEvent(EVENT_PRIESTESS_SPIRIT_CHAINS, 14000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_PRIESTESS_SHATTERING_SCREAM:
                castAtPlayer(SPELL_PRIESTESS_SHATTERING_SCREAM);
                scriptEvents.addEvent(EVENT_PRIESTESS_SHATTERING_SCREAM, Util::getRandomUInt(10000, 15000));
                break;
            case EVENT_PRIESTESS_SPIRIT_CHAINS:
                castAtPlayer(SPELL_PRIESTESS_SPIRIT_CHAINS, true);
                scriptEvents.addEvent(EVENT_PRIESTESS_SPIRIT_CHAINS, 14000);
                break;
            default:
                break;
        }
    }
};

// Soul Residue erupts instead of dying
class TombOfSargerasSoulResidueAI : public TombOfSargerasCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TombOfSargerasSoulResidueAI(c); }
    explicit TombOfSargerasSoulResidueAI(Creature* pCreature) : TombOfSargerasCombatAI(pCreature) {}

    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage == nullptr || *damage < getCreature()->getHealth())
            return;

        *damage = 0;
        if (mErupted)
            return;

        mErupted = true;
        setRooted(true);
        _setMeleeDisabled(true);
        getCreature()->addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        castSpellAOE(SPELL_RESIDUE_SOUL_ERUPTION, true);
        despawn(5000, 0);
    }

private:
    bool mErupted = false;
};

void SetupTombOfSargeras(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_TOMB_OF_SARGERAS, &TombOfSargerasInstanceScript::Create);

    // Goroth
    mgr->register_creature_script(BOSS_GOROTH, &TombOfSargerasGorothAI::Create);
    mgr->register_creature_script(NPC_GOROTH_INFERNAL_SPIKE, &TombOfSargerasPassiveAI::Create);

    // Harjatan
    mgr->register_creature_script(BOSS_HARJATAN, &TombOfSargerasHarjatanAI::Create);
    mgr->register_creature_script(NPC_RAZORJAW_GLADIATOR, &TombOfSargerasGladiatorAI::Create);
    mgr->register_creature_script(NPC_RAZORJAW_WAVEMENDER, &TombOfSargerasWavemenderAI::Create);

    // Mistress Sassz'ine
    mgr->register_creature_script(BOSS_MISTRESS_SASSZINE, &TombOfSargerasSasszineAI::Create);
    mgr->register_creature_script(NPC_SASSZINE_JELLYFISH, &TombOfSargerasJellyfishAI::Create);
    mgr->register_creature_script(NPC_SASSZINE_WAVERUNNER, &TombOfSargerasWaverunnerAI::Create);
    mgr->register_creature_script(NPC_SASSZINE_PIRANHADO, &TombOfSargerasPiranhadoAI::Create);

    // Fallen Avatar
    mgr->register_creature_script(BOSS_FALLEN_AVATAR, &TombOfSargerasFallenAvatarAI::Create);
    mgr->register_creature_script(NPC_AVATAR_MAIDEN_OF_VALOR, &TombOfSargerasMaidenOfValorAI::Create);
    mgr->register_creature_script(NPC_AVATAR_CONTAINMENT_PYLON, &TombOfSargerasPassiveAI::Create);
    mgr->register_creature_script(NPC_AVATAR_CORRUPTED_BLADE, &TombOfSargerasPassiveAI::Create);

    // Demonic Inquisition
    mgr->register_creature_script(BOSS_ATRIGAN, &TombOfSargerasAtriganAI::Create);
    mgr->register_creature_script(BOSS_BELAC, &TombOfSargerasBelacAI::Create);
    mgr->register_creature_script(NPC_INQUISITION_TORMENTED_SOUL, &TombOfSargerasTormentedSoulAI::Create);

    // Sisters of the Moon
    mgr->register_creature_script(NPC_SISTERS_MOON_STALKER, &TombOfSargerasMoonStalkerAI::Create);
    mgr->register_creature_script(BOSS_HUNTRESS_KASPARIAN, &TombOfSargerasKasparianAI::Create);
    mgr->register_creature_script(BOSS_CAPTAIN_YATHAE, &TombOfSargerasYathaeAI::Create);
    mgr->register_creature_script(BOSS_PRIESTESS_LUNASPYRE, &TombOfSargerasLunaspyreAI::Create);
    mgr->register_creature_script(NPC_SISTERS_MOONTALON, &TombOfSargerasMoontalonAI::Create);

    // Kil'jaeden
    mgr->register_creature_script(BOSS_KILJAEDEN, &TombOfSargerasKiljaedenAI::Create);
    mgr->register_creature_script(NPC_KILJAEDEN_ARMAGEDDON_STALKER, &TombOfSargerasArmageddonStalkerAI::Create);
    mgr->register_creature_script(NPC_KILJAEDEN_ERUPTING_REFLECTION, &TombOfSargerasEruptingReflectionAI::Create);
    mgr->register_creature_script(NPC_KILJAEDEN_SHADOWSOUL, &TombOfSargerasShadowsoulAI::Create);
    mgr->register_creature_script(NPC_KILJAEDEN_DEMONIC_OBELISK, &TombOfSargerasDemonicObeliskAI::Create);
    mgr->register_creature_script(NPC_KILJAEDEN_NETHER_RIFT, &TombOfSargerasNetherRiftAI::Create);

    // Maiden of Vigilance
    mgr->register_creature_script(BOSS_MAIDEN_OF_VIGILANCE, &TombOfSargerasMaidenOfVigilanceAI::Create);

    // The Desolate Host
    mgr->register_creature_script(NPC_HOST_SWIRLING_SOULS, &TombOfSargerasSwirlingSoulsAI::Create);
    mgr->register_creature_script(BOSS_ENGINE_OF_SOULS, &TombOfSargerasEngineOfSoulsAI::Create);
    mgr->register_creature_script(BOSS_SOUL_QUEEN_DEJAHNA, &TombOfSargerasSoulQueenDejahnaAI::Create);
    mgr->register_creature_script(BOSS_DESOLATE_HOST, &TombOfSargerasDesolateHostAI::Create);
    mgr->register_creature_script(NPC_HOST_REANIMATED_TEMPLAR, &TombOfSargerasReanimatedTemplarAI::Create);
    mgr->register_creature_script(NPC_HOST_GHASTLY_BONEWARDEN, &TombOfSargerasGhastlyBonewardenAI::Create);
    mgr->register_creature_script(NPC_HOST_FALLEN_PRIESTESS, &TombOfSargerasFallenPriestessAI::Create);
    mgr->register_creature_script(NPC_HOST_SOUL_RESIDUE, &TombOfSargerasSoulResidueAI::Create);
}
