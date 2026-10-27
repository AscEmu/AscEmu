/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Raid_Vault_Of_The_Incarnates.hpp"

#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <cmath>
#include <cstddef>
#include <map>
#include <utility>
#include <vector>

enum VaultOfTheIncarnatesEvents
{
    // table driven adds
    EVENT_VAULT_ADD_FIRST = 1,
    EVENT_VAULT_ADD_SECOND,

    // Eranog
    EVENT_ERANOG_FLAMERIFT,
    EVENT_ERANOG_RIFT_OPEN,
    EVENT_ERANOG_MOLTEN_CLEAVE,
    EVENT_ERANOG_INCINERATING_ROAR,
    EVENT_ERANOG_MOLTEN_SPIKES,
    EVENT_ERANOG_ARMY_OF_FLAME,
    EVENT_ERANOG_ARMY_FADES,
    EVENT_TARASEK_KILL_ORDER,

    // The Primal Council
    EVENT_COUNCIL_CONVOCATION,
    EVENT_KADROS_FROST_SPIKE,
    EVENT_KADROS_PRIMAL_BLIZZARD,
    EVENT_STORMLASH_LIGHTNING_BOLT,
    EVENT_STORMLASH_CHAIN_LIGHTNING,
    EVENT_STORMLASH_CONDUCTIVE_MARK,
    EVENT_OPALFANG_CRUSH,
    EVENT_OPALFANG_EARTHEN_PILLAR,
    EVENT_OPALFANG_QUAKING_PULSE,
    EVENT_EMBAR_SLASHING_BLAZE,
    EVENT_EMBAR_METEOR_AXES,
    EVENT_EMBAR_AXES_LAND,

    // Sennarth, the Cold Breath
    EVENT_SENNARTH_WEB_BLAST,
    EVENT_SENNARTH_CHILLING_BLAST,
    EVENT_SENNARTH_WEBS,
    EVENT_SENNARTH_BURST,
    EVENT_SENNARTH_BREATH_OF_ICE,
    EVENT_SENNARTH_SPIDERLINGS,
    EVENT_SENNARTH_ARACHNID,
    EVENT_SENNARTH_PERVASIVE_COLD,
    EVENT_SENNARTH_PERMAFROST,
    EVENT_SPIDERLING_ERUPTION,

    // Kurog Grimtotem
    EVENT_KUROG_SUNDERING_STRIKE,
    EVENT_KUROG_ALTAR_LEAVE,
    EVENT_KUROG_ALTAR_REACH,
    EVENT_KUROG_ALTAR_MINOR,
    EVENT_KUROG_ALTAR_MAJOR,
    EVENT_KUROG_ULTIMATE,
    EVENT_KUROG_BARRIER_WATCH,
    EVENT_KUROG_PRIMAL_ATTUNEMENT,

    // Raszageth the Storm-Eater
    EVENT_RASZAGETH_HURRICANE_WING,
    EVENT_RASZAGETH_STATIC_CHARGE,
    EVENT_RASZAGETH_VOLATILE_CURRENT,
    EVENT_RASZAGETH_ELECTRIFIED_JAWS,
    EVENT_RASZAGETH_LIGHTNING_BREATH,
    EVENT_RASZAGETH_LIGHTNING_STRIKES,
    EVENT_RASZAGETH_LIGHTNING_DEVASTATION,
    EVENT_RASZAGETH_FORCES_WATCH,
    EVENT_RASZAGETH_INTERMISSION_LIMIT,
    EVENT_RASZAGETH_STORMSURGE,
    EVENT_RASZAGETH_TEMPEST_WING,
    EVENT_RASZAGETH_FULMINATING_CHARGE,
    EVENT_RASZAGETH_STORM_EATER,
    EVENT_RASZAGETH_MAGNETIC_CHARGE,
    EVENT_RASZAGETH_THUNDEROUS_BLAST,
    EVENT_SPARK_ARM,
    EVENT_SPARK_BURST,
    EVENT_STORMFIEND_STORM_BREAK,
    EVENT_STORMFIEND_BALL_LIGHTNING,

    // Broodkeeper Diurna
    EVENT_DIURNA_MORTAL_CLAWS,
    EVENT_DIURNA_WILDFIRE,
    EVENT_DIURNA_GREATSTAFF,
    EVENT_DIURNA_SHROUD,
    EVENT_DIURNA_RAPID_INCUBATION,
    EVENT_DIURNA_REINFORCEMENTS,
    EVENT_DIURNA_STORM_FISSURE,

    // Dathea, Ascended
    EVENT_DATHEA_ZEPHYR_SLAM,
    EVENT_DATHEA_RAGING_BURST,
    EVENT_DATHEA_CONDUCTIVE_MARK,
    EVENT_DATHEA_CROSSWINDS,
    EVENT_DATHEA_CYCLONE,
    EVENT_DATHEA_COALESCING_STORM,
    EVENT_DATHEA_REACH_CHECK,

    // Terros
    EVENT_TERROS_ROCK_BLAST,
    EVENT_TERROS_CONCUSSIVE_SLAM,
    EVENT_TERROS_SHATTERING_IMPACT,
    EVENT_TERROS_FRACTURED_RUBBLE,
    EVENT_TERROS_RESONATING_ANNIHILATION,
    EVENT_TERROS_FRENZIED_DEVASTATION,
    EVENT_TERROS_REACH_CHECK,
    EVENT_TERROS_INFUSED_FALLOUT
};

namespace
{
    float const VaultTwoPi = 6.28318531f;

    // how a timed ability picks its target
    enum class VaultCastAt : uint8_t
    {
        Victim,
        Player,
        Ground,
        Self,
        Area
    };

    // council members answer this with 1 while channeling their convocation
    uint32_t const CouncilConvocationQuery = 1;

    uint32_t const PrimalCouncilMembers[4] = { BOSS_KADROS_ICEWRATH, BOSS_DATHEA_STORMLASH, BOSS_OPALFANG, BOSS_EMBAR_FIREPATH };

    uint32_t const EranogBerserkMs = 420000;
    uint32_t const CouncilBerserkMs = 480000;
    uint32_t const SennarthBerserkMs = 540000;
    uint32_t const KurogBerserkMs = 540000;
    uint32_t const RaszagethBerserkMs = 600000;
    uint32_t const DiurnaBerserkMs = 540000;
    uint32_t const DatheaBerserkMs = 480000;

    // Eranog fills his energy within a minute of Army of Talon
    uint32_t const EranogEnergyFullMs = 60000;
    uint32_t const EranogArmyMs = 22000;
    float const EranogArmyRing = 10.0f;

    int32_t const SennarthPrecipicePct = 40;
    float const SpiderlingEruptionRange = 4.0f;

    int32_t const KurogBarrierMarks[2] = { 70, 40 };
    uint32_t const KurogAltarStayMs = 45000;
    uint32_t const KurogAltarTravelMs = 6000;
    uint32_t const KurogEnergyFullMs = 40000;

    int32_t const RaszagethPrimalistStrikePct = 65;
    int32_t const RaszagethVaultFaltersPct = 40;
    uint32_t const RaszagethIntermissionLimitMs = 90000;

    int32_t const DiurnaScornedPct = 40;
    uint32_t const DiurnaEnergyFullMs = 35000;

    uint32_t const DatheaEnergyFullMs = 80000;
    size_t const DatheaTempestLimit = 4;
    float const DatheaReach = 12.0f;

    uint32_t const TerrosEnergyFullMs = 95000;
    uint32_t const TerrosAnnihilationLimit = 4;
    float const TerrosReach = 10.0f;

    uint32_t const StormlingSeekMs = 1000;
    float const StormlingFuseRange = 5.0f;
    uint32_t const TempestPulseMs = 2000;

    // Kurog's four altars, visited in this order
    enum KurogAltar : uint8_t
    {
        KUROG_ALTAR_FLAME,
        KUROG_ALTAR_FROST,
        KUROG_ALTAR_EARTH,
        KUROG_ALTAR_STORM,
        KUROG_ALTAR_COUNT
    };

    struct KurogAltarKit
    {
        uint32_t dominance;
        uint32_t minorSpell;
        VaultCastAt minorAt;
        uint32_t majorSpell;
        VaultCastAt majorAt;
        uint32_t ultimateSpell;
        VaultCastAt ultimateAt;
        uint32_t wroughtEntry;
        uint32_t creationEntry;
        char const* reachText;
        char const* ultimateText;
    };

    KurogAltarKit const KurogAltars[KUROG_ALTAR_COUNT] =
    {
        { SPELL_KUROG_FLAME_DOMINANCE, SPELL_KUROG_MAGMA_BURST, VaultCastAt::Area, SPELL_KUROG_MOLTEN_RUPTURE, VaultCastAt::Player,
          SPELL_KUROG_SEARING_CARNAGE, VaultCastAt::Area, NPC_FLAMEWROUGHT_ERADICATOR, NPC_BLAZING_FIEND,
          "Kurog Grimtotem draws on the Altar of Flame!", "Kurog Grimtotem unleashes Searing Carnage!" },
        { SPELL_KUROG_FROST_DOMINANCE, SPELL_KUROG_BITING_CHILL, VaultCastAt::Area, SPELL_KUROG_FRIGID_TORRENT, VaultCastAt::Self,
          SPELL_KUROG_ABSOLUTE_ZERO, VaultCastAt::Area, NPC_FROSTWROUGHT_DOMINATOR, NPC_FROZEN_DESTROYER,
          "Kurog Grimtotem draws on the Altar of Frost!", "Kurog Grimtotem calls for Absolute Zero!" },
        { SPELL_KUROG_EARTH_DOMINANCE, SPELL_KUROG_ERUPTING_BEDROCK, VaultCastAt::Player, SPELL_KUROG_ENVELOPING_EARTH, VaultCastAt::Area,
          SPELL_KUROG_SEISMIC_RUPTURE, VaultCastAt::Area, NPC_EARTHWROUGHT_SMASHER, NPC_TECTONIC_CRUSHER,
          "Kurog Grimtotem draws on the Altar of Earth!", "Kurog Grimtotem splits the ground with a Seismic Rupture!" },
        { SPELL_KUROG_STORM_DOMINANCE, SPELL_KUROG_SHOCKING_BURST, VaultCastAt::Area, SPELL_KUROG_LIGHTNING_CRASH, VaultCastAt::Area,
          SPELL_KUROG_THUNDER_STRIKE, VaultCastAt::Ground, NPC_STORMWROUGHT_DESPOILER, NPC_THUNDERING_RAVAGER,
          "Kurog Grimtotem draws on the Altar of Storm!", "Kurog Grimtotem brings down a Thunder Strike!" }
    };

    // one elite joins every Primalist Reinforcements wave, in turn
    uint32_t const DiurnaElites[4] = { NPC_TARASEK_EARTHREAVER, NPC_DRAGONSPAWN_FLAMEBENDER, NPC_JUVENILE_FROST_PROTO_DRAGON, NPC_DRAKONID_STORMBRINGER };

    // adds whose fight is a presence aura and up to two timed abilities
    struct VaultAddSpell
    {
        uint32_t spellId;
        VaultCastAt target;
        uint32_t firstMs;
        uint32_t repeatMs;
    };

    struct VaultAddKit
    {
        uint32_t entry;
        uint32_t presence;
        VaultAddSpell abilities[2];
    };

    VaultAddSpell const NoAddSpell = { 0, VaultCastAt::Self, 0, 0 };

    VaultAddKit const VaultAddKits[] =
    {
        // Eranog
        { NPC_PRIMAL_FLAME,                 0, { { SPELL_PRIMAL_FLAME_INCINERATE, VaultCastAt::Player, 4000, 9000 }, NoAddSpell } },
        { NPC_PRIMAL_FLAME_ALT,             0, { { SPELL_PRIMAL_FLAME_INCINERATE, VaultCastAt::Player, 4000, 9000 }, NoAddSpell } },

        // Sennarth
        { NPC_FROSTBREATH_ARACHNID,         SPELL_ARACHNID_CHILLING_AURA, { { SPELL_ARACHNID_FREEZING_BREATH, VaultCastAt::Victim, 6000, 14000 }, NoAddSpell } },
        { NPC_FROSTBREATH_ARACHNID_ALT,     SPELL_ARACHNID_CHILLING_AURA, { { SPELL_ARACHNID_FREEZING_BREATH, VaultCastAt::Victim, 6000, 14000 }, NoAddSpell } },

        // Kurog's elemental creations
        { NPC_BLAZING_FIEND,                SPELL_BLAZING_FIEND_PRESENCE, { { SPELL_BLAZING_FIEND_MAGMA_FLOW, VaultCastAt::Self, 8000, 25000 }, { SPELL_BLAZING_FIEND_SEARING_CARNAGE, VaultCastAt::Area, 15000, 30000 } } },
        { NPC_FROZEN_DESTROYER,             SPELL_FROZEN_DESTROYER_PRESENCE, { { SPELL_FROZEN_DESTROYER_FROST_BINDS, VaultCastAt::Area, 6000, 18000 }, { SPELL_FROZEN_DESTROYER_FREEZING_TEMPEST, VaultCastAt::Self, 14000, 30000 } } },
        { NPC_TECTONIC_CRUSHER,             SPELL_TECTONIC_CRUSHER_PRESENCE, { { SPELL_TECTONIC_CRUSHER_GROUND_SHATTER, VaultCastAt::Area, 7000, 20000 }, { SPELL_TECTONIC_CRUSHER_VIOLENT_UPHEAVAL, VaultCastAt::Area, 15000, 30000 } } },
        { NPC_TECTONIC_CRUSHER_ALT,         SPELL_TECTONIC_CRUSHER_PRESENCE, { { SPELL_TECTONIC_CRUSHER_GROUND_SHATTER, VaultCastAt::Area, 7000, 20000 }, { SPELL_TECTONIC_CRUSHER_VIOLENT_UPHEAVAL, VaultCastAt::Area, 15000, 30000 } } },
        { NPC_THUNDERING_RAVAGER,           SPELL_THUNDERING_RAVAGER_PRESENCE, { { SPELL_THUNDERING_RAVAGER_STORM_BREAK, VaultCastAt::Area, 8000, 20000 }, { SPELL_KUROG_THUNDER_STRIKE, VaultCastAt::Ground, 15000, 30000 } } },

        // Kurog's wrought elementals
        { NPC_FLAMEWROUGHT_ERADICATOR,      0, { { SPELL_FLAMEWROUGHT_FLAME_SMITE, VaultCastAt::Victim, 5000, 10000 }, { SPELL_FLAMEWROUGHT_RAGING_INFERNO, VaultCastAt::Player, 12000, 25000 } } },
        { NPC_FROSTWROUGHT_DOMINATOR,       0, { { SPELL_FROSTWROUGHT_FROST_SMITE, VaultCastAt::Victim, 5000, 10000 }, { SPELL_FROSTWROUGHT_ICY_TEMPEST, VaultCastAt::Player, 12000, 25000 } } },
        { NPC_EARTHWROUGHT_SMASHER,         0, { { SPELL_EARTHWROUGHT_EARTH_SMITE, VaultCastAt::Victim, 5000, 10000 }, { SPELL_KUROG_ERUPTING_BEDROCK, VaultCastAt::Player, 12000, 22000 } } },
        { NPC_STORMWROUGHT_DESPOILER,       0, { { SPELL_STORMWROUGHT_STORM_SMITE, VaultCastAt::Victim, 5000, 10000 }, { SPELL_STORMWROUGHT_ORB_LIGHTNING, VaultCastAt::Player, 12000, 20000 } } },
        { NPC_EARTH_BREAKER,                0, { { SPELL_EARTH_BREAKER_SKELETAL_FRACTURES, VaultCastAt::Victim, 2000, 4000 }, NoAddSpell } },

        // Raszageth's primalist forces
        { NPC_SURGING_RUINER,               SPELL_RUINER_SURGE, { { SPELL_RUINER_SURGING_BLAST, VaultCastAt::Area, 8000, 15000 }, NoAddSpell } },
        { NPC_OATHSWORN_VANGUARD,           0, { { SPELL_VANGUARD_WINDFORCE_STRIKES, VaultCastAt::Self, 4000, 20000 }, NoAddSpell } },
        { NPC_OATHSWORN_VANGUARD_ALT,       0, { { SPELL_VANGUARD_WINDFORCE_STRIKES, VaultCastAt::Self, 4000, 20000 }, NoAddSpell } },
        { NPC_STORMSEEKER_ACOLYTE,          0, { { SPELL_ACOLYTE_STORM_BOLT, VaultCastAt::Player, 2000, 4000 }, { SPELL_ACOLYTE_ASCENSION, VaultCastAt::Self, 20000, 30000 } } },
        { NPC_STORMSEEKER_ACOLYTE_ALT,      0, { { SPELL_ACOLYTE_STORM_BOLT, VaultCastAt::Player, 2000, 4000 }, { SPELL_ACOLYTE_ASCENSION, VaultCastAt::Self, 20000, 30000 } } },
        { NPC_FROSTFORGED_ZEALOT,           0, { { SPELL_ZEALOT_SHATTERING_SHROUD, VaultCastAt::Area, 6000, 20000 }, NoAddSpell } },
        { NPC_FLAMESWORN_HERALD,            0, { { SPELL_HERALD_FLAME_SHIELD, VaultCastAt::Self, 5000, 25000 }, NoAddSpell } },

        // Diurna's primalist reinforcements
        { NPC_TARASEK_LEGIONNAIRE,          0, { { SPELL_LEGIONNAIRE_VICIOUS_THRUST, VaultCastAt::Victim, 4000, 10000 }, NoAddSpell } },
        { NPC_TARASEK_LEGIONNAIRE_ALT,      0, { { SPELL_LEGIONNAIRE_VICIOUS_THRUST, VaultCastAt::Victim, 4000, 10000 }, NoAddSpell } },
        { NPC_PRIMALIST_MAGE,               0, { { SPELL_MAGE_ICE_BARRAGE, VaultCastAt::Player, 3000, 8000 }, NoAddSpell } },
        { NPC_TARASEK_EARTHREAVER,          0, { { SPELL_EARTHREAVER_BURROWING_STRIKE, VaultCastAt::Victim, 5000, 12000 }, { SPELL_EARTHREAVER_TREMORS, VaultCastAt::Ground, 10000, 20000 } } },
        { NPC_DRAGONSPAWN_FLAMEBENDER,      0, { { SPELL_FLAMEBENDER_FLAME_SENTRY, VaultCastAt::Area, 6000, 25000 }, { SPELL_FLAMEBENDER_CAUTERIZING_FLASHFLAMES, VaultCastAt::Area, 10000, 20000 } } },
        { NPC_DRAGONSPAWN_FLAMEBENDER_ALT,  0, { { SPELL_FLAMEBENDER_FLAME_SENTRY, VaultCastAt::Area, 6000, 25000 }, { SPELL_FLAMEBENDER_CAUTERIZING_FLASHFLAMES, VaultCastAt::Area, 10000, 20000 } } },
        { NPC_JUVENILE_FROST_PROTO_DRAGON,  0, { { SPELL_PROTO_DRAGON_RENDING_BITE, VaultCastAt::Victim, 4000, 10000 }, { SPELL_PROTO_DRAGON_CHILLING_TANTRUM, VaultCastAt::Self, 12000, 25000 } } },
        { NPC_DRAKONID_STORMBRINGER,        0, { { SPELL_STORMBRINGER_STATIC_JOLT, VaultCastAt::Victim, 3000, 8000 }, { SPELL_STORMBRINGER_IONIZING_CHARGE, VaultCastAt::Area, 10000, 22000 } } },

        // Dathea's storm servants
        { NPC_VOLATILE_INFUSER,             0, { { SPELL_INFUSER_AERIAL_SLASH, VaultCastAt::Victim, 4000, 8000 }, { SPELL_INFUSER_UNSTABLE_GUSTS, VaultCastAt::Ground, 8000, 15000 } } },
        { NPC_VOLATILE_INFUSER_ALT,         0, { { SPELL_INFUSER_AERIAL_SLASH, VaultCastAt::Victim, 4000, 8000 }, { SPELL_INFUSER_UNSTABLE_GUSTS, VaultCastAt::Ground, 8000, 15000 } } },
        { NPC_THUNDER_CALLER,               0, { { SPELL_THUNDER_CALLER_STORM_BOLT, VaultCastAt::Area, 3000, 6000 }, { SPELL_THUNDER_CALLER_THUNDERBOLT, VaultCastAt::Player, 10000, 15000 } } }
    };

    VaultAddKit const* findAddKit(uint32_t entry)
    {
        for (VaultAddKit const& kit : VaultAddKits)
        {
            if (kit.entry == entry)
                return &kit;
        }

        return nullptr;
    }

    LocationVector vaultRingPoint(LocationVector const& origin, float minRange, float maxRange)
    {
        float const angle = Util::getRandomFloat(VaultTwoPi);
        float const range = Util::getRandomFloat(minRange, maxRange);
        return LocationVector(origin.x + range * std::cos(angle), origin.y + range * std::sin(angle), origin.z, angle);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class VaultOfTheIncarnatesInstanceScript : public InstanceScript
{
public:
    explicit VaultOfTheIncarnatesInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(VaultOfTheIncarnatesEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new VaultOfTheIncarnatesInstanceScript(pMapMgr); }

    // a council member brought down early must not end the encounter
    bool setBossState(uint32_t id, EncounterStates state) override
    {
        if (state == EncounterStates::Performed && getBossState(id) == EncounterStates::InProgress && !isEncounterDown(id, nullptr))
            return false;

        return InstanceScript::setBossState(id, state);
    }

    void OnCreaturePushToWorld(Creature* pCreature) override
    {
        for (uint32_t entry : PrimalCouncilMembers)
        {
            if (pCreature->getEntry() == entry)
            {
                mCouncilGuids[entry] = pCreature->GetNewGUID();
                return;
            }
        }
    }

    void OnEncounterStateChange(uint32_t id, uint32_t state) override
    {
        if (id != DATA_PRIMAL_COUNCIL || state != EncounterStates::Failed)
            return;

        // a wiped council stands again in full
        for (Creature* pMember : getCouncil())
        {
            if (!pMember->isAlive())
                pMember->despawn(1000, 5000);
        }
    }

    std::vector<Creature*> getCouncil()
    {
        std::vector<Creature*> council;
        for (auto const& member : mCouncilGuids)
        {
            if (Creature* pMember = getCreatureByGuid(member.second))
                council.push_back(pMember);
        }

        return council;
    }

    // single boss encounters are down with their boss
    bool isEncounterDown(uint32_t dataId, Creature* pExcept)
    {
        if (dataId != DATA_PRIMAL_COUNCIL)
            return true;

        for (Creature* pMember : getCouncil())
        {
            if (pMember != pExcept && pMember->isAlive())
                return false;
        }

        return true;
    }

    bool isEncounterFighting(uint32_t dataId, Creature* pExcept)
    {
        if (dataId != DATA_PRIMAL_COUNCIL)
            return false;

        for (Creature* pMember : getCouncil())
        {
            if (pMember != pExcept && pMember->isAlive() && pMember->isInCombat())
                return true;
        }

        return false;
    }

    // a living member that has not started its convocation yet
    bool councilHoldsOut(Creature* pExcept)
    {
        for (Creature* pMember : getCouncil())
        {
            if (pMember == pExcept || !pMember->isAlive())
                continue;

            CreatureAIScript* pScript = pMember->GetScript();
            if (pScript == nullptr || pScript->GetCreatureData(CouncilConvocationQuery) == 0)
                return true;
        }

        return false;
    }

    // pulling one member pulls the council
    void rallyCouncil(CreatureAIScript* pPuller)
    {
        if (mRallying)
            return;

        mRallying = true;
        for (Creature* pMember : getCouncil())
        {
            if (pMember == pPuller->getCreature() || !pMember->isAlive() || pMember->isInCombat())
                continue;

            pPuller->setZoneWideCombat(pMember);
        }
        mRallying = false;
    }

    // the channeling members fall together with the last one
    void collapseCouncil(Creature* pSource)
    {
        if (mCollapsing)
            return;

        mCollapsing = true;
        for (Creature* pMember : getCouncil())
        {
            if (pMember == pSource || !pMember->isAlive())
                continue;

            CreatureAIScript* pScript = pMember->GetScript();
            if (pScript != nullptr && pScript->GetCreatureData(CouncilConvocationQuery) != 0)
                pMember->dealDamage(pMember, pMember->getHealth(), 0);
        }
        mCollapsing = false;
    }

private:
    std::map<uint32_t, WoWGuid> mCouncilGuids;
    bool mRallying = false;
    bool mCollapsing = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event clock, berserk and target helpers

class VaultOfTheIncarnatesCombatAI : public CreatureAIScript
{
public:
    explicit VaultOfTheIncarnatesCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

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

        if (isAlive())
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

            onEvent(eventId);
        }
    }

protected:
    virtual void onEngage() {}
    virtual void onDisengage() {}
    virtual void onCombatTick(uint32_t /*diff*/) {}
    virtual void onEvent(uint32_t /*eventId*/) {}

    VaultOfTheIncarnatesInstanceScript* getVault()
    {
        if (getCreature()->GetMapId() != MAP_VAULT_OF_THE_INCARNATES)
            return nullptr;

        return static_cast<VaultOfTheIncarnatesInstanceScript*>(getInstanceScript());
    }

    Unit* currentVictim() { return getCreature()->getAIInterface()->getCurrentTarget(); }

    bool isVictimOutOfReach(float reach)
    {
        Unit* pVictim = currentVictim();
        return pVictim != nullptr && getCreature()->getDistance(pVictim) > reach;
    }

    // distinct random players, the tank only when nobody else is around
    std::vector<Unit*> pickPlayers(size_t count, bool skipTank = false)
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

        size_t const picks = pool.size() < count ? pool.size() : count;
        for (size_t i = 0; i < picks; ++i)
        {
            size_t const swapWith = i + Util::getRandomUInt(static_cast<uint32_t>(pool.size() - 1 - i));
            std::swap(pool[i], pool[swapWith]);
        }

        pool.resize(picks);
        return pool;
    }

    Unit* pickPlayer(bool skipTank = false)
    {
        std::vector<Unit*> const picked = pickPlayers(1, skipTank);
        return picked.empty() ? nullptr : picked.front();
    }

    Unit* castAtPlayer(uint32_t spellId, bool skipTank = false, bool triggered = false)
    {
        Unit* pTarget = pickPlayer(skipTank);
        if (pTarget != nullptr)
            castSpell(pTarget, spellId, triggered);

        return pTarget;
    }

    void castAtPlayers(uint32_t spellId, size_t count, bool skipTank = false)
    {
        for (Unit* pTarget : pickPlayers(count, skipTank))
            castSpell(pTarget, spellId, true);
    }

    // ground targeted spell under a random player
    void castUnderPlayer(uint32_t spellId, bool skipTank = false, bool triggered = false)
    {
        if (Unit* pTarget = pickPlayer(skipTank))
            getCreature()->castSpellLoc(pTarget->GetPosition(), spellId, triggered);
    }

    void castBy(VaultCastAt target, uint32_t spellId)
    {
        switch (target)
        {
            case VaultCastAt::Victim:
                castSpellOnVictim(spellId);
                break;
            case VaultCastAt::Player:
                castAtPlayer(spellId);
                break;
            case VaultCastAt::Ground:
                castUnderPlayer(spellId);
                break;
            case VaultCastAt::Self:
                castSpellOnSelf(spellId);
                break;
            default:
                castSpellAOE(spellId);
                break;
        }
    }

    // summoned add that joins the fight at once
    Creature* callAdd(uint32_t entry, LocationVector const& spot, CreatureSummonDespawnType despawnType = CORPSE_DESPAWN, uint32_t durationMs = 0)
    {
        Creature* pAdd = summonCreature(entry, spot, despawnType, durationMs);
        if (pAdd != nullptr)
            setZoneWideCombat(pAdd);

        return pAdd;
    }

    LocationVector aroundSelf(float minRange, float maxRange) { return vaultRingPoint(getCreature()->GetPosition(), minRange, maxRange); }

    bool anyAlive(std::vector<uint64_t> const& guids)
    {
        for (uint64_t guid : guids)
        {
            Creature* pAdd = getCreature()->getWorldMapCreature(guid);
            if (pAdd != nullptr && pAdd->isAlive())
                return true;
        }

        return false;
    }

    void warn(char const* text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }

    void armBerserk(uint32_t delayMs) { mBerserkLeftMs = delayMs; }

    // a new stage drops every pending event
    void enterStage(uint32_t stage)
    {
        mStage = stage;
        scriptEvents.resetEvents();
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
        castSpellOnSelf(SPELL_VAULT_INCARNATES_BERSERK, true);
    }

    uint32_t mBerserkLeftMs = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame, boss state and council pull

class VaultOfTheIncarnatesBossAI : public VaultOfTheIncarnatesCombatAI
{
public:
    VaultOfTheIncarnatesBossAI(Creature* pCreature, uint32_t dataId) : VaultOfTheIncarnatesCombatAI(pCreature), mDataId(dataId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mDataId, EncounterStates::InProgress);
        }

        if (mDataId == DATA_PRIMAL_COUNCIL)
        {
            if (VaultOfTheIncarnatesInstanceScript* pVault = getVault())
                pVault->rallyCouncil(this);
        }

        VaultOfTheIncarnatesCombatAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        if (isAlive())
        {
            if (InstanceScript* pInstance = getInstanceScript())
            {
                pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

                // the wipe counts once the last council member has left the fight
                VaultOfTheIncarnatesInstanceScript* pVault = getVault();
                if (pVault == nullptr || !pVault->isEncounterFighting(mDataId, getCreature()))
                    pInstance->setBossState(mDataId, EncounterStates::Failed);
            }
        }

        VaultOfTheIncarnatesCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

            VaultOfTheIncarnatesInstanceScript* pVault = getVault();
            if (pVault == nullptr || pVault->isEncounterDown(mDataId, getCreature()))
                pInstance->setBossState(mDataId, EncounterStates::Performed);
        }

        scriptEvents.resetEvents();
        summons.despawnAll();
    }

protected:
    uint32_t mDataId;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Adds with a presence aura and timed abilities from the kit table

class VaultOfTheIncarnatesAddAI : public VaultOfTheIncarnatesCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VaultOfTheIncarnatesAddAI(c); }
    explicit VaultOfTheIncarnatesAddAI(Creature* pCreature) : VaultOfTheIncarnatesCombatAI(pCreature), mKit(findAddKit(pCreature->getEntry())) {}

protected:
    void onEngage() override
    {
        if (mKit == nullptr)
            return;

        if (mKit->presence != 0)
            castSpellOnSelf(mKit->presence, true);

        for (uint32_t slot = 0; slot < 2; ++slot)
        {
            if (mKit->abilities[slot].spellId != 0)
                scriptEvents.addEvent(EVENT_VAULT_ADD_FIRST + slot, mKit->abilities[slot].firstMs);
        }
    }

    void onEvent(uint32_t eventId) override
    {
        if (mKit == nullptr || eventId < EVENT_VAULT_ADD_FIRST || eventId > EVENT_VAULT_ADD_SECOND)
            return;

        VaultAddSpell const& ability = mKit->abilities[eventId - EVENT_VAULT_ADD_FIRST];
        castBy(ability.target, ability.spellId);

        if (ability.repeatMs != 0)
            scriptEvents.addEvent(eventId, ability.repeatMs);
    }

private:
    VaultAddKit const* mKit;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Eranog

class VaultOfTheIncarnatesEranogAI : public VaultOfTheIncarnatesBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VaultOfTheIncarnatesEranogAI(c); }
    explicit VaultOfTheIncarnatesEranogAI(Creature* pCreature) : VaultOfTheIncarnatesBossAI(pCreature, DATA_ERANOG) {}

protected:
    void onEngage() override
    {
        armBerserk(EranogBerserkMs);
        castSpellOnSelf(SPELL_ERANOG_BURNING_WOUND, true);
        beginArmyOfTalon();
    }

    void onDisengage() override
    {
        mRiftSpots.clear();
        _removeAura(SPELL_ERANOG_BURNING_WOUND);
        _removeAura(SPELL_ERANOG_ARMY_OF_FLAME);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ERANOG_FLAMERIFT:
                markRifts();
                scriptEvents.addEvent(EVENT_ERANOG_RIFT_OPEN, 5000);
                scriptEvents.addEvent(EVENT_ERANOG_FLAMERIFT, 32000);
                break;
            case EVENT_ERANOG_RIFT_OPEN:
                openRifts();
                break;
            case EVENT_ERANOG_MOLTEN_CLEAVE:
                warn("Eranog raises his axe for a Molten Cleave!");
                castSpellOnVictim(SPELL_ERANOG_MOLTEN_CLEAVE);
                scriptEvents.addEvent(EVENT_ERANOG_MOLTEN_CLEAVE, 26000);
                break;
            case EVENT_ERANOG_INCINERATING_ROAR:
                castSpellAOE(SPELL_ERANOG_INCINERATING_ROAR);
                scriptEvents.addEvent(EVENT_ERANOG_INCINERATING_ROAR, 22000);
                break;
            case EVENT_ERANOG_MOLTEN_SPIKES:
                castUnderPlayer(SPELL_ERANOG_MOLTEN_SPIKES, true);
                scriptEvents.addEvent(EVENT_ERANOG_MOLTEN_SPIKES, 28000);
                break;
            case EVENT_ERANOG_ARMY_OF_FLAME:
                beginArmyOfFlame();
                break;
            case EVENT_ERANOG_ARMY_FADES:
                _removeAura(SPELL_ERANOG_ARMY_OF_FLAME);
                beginArmyOfTalon();
                break;
            default:
                break;
        }
    }

private:
    void beginArmyOfTalon()
    {
        enterStage(1);
        mRiftSpots.clear();

        scriptEvents.addEvent(EVENT_ERANOG_FLAMERIFT, 8000);
        scriptEvents.addEvent(EVENT_ERANOG_MOLTEN_CLEAVE, 14000);
        scriptEvents.addEvent(EVENT_ERANOG_INCINERATING_ROAR, 18000);
        scriptEvents.addEvent(EVENT_ERANOG_MOLTEN_SPIKES, 24000);
        scriptEvents.addEvent(EVENT_ERANOG_ARMY_OF_FLAME, EranogEnergyFullMs);
    }

    // full energy: a ring of Primal Flames marches out
    void beginArmyOfFlame()
    {
        enterStage(2);
        warn("Eranog calls forth an Army of Flame!");
        castSpellOnSelf(SPELL_ERANOG_ARMY_OF_FLAME);

        uint32_t const flames = isHeroic() ? 8 : 6;
        LocationVector const origin = getCreature()->GetPosition();
        for (uint32_t i = 0; i < flames; ++i)
        {
            float const angle = VaultTwoPi * static_cast<float>(i) / static_cast<float>(flames);
            LocationVector const spot(origin.x + EranogArmyRing * std::cos(angle), origin.y + EranogArmyRing * std::sin(angle), origin.z, angle);
            callAdd(NPC_PRIMAL_FLAME, spot, TIMED_DESPAWN, EranogArmyMs + 3000);
        }

        scriptEvents.addEvent(EVENT_ERANOG_ARMY_FADES, EranogArmyMs);
    }

    void markRifts()
    {
        castSpellAOE(SPELL_ERANOG_FLAMERIFT);

        mRiftSpots.clear();
        for (Unit* pTarget : pickPlayers(isHeroic() ? 3 : 2, true))
            mRiftSpots.push_back(pTarget->GetPosition());
    }

    // each rift leaves a pool and lets a Flamescale Tarasek through
    void openRifts()
    {
        for (LocationVector const& spot : mRiftSpots)
        {
            getCreature()->castSpellLoc(spot, SPELL_ERANOG_LAVA_FLOW, true);
            callAdd(NPC_FLAMESCALE_TARASEK, spot);
        }

        mRiftSpots.clear();
    }

    std::vector<LocationVector> mRiftSpots;
};

// fixates one player with Kill Order
class VaultOfTheIncarnatesFlamescaleTarasekAI : public VaultOfTheIncarnatesCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VaultOfTheIncarnatesFlamescaleTarasekAI(c); }
    explicit VaultOfTheIncarnatesFlamescaleTarasekAI(Creature* pCreature) : VaultOfTheIncarnatesCombatAI(pCreature) {}

protected:
    void onEngage() override { scriptEvents.addEvent(EVENT_TARASEK_KILL_ORDER, 500); }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_TARASEK_KILL_ORDER)
            return;

        if (Unit* pTarget = pickPlayer(true))
        {
            castSpell(pTarget, SPELL_TARASEK_KILL_ORDER, true);
            addThreat(pTarget, 1000000.0f);
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// The Primal Council

// at 1 health a member channels its convocation until the whole council is down
class VaultOfTheIncarnatesCouncilMemberAI : public VaultOfTheIncarnatesBossAI
{
public:
    VaultOfTheIncarnatesCouncilMemberAI(Creature* pCreature, uint32_t convocationSpell, char const* convocationText)
        : VaultOfTheIncarnatesBossAI(pCreature, DATA_PRIMAL_COUNCIL), mConvocationSpell(convocationSpell), mConvocationText(convocationText) {}

    uint32_t GetCreatureData(uint32_t type) const override
    {
        return type == CouncilConvocationQuery && mConvoking ? 1 : 0;
    }

    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage == nullptr || !_isInCombat())
            return;

        uint32_t const health = getCreature()->getHealth();
        if (*damage < health)
            return;

        // the last member standing falls, the others hold on
        VaultOfTheIncarnatesInstanceScript* pVault = getVault();
        if (pVault == nullptr || !pVault->councilHoldsOut(getCreature()))
            return;

        *damage = health > 1 ? health - 1 : 0;
        if (!mConvoking)
            beginConvocation();
    }

    void OnDied(Unit* pKiller) override
    {
        VaultOfTheIncarnatesBossAI::OnDied(pKiller);

        VaultOfTheIncarnatesInstanceScript* pVault = getVault();
        if (pVault != nullptr && !pVault->councilHoldsOut(getCreature()))
            pVault->collapseCouncil(getCreature());
    }

protected:
    void onEngage() override
    {
        mConvoking = false;
        mConvocationPending = false;
        armBerserk(CouncilBerserkMs);
        onMemberEngage();
    }

    void onDisengage() override
    {
        mConvoking = false;
        mConvocationPending = false;
        setRooted(false);
        _setMeleeDisabled(false);
        _removeAura(mConvocationSpell);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (!mConvocationPending)
            return;

        // the channel breaks whatever the member was casting
        mConvocationPending = false;
        getCreature()->interruptSpell();
        castSpellOnSelf(mConvocationSpell);
        scriptEvents.addEvent(EVENT_COUNCIL_CONVOCATION, 5000);
    }

    void onEvent(uint32_t eventId) override
    {
        if (eventId == EVENT_COUNCIL_CONVOCATION)
        {
            if (!hasAura(mConvocationSpell))
                castSpellOnSelf(mConvocationSpell);

            scriptEvents.addEvent(EVENT_COUNCIL_CONVOCATION, 5000);
            return;
        }

        if (!mConvoking)
            onMemberEvent(eventId);
    }

    virtual void onMemberEngage() {}
    virtual void onMemberEvent(uint32_t /*eventId*/) {}

private:
    void beginConvocation()
    {
        mConvoking = true;
        mConvocationPending = true;
        scriptEvents.resetEvents();
        setRooted(true);
        _setMeleeDisabled(true);
        warn(mConvocationText);
    }

    uint32_t mConvocationSpell;
    char const* mConvocationText;
    bool mConvoking = false;
    bool mConvocationPending = false;
};

class VaultOfTheIncarnatesKadrosAI : public VaultOfTheIncarnatesCouncilMemberAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VaultOfTheIncarnatesKadrosAI(c); }
    explicit VaultOfTheIncarnatesKadrosAI(Creature* pCreature)
        : VaultOfTheIncarnatesCouncilMemberAI(pCreature, SPELL_KADROS_GLACIAL_CONVOCATION, "Kadros Icewrath begins a Glacial Convocation!") {}

protected:
    void onMemberEngage() override
    {
        scriptEvents.addEvent(EVENT_KADROS_FROST_SPIKE, 3000);
        scriptEvents.addEvent(EVENT_KADROS_PRIMAL_BLIZZARD, 30000);
    }

    void onMemberEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_KADROS_FROST_SPIKE:
                castSpellOnVictim(SPELL_KADROS_FROST_SPIKE);
                scriptEvents.addEvent(EVENT_KADROS_FROST_SPIKE, 7000);
                break;
            case EVENT_KADROS_PRIMAL_BLIZZARD:
                warn("Kadros Icewrath calls down a Primal Blizzard!");
                castSpellAOE(SPELL_KADROS_PRIMAL_BLIZZARD);
                scriptEvents.addEvent(EVENT_KADROS_PRIMAL_BLIZZARD, 75000);
                break;
            default:
                break;
        }
    }
};

class VaultOfTheIncarnatesStormlashAI : public VaultOfTheIncarnatesCouncilMemberAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VaultOfTheIncarnatesStormlashAI(c); }
    explicit VaultOfTheIncarnatesStormlashAI(Creature* pCreature)
        : VaultOfTheIncarnatesCouncilMemberAI(pCreature, SPELL_STORMLASH_STORMING_CONVOCATION, "Dathea Stormlash begins a Storming Convocation!") {}

protected:
    void onMemberEngage() override
    {
        scriptEvents.addEvent(EVENT_STORMLASH_LIGHTNING_BOLT, 2000);
        scriptEvents.addEvent(EVENT_STORMLASH_CHAIN_LIGHTNING, 10000);
        scriptEvents.addEvent(EVENT_STORMLASH_CONDUCTIVE_MARK, 15000);
    }

    void onMemberEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_STORMLASH_LIGHTNING_BOLT:
                castAtPlayer(SPELL_STORMLASH_LIGHTNING_BOLT);
                scriptEvents.addEvent(EVENT_STORMLASH_LIGHTNING_BOLT, 6000);
                break;
            case EVENT_STORMLASH_CHAIN_LIGHTNING:
                castAtPlayer(SPELL_STORMLASH_CHAIN_LIGHTNING, true);
                scriptEvents.addEvent(EVENT_STORMLASH_CHAIN_LIGHTNING, 16000);
                break;
            case EVENT_STORMLASH_CONDUCTIVE_MARK:
                castAtPlayers(SPELL_STORMLASH_CONDUCTIVE_MARK, isHeroic() ? 3 : 2, true);
                scriptEvents.addEvent(EVENT_STORMLASH_CONDUCTIVE_MARK, 30000);
                break;
            default:
                break;
        }
    }
};

class VaultOfTheIncarnatesOpalfangAI : public VaultOfTheIncarnatesCouncilMemberAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VaultOfTheIncarnatesOpalfangAI(c); }
    explicit VaultOfTheIncarnatesOpalfangAI(Creature* pCreature)
        : VaultOfTheIncarnatesCouncilMemberAI(pCreature, SPELL_OPALFANG_QUAKING_CONVOCATION, "Opalfang begins a Quaking Convocation!") {}

protected:
    void onMemberEngage() override
    {
        scriptEvents.addEvent(EVENT_OPALFANG_CRUSH, 10000);
        scriptEvents.addEvent(EVENT_OPALFANG_EARTHEN_PILLAR, 18000);
    }

    void onMemberEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_OPALFANG_CRUSH:
                castSpellOnVictim(SPELL_OPALFANG_CRUSH);
                scriptEvents.addEvent(EVENT_OPALFANG_CRUSH, 22000);
                break;
            case EVENT_OPALFANG_EARTHEN_PILLAR:
                for (Unit* pTarget : pickPlayers(isHeroic() ? 3 : 2))
                    getCreature()->castSpellLoc(pTarget->GetPosition(), SPELL_OPALFANG_EARTHEN_PILLAR, true);

                // heroic pillars erupt with Quaking Pulse
                if (isHeroic())
                    scriptEvents.addEvent(EVENT_OPALFANG_QUAKING_PULSE, 3000);
                scriptEvents.addEvent(EVENT_OPALFANG_EARTHEN_PILLAR, 30000);
                break;
            case EVENT_OPALFANG_QUAKING_PULSE:
                castSpellAOE(SPELL_OPALFANG_QUAKING_PULSE, true);
                break;
            default:
                break;
        }
    }
};

class VaultOfTheIncarnatesEmbarAI : public VaultOfTheIncarnatesCouncilMemberAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VaultOfTheIncarnatesEmbarAI(c); }
    explicit VaultOfTheIncarnatesEmbarAI(Creature* pCreature)
        : VaultOfTheIncarnatesCouncilMemberAI(pCreature, SPELL_EMBAR_BURNING_CONVOCATION, "Embar Firepath begins a Burning Convocation!") {}

protected:
    void onMemberEngage() override
    {
        mAxeTargets.clear();
        scriptEvents.addEvent(EVENT_EMBAR_SLASHING_BLAZE, 12000);
        scriptEvents.addEvent(EVENT_EMBAR_METEOR_AXES, 22000);
    }

    void onMemberEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_EMBAR_SLASHING_BLAZE:
                castSpellOnVictim(SPELL_EMBAR_SLASHING_BLAZE);
                scriptEvents.addEvent(EVENT_EMBAR_SLASHING_BLAZE, 25000);
                break;
            case EVENT_EMBAR_METEOR_AXES:
                warn("Embar Firepath hurls Meteor Axes!");
                mAxeTargets.clear();
                for (Unit* pTarget : pickPlayers(2, true))
                {
                    castSpell(pTarget, SPELL_EMBAR_METEOR_AXE_MARK, true);
                    mAxeTargets.push_back(pTarget->getGuid());
                }
                scriptEvents.addEvent(EVENT_EMBAR_AXES_LAND, 5000);
                scriptEvents.addEvent(EVENT_EMBAR_METEOR_AXES, 40000);
                break;
            case EVENT_EMBAR_AXES_LAND:
                for (uint64_t guid : mAxeTargets)
                {
                    Unit* pTarget = getCreature()->getWorldMapUnit(guid);
                    if (pTarget != nullptr && pTarget->isAlive())
                        castSpell(pTarget, SPELL_EMBAR_METEOR_AXE_THROW, true);
                }
                mAxeTargets.clear();
                break;
            default:
                break;
        }
    }

private:
    std::vector<uint64_t> mAxeTargets;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Sennarth, the Cold Breath, fought on the ground floor without the ascent

class VaultOfTheIncarnatesSennarthAI : public VaultOfTheIncarnatesBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VaultOfTheIncarnatesSennarthAI(c); }
    explicit VaultOfTheIncarnatesSennarthAI(Creature* pCreature) : VaultOfTheIncarnatesBossAI(pCreature, DATA_SENNARTH) {}

protected:
    void onEngage() override
    {
        armBerserk(SennarthBerserkMs);
        beginIceClimbers();
    }

    void onDisengage() override { _removeAura(SPELL_SENNARTH_APEX_OF_ICE); }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mStage == 1 && _getHealthPercent() <= SennarthPrecipicePct)
            beginFrozenPrecipice();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SENNARTH_WEB_BLAST:
                castSpellOnVictim(SPELL_SENNARTH_WEB_BLAST);
                scriptEvents.addEvent(EVENT_SENNARTH_WEB_BLAST, 15000);
                break;
            case EVENT_SENNARTH_CHILLING_BLAST:
                castSpellAOE(SPELL_SENNARTH_CHILLING_BLAST);
                scriptEvents.addEvent(EVENT_SENNARTH_CHILLING_BLAST, 30000);
                break;
            case EVENT_SENNARTH_WEBS:
                castSpellAOE(mStage == 1 ? SPELL_SENNARTH_ENVELOPING_WEBS : SPELL_SENNARTH_SUFFOCATING_WEBS);
                scriptEvents.addEvent(EVENT_SENNARTH_WEBS, 30000);
                break;
            case EVENT_SENNARTH_BURST:
                if (mStage == 1)
                {
                    warn("Sennarth screams, a Gossamer Burst pulls you in!");
                    castSpellAOE(SPELL_SENNARTH_GOSSAMER_BURST);
                }
                else
                {
                    warn("Sennarth screams, a Repelling Burst throws you back!");
                    castSpellAOE(SPELL_SENNARTH_REPELLING_BURST);
                }
                scriptEvents.addEvent(EVENT_SENNARTH_BURST, 36000);
                break;
            case EVENT_SENNARTH_BREATH_OF_ICE:
                warn("Sennarth draws in a Breath of Ice!");
                castSpellAOE(SPELL_SENNARTH_BREATH_OF_ICE);
                scriptEvents.addEvent(EVENT_SENNARTH_BREATH_OF_ICE, 42000);
                break;
            case EVENT_SENNARTH_SPIDERLINGS:
                for (uint32_t i = 0, count = isHeroic() ? 6 : 4; i < count; ++i)
                    callAdd(NPC_CAUSTIC_SPIDERLING, aroundSelf(15.0f, 25.0f));
                scriptEvents.addEvent(EVENT_SENNARTH_SPIDERLINGS, 30000);
                break;
            case EVENT_SENNARTH_ARACHNID:
                warn("A Frostbreath Arachnid joins the fight!");
                callAdd(NPC_FROSTBREATH_ARACHNID, aroundSelf(15.0f, 20.0f));
                scriptEvents.addEvent(EVENT_SENNARTH_ARACHNID, 45000);
                break;
            case EVENT_SENNARTH_PERVASIVE_COLD:
                castSpellAOE(SPELL_SENNARTH_PERVASIVE_COLD, true);
                scriptEvents.addEvent(EVENT_SENNARTH_PERVASIVE_COLD, 12000);
                break;
            case EVENT_SENNARTH_PERMAFROST:
                castSpellAOE(SPELL_SENNARTH_PERMAFROST);
                scriptEvents.addEvent(EVENT_SENNARTH_PERMAFROST, 35000);
                break;
            default:
                break;
        }
    }

private:
    void beginIceClimbers()
    {
        enterStage(1);
        scriptEvents.addEvent(EVENT_SENNARTH_WEB_BLAST, 6000);
        scriptEvents.addEvent(EVENT_SENNARTH_CHILLING_BLAST, 10000);
        scriptEvents.addEvent(EVENT_SENNARTH_WEBS, 15000);
        scriptEvents.addEvent(EVENT_SENNARTH_SPIDERLINGS, 18000);
        scriptEvents.addEvent(EVENT_SENNARTH_BURST, 26000);
        scriptEvents.addEvent(EVENT_SENNARTH_ARACHNID, 32000);
        scriptEvents.addEvent(EVENT_SENNARTH_BREATH_OF_ICE, 40000);
    }

    // the summit fight stays in the same room
    void beginFrozenPrecipice()
    {
        enterStage(2);
        warn("Sennarth reaches the Frozen Precipice!");
        castSpellOnSelf(SPELL_SENNARTH_APEX_OF_ICE, true);

        scriptEvents.addEvent(EVENT_SENNARTH_PERVASIVE_COLD, 5000);
        scriptEvents.addEvent(EVENT_SENNARTH_WEB_BLAST, 6000);
        scriptEvents.addEvent(EVENT_SENNARTH_CHILLING_BLAST, 9000);
        scriptEvents.addEvent(EVENT_SENNARTH_WEBS, 14000);
        scriptEvents.addEvent(EVENT_SENNARTH_SPIDERLINGS, 20000);
        scriptEvents.addEvent(EVENT_SENNARTH_BURST, 24000);
        scriptEvents.addEvent(EVENT_SENNARTH_PERMAFROST, 30000);
    }
};

// runs at its target and bursts there
class VaultOfTheIncarnatesCausticSpiderlingAI : public VaultOfTheIncarnatesCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VaultOfTheIncarnatesCausticSpiderlingAI(c); }
    explicit VaultOfTheIncarnatesCausticSpiderlingAI(Creature* pCreature) : VaultOfTheIncarnatesCombatAI(pCreature) {}

protected:
    void onEngage() override
    {
        mErupted = false;
        scriptEvents.addEvent(EVENT_SPIDERLING_ERUPTION, 1000);
    }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_SPIDERLING_ERUPTION || mErupted)
            return;

        Unit* pVictim = currentVictim();
        if (pVictim == nullptr || getCreature()->getDistance(pVictim) > SpiderlingEruptionRange)
        {
            scriptEvents.addEvent(EVENT_SPIDERLING_ERUPTION, 1000);
            return;
        }

        mErupted = true;
        castSpellAOE(SPELL_SPIDERLING_CAUSTIC_ERUPTION, true);
        despawn(500, 0);
    }

private:
    bool mErupted = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Kurog Grimtotem, altars visited in turn without moving between them

class VaultOfTheIncarnatesKurogAI : public VaultOfTheIncarnatesBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VaultOfTheIncarnatesKurogAI(c); }
    explicit VaultOfTheIncarnatesKurogAI(Creature* pCreature) : VaultOfTheIncarnatesBossAI(pCreature, DATA_KUROG_GRIMTOTEM) {}

protected:
    void onEngage() override
    {
        armBerserk(KurogBerserkMs);
        mBarriersRaised = 0;
        mCreations.clear();
        mAltar = static_cast<uint8_t>(Util::getRandomUInt(KUROG_ALTAR_COUNT - 1));
        reachAltar();
        beginDominance(KurogAltarStayMs);
    }

    void onDisengage() override
    {
        mCreations.clear();
        dropAltarAuras();
        _removeAura(SPELL_KUROG_PRIMAL_BARRIER);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mStage == 1 && mBarriersRaised < 2 && _getHealthPercent() <= KurogBarrierMarks[mBarriersRaised])
            raiseBarrier();
    }

    void onEvent(uint32_t eventId) override
    {
        KurogAltarKit const& altar = KurogAltars[mAltar];

        switch (eventId)
        {
            case EVENT_KUROG_SUNDERING_STRIKE:
                castSpellOnVictim(SPELL_KUROG_SUNDERING_STRIKE);
                scriptEvents.addEvent(EVENT_KUROG_SUNDERING_STRIKE, 12000);
                break;
            case EVENT_KUROG_ALTAR_LEAVE:
                leaveAltar();
                scriptEvents.addEvent(EVENT_KUROG_ALTAR_REACH, KurogAltarTravelMs);
                scriptEvents.addEvent(EVENT_KUROG_ALTAR_LEAVE, KurogAltarStayMs);
                break;
            case EVENT_KUROG_ALTAR_REACH:
                reachAltar();
                break;
            case EVENT_KUROG_ALTAR_MINOR:
                castBy(altar.minorAt, altar.minorSpell);
                scriptEvents.addEvent(EVENT_KUROG_ALTAR_MINOR, 20000);
                break;
            case EVENT_KUROG_ALTAR_MAJOR:
                castBy(altar.majorAt, altar.majorSpell);
                scriptEvents.addEvent(EVENT_KUROG_ALTAR_MAJOR, 25000);
                break;
            case EVENT_KUROG_ULTIMATE:
                unleashUltimate();
                scriptEvents.addEvent(EVENT_KUROG_ULTIMATE, KurogEnergyFullMs);
                break;
            case EVENT_KUROG_BARRIER_WATCH:
                if (anyAlive(mCreations))
                    scriptEvents.addEvent(EVENT_KUROG_BARRIER_WATCH, 2000);
                else
                    dropBarrier();
                break;
            case EVENT_KUROG_PRIMAL_ATTUNEMENT:
                castSpellAOE(SPELL_KUROG_PRIMAL_ATTUNEMENT, true);
                scriptEvents.addEvent(EVENT_KUROG_PRIMAL_ATTUNEMENT, 12000);
                break;
            default:
                break;
        }
    }

private:
    void beginDominance(uint32_t firstLeaveMs)
    {
        scriptEvents.addEvent(EVENT_KUROG_SUNDERING_STRIKE, 6000);
        scriptEvents.addEvent(EVENT_KUROG_ALTAR_MINOR, 8000);
        scriptEvents.addEvent(EVENT_KUROG_ALTAR_MAJOR, 14000);
        scriptEvents.addEvent(EVENT_KUROG_ULTIMATE, KurogEnergyFullMs);
        scriptEvents.addEvent(EVENT_KUROG_ALTAR_LEAVE, firstLeaveMs);
    }

    void dropAltarAuras()
    {
        _removeAura(SPELL_KUROG_ELEMENTAL_SURGE);
        for (KurogAltarKit const& altar : KurogAltars)
            _removeAura(altar.dominance);
    }

    // away from an altar he surges with raw elements
    void leaveAltar()
    {
        castSpellAOE(SPELL_KUROG_PRIMAL_SHIFT, true);
        _removeAura(KurogAltars[mAltar].dominance);
        castSpellOnSelf(SPELL_KUROG_ELEMENTAL_SURGE, true);
        mAltar = static_cast<uint8_t>((mAltar + 1) % KUROG_ALTAR_COUNT);
    }

    void reachAltar()
    {
        KurogAltarKit const& altar = KurogAltars[mAltar];
        _removeAura(SPELL_KUROG_ELEMENTAL_SURGE);
        castSpellOnSelf(altar.dominance, true);
        warn(altar.reachText);
    }

    // full energy: the altar's ultimate and a wrought elemental
    void unleashUltimate()
    {
        KurogAltarKit const& altar = KurogAltars[mAltar];
        warn(altar.ultimateText);
        castBy(altar.ultimateAt, altar.ultimateSpell);
        callAdd(altar.wroughtEntry, aroundSelf(8.0f, 14.0f));

        if (mAltar == KUROG_ALTAR_EARTH)
        {
            for (uint8_t i = 0; i < 2; ++i)
                callAdd(NPC_EARTH_BREAKER, aroundSelf(6.0f, 12.0f));
        }
    }

    // the current and the previous altar each send their incarnation
    void raiseBarrier()
    {
        ++mBarriersRaised;
        enterStage(2);
        getCreature()->interruptSpell();
        _removeAura(SPELL_KUROG_ELEMENTAL_SURGE);

        warn("Kurog Grimtotem shields himself with a Primal Barrier and calls his Elemental Creations!");
        castSpellOnSelf(SPELL_KUROG_PRIMAL_BARRIER, true);

        mCreations.clear();
        raiseCreation(KurogAltars[mAltar].creationEntry);
        raiseCreation(KurogAltars[(mAltar + KUROG_ALTAR_COUNT - 1) % KUROG_ALTAR_COUNT].creationEntry);

        scriptEvents.addEvent(EVENT_KUROG_BARRIER_WATCH, 2000);
    }

    void raiseCreation(uint32_t entry)
    {
        if (Creature* pCreation = callAdd(entry, aroundSelf(10.0f, 15.0f)))
            mCreations.push_back(pCreation->getGuid());
    }

    void dropBarrier()
    {
        _removeAura(SPELL_KUROG_PRIMAL_BARRIER);
        mCreations.clear();

        // after the second barrier every altar answers at once
        if (mBarriersRaised >= 2)
        {
            enterStage(3);
            warn("Kurog Grimtotem attunes himself to every altar at once!");
            scriptEvents.addEvent(EVENT_KUROG_PRIMAL_ATTUNEMENT, 5000);
        }
        else
        {
            enterStage(1);
        }

        beginDominance(5000);
    }

    uint8_t mAltar = KUROG_ALTAR_FLAME;
    uint8_t mBarriersRaised = 0;
    std::vector<uint64_t> mCreations;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Raszageth the Storm-Eater, the platforms stay out, she fights on the central one

class VaultOfTheIncarnatesRaszagethAI : public VaultOfTheIncarnatesBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VaultOfTheIncarnatesRaszagethAI(c); }
    explicit VaultOfTheIncarnatesRaszagethAI(Creature* pCreature) : VaultOfTheIncarnatesBossAI(pCreature, DATA_RASZAGETH) {}

protected:
    // stages: 1 winds of change, 2 primalist strike, 3 surging power, 4 vault falters, 5 storm incarnate
    void onEngage() override
    {
        armBerserk(RaszagethBerserkMs);
        mForces.clear();
        castSpellOnSelf(SPELL_RASZAGETH_ELECTRIC_SCALES, true);
        beginWindsOfChange();
    }

    void onDisengage() override
    {
        mForces.clear();
        land();
        _removeAura(SPELL_RASZAGETH_ELECTRIC_SCALES);
        _removeAura(SPELL_RASZAGETH_RUINOUS_SHROUD);
        _removeAura(SPELL_RASZAGETH_STORM_SHROUD);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mStage == 1 && _getHealthPercent() <= RaszagethPrimalistStrikePct)
            beginPrimalistStrike();
        else if (mStage == 3 && _getHealthPercent() <= RaszagethVaultFaltersPct)
            beginVaultFalters();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_RASZAGETH_HURRICANE_WING:
                warn("Raszageth beats her wings, a Hurricane Wing sweeps the platform!");
                castSpellAOE(SPELL_RASZAGETH_HURRICANE_WING);
                scriptEvents.addEvent(EVENT_RASZAGETH_HURRICANE_WING, 40000);
                break;
            case EVENT_RASZAGETH_STATIC_CHARGE:
                castAtPlayers(SPELL_RASZAGETH_STATIC_CHARGE, 3, true);
                scriptEvents.addEvent(EVENT_RASZAGETH_STATIC_CHARGE, 35000);
                break;
            case EVENT_RASZAGETH_VOLATILE_CURRENT:
                castAtPlayer(SPELL_RASZAGETH_VOLATILE_CURRENT, true, true);
                for (uint32_t i = 0, count = isHeroic() ? 6 : 4; i < count; ++i)
                    callAdd(NPC_VOLATILE_SPARK, aroundSelf(10.0f, 25.0f));
                scriptEvents.addEvent(EVENT_RASZAGETH_VOLATILE_CURRENT, 40000);
                break;
            case EVENT_RASZAGETH_ELECTRIFIED_JAWS:
                castSpellOnVictim(SPELL_RASZAGETH_ELECTRIFIED_JAWS);
                scriptEvents.addEvent(EVENT_RASZAGETH_ELECTRIFIED_JAWS, 16000);
                break;
            case EVENT_RASZAGETH_LIGHTNING_BREATH:
                warn("Raszageth unleashes a Lightning Breath!");
                castSpellOnSelf(SPELL_RASZAGETH_LIGHTNING_BREATH);
                scriptEvents.addEvent(EVENT_RASZAGETH_LIGHTNING_BREATH, 32000);
                break;
            case EVENT_RASZAGETH_LIGHTNING_STRIKES:
                castSpellOnSelf(SPELL_RASZAGETH_LIGHTNING_STRIKES, true);
                scriptEvents.addEvent(EVENT_RASZAGETH_LIGHTNING_STRIKES, 30000);
                break;
            case EVENT_RASZAGETH_LIGHTNING_DEVASTATION:
                castAtPlayer(SPELL_RASZAGETH_LIGHTNING_DEVASTATION, false, true);
                scriptEvents.addEvent(EVENT_RASZAGETH_LIGHTNING_DEVASTATION, 14000);
                break;
            case EVENT_RASZAGETH_FORCES_WATCH:
                if (anyAlive(mForces))
                    scriptEvents.addEvent(EVENT_RASZAGETH_FORCES_WATCH, 3000);
                else
                    endIntermission();
                break;
            case EVENT_RASZAGETH_INTERMISSION_LIMIT:
                endIntermission();
                break;
            case EVENT_RASZAGETH_STORMSURGE:
                warn("Raszageth is surrounded by a Stormsurge!");
                castSpellOnSelf(SPELL_RASZAGETH_STORMSURGE);
                scriptEvents.addEvent(EVENT_RASZAGETH_STORMSURGE, 60000);
                break;
            case EVENT_RASZAGETH_TEMPEST_WING:
                warn("Raszageth sends out a Tempest Wing!");
                castSpellAOE(SPELL_RASZAGETH_TEMPEST_WING);
                scriptEvents.addEvent(EVENT_RASZAGETH_TEMPEST_WING, 35000);
                break;
            case EVENT_RASZAGETH_FULMINATING_CHARGE:
                castAtPlayers(SPELL_RASZAGETH_FULMINATING_CHARGE, isHeroic() ? 3 : 2, true);
                scriptEvents.addEvent(EVENT_RASZAGETH_FULMINATING_CHARGE, 40000);
                break;
            case EVENT_RASZAGETH_STORM_EATER:
                warn("Raszageth devours the storm!");
                castSpellOnSelf(SPELL_RASZAGETH_STORM_EATER);
                scriptEvents.addEvent(EVENT_RASZAGETH_STORM_EATER, 45000);
                break;
            case EVENT_RASZAGETH_MAGNETIC_CHARGE:
                castAtPlayer(SPELL_RASZAGETH_MAGNETIC_CHARGE, true, true);
                scriptEvents.addEvent(EVENT_RASZAGETH_MAGNETIC_CHARGE, 30000);
                break;
            case EVENT_RASZAGETH_THUNDEROUS_BLAST:
                castSpellOnVictim(SPELL_RASZAGETH_THUNDEROUS_BLAST);
                scriptEvents.addEvent(EVENT_RASZAGETH_THUNDEROUS_BLAST, 25000);
                break;
            default:
                break;
        }
    }

private:
    void beginWindsOfChange()
    {
        enterStage(1);
        scriptEvents.addEvent(EVENT_RASZAGETH_HURRICANE_WING, 4000);
        scriptEvents.addEvent(EVENT_RASZAGETH_ELECTRIFIED_JAWS, 8000);
        scriptEvents.addEvent(EVENT_RASZAGETH_STATIC_CHARGE, 14000);
        scriptEvents.addEvent(EVENT_RASZAGETH_LIGHTNING_STRIKES, 18000);
        scriptEvents.addEvent(EVENT_RASZAGETH_VOLATILE_CURRENT, 24000);
        scriptEvents.addEvent(EVENT_RASZAGETH_LIGHTNING_BREATH, 30000);
    }

    // first intermission: the primalist forces storm in while she circles above
    void beginPrimalistStrike()
    {
        enterStage(2);
        takeFlight();
        castSpellOnSelf(SPELL_RASZAGETH_RUINOUS_SHROUD, true);
        warn("Raszageth takes to the skies as the Primalist forces strike!");

        mForces.clear();
        for (uint8_t i = 0; i < 2; ++i)
        {
            callForce(NPC_OATHSWORN_VANGUARD);
            callForce(NPC_STORMSEEKER_ACOLYTE);
        }
        callForce(NPC_SURGING_RUINER);

        if (isHeroic())
        {
            callForce(NPC_FROSTFORGED_ZEALOT);
            callForce(NPC_FLAMESWORN_HERALD);
        }

        scriptEvents.addEvent(EVENT_RASZAGETH_LIGHTNING_DEVASTATION, 6000);
        scriptEvents.addEvent(EVENT_RASZAGETH_FORCES_WATCH, 3000);
        scriptEvents.addEvent(EVENT_RASZAGETH_INTERMISSION_LIMIT, RaszagethIntermissionLimitMs);
    }

    void beginSurgingPower()
    {
        enterStage(3);
        land();
        _removeAura(SPELL_RASZAGETH_RUINOUS_SHROUD);

        scriptEvents.addEvent(EVENT_RASZAGETH_STORMSURGE, 8000);
        scriptEvents.addEvent(EVENT_RASZAGETH_ELECTRIFIED_JAWS, 10000);
        scriptEvents.addEvent(EVENT_RASZAGETH_LIGHTNING_STRIKES, 14000);
        scriptEvents.addEvent(EVENT_RASZAGETH_TEMPEST_WING, 18000);
        scriptEvents.addEvent(EVENT_RASZAGETH_FULMINATING_CHARGE, 24000);
        scriptEvents.addEvent(EVENT_RASZAGETH_VOLATILE_CURRENT, 30000);
    }

    // second intermission: a Colossal Stormfiend holds the vault
    void beginVaultFalters()
    {
        enterStage(4);
        takeFlight();
        castSpellOnSelf(SPELL_RASZAGETH_STORM_SHROUD, true);
        warn("The Vault falters, a Colossal Stormfiend takes shape!");

        mForces.clear();
        callForce(NPC_COLOSSAL_STORMFIEND);

        scriptEvents.addEvent(EVENT_RASZAGETH_LIGHTNING_DEVASTATION, 6000);
        scriptEvents.addEvent(EVENT_RASZAGETH_FORCES_WATCH, 3000);
    }

    void beginStormIncarnate()
    {
        enterStage(5);
        land();
        _removeAura(SPELL_RASZAGETH_STORM_SHROUD);
        warn("Raszageth becomes the Storm Incarnate!");
        castSpellOnSelf(SPELL_RASZAGETH_RAGING_STORM, true);

        scriptEvents.addEvent(EVENT_RASZAGETH_THUNDEROUS_BLAST, 8000);
        scriptEvents.addEvent(EVENT_RASZAGETH_MAGNETIC_CHARGE, 12000);
        scriptEvents.addEvent(EVENT_RASZAGETH_LIGHTNING_STRIKES, 16000);
        scriptEvents.addEvent(EVENT_RASZAGETH_TEMPEST_WING, 20000);
        scriptEvents.addEvent(EVENT_RASZAGETH_FULMINATING_CHARGE, 26000);
        scriptEvents.addEvent(EVENT_RASZAGETH_STORM_EATER, 30000);
        scriptEvents.addEvent(EVENT_RASZAGETH_LIGHTNING_BREATH, 34000);
    }

    void endIntermission()
    {
        mForces.clear();
        if (mStage == 2)
            beginSurgingPower();
        else if (mStage == 4)
            beginStormIncarnate();
    }

    void callForce(uint32_t entry)
    {
        if (Creature* pForce = callAdd(entry, aroundSelf(20.0f, 30.0f)))
            mForces.push_back(pForce->getGuid());
    }

    // airborne she cannot be reached
    void takeFlight()
    {
        getCreature()->interruptSpell();
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        setRooted(true);
        _setMeleeDisabled(true);
    }

    void land()
    {
        removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        setRooted(false);
        _setMeleeDisabled(false);
    }

    std::vector<uint64_t> mForces;
};

// destroyed once its Volatile shield is broken, bursts otherwise
class VaultOfTheIncarnatesVolatileSparkAI : public VaultOfTheIncarnatesCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VaultOfTheIncarnatesVolatileSparkAI(c); }
    explicit VaultOfTheIncarnatesVolatileSparkAI(Creature* pCreature) : VaultOfTheIncarnatesCombatAI(pCreature) {}

protected:
    void onEngage() override
    {
        mShielded = false;
        castSpellOnSelf(SPELL_SPARK_VOLATILE, true);
        scriptEvents.addEvent(EVENT_SPARK_ARM, 1500);
        scriptEvents.addEvent(EVENT_SPARK_BURST, 12000);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (!mShielded || hasAura(SPELL_SPARK_VOLATILE))
            return;

        mShielded = false;
        scriptEvents.resetEvents();
        getCreature()->dealDamage(getCreature(), getCreature()->getHealth(), 0);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SPARK_ARM:
                mShielded = hasAura(SPELL_SPARK_VOLATILE);
                break;
            case EVENT_SPARK_BURST:
                mShielded = false;
                castSpellAOE(SPELL_SPARK_BURST);
                despawn(3000, 0);
                break;
            default:
                break;
        }
    }

private:
    bool mShielded = false;
};

// breaks into Seeking Stormlings that try to fuse back into it
class VaultOfTheIncarnatesColossalStormfiendAI : public VaultOfTheIncarnatesCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VaultOfTheIncarnatesColossalStormfiendAI(c); }
    explicit VaultOfTheIncarnatesColossalStormfiendAI(Creature* pCreature) : VaultOfTheIncarnatesCombatAI(pCreature) {}

    void OnDied(Unit* /*pKiller*/) override
    {
        scriptEvents.resetEvents();
        summons.despawnAll();
    }

protected:
    void onEngage() override
    {
        castSpellOnSelf(SPELL_STORMFIEND_CRACKLING_ENERGY, true);
        scriptEvents.addEvent(EVENT_STORMFIEND_STORM_BREAK, 8000);
        scriptEvents.addEvent(EVENT_STORMFIEND_BALL_LIGHTNING, 15000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_STORMFIEND_STORM_BREAK:
                castSpellAOE(SPELL_STORMFIEND_STORM_BREAK);
                for (uint8_t i = 0; i < 2; ++i)
                    summonCreature(NPC_SEEKING_STORMLING, aroundSelf(15.0f, 25.0f), CORPSE_DESPAWN);
                scriptEvents.addEvent(EVENT_STORMFIEND_STORM_BREAK, 20000);
                break;
            case EVENT_STORMFIEND_BALL_LIGHTNING:
                castSpellOnSelf(SPELL_STORMFIEND_BALL_LIGHTNING);
                scriptEvents.addEvent(EVENT_STORMFIEND_BALL_LIGHTNING, 25000);
                break;
            default:
                break;
        }
    }
};

// walks back to the nearest Colossal Stormfiend and fuses into it
class VaultOfTheIncarnatesSeekingStormlingAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VaultOfTheIncarnatesSeekingStormlingAI(c); }
    explicit VaultOfTheIncarnatesSeekingStormlingAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void AIUpdate(unsigned long time_passed) override
    {
        if (mFused || !isAlive())
            return;

        if (!mPrepared)
        {
            mPrepared = true;
            setReactState(REACT_PASSIVE);
            castSpellOnSelf(SPELL_STORMLING_INDOMITABLE_FOCUS, true);
        }

        mSeekClockMs += static_cast<uint32_t>(time_passed);
        if (mSeekClockMs < StormlingSeekMs)
            return;

        mSeekClockMs = 0;

        Creature* pFiend = findStormfiend();
        if (pFiend == nullptr)
        {
            mFused = true;
            despawn(1000, 0);
            return;
        }

        if (getCreature()->getDistance(pFiend) <= StormlingFuseRange)
        {
            mFused = true;
            castSpell(pFiend, SPELL_STORMLING_FUSE, true);
            despawn(1000, 0);
            return;
        }

        moveTo(pFiend->GetPositionX(), pFiend->GetPositionY(), pFiend->GetPositionZ(), false);
    }

private:
    Creature* findStormfiend()
    {
        uint32_t const fiendEntries[2] = { NPC_COLOSSAL_STORMFIEND, NPC_COLOSSAL_STORMFIEND_ALT };
        for (uint32_t entry : fiendEntries)
        {
            Creature* pFiend = findNearestCreature(entry, 150.0f);
            if (pFiend != nullptr && pFiend->isAlive())
                return pFiend;
        }

        return nullptr;
    }

    bool mPrepared = false;
    bool mFused = false;
    uint32_t mSeekClockMs = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Broodkeeper Diurna, without the egg clutch

class VaultOfTheIncarnatesDiurnaAI : public VaultOfTheIncarnatesBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VaultOfTheIncarnatesDiurnaAI(c); }
    explicit VaultOfTheIncarnatesDiurnaAI(Creature* pCreature) : VaultOfTheIncarnatesBossAI(pCreature, DATA_BROODKEEPER_DIURNA) {}

protected:
    void onEngage() override
    {
        armBerserk(DiurnaBerserkMs);
        mNextElite = 0;
        beginPrimalistClutch();
    }

    void onDisengage() override { _removeAura(SPELL_DIURNA_BROODKEEPERS_FURY); }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mStage == 1 && _getHealthPercent() <= DiurnaScornedPct)
            beginBroodkeeperScorned();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_DIURNA_MORTAL_CLAWS:
                // heroic: the scorned broodkeeper slams instead
                castSpellOnVictim(mStage == 2 && isHeroic() ? SPELL_DIURNA_MORTAL_STONESLAM : SPELL_DIURNA_MORTAL_STONECLAWS);
                scriptEvents.addEvent(EVENT_DIURNA_MORTAL_CLAWS, 12000);
                break;
            case EVENT_DIURNA_WILDFIRE:
                castSpellAOE(SPELL_DIURNA_WILDFIRE);
                scriptEvents.addEvent(EVENT_DIURNA_WILDFIRE, 22000);
                break;
            case EVENT_DIURNA_GREATSTAFF:
                castAtPlayer(mStage == 1 ? SPELL_DIURNA_GREATSTAFF : SPELL_DIURNA_EMPOWERED_GREATSTAFF, true);
                scriptEvents.addEvent(EVENT_DIURNA_GREATSTAFF, mStage == 1 ? 25000 : 22000);
                break;
            case EVENT_DIURNA_SHROUD:
                if (mStage == 1)
                {
                    warn("Broodkeeper Diurna wraps her foes in an Icy Shroud!");
                    castSpellAOE(SPELL_DIURNA_ICY_SHROUD);
                }
                else
                {
                    warn("Broodkeeper Diurna wraps her foes in a Frozen Shroud!");
                    castSpellAOE(SPELL_DIURNA_FROZEN_SHROUD);
                }
                scriptEvents.addEvent(EVENT_DIURNA_SHROUD, 40000);
                break;
            case EVENT_DIURNA_RAPID_INCUBATION:
                warn("Broodkeeper Diurna calls for a Rapid Incubation!");
                castAtPlayer(SPELL_DIURNA_RAPID_INCUBATION, false, true);
                for (uint32_t i = 0, count = isHeroic() ? 3 : 2; i < count; ++i)
                    callAdd(NPC_NASCENT_PROTO_DRAGON, aroundSelf(15.0f, 25.0f));
                scriptEvents.addEvent(EVENT_DIURNA_RAPID_INCUBATION, DiurnaEnergyFullMs);
                break;
            case EVENT_DIURNA_REINFORCEMENTS:
                callReinforcements();
                scriptEvents.addEvent(EVENT_DIURNA_REINFORCEMENTS, 50000);
                break;
            case EVENT_DIURNA_STORM_FISSURE:
                castUnderPlayer(SPELL_DIURNA_STORM_FISSURE);
                scriptEvents.addEvent(EVENT_DIURNA_STORM_FISSURE, 30000);
                break;
            default:
                break;
        }
    }

private:
    void beginPrimalistClutch()
    {
        enterStage(1);
        scriptEvents.addEvent(EVENT_DIURNA_MORTAL_CLAWS, 6000);
        scriptEvents.addEvent(EVENT_DIURNA_WILDFIRE, 9000);
        scriptEvents.addEvent(EVENT_DIURNA_GREATSTAFF, 15000);
        scriptEvents.addEvent(EVENT_DIURNA_SHROUD, 25000);
        scriptEvents.addEvent(EVENT_DIURNA_REINFORCEMENTS, 30000);
        scriptEvents.addEvent(EVENT_DIURNA_RAPID_INCUBATION, DiurnaEnergyFullMs);

        if (isHeroic())
            scriptEvents.addEvent(EVENT_DIURNA_STORM_FISSURE, 20000);
    }

    // no more eggs and no more reinforcements, only fury
    void beginBroodkeeperScorned()
    {
        enterStage(2);
        warn("Broodkeeper Diurna is consumed by Broodkeeper's Fury!");
        castSpellOnSelf(SPELL_DIURNA_BROODKEEPERS_FURY, true);

        scriptEvents.addEvent(EVENT_DIURNA_MORTAL_CLAWS, 6000);
        scriptEvents.addEvent(EVENT_DIURNA_WILDFIRE, 9000);
        scriptEvents.addEvent(EVENT_DIURNA_GREATSTAFF, 12000);
        scriptEvents.addEvent(EVENT_DIURNA_SHROUD, 20000);

        if (isHeroic())
            scriptEvents.addEvent(EVENT_DIURNA_STORM_FISSURE, 25000);
    }

    void callReinforcements()
    {
        warn("Primalist Reinforcements arrive!");

        for (uint8_t i = 0; i < 2; ++i)
            callAdd(NPC_TARASEK_LEGIONNAIRE, aroundSelf(30.0f, 40.0f));
        callAdd(NPC_PRIMALIST_MAGE, aroundSelf(30.0f, 40.0f));

        callAdd(DiurnaElites[mNextElite], aroundSelf(30.0f, 40.0f));
        mNextElite = (mNextElite + 1) % 4;
    }

    uint32_t mNextElite = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Dathea, Ascended

class VaultOfTheIncarnatesDatheaAI : public VaultOfTheIncarnatesBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VaultOfTheIncarnatesDatheaAI(c); }
    explicit VaultOfTheIncarnatesDatheaAI(Creature* pCreature) : VaultOfTheIncarnatesBossAI(pCreature, DATA_DATHEA_ASCENDED) {}

protected:
    void onEngage() override
    {
        armBerserk(DatheaBerserkMs);
        mTempests.clear();

        scriptEvents.addEvent(EVENT_DATHEA_REACH_CHECK, 2000);
        scriptEvents.addEvent(EVENT_DATHEA_ZEPHYR_SLAM, 7000);
        scriptEvents.addEvent(EVENT_DATHEA_RAGING_BURST, 10000);
        scriptEvents.addEvent(EVENT_DATHEA_CONDUCTIVE_MARK, 14000);
        scriptEvents.addEvent(EVENT_DATHEA_CYCLONE, 30000);
        scriptEvents.addEvent(EVENT_DATHEA_CROSSWINDS, 33000);
        scriptEvents.addEvent(EVENT_DATHEA_COALESCING_STORM, DatheaEnergyFullMs);
    }

    void onDisengage() override { mTempests.clear(); }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_DATHEA_REACH_CHECK:
                // a tank out of reach gets buffeted
                if (isVictimOutOfReach(DatheaReach))
                {
                    castSpellOnVictim(SPELL_DATHEA_AERIAL_BUFFET);
                    scriptEvents.addEvent(EVENT_DATHEA_REACH_CHECK, 8000);
                }
                else
                {
                    scriptEvents.addEvent(EVENT_DATHEA_REACH_CHECK, 2000);
                }
                break;
            case EVENT_DATHEA_ZEPHYR_SLAM:
                castSpellOnVictim(SPELL_DATHEA_ZEPHYR_SLAM);
                scriptEvents.addEvent(EVENT_DATHEA_ZEPHYR_SLAM, 16000);
                break;
            case EVENT_DATHEA_RAGING_BURST:
                ragingBurst();
                scriptEvents.addEvent(EVENT_DATHEA_RAGING_BURST, 60000);
                break;
            case EVENT_DATHEA_CONDUCTIVE_MARK:
                castAtPlayers(SPELL_DATHEA_CONDUCTIVE_MARK, isHeroic() ? 3 : 2, true);
                scriptEvents.addEvent(EVENT_DATHEA_CONDUCTIVE_MARK, 26000);
                break;
            case EVENT_DATHEA_CYCLONE:
                warn("Dathea, Ascended conjures a Cyclone!");
                castSpellAOE(SPELL_DATHEA_CYCLONE);
                scriptEvents.addEvent(EVENT_DATHEA_CYCLONE, 45000);
                break;
            case EVENT_DATHEA_CROSSWINDS:
                crosswinds();
                scriptEvents.addEvent(EVENT_DATHEA_CROSSWINDS, 33000);
                break;
            case EVENT_DATHEA_COALESCING_STORM:
                coalescingStorm();
                scriptEvents.addEvent(EVENT_DATHEA_COALESCING_STORM, DatheaEnergyFullMs);
                break;
            default:
                break;
        }
    }

private:
    void ragingBurst()
    {
        castSpellAOE(SPELL_DATHEA_RAGING_BURST);

        // forget tempests that are gone
        std::vector<uint64_t> standing;
        for (uint64_t guid : mTempests)
        {
            if (getCreature()->getWorldMapCreature(guid) != nullptr)
                standing.push_back(guid);
        }
        mTempests.swap(standing);

        for (Unit* pTarget : pickPlayers(2, true))
        {
            if (mTempests.size() >= DatheaTempestLimit)
                break;

            if (Creature* pTempest = summonCreature(NPC_RAGING_TEMPEST, pTarget->GetPosition(), MANUAL_DESPAWN))
                mTempests.push_back(pTempest->getGuid());
        }
    }

    // every Raging Tempest blows to a new spot
    void crosswinds()
    {
        castSpellOnSelf(SPELL_DATHEA_CROSSWINDS);

        for (uint64_t guid : mTempests)
        {
            Creature* pTempest = getCreature()->getWorldMapCreature(guid);
            if (pTempest == nullptr || pTempest->GetScript() == nullptr)
                continue;

            LocationVector const spot = aroundSelf(10.0f, 35.0f);
            pTempest->GetScript()->moveTo(spot.x, spot.y, spot.z, true);
        }
    }

    // full energy: Volatile Infusers and Thunder Callers answer
    void coalescingStorm()
    {
        warn("Dathea, Ascended gathers a Coalescing Storm!");
        castSpellOnSelf(SPELL_DATHEA_COALESCING_STORM);

        for (uint32_t i = 0, count = isHeroic() ? 2 : 1; i < count; ++i)
            callAdd(NPC_VOLATILE_INFUSER, aroundSelf(20.0f, 30.0f));

        for (uint8_t i = 0; i < 3; ++i)
            callAdd(NPC_THUNDER_CALLER, aroundSelf(25.0f, 35.0f));
    }

    std::vector<uint64_t> mTempests;
};

// untargetable tempest pulsing Raging Winds, moved by Crosswinds
class VaultOfTheIncarnatesRagingTempestAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VaultOfTheIncarnatesRagingTempestAI(c); }
    explicit VaultOfTheIncarnatesRagingTempestAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void AIUpdate(unsigned long time_passed) override
    {
        if (!isAlive())
            return;

        if (!mPrepared)
        {
            mPrepared = true;
            setReactState(REACT_PASSIVE);
            _setMeleeDisabled(true);
            addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        }

        mPulseClockMs += static_cast<uint32_t>(time_passed);
        if (mPulseClockMs < TempestPulseMs)
            return;

        mPulseClockMs = 0;
        castSpellAOE(SPELL_TEMPEST_RAGING_WINDS, true);
    }

private:
    bool mPrepared = false;
    uint32_t mPulseClockMs = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Terros, without the Awakened Earth pillars

class VaultOfTheIncarnatesTerrosAI : public VaultOfTheIncarnatesBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VaultOfTheIncarnatesTerrosAI(c); }
    explicit VaultOfTheIncarnatesTerrosAI(Creature* pCreature) : VaultOfTheIncarnatesBossAI(pCreature, DATA_TERROS) {}

protected:
    void onEngage() override
    {
        mAnnihilations = 0;

        scriptEvents.addEvent(EVENT_TERROS_REACH_CHECK, 3000);
        scriptEvents.addEvent(EVENT_TERROS_ROCK_BLAST, 6000);
        scriptEvents.addEvent(EVENT_TERROS_CONCUSSIVE_SLAM, 12000);
        scriptEvents.addEvent(EVENT_TERROS_SHATTERING_IMPACT, 24000);
        scriptEvents.addEvent(EVENT_TERROS_RESONATING_ANNIHILATION, 90000);

        if (isHeroic())
            scriptEvents.addEvent(EVENT_TERROS_INFUSED_FALLOUT, 18000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_TERROS_REACH_CHECK:
                // nobody in reach: the barrage hits everyone
                if (isVictimOutOfReach(TerrosReach))
                {
                    castSpellOnSelf(SPELL_TERROS_TECTONIC_BARRAGE);
                    scriptEvents.addEvent(EVENT_TERROS_REACH_CHECK, 10000);
                }
                else
                {
                    scriptEvents.addEvent(EVENT_TERROS_REACH_CHECK, 3000);
                }
                break;
            case EVENT_TERROS_ROCK_BLAST:
                if (castAtPlayer(SPELL_TERROS_ROCK_BLAST, true) != nullptr)
                    warn("Terros hurls a Rock Blast!");
                scriptEvents.addEvent(EVENT_TERROS_ROCK_BLAST, 33000);
                break;
            case EVENT_TERROS_CONCUSSIVE_SLAM:
                castSpellOnVictim(SPELL_TERROS_CONCUSSIVE_SLAM);
                scriptEvents.addEvent(EVENT_TERROS_CONCUSSIVE_SLAM, 20000);
                break;
            case EVENT_TERROS_SHATTERING_IMPACT:
                castUnderPlayer(SPELL_TERROS_SHATTERING_IMPACT);
                scriptEvents.addEvent(EVENT_TERROS_FRACTURED_RUBBLE, 2500);
                scriptEvents.addEvent(EVENT_TERROS_SHATTERING_IMPACT, 35000);
                break;
            case EVENT_TERROS_FRACTURED_RUBBLE:
                for (Unit* pTarget : pickPlayers(isHeroic() ? 4 : 3))
                    getCreature()->castSpellLoc(pTarget->GetPosition(), SPELL_TERROS_FRACTURED_RUBBLE, true);
                break;
            case EVENT_TERROS_RESONATING_ANNIHILATION:
                resonatingAnnihilation();
                break;
            case EVENT_TERROS_FRENZIED_DEVASTATION:
                castSpellAOE(SPELL_TERROS_FRENZIED_DEVASTATION);
                scriptEvents.addEvent(EVENT_TERROS_FRENZIED_DEVASTATION, 10000);
                break;
            case EVENT_TERROS_INFUSED_FALLOUT:
                castAtPlayers(SPELL_TERROS_INFUSED_FALLOUT, 2);
                scriptEvents.addEvent(EVENT_TERROS_INFUSED_FALLOUT, 30000);
                break;
            default:
                break;
        }
    }

private:
    // full energy; after the last one the chamber is spent and he goes into a frenzy
    void resonatingAnnihilation()
    {
        ++mAnnihilations;
        warn("Terros gathers a Resonating Annihilation!");

        if (Unit* pVictim = currentVictim())
            getCreature()->castSpellLoc(pVictim->GetPosition(), SPELL_TERROS_RESONATING_ANNIHILATION, false);
        else
            castSpellAOE(SPELL_TERROS_RESONATING_ANNIHILATION);

        scriptEvents.addEvent(EVENT_TERROS_FRACTURED_RUBBLE, 3000);

        if (mAnnihilations < TerrosAnnihilationLimit)
        {
            scriptEvents.addEvent(EVENT_TERROS_RESONATING_ANNIHILATION, TerrosEnergyFullMs);
            return;
        }

        warn("Terros flies into a Frenzied Devastation!");
        scriptEvents.addEvent(EVENT_TERROS_FRENZIED_DEVASTATION, 5000);
    }

    uint32_t mAnnihilations = 0;
};

void SetupVaultOfTheIncarnates(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_VAULT_OF_THE_INCARNATES, &VaultOfTheIncarnatesInstanceScript::Create);

    // Eranog
    mgr->register_creature_script(BOSS_ERANOG, &VaultOfTheIncarnatesEranogAI::Create);
    mgr->register_creature_script(NPC_FLAMESCALE_TARASEK, &VaultOfTheIncarnatesFlamescaleTarasekAI::Create);

    // The Primal Council
    mgr->register_creature_script(BOSS_KADROS_ICEWRATH, &VaultOfTheIncarnatesKadrosAI::Create);
    mgr->register_creature_script(BOSS_DATHEA_STORMLASH, &VaultOfTheIncarnatesStormlashAI::Create);
    mgr->register_creature_script(BOSS_OPALFANG, &VaultOfTheIncarnatesOpalfangAI::Create);
    mgr->register_creature_script(BOSS_EMBAR_FIREPATH, &VaultOfTheIncarnatesEmbarAI::Create);

    // Sennarth, the Cold Breath
    mgr->register_creature_script(BOSS_SENNARTH, &VaultOfTheIncarnatesSennarthAI::Create);
    mgr->register_creature_script(NPC_CAUSTIC_SPIDERLING, &VaultOfTheIncarnatesCausticSpiderlingAI::Create);

    // Kurog Grimtotem
    mgr->register_creature_script(BOSS_KUROG_GRIMTOTEM, &VaultOfTheIncarnatesKurogAI::Create);

    // Raszageth the Storm-Eater
    mgr->register_creature_script(BOSS_RASZAGETH, &VaultOfTheIncarnatesRaszagethAI::Create);
    mgr->register_creature_script(NPC_VOLATILE_SPARK, &VaultOfTheIncarnatesVolatileSparkAI::Create);
    mgr->register_creature_script(NPC_COLOSSAL_STORMFIEND, &VaultOfTheIncarnatesColossalStormfiendAI::Create);
    mgr->register_creature_script(NPC_COLOSSAL_STORMFIEND_ALT, &VaultOfTheIncarnatesColossalStormfiendAI::Create);
    mgr->register_creature_script(NPC_SEEKING_STORMLING, &VaultOfTheIncarnatesSeekingStormlingAI::Create);

    // Broodkeeper Diurna
    mgr->register_creature_script(BOSS_BROODKEEPER_DIURNA, &VaultOfTheIncarnatesDiurnaAI::Create);

    // Dathea, Ascended
    mgr->register_creature_script(BOSS_DATHEA_ASCENDED, &VaultOfTheIncarnatesDatheaAI::Create);
    mgr->register_creature_script(NPC_RAGING_TEMPEST, &VaultOfTheIncarnatesRagingTempestAI::Create);

    // Terros
    mgr->register_creature_script(BOSS_TERROS, &VaultOfTheIncarnatesTerrosAI::Create);

    // adds driven by the kit table
    for (VaultAddKit const& kit : VaultAddKits)
        mgr->register_creature_script(kit.entry, &VaultOfTheIncarnatesAddAI::Create);
}
