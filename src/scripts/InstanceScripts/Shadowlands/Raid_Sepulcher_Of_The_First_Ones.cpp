/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Raid_Sepulcher_Of_The_First_Ones.hpp"

#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <cmath>
#include <cstddef>
#include <iterator>
#include <set>
#include <utility>
#include <vector>

enum SepulcherOfTheFirstOnesEvents
{
    // Vigilant Guardian
    EVENT_SEPULCHER_GUARDIAN_REFRACTED_BLAST = 1,
    EVENT_SEPULCHER_GUARDIAN_DERESOLUTION,
    EVENT_SEPULCHER_GUARDIAN_EXPOSED_CORE,
    EVENT_SEPULCHER_GUARDIAN_SENTRIES,
    EVENT_SEPULCHER_GUARDIAN_AUTOMA,
    EVENT_SEPULCHER_GUARDIAN_PNEUMATIC_IMPACT,
    EVENT_SEPULCHER_GUARDIAN_MATTER_DISSOLUTION,
    EVENT_SEPULCHER_GUARDIAN_SPLIT_RESOLUTION,
    EVENT_SEPULCHER_GUARDIAN_FRACTURED_CORE,
    EVENT_SEPULCHER_GUARDIAN_CORE_OVERLOAD,

    // Halondrus the Reclaimer
    EVENT_SEPULCHER_HALONDRUS_TREMORS,
    EVENT_SEPULCHER_HALONDRUS_MOTES_ARRIVE,
    EVENT_SEPULCHER_HALONDRUS_EARTHBREAKER,
    EVENT_SEPULCHER_HALONDRUS_LIGHTSHATTER,
    EVENT_SEPULCHER_HALONDRUS_PLANETCRACKER,
    EVENT_SEPULCHER_HALONDRUS_CRUSHING_PRISM,
    EVENT_SEPULCHER_HALONDRUS_LANCE,
    EVENT_SEPULCHER_HALONDRUS_FULL_ENERGY,
    EVENT_SEPULCHER_HALONDRUS_QUAKING_STEPS,
    EVENT_SEPULCHER_HALONDRUS_SHATTER,
    EVENT_SEPULCHER_HALONDRUS_SETTLED,

    // The Jailer
    EVENT_SEPULCHER_JAILER_DOMINATION,
    EVENT_SEPULCHER_JAILER_OPPRESSION,
    EVENT_SEPULCHER_JAILER_MARTYRDOM,
    EVENT_SEPULCHER_JAILER_TORMENT,
    EVENT_SEPULCHER_JAILER_DAMNATION,
    EVENT_SEPULCHER_JAILER_ATTUNEMENT,
    EVENT_SEPULCHER_JAILER_SHATTERING_BLAST,
    EVENT_SEPULCHER_JAILER_COMPULSION,
    EVENT_SEPULCHER_JAILER_DECIMATOR,
    EVENT_SEPULCHER_JAILER_RUNE_OF_DOMINATION,
    EVENT_SEPULCHER_JAILER_ANGUISH,
    EVENT_SEPULCHER_JAILER_DEFILE,

    // Lihuvim, Principal Architect
    EVENT_SEPULCHER_LIHUVIM_CASCADE,
    EVENT_SEPULCHER_LIHUVIM_RESONANCE,
    EVENT_SEPULCHER_LIHUVIM_DECONSTRUCT,
    EVENT_SEPULCHER_LIHUVIM_MOTES,
    EVENT_SEPULCHER_LIHUVIM_COSMIC_SHIFT,
    EVENT_SEPULCHER_LIHUVIM_SYNTHESIZE,
    EVENT_SEPULCHER_LIHUVIM_SYNTHESIS_DONE,

    // Dausegne, the Fallen Oracle
    EVENT_SEPULCHER_DAUSEGNE_ARC,
    EVENT_SEPULCHER_DAUSEGNE_BARRAGE,
    EVENT_SEPULCHER_DAUSEGNE_HALO,
    EVENT_SEPULCHER_DAUSEGNE_CORE,
    EVENT_SEPULCHER_DAUSEGNE_SIPHON,
    EVENT_SEPULCHER_DAUSEGNE_BARRIER,
    EVENT_SEPULCHER_DAUSEGNE_DOMINION,
    EVENT_SEPULCHER_DAUSEGNE_TOTAL_DOMINION,

    // Skolex, the Insatiable Ravener
    EVENT_SEPULCHER_SKOLEX_JAWS,
    EVENT_SEPULCHER_SKOLEX_DUST_FLAIL,
    EVENT_SEPULCHER_SKOLEX_DUST_BLAST,
    EVENT_SEPULCHER_SKOLEX_RETCH,
    EVENT_SEPULCHER_SKOLEX_DEVOURING_BLOOD,
    EVENT_SEPULCHER_SKOLEX_BURROW,
    EVENT_SEPULCHER_SKOLEX_RESURFACE,

    // Lords of Dread
    EVENT_SEPULCHER_MALGANIS_CLAWS,
    EVENT_SEPULCHER_MALGANIS_CARRION,
    EVENT_SEPULCHER_MALGANIS_SHADOWS,
    EVENT_SEPULCHER_MALGANIS_RAMPAGE,
    EVENT_SEPULCHER_MALGANIS_DARKNESS,
    EVENT_SEPULCHER_MALGANIS_DECAY,
    EVENT_SEPULCHER_KINTESSA_STRIKE,
    EVENT_SEPULCHER_KINTESSA_TREPIDATION,
    EVENT_SEPULCHER_KINTESSA_SLUMBER,
    EVENT_SEPULCHER_KINTESSA_INFILTRATION,
    EVENT_SEPULCHER_KINTESSA_SWARM,
    EVENT_SEPULCHER_KINTESSA_RAMPAGE,

    // Prototype Pantheon
    EVENT_SEPULCHER_WAR_GLOOM_BOLT,
    EVENT_SEPULCHER_WAR_RITUAL,
    EVENT_SEPULCHER_WAR_DEATHTOUCH,
    EVENT_SEPULCHER_DUTY_STRIKES,
    EVENT_SEPULCHER_DUTY_VOLLEY,
    EVENT_SEPULCHER_DUTY_CALL,
    EVENT_SEPULCHER_RENEWAL_BOLT,
    EVENT_SEPULCHER_RENEWAL_STAMPEDE,
    EVENT_SEPULCHER_RENEWAL_SEEDS,
    EVENT_SEPULCHER_RENEWAL_ANIMASTORM,
    EVENT_SEPULCHER_ABSOLUTION_PAIN,
    EVENT_SEPULCHER_ABSOLUTION_BURDEN,
    EVENT_SEPULCHER_ABSOLUTION_HAND,
    EVENT_SEPULCHER_ABSOLUTION_HUNTER,

    // Anduin Wrynn
    EVENT_SEPULCHER_ANDUIN_HOPEBREAKER,
    EVENT_SEPULCHER_ANDUIN_WORD_PAIN,
    EVENT_SEPULCHER_ANDUIN_BARRIER,
    EVENT_SEPULCHER_ANDUIN_BLASPHEMY,
    EVENT_SEPULCHER_ANDUIN_WICKED_STAR,
    EVENT_SEPULCHER_ANDUIN_KINGSMOURNE,
    EVENT_SEPULCHER_ANDUIN_INTERMISSION,
    EVENT_SEPULCHER_ANDUIN_REMNANT_RISES,
    EVENT_SEPULCHER_ANDUIN_REMNANT_WAKES,
    EVENT_SEPULCHER_ANDUIN_MARCH,
    EVENT_SEPULCHER_ANDUIN_INTERMISSION_TIMEOUT,
    EVENT_SEPULCHER_ANDUIN_REFLECTIONS,
    EVENT_SEPULCHER_ANDUIN_BEACON,
    EVENT_SEPULCHER_ANDUIN_EMPOWERED_HOPEBREAKER,
    EVENT_SEPULCHER_ANDUIN_HOPELESSNESS,
    EVENT_SEPULCHER_REMNANT_ARMY,
    EVENT_SEPULCHER_REMNANT_SOUL_REAPER,
    EVENT_SEPULCHER_REMNANT_RETURN,

    // Rygelon
    EVENT_SEPULCHER_RYGELON_ECLIPSE,
    EVENT_SEPULCHER_RYGELON_COLLAPSE,
    EVENT_SEPULCHER_RYGELON_COSMOS,
    EVENT_SEPULCHER_RYGELON_TERMINATOR,
    EVENT_SEPULCHER_RYGELON_MATTER,
    EVENT_SEPULCHER_RYGELON_MASSIVE_BANG,
    EVENT_SEPULCHER_RYGELON_SINGULARITY,
    EVENT_SEPULCHER_RYGELON_SHATTER_SPHERE,
    EVENT_SEPULCHER_RYGELON_PLASMA,
    EVENT_SEPULCHER_RYGELON_SINGULARITY_END,

    // Artificer Xy'mox
    EVENT_SEPULCHER_XYMOX_TEAR,
    EVENT_SEPULCHER_XYMOX_GLYPH,
    EVENT_SEPULCHER_XYMOX_STASIS_TRAP,
    EVENT_SEPULCHER_XYMOX_SPARKNOVA,
    EVENT_SEPULCHER_XYMOX_GUNSHIP,
    EVENT_SEPULCHER_XYMOX_RINGS,
    EVENT_SEPULCHER_XYMOX_CARTEL_ELITE,

    // timed casts of the adds, one event per table slot
    EVENT_SEPULCHER_ADD_CAST_FIRST = 300
};

namespace
{
    float const SepulcherTwoPi = 6.28318531f;
    float const SepulcherQuarterTurn = 1.57079633f;
    uint32_t const SepulcherFriendlyFaction = 35;

    uint32_t const SepulcherBerserkMs = 600000;
    uint32_t const GuardianBerserkMs = 540000;
    uint32_t const LihuvimBerserkMs = 540000;
    uint32_t const SkolexDestroyMs = 480000;
    uint32_t const AnduinBerserkMs = 900000;

    // energy, mana and cosmic energy bars run as fill times
    uint32_t const HalondrusEnergyFillMs = 75000;
    uint32_t const LihuvimSynthesizeFillMs = 70000;
    uint32_t const DausegneSiphonFillMs = 70000;
    uint32_t const MalganisDarknessFillMs = 85000;
    uint32_t const KintessaInfiltrationFillMs = 120000;
    uint32_t const RygelonBangFillMs = 100000;

    int32_t const GuardianRollOutPct = 40;
    int32_t const GuardianSelfDestructPct = 15;

    int32_t const HalondrusRelocationMarks[2] = { 80, 50 };
    uint8_t const HalondrusRelocationCount = 2;
    uint32_t const HalondrusRelocationMs = 40000;
    uint32_t const HalondrusMoteTravelMs = 18000;

    int32_t const JailerStageTwoPct = 70;
    int32_t const JailerStageThreePct = 40;

    uint8_t const DausegneReservoirCount = 4;

    int32_t const PantheonRetreatPct = 40;

    int32_t const XymoxDecipherMarks[2] = { 75, 50 };
    uint8_t const XymoxDecipherCount = 2;
    uint32_t const XymoxDecipherMaxMs = 90000;
    uint32_t const XymoxSupernovaMs = 12000;

    uint32_t const RygelonSingularityMs = 40000;

    // Anduin's stages, the intermissions sit in between
    uint32_t const AnduinStageOne = 1;
    uint32_t const AnduinFirstIntermission = 2;
    uint32_t const AnduinStageTwo = 3;
    uint32_t const AnduinSecondIntermission = 4;
    uint32_t const AnduinStageThree = 5;

    uint32_t const AnduinFirstIntermissionMs = 150000;
    uint32_t const AnduinSecondIntermissionMs = 169000;
    uint32_t const RemnantReturnMs = 67500;
    int32_t const AnduinFinalStagePct = 10;

    uint8_t const MarchLaneCount = 8;
    float const MarchRimRange = 38.0f;
    uint32_t const MarchLifetimeMs = 28000;
    uint32_t const MarchPulseMs = 2000;

    // Domination's Grasp, the platform Anduin fights on
    LocationVector const DominationGraspCentre(-3825.06f, -2715.46f, 91.36f, 1.626f);

    uint32_t const LordsOfDreadMembers[] = { BOSS_MALGANIS, BOSS_KINTESSA };
    uint32_t const PantheonMembers[] = { BOSS_PROTOTYPE_OF_WAR, BOSS_PROTOTYPE_OF_DUTY, BOSS_PROTOTYPE_OF_RENEWAL, BOSS_PROTOTYPE_OF_ABSOLUTION };

    // automa synthesized per wave, 0 ends a wave
    uint32_t const LihuvimAutomaWaves[3][3] =
    {
        { NPC_ACQUISITIONS_AUTOMA, NPC_DEGENERATION_AUTOMA, 0 },
        { NPC_GUARDIAN_AUTOMA, NPC_DEFENSE_MATRIX_AUTOMA, 0 },
        { NPC_ACQUISITIONS_AUTOMA, NPC_GUARDIAN_AUTOMA, NPC_DEGENERATION_AUTOMA }
    };

    enum class SepulcherAim : uint8_t
    {
        Victim,
        RandomPlayer,
        Area,
        Self
    };

    // a zero repeat casts once
    struct SepulcherTimedCast
    {
        uint32_t spellId;
        SepulcherAim aim;
        uint32_t firstMs;
        uint32_t repeatMs;
    };

    std::vector<SepulcherTimedCast> const SentryCasts =
    {
        { SPELL_SENTRY_DISSONANCE,              SepulcherAim::Victim,       5000,   10000 },
        { SPELL_SENTRY_WAVE_OF_DISINTEGRATION,  SepulcherAim::Area,         12000,  18000 }
    };

    std::vector<SepulcherTimedCast> const MateriumCasts =
    {
        { SPELL_MATERIUM_ADAPTIVE_SUBROUTINES,  SepulcherAim::Self,         1000,   0 }
    };

    std::vector<SepulcherTimedCast> const DroneCasts =
    {
        { SPELL_DRONE_BLAST,                    SepulcherAim::RandomPlayer, 2000,   5000 },
        { SPELL_DRONE_SEARING_ABLATION,         SepulcherAim::RandomPlayer, 12000,  20000 }
    };

    std::vector<SepulcherTimedCast> const DefenseMatrixCasts =
    {
        { SPELL_MATRIX_FORCE_FIELD,             SepulcherAim::Self,         2000,   0 }
    };

    std::vector<SepulcherTimedCast> const IncarnationCasts =
    {
        { SPELL_INCARNATION_CRY_OF_LOATHING,    SepulcherAim::Area,         4000,   10000 }
    };

    std::vector<SepulcherTimedCast> const DegenerationAutomaCasts =
    {
        { SPELL_DEGENERATION_DEGENERATE,        SepulcherAim::RandomPlayer, 4000,   12000 }
    };

    std::vector<SepulcherTimedCast> const AcquisitionsAutomaCasts =
    {
        { SPELL_ACQUISITIONS_INSTABILITY,       SepulcherAim::Self,         1000,   0 },
        { SPELL_ACQUISITIONS_DETONATION,        SepulcherAim::Area,         15000,  20000 }
    };

    std::vector<SepulcherTimedCast> const GuardianAutomaCasts =
    {
        { SPELL_AUTOMA_SENTRY_FIRE,             SepulcherAim::RandomPlayer, 3000,   8000 },
        { SPELL_AUTOMA_FORM_SENTRY,             SepulcherAim::Self,         5000,   20000 }
    };

    std::vector<SepulcherTimedCast> const DefenseMatrixAutomaCasts =
    {
        { SPELL_DEFENSE_EPHEMERAL_BARRIER,      SepulcherAim::Self,         3000,   20000 }
    };

    std::vector<SepulcherTimedCast> const DominationCoreCasts =
    {
        { SPELL_CORE_DOMINATION_BOLT,           SepulcherAim::RandomPlayer, 2000,   5000 },
        { SPELL_CORE_ENCROACHING_DOMINION,      SepulcherAim::Area,         8000,   15000 }
    };

    std::vector<SepulcherTimedCast> const InchoateShadowCasts =
    {
        { SPELL_SHADOW_RAVENOUS_HUNGER,         SepulcherAim::Area,         5000,   12000 }
    };

    std::vector<SepulcherTimedCast> const FiendishSoulCasts =
    {
        { SPELL_FIENDISH_CLAWS_LEAP,            SepulcherAim::RandomPlayer, 1000,   10000 },
        { SPELL_FIENDISH_NECROTIC_CLAWS,        SepulcherAim::Victim,       10000,  7000 }
    };

    std::vector<SepulcherTimedCast> const GrimReflectionCasts =
    {
        { SPELL_REFLECTION_PSYCHIC_TERROR,      SepulcherAim::Area,         1000,   3000 }
    };

    std::vector<SepulcherTimedCast> const UnstableCoreCasts =
    {
        { SPELL_UNSTABLE_CORE_STELLAR_DECAY,    SepulcherAim::RandomPlayer, 3000,   8000 }
    };

    std::vector<SepulcherTimedCast> const XyAcolyteCasts =
    {
        { SPELL_ACOLYTE_MASSIVE_BLAST,          SepulcherAim::Victim,       5000,   12000 },
        { SPELL_ACOLYTE_HYPERLIGHT_ASCENSION,   SepulcherAim::Self,         15000,  25000 }
    };

    std::vector<SepulcherTimedCast> const XySpellslingerCasts =
    {
        { SPELL_SPELLSLINGER_DEBILITATING_RAY,  SepulcherAim::RandomPlayer, 3000,   8000 }
    };

    std::vector<SepulcherTimedCast> const CartelPlundererCasts =
    {
        { SPELL_PLUNDERER_ANCIENT_EXHAUST,      SepulcherAim::Area,         5000,   15000 },
        { SPELL_PLUNDERER_RELIC_COLLAPSE,       SepulcherAim::Area,         20000,  30000 }
    };

    std::vector<SepulcherTimedCast> const CartelOverseerCasts =
    {
        { SPELL_OVERSEER_MASSIVE_BLAST,         SepulcherAim::Victim,       4000,   12000 },
        { SPELL_OVERSEER_ROTARY_BODY_ARMOR,     SepulcherAim::Self,         10000,  30000 }
    };

    // random point on a ring around origin
    LocationVector sepulcherRingPoint(LocationVector const& origin, float minRange, float maxRange)
    {
        float const angle = Util::getRandomFloat(SepulcherTwoPi);
        float const range = Util::getRandomFloat(minRange, maxRange);
        return LocationVector(origin.x + range * std::cos(angle), origin.y + range * std::sin(angle), origin.z, angle);
    }

    // point on Domination's Grasp facing the middle of the platform
    LocationVector graspPoint(float angle, float range)
    {
        float const facing = std::fmod(angle + SepulcherTwoPi / 2.0f, SepulcherTwoPi);
        return LocationVector(DominationGraspCentre.x + range * std::cos(angle), DominationGraspCentre.y + range * std::sin(angle), DominationGraspCentre.z, facing);
    }

    std::vector<uint32_t> groupEntries(uint32_t dataId)
    {
        if (dataId == DATA_LORDS_OF_DREAD)
            return std::vector<uint32_t>(std::begin(LordsOfDreadMembers), std::end(LordsOfDreadMembers));

        if (dataId == DATA_PROTOTYPE_PANTHEON)
            return std::vector<uint32_t>(std::begin(PantheonMembers), std::end(PantheonMembers));

        return {};
    }

    // counts down and reports the moment it runs out
    bool countDown(uint32_t diff, uint32_t& leftMs)
    {
        if (leftMs == 0)
            return false;

        if (leftMs > diff)
        {
            leftMs -= diff;
            return false;
        }

        leftMs = 0;
        return true;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class SepulcherOfTheFirstOnesInstanceScript : public InstanceScript
{
public:
    explicit SepulcherOfTheFirstOnesInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(SepulcherOfTheFirstOnesEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new SepulcherOfTheFirstOnesInstanceScript(pMapMgr); }

    // a fallen member must not end a group encounter while another one stands
    bool setBossState(uint32_t id, EncounterStates state) override
    {
        if (state == EncounterStates::Performed && getBossState(id) == EncounterStates::InProgress && !isGroupDown(id, nullptr))
            return false;

        return InstanceScript::setBossState(id, state);
    }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        switch (pGameObject->getEntry())
        {
            case GO_BRIDGE_TO_ANDUIN:
                if (mBridgeToAnduinGuids.insert(pGameObject->GetNewGUID()).second)
                    updateBridges();
                break;
            case GO_BRIDGE_AFTER_ANDUIN:
                if (mBridgeAfterAnduinGuids.insert(pGameObject->GetNewGUID()).second)
                    updateBridges();
                break;
            default:
                break;
        }
    }

    void OnEncounterStateChange(uint32_t id, uint32_t state) override
    {
        if (id == DATA_ANDUIN_WRYNN)
            updateBridges();

        if (id == DATA_PROTOTYPE_PANTHEON && state != EncounterStates::InProgress)
            mPantheonStage = 0;

        if (state != EncounterStates::Failed)
            return;

        // a wiped group fights again in full
        for (Creature* pMember : getGroup(id))
        {
            if (!pMember->isAlive())
                pMember->despawn(1000, 5000);
            else if (pMember->GetScript() != nullptr)
                pMember->GetScript()->DoAction(ACTION_PANTHEON_RESTORE);
        }
    }

    std::vector<Creature*> getGroup(uint32_t dataId)
    {
        std::vector<Creature*> group;
        for (uint32_t entry : groupEntries(dataId))
        {
            for (Creature* pMember : getCreatureSetForEntry(entry))
                group.push_back(pMember);
        }

        return group;
    }

    // single boss encounters are down with their boss
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
    void rallyGroup(uint32_t dataId, CreatureAIScript* pPuller)
    {
        if (mRallying)
            return;

        mRallying = true;
        for (Creature* pMember : getGroup(dataId))
        {
            if (pMember == pPuller->getCreature() || !pMember->isAlive() || pMember->isInCombat())
                continue;

            pPuller->setZoneWideCombat(pMember);
        }
        mRallying = false;
    }

    void notifyLiving(uint32_t entry, int32_t action)
    {
        for (Creature* pCreature : getCreatureSetForEntry(entry))
        {
            if (pCreature->isAlive() && pCreature->GetScript() != nullptr)
                pCreature->GetScript()->DoAction(action);
        }
    }

    // Prototype Pantheon: 1 War and Duty, 2 Renewal and Absolution, 3 all of them
    uint8_t getPantheonStage() const { return mPantheonStage; }

    void beginPantheon()
    {
        if (mPantheonStage == 0)
            mPantheonStage = 1;
    }

    void advancePantheon(uint8_t stage)
    {
        if (stage <= mPantheonStage)
            return;

        mPantheonStage = stage;
        for (Creature* pMember : getGroup(DATA_PROTOTYPE_PANTHEON))
        {
            if (pMember->isAlive() && pMember->GetScript() != nullptr)
                pMember->GetScript()->DoAction(ACTION_PANTHEON_STAGE);
        }
    }

    bool isPantheonFighting(Creature* pExcept)
    {
        if (mPantheonStage == 0)
            return false;

        for (Creature* pMember : getGroup(DATA_PROTOTYPE_PANTHEON))
        {
            if (pMember != pExcept && pMember->isAlive() && pMember->isInCombat())
                return true;
        }

        return false;
    }

private:
    // a bridge of trapdoor type is gone while open
    void updateBridges()
    {
        EncounterStates const state = getBossState(DATA_ANDUIN_WRYNN);

        for (WoWGuid const& guid : mBridgeToAnduinGuids)
        {
            if (GameObject* pBridge = getGameObjectByGuid(guid))
                setOpen(pBridge, state == EncounterStates::InProgress);
        }

        for (WoWGuid const& guid : mBridgeAfterAnduinGuids)
        {
            if (GameObject* pBridge = getGameObjectByGuid(guid))
                setOpen(pBridge, state != EncounterStates::Performed);
        }
    }

    void setOpen(GameObject* pGameObject, bool open)
    {
        uint8_t const goState = open ? GO_STATE_OPEN : GO_STATE_CLOSED;
        if (pGameObject->getState() != goState)
            pGameObject->setState(goState);
    }

    std::set<WoWGuid> mBridgeToAnduinGuids;
    std::set<WoWGuid> mBridgeAfterAnduinGuids;
    uint8_t mPantheonStage = 0;
    bool mRallying = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event tick, berserk and target helpers

class SepulcherCombatAI : public CreatureAIScript
{
public:
    explicit SepulcherCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

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

        if (countDown(diff, mBerserkLeftMs))
            castSpellOnSelf(mBerserkSpellId, true);

        onCombatTick(diff);

        // due events wait while a cast is running
        while (!_isCasting())
        {
            uint32_t const eventId = scriptEvents.getFinishedEvent();
            if (eventId == 0)
                break;

            onEvent(eventId);
        }
    }

protected:
    virtual void onEngage() {}
    virtual void onDisengage() {}
    virtual void onCombatTick(uint32_t /*diff*/) {}
    virtual void onEvent(uint32_t /*eventId*/) {}

    SepulcherOfTheFirstOnesInstanceScript* getSepulcher()
    {
        if (getCreature()->GetMapId() != MAP_SEPULCHER_OF_THE_FIRST_ONES)
            return nullptr;

        return static_cast<SepulcherOfTheFirstOnesInstanceScript*>(getInstanceScript());
    }

    // distinct random players, the tank only when nobody else is left
    std::vector<Unit*> pickPlayers(size_t count, bool skipTank)
    {
        TargetFilter const filter = skipTank ? TargetFilter(TargetFilter_Player | TargetFilter_NotCurrent) : TargetFilter_Player;
        FilterArgs const args(filter);

        std::vector<Unit*> pool;
        for (Object* pObject : getCreature()->getInRangePlayersSet())
        {
            if (pObject != nullptr && isValidUnitTarget(pObject, filter, args))
                pool.push_back(static_cast<Unit*>(pObject));
        }

        if (pool.empty() && skipTank)
            return pickPlayers(count, false);

        for (size_t i = 0; i < pool.size() && i < count; ++i)
        {
            size_t const pick = i + Util::getRandomUInt(static_cast<uint32_t>(pool.size() - 1 - i));
            std::swap(pool[i], pool[pick]);
        }

        if (pool.size() > count)
            pool.resize(count);

        return pool;
    }

    Unit* castAtPlayer(uint32_t spellId, bool skipTank, bool triggered = false)
    {
        std::vector<Unit*> const picked = pickPlayers(1, skipTank);
        if (picked.empty())
            return nullptr;

        castSpell(picked.front(), spellId, triggered);
        return picked.front();
    }

    void castAtPlayers(uint32_t spellId, size_t count, bool skipTank)
    {
        for (Unit* pTarget : pickPlayers(count, skipTank))
            castSpell(pTarget, spellId, true);
    }

    // summoned add that joins the fight at once
    Creature* callAdd(uint32_t entry, LocationVector const& spot)
    {
        Creature* pAdd = summonCreature(entry, spot, CORPSE_DESPAWN);
        if (pAdd != nullptr)
            setZoneWideCombat(pAdd);

        return pAdd;
    }

    // summoned object the raid has to deal with, it never attacks
    Creature* placeObject(uint32_t entry, LocationVector const& spot)
    {
        return summonCreature(entry, spot, CORPSE_DESPAWN);
    }

    LocationVector aroundSelf(float minRange, float maxRange)
    {
        return sepulcherRingPoint(getCreature()->GetPosition(), minRange, maxRange);
    }

    void warn(char const* text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }

    void armBerserk(uint32_t delayMs, uint32_t spellId = SPELL_SEPULCHER_BERSERK)
    {
        mBerserkLeftMs = delayMs;
        mBerserkSpellId = spellId;
    }

    // a new stage drops every pending event
    void enterStage(uint32_t stage)
    {
        mStage = stage;
        scriptEvents.resetEvents();
    }

    uint32_t mStage = 0;

private:
    uint32_t mBerserkLeftMs = 0;
    uint32_t mBerserkSpellId = SPELL_SEPULCHER_BERSERK;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame, boss state and group pull

class SepulcherBossAI : public SepulcherCombatAI
{
public:
    SepulcherBossAI(Creature* pCreature, uint32_t dataId) : SepulcherCombatAI(pCreature), mDataId(dataId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mDataId, EncounterStates::InProgress);
        }

        if (SepulcherOfTheFirstOnesInstanceScript* pSepulcher = getSepulcher())
            pSepulcher->rallyGroup(mDataId, this);

        SepulcherCombatAI::OnCombatStart(pTarget);
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

        SepulcherCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

            SepulcherOfTheFirstOnesInstanceScript* pSepulcher = getSepulcher();
            if (pSepulcher == nullptr || pSepulcher->isGroupDown(mDataId, getCreature()))
                pInstance->setBossState(mDataId, EncounterStates::Performed);
        }

        scriptEvents.resetEvents();
        summons.despawnAll();
    }

protected:
    uint32_t mDataId;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Adds driven by a cast table

class SepulcherAddAI : public SepulcherCombatAI
{
public:
    SepulcherAddAI(Creature* pCreature, std::vector<SepulcherTimedCast> const& casts, bool rooted)
        : SepulcherCombatAI(pCreature), mCasts(casts), mRooted(rooted) {}

protected:
    void onEngage() override
    {
        if (mRooted)
            setRooted(true);

        for (size_t i = 0; i < mCasts.size(); ++i)
            scriptEvents.addEvent(EVENT_SEPULCHER_ADD_CAST_FIRST + static_cast<uint32_t>(i), mCasts[i].firstMs);
    }

    void onDisengage() override
    {
        if (mRooted)
            setRooted(false);
    }

    void onEvent(uint32_t eventId) override
    {
        if (eventId < EVENT_SEPULCHER_ADD_CAST_FIRST)
            return;

        size_t const slot = eventId - EVENT_SEPULCHER_ADD_CAST_FIRST;
        if (slot >= mCasts.size())
            return;

        SepulcherTimedCast const& cast = mCasts[slot];
        switch (cast.aim)
        {
            case SepulcherAim::Victim:
                castSpellOnVictim(cast.spellId);
                break;
            case SepulcherAim::RandomPlayer:
                castAtPlayer(cast.spellId, false);
                break;
            case SepulcherAim::Area:
                castSpellAOE(cast.spellId);
                break;
            case SepulcherAim::Self:
                castSpellOnSelf(cast.spellId, true);
                break;
        }

        if (cast.repeatMs != 0)
            scriptEvents.addEvent(eventId, cast.repeatMs);
    }

private:
    std::vector<SepulcherTimedCast> const& mCasts;
    bool mRooted;
};

// objects of a fight that only wait to be destroyed
class SepulcherObjectAI : public CreatureAIScript
{
public:
    SepulcherObjectAI(Creature* pCreature, bool rooted) : CreatureAIScript(pCreature), mRooted(rooted) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        if (mRooted)
            setRooted(true);
    }

private:
    bool mRooted;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Vigilant Guardian

class SepulcherVigilantGuardianAI : public SepulcherBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SepulcherVigilantGuardianAI(c); }
    explicit SepulcherVigilantGuardianAI(Creature* pCreature) : SepulcherBossAI(pCreature, DATA_VIGILANT_GUARDIAN) {}

protected:
    void onEngage() override
    {
        mSentryWave = 0;
        mAutomaWave = 0;
        armBerserk(GuardianBerserkMs);
        scriptEvents.addEvent(EVENT_SEPULCHER_GUARDIAN_REFRACTED_BLAST, 8000);
        scriptEvents.addEvent(EVENT_SEPULCHER_GUARDIAN_SENTRIES, 10000);
        scriptEvents.addEvent(EVENT_SEPULCHER_GUARDIAN_AUTOMA, 20000);
        scriptEvents.addEvent(EVENT_SEPULCHER_GUARDIAN_DERESOLUTION, 25000);
        scriptEvents.addEvent(EVENT_SEPULCHER_GUARDIAN_EXPOSED_CORE, 45000);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        int32_t const health = _getHealthPercent();
        if (mStage == 1 && health <= GuardianRollOutPct)
            rollOut();
        else if (mStage == 2 && health <= GuardianSelfDestructPct)
            selfDestruct();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SEPULCHER_GUARDIAN_REFRACTED_BLAST:
                castAtPlayer(SPELL_GUARDIAN_REFRACTED_BLAST, true);
                scriptEvents.addEvent(eventId, 12000);
                break;
            case EVENT_SEPULCHER_GUARDIAN_DERESOLUTION:
                if (castAtPlayer(SPELL_GUARDIAN_DERESOLUTION, true) != nullptr)
                    warn("The Vigilant Guardian fires a Deresolution beam!");
                scriptEvents.addEvent(eventId, 35000);
                break;
            case EVENT_SEPULCHER_GUARDIAN_EXPOSED_CORE:
                warn("The Vigilant Guardian exposes its core! Take shelter in a Force Field!");
                castSpellAOE(SPELL_GUARDIAN_EXPOSED_CORE);
                scriptEvents.addEvent(eventId, 60000);
                break;
            case EVENT_SEPULCHER_GUARDIAN_SENTRIES:
                awakenSentries();
                scriptEvents.addEvent(eventId, 40000);
                break;
            case EVENT_SEPULCHER_GUARDIAN_AUTOMA:
                powerUpAutoma();
                scriptEvents.addEvent(eventId, 50000);
                break;
            case EVENT_SEPULCHER_GUARDIAN_PNEUMATIC_IMPACT:
                castSpellOnVictim(SPELL_GUARDIAN_PNEUMATIC_IMPACT);
                scriptEvents.addEvent(eventId, 14000);
                break;
            case EVENT_SEPULCHER_GUARDIAN_MATTER_DISSOLUTION:
                castAtPlayers(SPELL_GUARDIAN_MATTER_DISSOLUTION, 2, true);
                scriptEvents.addEvent(eventId, 22000);
                break;
            case EVENT_SEPULCHER_GUARDIAN_SPLIT_RESOLUTION:
                castAtPlayer(SPELL_GUARDIAN_SPLIT_RESOLUTION, true);
                scriptEvents.addEvent(eventId, 28000);
                break;
            case EVENT_SEPULCHER_GUARDIAN_FRACTURED_CORE:
                castSpellAOE(SPELL_GUARDIAN_FRACTURED_CORE);
                scriptEvents.addEvent(eventId, 35000);
                break;
            case EVENT_SEPULCHER_GUARDIAN_CORE_OVERLOAD:
                castSpellAOE(SPELL_GUARDIAN_CORE_OVERLOAD, true);
                scriptEvents.addEvent(eventId, 20000);
                break;
            default:
                break;
        }
    }

private:
    // battle mode from 40%
    void rollOut()
    {
        enterStage(2);
        warn("The Vigilant Guardian rolls forward and transforms into its battle mode!");
        scriptEvents.addEvent(EVENT_SEPULCHER_GUARDIAN_PNEUMATIC_IMPACT, 4000);
        scriptEvents.addEvent(EVENT_SEPULCHER_GUARDIAN_REFRACTED_BLAST, 6000);
        scriptEvents.addEvent(EVENT_SEPULCHER_GUARDIAN_MATTER_DISSOLUTION, 10000);
        scriptEvents.addEvent(EVENT_SEPULCHER_GUARDIAN_SENTRIES, 15000);
        scriptEvents.addEvent(EVENT_SEPULCHER_GUARDIAN_SPLIT_RESOLUTION, 18000);
        scriptEvents.addEvent(EVENT_SEPULCHER_GUARDIAN_FRACTURED_CORE, 25000);
        scriptEvents.addEvent(EVENT_SEPULCHER_GUARDIAN_AUTOMA, 30000);
    }

    // the battle mode casts go on next to the overload
    void selfDestruct()
    {
        mStage = 3;
        warn("The Vigilant Guardian activates its self-destruct sequence!");
        scriptEvents.addEvent(EVENT_SEPULCHER_GUARDIAN_CORE_OVERLOAD, 1000);
    }

    // sentries and volatile materium take turns
    void awakenSentries()
    {
        if (mSentryWave++ % 2 == 0)
        {
            warn("The Vigilant Guardian awakens a Pre-Fabricated Sentry!");
            callAdd(NPC_PRE_FABRICATED_SENTRY, aroundSelf(10.0f, 20.0f));
            return;
        }

        for (uint8_t i = 0; i < 3; ++i)
            callAdd(NPC_VOLATILE_MATERIUM, aroundSelf(8.0f, 18.0f));
    }

    // point defense drones and defense matrixes take turns
    void powerUpAutoma()
    {
        uint32_t const entry = (mAutomaWave++ % 2 == 0) ? NPC_POINT_DEFENSE_DRONE : NPC_AUTOMATED_DEFENSE_MATRIX;
        callAdd(entry, aroundSelf(15.0f, 25.0f));
    }

    uint32_t mSentryWave = 0;
    uint32_t mAutomaWave = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Halondrus the Reclaimer

class SepulcherHalondrusAI : public SepulcherBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SepulcherHalondrusAI(c); }
    explicit SepulcherHalondrusAI(Creature* pCreature) : SepulcherBossAI(pCreature, DATA_HALONDRUS) {}

protected:
    void onEngage() override
    {
        mRelocations = 0;
        mRelocating = false;
        armBerserk(SepulcherBerserkMs);
        castSpellOnSelf(SPELL_HALONDRUS_RECLAMATION_FORM, true);
        startReclamation();
    }

    void onDisengage() override
    {
        mRelocating = false;
        _removeAura(SPELL_HALONDRUS_RELOCATION_FORM);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mRelocating || mRelocations >= HalondrusRelocationCount)
            return;

        if (_getHealthPercent() <= HalondrusRelocationMarks[mRelocations])
            relocate();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SEPULCHER_HALONDRUS_TREMORS:
                castSpellAOE(SPELL_HALONDRUS_SEISMIC_TREMORS);
                releaseMotes();
                scriptEvents.addEvent(eventId, 26000);
                break;
            case EVENT_SEPULCHER_HALONDRUS_MOTES_ARRIVE:
                motesArrive();
                break;
            case EVENT_SEPULCHER_HALONDRUS_EARTHBREAKER:
                castSpellAOE(SPELL_HALONDRUS_EARTHBREAKER);
                scriptEvents.addEvent(eventId, mRelocating ? 20000 : 28000);
                break;
            case EVENT_SEPULCHER_HALONDRUS_LIGHTSHATTER:
                castSpellOnVictim(SPELL_HALONDRUS_LIGHTSHATTER_BEAM);
                scriptEvents.addEvent(eventId, 16000);
                break;
            case EVENT_SEPULCHER_HALONDRUS_PLANETCRACKER:
                warn("Halondrus charges a Planetcracker Beam!");
                castSpellAOE(SPELL_HALONDRUS_PLANETCRACKER_BEAM);
                scriptEvents.addEvent(eventId, 40000);
                break;
            case EVENT_SEPULCHER_HALONDRUS_CRUSHING_PRISM:
                castAtPlayers(SPELL_HALONDRUS_CRUSHING_PRISM, 3, true);
                scriptEvents.addEvent(eventId, mRelocating ? 25000 : 35000);
                break;
            case EVENT_SEPULCHER_HALONDRUS_LANCE:
                castAtPlayer(SPELL_HALONDRUS_LANCE, true);
                scriptEvents.addEvent(eventId, 25000);
                break;
            case EVENT_SEPULCHER_HALONDRUS_FULL_ENERGY:
                unleashEnergy();
                scriptEvents.addEvent(eventId, HalondrusEnergyFillMs);
                break;
            case EVENT_SEPULCHER_HALONDRUS_QUAKING_STEPS:
                castSpellAOE(SPELL_HALONDRUS_QUAKING_STEPS, true);
                scriptEvents.addEvent(eventId, 6000);
                break;
            case EVENT_SEPULCHER_HALONDRUS_SHATTER:
                castSpellAOE(SPELL_HALONDRUS_SHATTER);
                scriptEvents.addEvent(eventId, 15000);
                break;
            case EVENT_SEPULCHER_HALONDRUS_SETTLED:
                settle();
                break;
            default:
                break;
        }
    }

private:
    // stage one before the cliffs, stage three after the second relocation
    void startReclamation()
    {
        bool const overdrive = mRelocations >= HalondrusRelocationCount;
        enterStage(overdrive ? 3 : 1);
        scriptEvents.addEvent(EVENT_SEPULCHER_HALONDRUS_LIGHTSHATTER, 6000);
        scriptEvents.addEvent(EVENT_SEPULCHER_HALONDRUS_TREMORS, 8000);
        scriptEvents.addEvent(EVENT_SEPULCHER_HALONDRUS_EARTHBREAKER, 14000);
        scriptEvents.addEvent(EVENT_SEPULCHER_HALONDRUS_PLANETCRACKER, 30000);
        scriptEvents.addEvent(EVENT_SEPULCHER_HALONDRUS_FULL_ENERGY, HalondrusEnergyFillMs);
        if (overdrive)
            return;

        scriptEvents.addEvent(EVENT_SEPULCHER_HALONDRUS_CRUSHING_PRISM, 20000);
        scriptEvents.addEvent(EVENT_SEPULCHER_HALONDRUS_LANCE, 24000);
    }

    // Relocation Form at 80% and 50%
    void relocate()
    {
        mRelocating = true;
        ++mRelocations;
        enterStage(2);
        warn("Halondrus shifts into Relocation Form and crosses the Shimmering Cliffs!");
        _removeAura(SPELL_HALONDRUS_RECLAMATION_FORM);
        castSpellOnSelf(SPELL_HALONDRUS_RELOCATION_FORM, true);
        scriptEvents.addEvent(EVENT_SEPULCHER_HALONDRUS_QUAKING_STEPS, 2000);
        scriptEvents.addEvent(EVENT_SEPULCHER_HALONDRUS_SHATTER, 8000);
        scriptEvents.addEvent(EVENT_SEPULCHER_HALONDRUS_EARTHBREAKER, 12000);
        scriptEvents.addEvent(EVENT_SEPULCHER_HALONDRUS_CRUSHING_PRISM, 16000);
        scriptEvents.addEvent(EVENT_SEPULCHER_HALONDRUS_LANCE, 20000);
        scriptEvents.addEvent(EVENT_SEPULCHER_HALONDRUS_SETTLED, HalondrusRelocationMs);
    }

    void settle()
    {
        mRelocating = false;
        _removeAura(SPELL_HALONDRUS_RELOCATION_FORM);
        castSpellOnSelf(SPELL_HALONDRUS_RECLAMATION_FORM, true);
        if (mRelocations >= HalondrusRelocationCount)
            warn("Halondrus drives the Eternity Engine into overdrive!");

        startReclamation();
    }

    // Reclaim in stage one, Eternity Overdrive in stage three
    void unleashEnergy()
    {
        if (mStage == 3)
        {
            warn("Halondrus unleashes Eternity Overdrive!");
            castSpellAOE(SPELL_HALONDRUS_ETERNITY_OVERDRIVE);
            return;
        }

        warn("Halondrus begins to Reclaim the ephemeral motes!");
        castSpellAOE(SPELL_HALONDRUS_RECLAIM);
        motesArrive();
    }

    void releaseMotes()
    {
        for (uint8_t i = 0; i < 3; ++i)
            placeObject(NPC_EPHEMERAL_MOTE, aroundSelf(15.0f, 25.0f));

        scriptEvents.addEvent(EVENT_SEPULCHER_HALONDRUS_MOTES_ARRIVE, HalondrusMoteTravelMs);
    }

    // every mote left alive reaches Halondrus and erupts
    void motesArrive()
    {
        InstanceScript* pInstance = getInstanceScript();
        if (pInstance == nullptr)
            return;

        uint8_t arrived = 0;
        for (Creature* pMote : pInstance->getCreatureSetForEntry(NPC_EPHEMERAL_MOTE))
        {
            if (!pMote->isAlive())
                continue;

            pMote->despawn(100, 0);
            ++arrived;
        }

        if (arrived == 0)
            return;

        warn("Ephemeral Motes reach Halondrus and erupt!");
        for (uint8_t i = 0; i < arrived; ++i)
            castSpellAOE(SPELL_HALONDRUS_EPHEMERAL_ERUPTION, true);
    }

    uint8_t mRelocations = 0;
    bool mRelocating = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// The Jailer

class SepulcherJailerAI : public SepulcherBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SepulcherJailerAI(c); }
    explicit SepulcherJailerAI(Creature* pCreature) : SepulcherBossAI(pCreature, DATA_THE_JAILER) {}

protected:
    void onEngage() override
    {
        armBerserk(SepulcherBerserkMs);
        scriptEvents.addEvent(EVENT_SEPULCHER_JAILER_MARTYRDOM, 8000);
        scriptEvents.addEvent(EVENT_SEPULCHER_JAILER_OPPRESSION, 12000);
        scriptEvents.addEvent(EVENT_SEPULCHER_JAILER_TORMENT, 18000);
        scriptEvents.addEvent(EVENT_SEPULCHER_JAILER_DOMINATION, 25000);
        scriptEvents.addEvent(EVENT_SEPULCHER_JAILER_DAMNATION, 30000);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        int32_t const health = _getHealthPercent();
        if (mStage == 1 && health <= JailerStageTwoPct)
            startUnholyAttunement();
        else if (mStage == 2 && health <= JailerStageThreePct)
            startEternitysEnd();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SEPULCHER_JAILER_DOMINATION:
                warn("The Jailer asserts Relentless Domination! Break his line of sight!");
                castSpellAOE(SPELL_JAILER_RELENTLESS_DOMINATION);
                scriptEvents.addEvent(eventId, 55000);
                break;
            case EVENT_SEPULCHER_JAILER_OPPRESSION:
                castSpellAOE(SPELL_JAILER_CHAINS_OF_OPPRESSION);
                scriptEvents.addEvent(eventId, 30000);
                break;
            case EVENT_SEPULCHER_JAILER_MARTYRDOM:
                castSpellOnVictim(SPELL_JAILER_MARTYRDOM);
                scriptEvents.addEvent(eventId, 25000);
                break;
            case EVENT_SEPULCHER_JAILER_TORMENT:
                castSpellAOE(SPELL_JAILER_TORMENT);
                if (mStage == 3)
                {
                    warn("An Incarnation of Torment takes shape!");
                    callAdd(NPC_INCARNATION_OF_TORMENT, aroundSelf(10.0f, 20.0f));
                }
                scriptEvents.addEvent(eventId, mStage == 3 ? 35000 : 32000);
                break;
            case EVENT_SEPULCHER_JAILER_DAMNATION:
                castAtPlayers(SPELL_JAILER_RUNE_OF_DAMNATION, 3, true);
                scriptEvents.addEvent(eventId, 36000);
                break;
            case EVENT_SEPULCHER_JAILER_ATTUNEMENT:
                warn("The Jailer begins Unholy Attunement!");
                castSpellAOE(SPELL_JAILER_UNHOLY_ATTUNEMENT);
                scriptEvents.addEvent(eventId, 45000);
                break;
            case EVENT_SEPULCHER_JAILER_SHATTERING_BLAST:
                castSpellOnVictim(SPELL_JAILER_SHATTERING_BLAST);
                scriptEvents.addEvent(eventId, 30000);
                break;
            case EVENT_SEPULCHER_JAILER_COMPULSION:
                castAtPlayers(SPELL_JAILER_RUNE_OF_COMPULSION, 3, true);
                scriptEvents.addEvent(eventId, 40000);
                break;
            case EVENT_SEPULCHER_JAILER_DECIMATOR:
                castAtPlayer(SPELL_JAILER_DECIMATOR, true);
                scriptEvents.addEvent(eventId, 40000);
                break;
            case EVENT_SEPULCHER_JAILER_RUNE_OF_DOMINATION:
                castAtPlayers(SPELL_JAILER_RUNE_OF_DOMINATION, 2, true);
                scriptEvents.addEvent(eventId, 45000);
                break;
            case EVENT_SEPULCHER_JAILER_ANGUISH:
                castAtPlayers(SPELL_JAILER_CHAINS_OF_ANGUISH, 2, true);
                scriptEvents.addEvent(eventId, 40000);
                break;
            case EVENT_SEPULCHER_JAILER_DEFILE:
                castAtPlayer(SPELL_JAILER_DEFILE, true);
                scriptEvents.addEvent(eventId, 30000);
                break;
            default:
                break;
        }
    }

private:
    void startUnholyAttunement()
    {
        enterStage(2);
        warn("The Jailer tightens his grip on the Machine of Origination!");
        scriptEvents.addEvent(EVENT_SEPULCHER_JAILER_ATTUNEMENT, 6000);
        scriptEvents.addEvent(EVENT_SEPULCHER_JAILER_SHATTERING_BLAST, 12000);
        scriptEvents.addEvent(EVENT_SEPULCHER_JAILER_TORMENT, 16000);
        scriptEvents.addEvent(EVENT_SEPULCHER_JAILER_COMPULSION, 20000);
        scriptEvents.addEvent(EVENT_SEPULCHER_JAILER_DECIMATOR, 28000);
    }

    void startEternitysEnd()
    {
        enterStage(3);
        warn("The Jailer brings Eternity's End!");
        scriptEvents.addEvent(EVENT_SEPULCHER_JAILER_RUNE_OF_DOMINATION, 10000);
        scriptEvents.addEvent(EVENT_SEPULCHER_JAILER_TORMENT, 14000);
        scriptEvents.addEvent(EVENT_SEPULCHER_JAILER_ANGUISH, 18000);
        scriptEvents.addEvent(EVENT_SEPULCHER_JAILER_DECIMATOR, 25000);
        scriptEvents.addEvent(EVENT_SEPULCHER_JAILER_DEFILE, 30000);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Lihuvim, Principal Architect

class SepulcherLihuvimAI : public SepulcherBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SepulcherLihuvimAI(c); }
    explicit SepulcherLihuvimAI(Creature* pCreature) : SepulcherBossAI(pCreature, DATA_LIHUVIM) {}

protected:
    void onEngage() override
    {
        mWave = 0;
        armBerserk(LihuvimBerserkMs);
        scriptEvents.addEvent(EVENT_SEPULCHER_LIHUVIM_CASCADE, 6000);
        scriptEvents.addEvent(EVENT_SEPULCHER_LIHUVIM_RESONANCE, 10000);
        scriptEvents.addEvent(EVENT_SEPULCHER_LIHUVIM_DECONSTRUCT, 15000);
        scriptEvents.addEvent(EVENT_SEPULCHER_LIHUVIM_MOTES, 20000);
        scriptEvents.addEvent(EVENT_SEPULCHER_LIHUVIM_COSMIC_SHIFT, 25000);
        scriptEvents.addEvent(EVENT_SEPULCHER_LIHUVIM_SYNTHESIZE, LihuvimSynthesizeFillMs);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SEPULCHER_LIHUVIM_CASCADE:
                castSpellAOE(SPELL_LIHUVIM_PROTOFORM_CASCADE);
                scriptEvents.addEvent(eventId, 14000);
                break;
            case EVENT_SEPULCHER_LIHUVIM_RESONANCE:
                castSpellOnVictim(SPELL_LIHUVIM_RESONANCE);
                scriptEvents.addEvent(eventId, 20000);
                break;
            case EVENT_SEPULCHER_LIHUVIM_DECONSTRUCT:
                castSpellOnVictim(SPELL_LIHUVIM_DECONSTRUCTING_ENERGY);
                scriptEvents.addEvent(eventId, 35000);
                break;
            case EVENT_SEPULCHER_LIHUVIM_MOTES:
                castAtPlayers(SPELL_LIHUVIM_MANIFEST_MOTE, 2, true);
                scriptEvents.addEvent(eventId, 30000);
                break;
            case EVENT_SEPULCHER_LIHUVIM_COSMIC_SHIFT:
                warn("Lihuvim unleashes a Cosmic Shift!");
                castSpellAOE(SPELL_LIHUVIM_COSMIC_SHIFT);
                scriptEvents.addEvent(eventId, 30000);
                break;
            case EVENT_SEPULCHER_LIHUVIM_SYNTHESIZE:
                warn("Lihuvim begins to Synthesize Protoform Automa!");
                castSpellOnSelf(SPELL_LIHUVIM_SYNTHESIZE);
                scriptEvents.addEvent(EVENT_SEPULCHER_LIHUVIM_SYNTHESIS_DONE, 6000);
                scriptEvents.addEvent(eventId, LihuvimSynthesizeFillMs);
                break;
            case EVENT_SEPULCHER_LIHUVIM_SYNTHESIS_DONE:
                castSpellAOE(SPELL_LIHUVIM_PROTOFORM_RADIANCE, true);
                synthesizeAutoma();
                break;
            default:
                break;
        }
    }

private:
    // the lattices left over become automa
    void synthesizeAutoma()
    {
        uint32_t const* wave = LihuvimAutomaWaves[mWave++ % 3];
        for (uint8_t i = 0; i < 3 && wave[i] != 0; ++i)
            callAdd(wave[i], aroundSelf(12.0f, 22.0f));
    }

    uint32_t mWave = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Dausegne, the Fallen Oracle

class SepulcherDausegneAI : public SepulcherBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SepulcherDausegneAI(c); }
    explicit SepulcherDausegneAI(Creature* pCreature) : SepulcherBossAI(pCreature, DATA_DAUSEGNE) {}

protected:
    void onEngage() override
    {
        mReservoirs = 0;
        armBerserk(SepulcherBerserkMs);
        castSpellOnSelf(SPELL_DAUSEGNE_INFUSED_STRIKES, true);
        scriptEvents.addEvent(EVENT_SEPULCHER_DAUSEGNE_ARC, 8000);
        scriptEvents.addEvent(EVENT_SEPULCHER_DAUSEGNE_CORE, 12000);
        scriptEvents.addEvent(EVENT_SEPULCHER_DAUSEGNE_BARRAGE, 15000);
        scriptEvents.addEvent(EVENT_SEPULCHER_DAUSEGNE_HALO, 22000);
        scriptEvents.addEvent(EVENT_SEPULCHER_DAUSEGNE_SIPHON, DausegneSiphonFillMs);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SEPULCHER_DAUSEGNE_ARC:
                castSpellOnVictim(SPELL_DAUSEGNE_OBLITERATION_ARC);
                scriptEvents.addEvent(eventId, 22000);
                break;
            case EVENT_SEPULCHER_DAUSEGNE_BARRAGE:
                castAtPlayer(SPELL_DAUSEGNE_STAGGERING_BARRAGE, true);
                scriptEvents.addEvent(eventId, 28000);
                break;
            case EVENT_SEPULCHER_DAUSEGNE_HALO:
                warn("Dausegne releases a Disintegration Halo!");
                castSpellAOE(SPELL_DAUSEGNE_DISINTEGRATION_HALO);
                scriptEvents.addEvent(eventId, 35000);
                break;
            case EVENT_SEPULCHER_DAUSEGNE_CORE:
                callAdd(NPC_DOMINATION_CORE, aroundSelf(15.0f, 25.0f));
                scriptEvents.addEvent(eventId, 32000);
                break;
            case EVENT_SEPULCHER_DAUSEGNE_SIPHON:
                siphonReservoir();
                break;
            case EVENT_SEPULCHER_DAUSEGNE_BARRIER:
                castSpellOnSelf(SPELL_DAUSEGNE_SIPHONED_BARRIER, true);
                break;
            case EVENT_SEPULCHER_DAUSEGNE_DOMINION:
                warn("Dausegne channels Inevitable Dominion!");
                castSpellAOE(SPELL_DAUSEGNE_INEVITABLE_DOMINION);
                break;
            case EVENT_SEPULCHER_DAUSEGNE_TOTAL_DOMINION:
                warn("Every reservoir is dominated! Dausegne seizes Total Dominion!");
                castSpellOnSelf(SPELL_DAUSEGNE_TOTAL_DOMINION, true);
                break;
            default:
                break;
        }
    }

private:
    // out of mana she drains a reservoir, the last one ends in Total Dominion
    void siphonReservoir()
    {
        ++mReservoirs;
        warn("Dausegne drains a Reservoir of the First Ones!");
        castSpellOnSelf(SPELL_DAUSEGNE_SIPHON_RESERVOIR);
        scriptEvents.addEvent(EVENT_SEPULCHER_DAUSEGNE_BARRIER, 2000);
        scriptEvents.addEvent(EVENT_SEPULCHER_DAUSEGNE_DOMINION, 4000);

        if (mReservoirs < DausegneReservoirCount)
            scriptEvents.addEvent(EVENT_SEPULCHER_DAUSEGNE_SIPHON, DausegneSiphonFillMs);
        else
            scriptEvents.addEvent(EVENT_SEPULCHER_DAUSEGNE_TOTAL_DOMINION, 20000);
    }

    uint8_t mReservoirs = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Skolex, the Insatiable Ravener

class SepulcherSkolexAI : public SepulcherBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SepulcherSkolexAI(c); }
    explicit SepulcherSkolexAI(Creature* pCreature) : SepulcherBossAI(pCreature, DATA_SKOLEX) {}

protected:
    void onEngage() override
    {
        mRiftmawNext = true;
        armBerserk(SkolexDestroyMs, SPELL_SKOLEX_DESTROY);
        scriptEvents.addEvent(EVENT_SEPULCHER_SKOLEX_JAWS, 6000);
        scriptEvents.addEvent(EVENT_SEPULCHER_SKOLEX_DUST_FLAIL, 10000);
        scriptEvents.addEvent(EVENT_SEPULCHER_SKOLEX_DUST_BLAST, 14000);
        scriptEvents.addEvent(EVENT_SEPULCHER_SKOLEX_DEVOURING_BLOOD, 22000);
        scriptEvents.addEvent(EVENT_SEPULCHER_SKOLEX_RETCH, 30000);
        scriptEvents.addEvent(EVENT_SEPULCHER_SKOLEX_BURROW, 45000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SEPULCHER_SKOLEX_JAWS:
                // Riftmaw and Rend take turns on the tank
                castSpellOnVictim(mRiftmawNext ? SPELL_SKOLEX_RIFTMAW : SPELL_SKOLEX_REND);
                mRiftmawNext = !mRiftmawNext;
                scriptEvents.addEvent(eventId, 9000);
                break;
            case EVENT_SEPULCHER_SKOLEX_DUST_FLAIL:
                castSpellAOE(SPELL_SKOLEX_DUST_FLAIL);
                scriptEvents.addEvent(eventId, 18000);
                break;
            case EVENT_SEPULCHER_SKOLEX_DUST_BLAST:
                castAtPlayer(SPELL_SKOLEX_DUST_BLAST, true);
                scriptEvents.addEvent(eventId, 22000);
                break;
            case EVENT_SEPULCHER_SKOLEX_RETCH:
                warn("Skolex begins to Retch!");
                castSpellOnVictim(SPELL_SKOLEX_RETCH);
                scriptEvents.addEvent(eventId, 34000);
                break;
            case EVENT_SEPULCHER_SKOLEX_DEVOURING_BLOOD:
                castAtPlayers(SPELL_SKOLEX_DEVOURING_BLOOD, 2, true);
                scriptEvents.addEvent(eventId, 40000);
                break;
            case EVENT_SEPULCHER_SKOLEX_BURROW:
                warn("Skolex burrows toward a clump of players!");
                castAtPlayer(SPELL_SKOLEX_RAVENING_BURROW, true);
                scriptEvents.addEvent(EVENT_SEPULCHER_SKOLEX_RESURFACE, 4000);
                scriptEvents.addEvent(eventId, 70000);
                break;
            case EVENT_SEPULCHER_SKOLEX_RESURFACE:
                castSpellOnSelf(SPELL_SKOLEX_INSATIABLE, true);
                castSpellAOE(SPELL_SKOLEX_DUST_WAVE);
                break;
            default:
                break;
        }
    }

private:
    bool mRiftmawNext = true;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Lords of Dread

class SepulcherMalGanisAI : public SepulcherBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SepulcherMalGanisAI(c); }
    explicit SepulcherMalGanisAI(Creature* pCreature) : SepulcherBossAI(pCreature, DATA_LORDS_OF_DREAD) {}

protected:
    void onEngage() override
    {
        armBerserk(SepulcherBerserkMs);
        scriptEvents.addEvent(EVENT_SEPULCHER_MALGANIS_CLAWS, 6000);
        scriptEvents.addEvent(EVENT_SEPULCHER_MALGANIS_CARRION, 12000);
        scriptEvents.addEvent(EVENT_SEPULCHER_MALGANIS_SHADOWS, 25000);
        scriptEvents.addEvent(EVENT_SEPULCHER_MALGANIS_RAMPAGE, 40000);
        scriptEvents.addEvent(EVENT_SEPULCHER_MALGANIS_DARKNESS, MalganisDarknessFillMs);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SEPULCHER_MALGANIS_CLAWS:
                castSpellOnVictim(SPELL_MALGANIS_LEECHING_CLAWS);
                scriptEvents.addEvent(eventId, 14000);
                break;
            case EVENT_SEPULCHER_MALGANIS_CARRION:
                castAtPlayers(SPELL_MALGANIS_CLOUD_OF_CARRION, 2, true);
                scriptEvents.addEvent(eventId, 30000);
                break;
            case EVENT_SEPULCHER_MALGANIS_SHADOWS:
                warn("Mal'Ganis calls forth Inchoate Shadows!");
                castSpellOnSelf(SPELL_MALGANIS_MANIFEST_SHADOWS);
                for (uint8_t i = 0; i < 3; ++i)
                    callAdd(NPC_INCHOATE_SHADOW, aroundSelf(10.0f, 20.0f));
                scriptEvents.addEvent(eventId, 50000);
                break;
            case EVENT_SEPULCHER_MALGANIS_RAMPAGE:
                warn("The Lords of Dread unleash a Rampaging Swarm!");
                castSpellAOE(SPELL_LORDS_RAMPAGING_SWARM);
                tellKintessa(ACTION_LORDS_RAMPAGE);
                scriptEvents.addEvent(eventId, 60000);
                break;
            case EVENT_SEPULCHER_MALGANIS_DARKNESS:
                warn("Mal'Ganis and Kin'tessa dissolve into their swarm forms!");
                castSpellOnSelf(SPELL_MALGANIS_UNTO_DARKNESS);
                tellKintessa(ACTION_LORDS_SWARM);
                scriptEvents.addEvent(EVENT_SEPULCHER_MALGANIS_DECAY, 2000);
                scriptEvents.addEvent(eventId, MalganisDarknessFillMs);
                break;
            case EVENT_SEPULCHER_MALGANIS_DECAY:
                castSpellAOE(SPELL_MALGANIS_SWARM_OF_DECAY);
                break;
            default:
                break;
        }
    }

private:
    void tellKintessa(int32_t action)
    {
        if (SepulcherOfTheFirstOnesInstanceScript* pSepulcher = getSepulcher())
            pSepulcher->notifyLiving(BOSS_KINTESSA, action);
    }
};

class SepulcherKintessaAI : public SepulcherBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SepulcherKintessaAI(c); }
    explicit SepulcherKintessaAI(Creature* pCreature) : SepulcherBossAI(pCreature, DATA_LORDS_OF_DREAD) {}

    // Mal'Ganis leads the shared swarm casts
    void DoAction(int32_t action) override
    {
        if (!_isInCombat())
            return;

        if (action == ACTION_LORDS_SWARM)
            scriptEvents.addEvent(EVENT_SEPULCHER_KINTESSA_SWARM, 2000);
        else if (action == ACTION_LORDS_RAMPAGE)
            scriptEvents.addEvent(EVENT_SEPULCHER_KINTESSA_RAMPAGE, 500);
    }

protected:
    void onEngage() override
    {
        armBerserk(SepulcherBerserkMs);
        scriptEvents.addEvent(EVENT_SEPULCHER_KINTESSA_STRIKE, 5000);
        scriptEvents.addEvent(EVENT_SEPULCHER_KINTESSA_TREPIDATION, 15000);
        scriptEvents.addEvent(EVENT_SEPULCHER_KINTESSA_SLUMBER, 20000);
        scriptEvents.addEvent(EVENT_SEPULCHER_KINTESSA_INFILTRATION, KintessaInfiltrationFillMs);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SEPULCHER_KINTESSA_STRIKE:
                castSpellOnVictim(SPELL_KINTESSA_ANGUISHING_STRIKE);
                scriptEvents.addEvent(eventId, 12000);
                break;
            case EVENT_SEPULCHER_KINTESSA_TREPIDATION:
                castAtPlayer(SPELL_KINTESSA_FEARFUL_TREPIDATION, true);
                scriptEvents.addEvent(eventId, 30000);
                break;
            case EVENT_SEPULCHER_KINTESSA_SLUMBER:
                castSpellAOE(SPELL_KINTESSA_SLUMBER_CLOUD);
                scriptEvents.addEvent(eventId, 35000);
                break;
            case EVENT_SEPULCHER_KINTESSA_INFILTRATION:
                warn("Kin'tessa begins an Infiltration of Dread! Unmask the dreadlords!");
                castSpellAOE(SPELL_KINTESSA_INFILTRATION);
                scriptEvents.addEvent(eventId, KintessaInfiltrationFillMs);
                break;
            case EVENT_SEPULCHER_KINTESSA_SWARM:
                castSpellAOE(SPELL_KINTESSA_SWARM_OF_DARKNESS);
                break;
            case EVENT_SEPULCHER_KINTESSA_RAMPAGE:
                castSpellAOE(SPELL_LORDS_RAMPAGING_SWARM);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Prototype Pantheon

// a prototype fights in its own stage and in the last one, it waits in the forge otherwise
class SepulcherPrototypeAI : public SepulcherBossAI
{
public:
    SepulcherPrototypeAI(Creature* pCreature, uint8_t ownStage) : SepulcherBossAI(pCreature, DATA_PROTOTYPE_PANTHEON), mOwnStage(ownStage) {}

    void OnCombatStop(Unit* pTarget) override
    {
        // a waiting member may drop out while the others still fight
        SepulcherOfTheFirstOnesInstanceScript* pSepulcher = getSepulcher();
        if (!mActive && pSepulcher != nullptr && pSepulcher->isPantheonFighting(getCreature()))
        {
            SepulcherCombatAI::OnCombatStop(pTarget);
            return;
        }

        restore();
        SepulcherBossAI::OnCombatStop(pTarget);
    }

    void DoAction(int32_t action) override
    {
        if (!getCreature()->isAlive())
            return;

        if (action == ACTION_PANTHEON_RESTORE)
        {
            restore();
            return;
        }

        if (action != ACTION_PANTHEON_STAGE)
            return;

        applyStage();
        if (mActive && !_isInCombat())
            setZoneWideCombat(getCreature());
    }

protected:
    void onEngage() override
    {
        if (SepulcherOfTheFirstOnesInstanceScript* pSepulcher = getSepulcher())
            pSepulcher->beginPantheon();

        armBerserk(SepulcherBerserkMs);
        applyStage();
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (!mActive || mRetreated || currentStage() != mOwnStage || _getHealthPercent() > PantheonRetreatPct)
            return;

        SepulcherOfTheFirstOnesInstanceScript* pSepulcher = getSepulcher();
        if (pSepulcher == nullptr)
            return;

        mRetreated = true;
        if (mOwnStage == 1)
            warn("War and Duty retreat to the Progenitor Forge! Renewal and Absolution take the field!");
        else
            warn("The Pantheon of Death assembles for a final showdown!");

        pSepulcher->advancePantheon(static_cast<uint8_t>(mOwnStage + 1));
    }

    virtual void scheduleCasts() = 0;

private:
    uint8_t currentStage()
    {
        SepulcherOfTheFirstOnesInstanceScript* pSepulcher = getSepulcher();
        return pSepulcher != nullptr ? pSepulcher->getPantheonStage() : mOwnStage;
    }

    void applyStage()
    {
        uint8_t const stage = currentStage();
        mActive = stage == mOwnStage || stage >= 3;
        scriptEvents.resetEvents();

        if (!mActive)
        {
            setRooted(true);
            _setMeleeDisabled(true);
            getCreature()->addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            return;
        }

        setRooted(false);
        _setMeleeDisabled(false);
        getCreature()->removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        scheduleCasts();
    }

    void restore()
    {
        mActive = true;
        mRetreated = false;
        setRooted(false);
        _setMeleeDisabled(false);
        getCreature()->removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
    }

    uint8_t mOwnStage;
    bool mActive = true;
    bool mRetreated = false;
};

class SepulcherPrototypeOfWarAI : public SepulcherPrototypeAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SepulcherPrototypeOfWarAI(c); }
    explicit SepulcherPrototypeOfWarAI(Creature* pCreature) : SepulcherPrototypeAI(pCreature, 1) {}

protected:
    void scheduleCasts() override
    {
        scriptEvents.addEvent(EVENT_SEPULCHER_WAR_GLOOM_BOLT, 3000);
        scriptEvents.addEvent(EVENT_SEPULCHER_WAR_RITUAL, 15000);
        if (isHeroic())
            scriptEvents.addEvent(EVENT_SEPULCHER_WAR_DEATHTOUCH, 25000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SEPULCHER_WAR_GLOOM_BOLT:
                castAtPlayer(SPELL_WAR_GLOOM_BOLT, false);
                scriptEvents.addEvent(eventId, 6000);
                break;
            case EVENT_SEPULCHER_WAR_RITUAL:
                warn("The Prototype of War begins a Necrotic Ritual!");
                castSpellOnSelf(SPELL_WAR_NECROTIC_RITUAL);
                for (uint8_t i = 0; i < 3; ++i)
                    placeObject(NPC_NECROTIC_RITUALIST, aroundSelf(8.0f, 14.0f));
                scriptEvents.addEvent(eventId, 45000);
                break;
            case EVENT_SEPULCHER_WAR_DEATHTOUCH:
                castAtPlayer(SPELL_WAR_RUNECARVERS_DEATHTOUCH, true, true);
                scriptEvents.addEvent(eventId, 40000);
                break;
            default:
                break;
        }
    }
};

class SepulcherPrototypeOfDutyAI : public SepulcherPrototypeAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SepulcherPrototypeOfDutyAI(c); }
    explicit SepulcherPrototypeOfDutyAI(Creature* pCreature) : SepulcherPrototypeAI(pCreature, 1) {}

protected:
    void scheduleCasts() override
    {
        scriptEvents.addEvent(EVENT_SEPULCHER_DUTY_STRIKES, 6000);
        scriptEvents.addEvent(EVENT_SEPULCHER_DUTY_VOLLEY, 20000);
        if (isHeroic())
            scriptEvents.addEvent(EVENT_SEPULCHER_DUTY_CALL, 30000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SEPULCHER_DUTY_STRIKES:
                castSpellOnVictim(SPELL_DUTY_HUMBLING_STRIKES);
                scriptEvents.addEvent(eventId, 15000);
                break;
            case EVENT_SEPULCHER_DUTY_VOLLEY:
                warn("The Prototype of Duty unleashes a Pinning Volley!");
                castSpellAOE(SPELL_DUTY_PINNING_VOLLEY);
                scriptEvents.addEvent(eventId, 40000);
                break;
            case EVENT_SEPULCHER_DUTY_CALL:
                castSpellOnSelf(SPELL_DUTY_ASCENSIONS_CALL);
                scriptEvents.addEvent(eventId, 50000);
                break;
            default:
                break;
        }
    }
};

class SepulcherPrototypeOfRenewalAI : public SepulcherPrototypeAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SepulcherPrototypeOfRenewalAI(c); }
    explicit SepulcherPrototypeOfRenewalAI(Creature* pCreature) : SepulcherPrototypeAI(pCreature, 2) {}

protected:
    void scheduleCasts() override
    {
        scriptEvents.addEvent(EVENT_SEPULCHER_RENEWAL_BOLT, 3000);
        scriptEvents.addEvent(EVENT_SEPULCHER_RENEWAL_STAMPEDE, 15000);
        scriptEvents.addEvent(EVENT_SEPULCHER_RENEWAL_SEEDS, 20000);
        if (isHeroic())
            scriptEvents.addEvent(EVENT_SEPULCHER_RENEWAL_ANIMASTORM, 35000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SEPULCHER_RENEWAL_BOLT:
                castAtPlayer(SPELL_RENEWAL_ANIMA_BOLT, false);
                scriptEvents.addEvent(eventId, 6000);
                break;
            case EVENT_SEPULCHER_RENEWAL_STAMPEDE:
                warn("The Prototype of Renewal calls a Wild Stampede!");
                castSpellAOE(SPELL_RENEWAL_WILD_STAMPEDE);
                scriptEvents.addEvent(eventId, 35000);
                break;
            case EVENT_SEPULCHER_RENEWAL_SEEDS:
                castSpellAOE(SPELL_RENEWAL_WITHERING_SEEDS);
                scriptEvents.addEvent(eventId, 40000);
                break;
            case EVENT_SEPULCHER_RENEWAL_ANIMASTORM:
                castSpellAOE(SPELL_RENEWAL_ANIMASTORM);
                scriptEvents.addEvent(eventId, 60000);
                break;
            default:
                break;
        }
    }
};

class SepulcherPrototypeOfAbsolutionAI : public SepulcherPrototypeAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SepulcherPrototypeOfAbsolutionAI(c); }
    explicit SepulcherPrototypeOfAbsolutionAI(Creature* pCreature) : SepulcherPrototypeAI(pCreature, 2) {}

protected:
    void scheduleCasts() override
    {
        scriptEvents.addEvent(EVENT_SEPULCHER_ABSOLUTION_PAIN, 8000);
        scriptEvents.addEvent(EVENT_SEPULCHER_ABSOLUTION_BURDEN, 12000);
        scriptEvents.addEvent(EVENT_SEPULCHER_ABSOLUTION_HAND, 30000);
        if (isHeroic())
            scriptEvents.addEvent(EVENT_SEPULCHER_ABSOLUTION_HUNTER, 20000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SEPULCHER_ABSOLUTION_PAIN:
                castSpellOnVictim(SPELL_ABSOLUTION_WRACKING_PAIN);
                scriptEvents.addEvent(eventId, 25000);
                break;
            case EVENT_SEPULCHER_ABSOLUTION_BURDEN:
                castSpellAOE(SPELL_ABSOLUTION_BURDEN_OF_SIN);
                scriptEvents.addEvent(eventId, 30000);
                break;
            case EVENT_SEPULCHER_ABSOLUTION_HAND:
                warn("The Prototype of Absolution calls down the Hand of Destruction!");
                castSpellAOE(SPELL_ABSOLUTION_HAND_OF_DESTRUCTION);
                scriptEvents.addEvent(eventId, 50000);
                break;
            case EVENT_SEPULCHER_ABSOLUTION_HUNTER:
                castAtPlayer(SPELL_ABSOLUTION_NIGHT_HUNTER, true, true);
                scriptEvents.addEvent(eventId, 40000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Anduin Wrynn

class SepulcherAnduinAI : public SepulcherBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SepulcherAnduinAI(c); }
    explicit SepulcherAnduinAI(Creature* pCreature) : SepulcherBossAI(pCreature, DATA_ANDUIN_WRYNN) {}

    // freed on an earlier visit
    void OnLoad() override
    {
        InstanceScript* pInstance = getInstanceScript();
        if (pInstance != nullptr && pInstance->getBossState(DATA_ANDUIN_WRYNN) == EncounterStates::Performed)
            standDown();
    }

    void OnCombatStop(Unit* pTarget) override
    {
        if (mFreed)
        {
            SepulcherCombatAI::OnCombatStop(pTarget);
            return;
        }

        release();
        mMarchesLeft = 0;
        SepulcherBossAI::OnCombatStop(pTarget);
    }

    // Anduin is never slain, he yields at his last breath
    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage == nullptr)
            return;

        if (mFreed)
        {
            *damage = 0;
            return;
        }

        uint32_t const health = getCreature()->getHealth();
        if (*damage < health)
            return;

        *damage = health > 1 ? health - 1 : 0;
        mYieldPending = true;
    }

    void DoAction(int32_t action) override
    {
        if (action != ACTION_ANDUIN_INTERMISSION_OVER || mFreed || !_isInCombat())
            return;

        if (mStage == AnduinFirstIntermission)
            leaveIntermission(AnduinStageTwo);
        else if (mStage == AnduinSecondIntermission)
            leaveIntermission(AnduinStageThree);
    }

protected:
    void onEngage() override
    {
        mYieldPending = false;
        armBerserk(AnduinBerserkMs);
        castSpellOnSelf(SPELL_ANDUIN_DARK_ZEAL, true);
        startKingsmourneHungers();
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mFreed)
            return;

        if (mYieldPending)
        {
            yield();
            return;
        }

        // a fast raid skips what is left of the intermissions
        if (mStage < AnduinStageThree && _getHealthPercent() <= AnduinFinalStagePct)
        {
            clearIntermission();
            startMomentOfClarity();
        }
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SEPULCHER_ANDUIN_HOPEBREAKER:
                castSpellOnSelf(SPELL_ANDUIN_HOPEBREAKER);
                scriptEvents.addEvent(eventId, mStage == AnduinStageOne ? 30000 : 28000);
                break;
            case EVENT_SEPULCHER_ANDUIN_WORD_PAIN:
                castSpellOnSelf(SPELL_ANDUIN_DOMINATION_WORD_PAIN);
                scriptEvents.addEvent(eventId, 13000);
                break;
            case EVENT_SEPULCHER_ANDUIN_BARRIER:
                warn("Anduin raises a Befouled Barrier!");
                castSpellOnSelf(SPELL_ANDUIN_BEFOULED_BARRIER);
                scriptEvents.addEvent(eventId, mStage == AnduinStageOne ? 50000 : 47000);
                break;
            case EVENT_SEPULCHER_ANDUIN_BLASPHEMY:
                warn("Anduin brands the raid with Blasphemy!");
                castSpellOnSelf(SPELL_ANDUIN_BLASPHEMY);
                scriptEvents.addEvent(eventId, 52000);
                break;
            case EVENT_SEPULCHER_ANDUIN_WICKED_STAR:
                castSpellOnSelf(SPELL_ANDUIN_WICKED_STAR);
                scriptEvents.addEvent(eventId, mStage == AnduinStageThree ? 58500 : (mStage == AnduinStageOne ? 32000 : 30000));
                break;
            case EVENT_SEPULCHER_ANDUIN_KINGSMOURNE:
                warn("Kingsmourne Hungers!");
                castSpellOnSelf(SPELL_ANDUIN_KINGSMOURNE_HUNGERS);
                scriptEvents.addEvent(eventId, 60000);
                break;
            case EVENT_SEPULCHER_ANDUIN_INTERMISSION:
                enterIntermission(mStage == AnduinStageOne ? AnduinFirstIntermission : AnduinSecondIntermission);
                break;
            case EVENT_SEPULCHER_ANDUIN_REMNANT_RISES:
                raiseRemnant();
                break;
            case EVENT_SEPULCHER_ANDUIN_REMNANT_WAKES:
                wakeRemnant();
                break;
            case EVENT_SEPULCHER_ANDUIN_MARCH:
                sendMarch();
                if (mMarchesLeft > 0)
                    scriptEvents.addEvent(eventId, 8000);
                break;
            case EVENT_SEPULCHER_ANDUIN_INTERMISSION_TIMEOUT:
                DoAction(ACTION_ANDUIN_INTERMISSION_OVER);
                break;
            case EVENT_SEPULCHER_ANDUIN_REFLECTIONS:
                warn("Anduin conjures Grim Reflections!");
                castSpellOnSelf(SPELL_ANDUIN_GRIM_REFLECTIONS);
                for (uint8_t i = 0; i < 4; ++i)
                    callAdd(NPC_GRIM_REFLECTION, graspPoint(SepulcherQuarterTurn * i, 24.0f));
                scriptEvents.addEvent(eventId, 87000);
                break;
            case EVENT_SEPULCHER_ANDUIN_BEACON:
                castSpellOnSelf(SPELL_ANDUIN_BEACON_OF_HOPE);
                break;
            case EVENT_SEPULCHER_ANDUIN_EMPOWERED_HOPEBREAKER:
                warn("Anduin unleashes an Empowered Hopebreaker!");
                castSpellOnSelf(SPELL_ANDUIN_EMPOWERED_HOPEBREAKER);
                scriptEvents.addEvent(eventId, 58400);
                break;
            case EVENT_SEPULCHER_ANDUIN_HOPELESSNESS:
                castSpellAOE(SPELL_ANDUIN_HOPELESSNESS);
                scriptEvents.addEvent(eventId, 58500);
                break;
            default:
                break;
        }
    }

private:
    void startKingsmourneHungers()
    {
        enterStage(AnduinStageOne);
        scriptEvents.addEvent(EVENT_SEPULCHER_ANDUIN_HOPEBREAKER, 5000);
        scriptEvents.addEvent(EVENT_SEPULCHER_ANDUIN_WORD_PAIN, 7000);
        scriptEvents.addEvent(EVENT_SEPULCHER_ANDUIN_BARRIER, 17000);
        scriptEvents.addEvent(EVENT_SEPULCHER_ANDUIN_BLASPHEMY, 30000);
        scriptEvents.addEvent(EVENT_SEPULCHER_ANDUIN_KINGSMOURNE, 45000);
        scriptEvents.addEvent(EVENT_SEPULCHER_ANDUIN_WICKED_STAR, 55000);
        scriptEvents.addEvent(EVENT_SEPULCHER_ANDUIN_INTERMISSION, AnduinFirstIntermissionMs);
    }

    void startGrimReflections()
    {
        enterStage(AnduinStageTwo);
        scriptEvents.addEvent(EVENT_SEPULCHER_ANDUIN_REFLECTIONS, 8700);
        scriptEvents.addEvent(EVENT_SEPULCHER_ANDUIN_WORD_PAIN, 11500);
        scriptEvents.addEvent(EVENT_SEPULCHER_ANDUIN_HOPEBREAKER, 13600);
        scriptEvents.addEvent(EVENT_SEPULCHER_ANDUIN_WICKED_STAR, 18500);
        scriptEvents.addEvent(EVENT_SEPULCHER_ANDUIN_KINGSMOURNE, 48600);
        scriptEvents.addEvent(EVENT_SEPULCHER_ANDUIN_BARRIER, 80600);
        scriptEvents.addEvent(EVENT_SEPULCHER_ANDUIN_INTERMISSION, AnduinSecondIntermissionMs);
    }

    void startMomentOfClarity()
    {
        enterStage(AnduinStageThree);
        warn("Anduin fights his domination in a Moment of Clarity!");
        scriptEvents.addEvent(EVENT_SEPULCHER_ANDUIN_BEACON, 500);
        scriptEvents.addEvent(EVENT_SEPULCHER_ANDUIN_EMPOWERED_HOPEBREAKER, 11500);
        scriptEvents.addEvent(EVENT_SEPULCHER_ANDUIN_HOPELESSNESS, 21700);
        scriptEvents.addEvent(EVENT_SEPULCHER_ANDUIN_WICKED_STAR, 41000);
    }

    // held in Domination's Grasp while the Remnant fights
    void enterIntermission(uint32_t stage)
    {
        enterStage(stage);
        warn("Anduin is held in Domination's Grasp! The Remnant of a Fallen King rises!");
        setRooted(true);
        _setMeleeDisabled(true);
        castSpellOnSelf(SPELL_ANDUIN_FORCE_OF_WILL, true);

        scriptEvents.addEvent(EVENT_SEPULCHER_ANDUIN_REMNANT_RISES, 4000);
        scriptEvents.addEvent(EVENT_SEPULCHER_ANDUIN_REMNANT_WAKES, 8000);
        scriptEvents.addEvent(EVENT_SEPULCHER_ANDUIN_INTERMISSION_TIMEOUT, 8000 + RemnantReturnMs + 7000);

        mMarchesLeft = 0;
        if (stage != AnduinSecondIntermission)
            return;

        // the walls of the damned march in from every lane once
        for (uint8_t i = 0; i < MarchLaneCount; ++i)
            mMarchLanes[i] = i;

        for (uint8_t i = MarchLaneCount - 1; i > 0; --i)
            std::swap(mMarchLanes[i], mMarchLanes[Util::getRandomUInt(i)]);

        mMarchesLeft = MarchLaneCount;
        scriptEvents.addEvent(EVENT_SEPULCHER_ANDUIN_MARCH, 10000);
    }

    void leaveIntermission(uint32_t nextStage)
    {
        clearIntermission();
        if (nextStage == AnduinStageTwo)
            startGrimReflections();
        else
            startMomentOfClarity();
    }

    void clearIntermission()
    {
        release();
        mMarchesLeft = 0;
        summons.DoActionForEntry(ACTION_REMNANT_DISMISS, NPC_REMNANT_OF_A_FALLEN_KING);
        summons.despawnEntry(NPC_MARCH_OF_THE_DAMNED);
    }

    void release()
    {
        setRooted(false);
        _setMeleeDisabled(false);
        _removeAura(SPELL_ANDUIN_FORCE_OF_WILL);
    }

    void raiseRemnant()
    {
        Creature* pRemnant = summonCreature(NPC_REMNANT_OF_A_FALLEN_KING, graspPoint(SepulcherQuarterTurn, 15.0f), MANUAL_DESPAWN);
        if (pRemnant != nullptr)
            castSpell(pRemnant, SPELL_ANDUIN_DOMINATIONS_GRASP, true);
    }

    void wakeRemnant()
    {
        Creature* pRemnant = findNearestCreature(NPC_REMNANT_OF_A_FALLEN_KING, 80.0f);
        if (pRemnant != nullptr && pRemnant->isAlive() && !pRemnant->isInCombat())
            setZoneWideCombat(pRemnant);
    }

    void sendMarch()
    {
        if (mMarchesLeft == 0)
            return;

        --mMarchesLeft;
        float const laneAngle = SepulcherTwoPi / MarchLaneCount;
        float const angle = laneAngle * mMarchLanes[mMarchesLeft] + laneAngle / 2.0f;
        summonCreature(NPC_MARCH_OF_THE_DAMNED, graspPoint(angle, MarchRimRange), TIMED_DESPAWN, MarchLifetimeMs);
    }

    void yield()
    {
        mYieldPending = false;
        scriptEvents.resetEvents();
        summons.despawnAll();
        warn("Anduin falls to his knees, freed from Kingsmourne's grasp!");

        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
            pInstance->setBossState(DATA_ANDUIN_WRYNN, EncounterStates::Performed);
        }

        release();
        standDown();

        if (_isInCombat())
            getCreature()->getAIInterface()->enterEvadeMode();
    }

    void standDown()
    {
        mFreed = true;
        setReactState(REACT_PASSIVE);
        setCanEnterCombat(false);
        getCreature()->addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        getCreature()->setFaction(SepulcherFriendlyFaction);
        castSpellOnSelf(SPELL_ANDUIN_KNEEL_POSE, true);
    }

    bool mYieldPending = false;
    bool mFreed = false;
    uint8_t mMarchesLeft = 0;
    uint8_t mMarchLanes[MarchLaneCount] = {};
};

// fights in the intermissions until it returns to Kingsmourne
class SepulcherRemnantAI : public SepulcherCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SepulcherRemnantAI(c); }
    explicit SepulcherRemnantAI(Creature* pCreature) : SepulcherCombatAI(pCreature) {}

    void DoAction(int32_t action) override
    {
        if (action != ACTION_REMNANT_DISMISS || mGone)
            return;

        mGone = true;
        scriptEvents.resetEvents();
        summons.despawnAll();
        despawn(500, 0);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        scriptEvents.resetEvents();
        summons.despawnAll();
        releaseAnduin();
    }

protected:
    void onEngage() override
    {
        castSpellAOE(SPELL_REMNANT_DARK_PRESENCE, true);
        castSpellAOE(SPELL_REMNANT_REMORSELESS_WINTER, true);
        scriptEvents.addEvent(EVENT_SEPULCHER_REMNANT_ARMY, 1000);
        scriptEvents.addEvent(EVENT_SEPULCHER_REMNANT_SOUL_REAPER, 8000);
        scriptEvents.addEvent(EVENT_SEPULCHER_REMNANT_RETURN, RemnantReturnMs);
    }

    // a wipe sends the shade away
    void onDisengage() override
    {
        if (mGone)
            return;

        mGone = true;
        despawn(1000, 0);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SEPULCHER_REMNANT_ARMY:
                warn("The Remnant of a Fallen King raises the Army of the Dead!");
                castSpellOnSelf(SPELL_REMNANT_ARMY_OF_THE_DEAD);
                for (uint8_t i = 0; i < 4; ++i)
                    callAdd(NPC_FIENDISH_SOUL, aroundSelf(8.0f, 15.0f));
                callAdd(NPC_MONSTROUS_SOUL, aroundSelf(25.0f, 35.0f));
                scriptEvents.addEvent(eventId, 36900);
                break;
            case EVENT_SEPULCHER_REMNANT_SOUL_REAPER:
                castSpellOnVictim(SPELL_REMNANT_SOUL_REAPER);
                scriptEvents.addEvent(eventId, 12000);
                break;
            case EVENT_SEPULCHER_REMNANT_RETURN:
                warn("The Remnant of a Fallen King returns to Kingsmourne!");
                castSpellOnSelf(SPELL_REMNANT_RETURN_TO_KINGSMOURNE, true);
                mGone = true;
                summons.despawnAll();
                releaseAnduin();
                despawn(2000, 0);
                break;
            default:
                break;
        }
    }

private:
    void releaseAnduin()
    {
        if (SepulcherOfTheFirstOnesInstanceScript* pSepulcher = getSepulcher())
            pSepulcher->notifyLiving(BOSS_ANDUIN_WRYNN, ACTION_ANDUIN_INTERMISSION_OVER);
    }

    bool mGone = false;
};

// detonates once it is worn down
class SepulcherMonstrousSoulAI : public SepulcherCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SepulcherMonstrousSoulAI(c); }
    explicit SepulcherMonstrousSoulAI(Creature* pCreature) : SepulcherCombatAI(pCreature) {}

protected:
    void onEngage() override
    {
        mDetonated = false;
        castSpellOnSelf(SPELL_MONSTROUS_UNRAVELING_FRENZY, true);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mDetonated || _getHealthPercent() > 35)
            return;

        mDetonated = true;
        warn("A Monstrous Soul begins a Necrotic Detonation!");
        castSpellAOE(SPELL_MONSTROUS_NECROTIC_DETONATION);
    }

private:
    bool mDetonated = false;
};

// a wall of the damned crossing Domination's Grasp
class SepulcherMarchOfTheDamnedAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SepulcherMarchOfTheDamnedAI(c); }
    explicit SepulcherMarchOfTheDamnedAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        setCanEnterCombat(false);
        getCreature()->addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        mStartLeftMs = 1000;
        mPulseLeftMs = MarchPulseMs;
    }

    void AIUpdate(unsigned long time_passed) override
    {
        uint32_t const diff = static_cast<uint32_t>(time_passed);

        // walks through the middle to the far rim
        if (countDown(diff, mStartLeftMs))
        {
            LocationVector const& here = getCreature()->GetPosition();
            LocationVector const farRim(2.0f * DominationGraspCentre.x - here.x, 2.0f * DominationGraspCentre.y - here.y, here.z, here.o);
            movePoint(1, farRim, false);
        }

        if (countDown(diff, mPulseLeftMs))
        {
            castSpellAOE(SPELL_MARCH_OF_THE_DAMNED, true);
            mPulseLeftMs = MarchPulseMs;
        }
    }

private:
    uint32_t mStartLeftMs = 0;
    uint32_t mPulseLeftMs = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Rygelon

class SepulcherRygelonAI : public SepulcherBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SepulcherRygelonAI(c); }
    explicit SepulcherRygelonAI(Creature* pCreature) : SepulcherBossAI(pCreature, DATA_RYGELON) {}

    // destroyed quasars and matter leave their fields behind
    void OnSummonDies(Creature* pSummon, Unit* /*pKiller*/) override
    {
        if (pSummon == nullptr || !_isInCombat())
            return;

        if (pSummon->getEntry() == NPC_COLLAPSING_QUASAR)
            getCreature()->castSpellLoc(pSummon->GetPosition(), SPELL_RYGELON_QUASAR_FIELD, true);
        else if (pSummon->getEntry() == NPC_UNSTABLE_MATTER)
            getCreature()->castSpellLoc(pSummon->GetPosition(), SPELL_RYGELON_MATTER_FIELD, true);
    }

protected:
    void onEngage() override
    {
        armBerserk(SepulcherBerserkMs);
        castSpellOnSelf(SPELL_RYGELON_CORRUPTED_STRIKES, true);
        startCosmos();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SEPULCHER_RYGELON_ECLIPSE:
                castAtPlayers(SPELL_RYGELON_DARK_ECLIPSE, 3, true);
                scriptEvents.addEvent(eventId, 30000);
                break;
            case EVENT_SEPULCHER_RYGELON_COLLAPSE:
                castSpellAOE(SPELL_RYGELON_CELESTIAL_COLLAPSE);
                for (uint8_t i = 0; i < 3; ++i)
                    placeObject(NPC_COLLAPSING_QUASAR, aroundSelf(20.0f, 30.0f));
                scriptEvents.addEvent(eventId, 35000);
                break;
            case EVENT_SEPULCHER_RYGELON_COSMOS:
                castSpellOnSelf(SPELL_RYGELON_MANIFEST_COSMOS);
                for (uint8_t i = 0; i < 2; ++i)
                    placeObject(NPC_COSMIC_CORE, aroundSelf(15.0f, 25.0f));
                scriptEvents.addEvent(eventId, 45000);
                break;
            case EVENT_SEPULCHER_RYGELON_TERMINATOR:
                warn("Rygelon unleashes a Celestial Terminator!");
                castSpellAOE(SPELL_RYGELON_CELESTIAL_TERMINATOR);
                scriptEvents.addEvent(eventId, 45000);
                break;
            case EVENT_SEPULCHER_RYGELON_MATTER:
                warn("Unstable Matter coalesces around Rygelon!");
                for (uint8_t i = 0; i < 3; ++i)
                    placeObject(NPC_UNSTABLE_MATTER, aroundSelf(15.0f, 25.0f));
                break;
            case EVENT_SEPULCHER_RYGELON_MASSIVE_BANG:
                warn("Rygelon begins to cast Massive Bang! Seek The Singularity!");
                castSpellAOE(SPELL_RYGELON_MASSIVE_BANG);
                scriptEvents.addEvent(EVENT_SEPULCHER_RYGELON_SINGULARITY, 10000);
                break;
            case EVENT_SEPULCHER_RYGELON_SINGULARITY:
                enterSingularity();
                break;
            case EVENT_SEPULCHER_RYGELON_SHATTER_SPHERE:
                castSpellAOE(SPELL_RYGELON_SHATTER_SPHERE);
                for (uint8_t i = 0; i < 2; ++i)
                    placeObject(NPC_UNSTABLE_ANTIMATTER, aroundSelf(8.0f, 15.0f));
                scriptEvents.addEvent(eventId, 15000);
                break;
            case EVENT_SEPULCHER_RYGELON_PLASMA:
                castAtPlayer(SPELL_RYGELON_RADIANT_PLASMA, true);
                scriptEvents.addEvent(eventId, 12000);
                break;
            case EVENT_SEPULCHER_RYGELON_SINGULARITY_END:
                warn("The Singularity collapses!");
                startCosmos();
                break;
            default:
                break;
        }
    }

private:
    // cosmic energy fills toward Massive Bang
    void startCosmos()
    {
        enterStage(1);
        scriptEvents.addEvent(EVENT_SEPULCHER_RYGELON_ECLIPSE, 8000);
        scriptEvents.addEvent(EVENT_SEPULCHER_RYGELON_COLLAPSE, 15000);
        scriptEvents.addEvent(EVENT_SEPULCHER_RYGELON_COSMOS, 25000);
        scriptEvents.addEvent(EVENT_SEPULCHER_RYGELON_TERMINATOR, 35000);
        scriptEvents.addEvent(EVENT_SEPULCHER_RYGELON_MATTER, RygelonBangFillMs - 25000);
        scriptEvents.addEvent(EVENT_SEPULCHER_RYGELON_MASSIVE_BANG, RygelonBangFillMs);
    }

    void enterSingularity()
    {
        enterStage(2);
        warn("Rygelon draws the raid into The Singularity!");
        for (uint8_t i = 0; i < 2; ++i)
            callAdd(NPC_UNSTABLE_CORE, aroundSelf(10.0f, 20.0f));

        scriptEvents.addEvent(EVENT_SEPULCHER_RYGELON_SHATTER_SPHERE, 5000);
        scriptEvents.addEvent(EVENT_SEPULCHER_RYGELON_PLASMA, 10000);
        scriptEvents.addEvent(EVENT_SEPULCHER_RYGELON_SINGULARITY_END, RygelonSingularityMs);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Artificer Xy'mox

class SepulcherXymoxAI : public SepulcherBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SepulcherXymoxAI(c); }
    explicit SepulcherXymoxAI(Creature* pCreature) : SepulcherBossAI(pCreature, DATA_ARTIFICER_XYMOX) {}

    // the relic opens up once the reinforcements are slain
    void OnSummonDies(Creature* pSummon, Unit* /*pKiller*/) override
    {
        if (pSummon == nullptr || mStage != 2 || mReinforcementsLeft == 0)
            return;

        if (pSummon->getEntry() != NPC_XY_ACOLYTE && pSummon->getEntry() != NPC_XY_SPELLSLINGER)
            return;

        if (--mReinforcementsLeft == 0)
            finishDecipher();
    }

protected:
    void onEngage() override
    {
        mDeciphers = 0;
        mReinforcementsLeft = 0;
        armBerserk(SepulcherBerserkMs);
        startPlunder();
    }

    void onDisengage() override
    {
        mSupernovaLeftMs = 0;
        mDecipherLeftMs = 0;
        setRooted(false);
        _setMeleeDisabled(false);
        _removeAura(SPELL_XYMOX_GENESIS_BULWARK);
    }

    void onCombatTick(uint32_t diff) override
    {
        // the decipher channel must not hold back the supernova
        if (mStage == 2)
        {
            if (countDown(diff, mSupernovaLeftMs))
            {
                castSpellAOE(SPELL_XYMOX_GENESIS_SUPERNOVA, true);
                mSupernovaLeftMs = XymoxSupernovaMs;
            }

            if (countDown(diff, mDecipherLeftMs))
                finishDecipher();

            return;
        }

        if (mDeciphers >= XymoxDecipherCount)
            return;

        if (_getHealthPercent() <= XymoxDecipherMarks[mDeciphers])
            startDecipher();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SEPULCHER_XYMOX_TEAR:
                castAtPlayers(SPELL_XYMOX_DIMENSIONAL_TEAR, 2, true);
                scriptEvents.addEvent(eventId, 35000);
                break;
            case EVENT_SEPULCHER_XYMOX_GLYPH:
                castSpellOnVictim(SPELL_XYMOX_GLYPH_OF_RELOCATION);
                scriptEvents.addEvent(eventId, 30000);
                break;
            case EVENT_SEPULCHER_XYMOX_STASIS_TRAP:
                castSpellAOE(SPELL_XYMOX_STASIS_TRAP);
                scriptEvents.addEvent(eventId, 30000);
                break;
            case EVENT_SEPULCHER_XYMOX_SPARKNOVA:
                castSpellAOE(SPELL_XYMOX_HYPERLIGHT_SPARKNOVA);
                scriptEvents.addEvent(eventId, 25000);
                break;
            case EVENT_SEPULCHER_XYMOX_GUNSHIP:
                warn("Xy'mox calls in a Gunship Barrage!");
                castSpellAOE(SPELL_XYMOX_GUNSHIP_BARRAGE);
                scriptEvents.addEvent(eventId, mStage == 2 ? 40000 : 50000);
                break;
            case EVENT_SEPULCHER_XYMOX_RINGS:
                warn("The Genesis Relic sends out Genesis Rings!");
                castSpellAOE(SPELL_XYMOX_GENESIS_RINGS);
                scriptEvents.addEvent(eventId, 40000);
                break;
            case EVENT_SEPULCHER_XYMOX_CARTEL_ELITE:
                callAdd(NPC_CARTEL_PLUNDERER, aroundSelf(10.0f, 20.0f));
                callAdd(NPC_CARTEL_OVERSEER, aroundSelf(10.0f, 20.0f));
                scriptEvents.addEvent(eventId, 60000);
                break;
            default:
                break;
        }
    }

private:
    // stage one before the relic, stage three after the second decipher
    void startPlunder()
    {
        enterStage(mDeciphers >= XymoxDecipherCount ? 3 : 1);
        scriptEvents.addEvent(EVENT_SEPULCHER_XYMOX_STASIS_TRAP, 6000);
        scriptEvents.addEvent(EVENT_SEPULCHER_XYMOX_TEAR, 10000);
        scriptEvents.addEvent(EVENT_SEPULCHER_XYMOX_GLYPH, 15000);
        scriptEvents.addEvent(EVENT_SEPULCHER_XYMOX_SPARKNOVA, 20000);
        scriptEvents.addEvent(EVENT_SEPULCHER_XYMOX_RINGS, 28000);
        scriptEvents.addEvent(EVENT_SEPULCHER_XYMOX_GUNSHIP, 40000);
        if (isHeroic())
            scriptEvents.addEvent(EVENT_SEPULCHER_XYMOX_CARTEL_ELITE, 30000);
    }

    // Decipher Relic at 75% and 50%
    void startDecipher()
    {
        ++mDeciphers;
        enterStage(2);
        warn("Xy'mox calls in Xy Reinforcements and begins to Decipher the Relic!");
        setRooted(true);
        _setMeleeDisabled(true);
        castSpellOnSelf(SPELL_XYMOX_GENESIS_BULWARK, true);
        castSpellOnSelf(SPELL_XYMOX_DECIPHER_RELIC);

        mReinforcementsLeft = 0;
        if (callAdd(NPC_XY_ACOLYTE, aroundSelf(10.0f, 18.0f)) != nullptr)
            ++mReinforcementsLeft;

        for (uint8_t i = 0; i < 2; ++i)
        {
            if (callAdd(NPC_XY_SPELLSLINGER, aroundSelf(12.0f, 22.0f)) != nullptr)
                ++mReinforcementsLeft;
        }

        mSupernovaLeftMs = 10000;
        mDecipherLeftMs = XymoxDecipherMaxMs;
        scriptEvents.addEvent(EVENT_SEPULCHER_XYMOX_GUNSHIP, 20000);
    }

    void finishDecipher()
    {
        mReinforcementsLeft = 0;
        mSupernovaLeftMs = 0;
        mDecipherLeftMs = 0;
        warn("The Genesis Bulwark around Xy'mox fades!");
        getCreature()->interruptSpell();
        setRooted(false);
        _setMeleeDisabled(false);
        _removeAura(SPELL_XYMOX_GENESIS_BULWARK);
        startPlunder();
    }

    uint8_t mDeciphers = 0;
    uint8_t mReinforcementsLeft = 0;
    uint32_t mSupernovaLeftMs = 0;
    uint32_t mDecipherLeftMs = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Add factories

namespace
{
    CreatureAIScript* createSentryAI(Creature* c) { return new SepulcherAddAI(c, SentryCasts, false); }
    CreatureAIScript* createMateriumAI(Creature* c) { return new SepulcherAddAI(c, MateriumCasts, false); }
    CreatureAIScript* createDroneAI(Creature* c) { return new SepulcherAddAI(c, DroneCasts, true); }
    CreatureAIScript* createDefenseMatrixAI(Creature* c) { return new SepulcherAddAI(c, DefenseMatrixCasts, true); }
    CreatureAIScript* createIncarnationAI(Creature* c) { return new SepulcherAddAI(c, IncarnationCasts, false); }
    CreatureAIScript* createDegenerationAutomaAI(Creature* c) { return new SepulcherAddAI(c, DegenerationAutomaCasts, false); }
    CreatureAIScript* createAcquisitionsAutomaAI(Creature* c) { return new SepulcherAddAI(c, AcquisitionsAutomaCasts, false); }
    CreatureAIScript* createGuardianAutomaAI(Creature* c) { return new SepulcherAddAI(c, GuardianAutomaCasts, false); }
    CreatureAIScript* createDefenseMatrixAutomaAI(Creature* c) { return new SepulcherAddAI(c, DefenseMatrixAutomaCasts, true); }
    CreatureAIScript* createDominationCoreAI(Creature* c) { return new SepulcherAddAI(c, DominationCoreCasts, true); }
    CreatureAIScript* createInchoateShadowAI(Creature* c) { return new SepulcherAddAI(c, InchoateShadowCasts, false); }
    CreatureAIScript* createFiendishSoulAI(Creature* c) { return new SepulcherAddAI(c, FiendishSoulCasts, false); }
    CreatureAIScript* createGrimReflectionAI(Creature* c) { return new SepulcherAddAI(c, GrimReflectionCasts, true); }
    CreatureAIScript* createUnstableCoreAI(Creature* c) { return new SepulcherAddAI(c, UnstableCoreCasts, false); }
    CreatureAIScript* createXyAcolyteAI(Creature* c) { return new SepulcherAddAI(c, XyAcolyteCasts, false); }
    CreatureAIScript* createXySpellslingerAI(Creature* c) { return new SepulcherAddAI(c, XySpellslingerCasts, false); }
    CreatureAIScript* createCartelPlundererAI(Creature* c) { return new SepulcherAddAI(c, CartelPlundererCasts, false); }
    CreatureAIScript* createCartelOverseerAI(Creature* c) { return new SepulcherAddAI(c, CartelOverseerCasts, false); }

    CreatureAIScript* createDriftingObjectAI(Creature* c) { return new SepulcherObjectAI(c, false); }
    CreatureAIScript* createFixedObjectAI(Creature* c) { return new SepulcherObjectAI(c, true); }
}

void SetupSepulcherOfTheFirstOnes(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_SEPULCHER_OF_THE_FIRST_ONES, &SepulcherOfTheFirstOnesInstanceScript::Create);

    // Vigilant Guardian
    mgr->register_creature_script(BOSS_VIGILANT_GUARDIAN, &SepulcherVigilantGuardianAI::Create);
    mgr->register_creature_script(NPC_PRE_FABRICATED_SENTRY, &createSentryAI);
    mgr->register_creature_script(NPC_PRE_FABRICATED_SENTRY_ALT, &createSentryAI);
    mgr->register_creature_script(NPC_VOLATILE_MATERIUM, &createMateriumAI);
    mgr->register_creature_script(NPC_POINT_DEFENSE_DRONE, &createDroneAI);
    mgr->register_creature_script(NPC_AUTOMATED_DEFENSE_MATRIX, &createDefenseMatrixAI);

    // Halondrus the Reclaimer
    mgr->register_creature_script(BOSS_HALONDRUS, &SepulcherHalondrusAI::Create);
    mgr->register_creature_script(NPC_EPHEMERAL_MOTE, &createDriftingObjectAI);

    // The Jailer
    mgr->register_creature_script(BOSS_THE_JAILER, &SepulcherJailerAI::Create);
    mgr->register_creature_script(NPC_INCARNATION_OF_TORMENT, &createIncarnationAI);

    // Lihuvim, Principal Architect
    mgr->register_creature_script(BOSS_LIHUVIM, &SepulcherLihuvimAI::Create);
    mgr->register_creature_script(NPC_DEGENERATION_AUTOMA, &createDegenerationAutomaAI);
    mgr->register_creature_script(NPC_DEGENERATION_AUTOMA_ALT, &createDegenerationAutomaAI);
    mgr->register_creature_script(NPC_ACQUISITIONS_AUTOMA, &createAcquisitionsAutomaAI);
    mgr->register_creature_script(NPC_ACQUISITIONS_AUTOMA_ALT, &createAcquisitionsAutomaAI);
    mgr->register_creature_script(NPC_GUARDIAN_AUTOMA, &createGuardianAutomaAI);
    mgr->register_creature_script(NPC_GUARDIAN_AUTOMA_ALT, &createGuardianAutomaAI);
    mgr->register_creature_script(NPC_DEFENSE_MATRIX_AUTOMA, &createDefenseMatrixAutomaAI);

    // Dausegne, the Fallen Oracle
    mgr->register_creature_script(BOSS_DAUSEGNE, &SepulcherDausegneAI::Create);
    mgr->register_creature_script(NPC_DOMINATION_CORE, &createDominationCoreAI);

    // Skolex, the Insatiable Ravener
    mgr->register_creature_script(BOSS_SKOLEX, &SepulcherSkolexAI::Create);

    // Lords of Dread
    mgr->register_creature_script(BOSS_MALGANIS, &SepulcherMalGanisAI::Create);
    mgr->register_creature_script(BOSS_KINTESSA, &SepulcherKintessaAI::Create);
    mgr->register_creature_script(NPC_INCHOATE_SHADOW, &createInchoateShadowAI);

    // Prototype Pantheon
    mgr->register_creature_script(BOSS_PROTOTYPE_OF_WAR, &SepulcherPrototypeOfWarAI::Create);
    mgr->register_creature_script(BOSS_PROTOTYPE_OF_DUTY, &SepulcherPrototypeOfDutyAI::Create);
    mgr->register_creature_script(BOSS_PROTOTYPE_OF_RENEWAL, &SepulcherPrototypeOfRenewalAI::Create);
    mgr->register_creature_script(BOSS_PROTOTYPE_OF_ABSOLUTION, &SepulcherPrototypeOfAbsolutionAI::Create);
    mgr->register_creature_script(NPC_NECROTIC_RITUALIST, &createFixedObjectAI);

    // Anduin Wrynn
    mgr->register_creature_script(BOSS_ANDUIN_WRYNN, &SepulcherAnduinAI::Create);
    mgr->register_creature_script(NPC_REMNANT_OF_A_FALLEN_KING, &SepulcherRemnantAI::Create);
    mgr->register_creature_script(NPC_FIENDISH_SOUL, &createFiendishSoulAI);
    mgr->register_creature_script(NPC_MONSTROUS_SOUL, &SepulcherMonstrousSoulAI::Create);
    mgr->register_creature_script(NPC_GRIM_REFLECTION, &createGrimReflectionAI);
    mgr->register_creature_script(NPC_MARCH_OF_THE_DAMNED, &SepulcherMarchOfTheDamnedAI::Create);

    // Rygelon
    mgr->register_creature_script(BOSS_RYGELON, &SepulcherRygelonAI::Create);
    mgr->register_creature_script(NPC_COLLAPSING_QUASAR, &createFixedObjectAI);
    mgr->register_creature_script(NPC_COSMIC_CORE, &createFixedObjectAI);
    mgr->register_creature_script(NPC_UNSTABLE_MATTER, &createFixedObjectAI);
    mgr->register_creature_script(NPC_UNSTABLE_ANTIMATTER, &createFixedObjectAI);
    mgr->register_creature_script(NPC_UNSTABLE_CORE, &createUnstableCoreAI);

    // Artificer Xy'mox
    mgr->register_creature_script(BOSS_ARTIFICER_XYMOX, &SepulcherXymoxAI::Create);
    mgr->register_creature_script(NPC_XY_ACOLYTE, &createXyAcolyteAI);
    mgr->register_creature_script(NPC_XY_SPELLSLINGER, &createXySpellslingerAI);
    mgr->register_creature_script(NPC_CARTEL_PLUNDERER, &createCartelPlundererAI);
    mgr->register_creature_script(NPC_CARTEL_OVERSEER, &createCartelOverseerAI);
}
