/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Dawn_Of_The_Infinite.hpp"

#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <cmath>
#include <list>
#include <vector>

enum DawnOfTheInfiniteEvents
{
    EVENT_CHRONIKAR_CHRONOSHEAR = 1,
    EVENT_CHRONIKAR_SAND_STOMP,
    EVENT_CHRONIKAR_EON_FRAGMENTS,

    EVENT_TIMEWAYS_CHRONO_FADED,
    EVENT_TIMEWAYS_FRAGMENTS_OF_TIME,
    EVENT_TIMEWAYS_CONFLUX,

    EVENT_BLIGHT_CORROSIVE_INFUSION,
    EVENT_BLIGHT_RECLAMATION,
    EVENT_BLIGHT_NECROTIC_WINDS,
    EVENT_LOSZKELETH_NECROFROST,
    EVENT_DAZHAK_NOXIOUS_EJECTION,
    EVENT_DAZHAK_BLIGHTBREATH,

    EVENT_IRIDIKRON_PULVERIZING_EXHALATION,
    EVENT_IRIDIKRON_EARTHSURGE,

    EVENT_TYR_HAND_TECHNIQUE,
    EVENT_TYR_SPARK_OF_TYR,
    EVENT_TYR_RECHARGED,

    EVENT_MORCHIE_SAND_BLAST,
    EVENT_MORCHIE_TIME_TRAPS,
    EVENT_MORCHIE_FAMILIAR_FACES,
    EVENT_MORCHIE_MORE_PROBLEMS,
    EVENT_MORCHIE_DRAGONS_BREATH,

    EVENT_COMMANDER_HEAVY_STRIKE,
    EVENT_COMMANDER_SHOCKWAVE,
    EVENT_COMMANDER_SHOCKWAVE_WAVE,
    EVENT_COMMANDER_BLADESTORM,
    EVENT_COMMANDER_RALLY,
    EVENT_COMMANDER_SHOUT,
    EVENT_TROOPER_VOLLEY,
    EVENT_TROOPER_SPECIAL,

    EVENT_DEIOS_TEMPORAL_BREATH,
    EVENT_DEIOS_INFINITY_ORB,
    EVENT_DEIOS_SUMMON_KEEPER,
    EVENT_DEIOS_KEEPER_ARRIVES,
    EVENT_KEEPER_INFINITE_BLAST,
    EVENT_KEEPER_CHRONAL_BURN,
    EVENT_KEEPER_TIME_RIFT
};

enum DawnOfTheInfiniteCreatureData
{
    // player a Familiar Face mirrors
    FACE_DATA_MIRRORED = 1
};

namespace
{
    float const DawnOfTheInfiniteTwoPi = 6.28318531f;

    uint32_t const ChronikarEnergyPerSecond = 3;
    uint32_t const ChronikarFragmentTargets = 3;

    float const TimewaysMeleeReach = 8.0f;

    int32_t const BlightAhnzonPct = 70;
    int32_t const BlightPairPct = 40;

    uint32_t const IridikronEnergyPerSecond = 2;

    uint32_t const TyrTechniqueCost = 25;
    uint32_t const TyrSiphonMs = 15000;

    uint32_t const MorchieAlternateCount = 3;
    uint32_t const MorchieAlternateLifeMs = 6000;
    uint32_t const MorchieFaceLimit = 5;
    uint32_t const MorchieTrapsPerCast = 3;
    uint32_t const MorchieTrapLifeMs = 60000;
    float const MorchieTrapReach = 2.5f;

    uint32_t const CommanderShockwaveCount = 3;

    uint32_t const DeiosKeeperCount = 4;
    uint32_t const DeiosRiftCollapsePct = 10;

    // each side brings its own commander and troops
    struct BattlefieldCommanderKit
    {
        uint32_t heavyStrike;
        uint32_t shockwave;
        uint32_t rally;
        uint32_t battleInstinct;
        uint32_t shout;
        uint32_t troops[4];
        char const* rallyText;
    };

    BattlefieldCommanderKit const GrommashKit =
    {
        SPELL_GROMMASH_DECAPITATE, SPELL_GROMMASH_SHOCKWAVE, SPELL_GROMMASH_FOR_THE_HORDE, SPELL_GROMMASH_THIRST_FOR_BATTLE, SPELL_GROMMASH_WAR_CRY,
        { NPC_HORDE_GRUNT, NPC_HORDE_GRUNT, NPC_HORDE_AXE_THROWER, NPC_HORDE_WARLOCK },
        "Grommash Hellscream rallies the Forces of the Horde!"
    };

    BattlefieldCommanderKit const LotharKit =
    {
        SPELL_LOTHAR_MORTAL_STRIKES, SPELL_LOTHAR_SHOCKWAVE, SPELL_LOTHAR_FOR_THE_ALLIANCE, SPELL_LOTHAR_BATTLE_SENSES, SPELL_LOTHAR_BATTLE_CRY,
        { NPC_ALLIANCE_FOOTMAN, NPC_ALLIANCE_FOOTMAN, NPC_ALLIANCE_ARCHER, NPC_ALLIANCE_CONJUROR },
        "Anduin Lothar rallies the Forces of the Alliance!"
    };

    // ranged troops: a quick shot and a slower special
    struct BattlefieldTrooperKit
    {
        uint32_t volley;
        uint32_t volleyMs;
        uint32_t special;
        uint32_t specialMs;
    };

    BattlefieldTrooperKit const AxeThrowerKit = { SPELL_AXE_THROWER_AXE_THROW, 3500, SPELL_AXE_THROWER_SERRATED_AXE, 12000 };
    BattlefieldTrooperKit const WarlockKit = { SPELL_WARLOCK_IMMOLATE, 6000, SPELL_WARLOCK_RAIN_OF_FIRE, 15000 };
    BattlefieldTrooperKit const ArcherKit = { SPELL_ARCHER_SHOOT, 3000, SPELL_ARCHER_SERRATED_ARROWS, 12000 };
    BattlefieldTrooperKit const ConjurorKit = { SPELL_CONJUROR_FIREBALL, 4000, SPELL_CONJUROR_BLIZZARD, 15000 };

    std::vector<Unit*> playersAround(Creature* pCreature, float range)
    {
        std::vector<Unit*> players;
        for (Object* pObject : pCreature->getInRangePlayersSet())
        {
            Unit* pPlayer = static_cast<Unit*>(pObject);
            if (pPlayer != nullptr && pPlayer->isAlive() && pCreature->getDistance(pPlayer) <= range)
                players.push_back(pPlayer);
        }

        return players;
    }

    LocationVector ringPoint(LocationVector const& centre, float radius, float angle)
    {
        return LocationVector(centre.x + radius * std::cos(angle), centre.y + radius * std::sin(angle), centre.z, angle);
    }

    LocationVector pointNear(LocationVector const& origin, float minRange, float maxRange)
    {
        return ringPoint(origin, Util::getRandomFloat(minRange, maxRange), Util::getRandomFloat(DawnOfTheInfiniteTwoPi));
    }

    std::vector<Creature*> livingAround(CreatureAIScript* pScript, uint32_t entry, float range)
    {
        std::list<Creature*> found;
        pScript->GetCreatureListWithEntryInGrid(found, entry, range);

        std::vector<Creature*> living;
        for (Creature* pCreature : found)
        {
            if (pCreature != nullptr && pCreature->isAlive())
                living.push_back(pCreature);
        }

        return living;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class DawnOfTheInfiniteInstanceScript : public InstanceScript
{
public:
    explicit DawnOfTheInfiniteInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(DawnOfTheInfiniteEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new DawnOfTheInfiniteInstanceScript(pMapMgr); }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event clock while in combat

class DawnOfTheInfiniteCombatAI : public CreatureAIScript
{
public:
    explicit DawnOfTheInfiniteCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override { onEngage(); }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        scriptEvents.resetEvents();
        summons.despawnAll();

        if (isAlive())
            onReset();
    }

    void onSummonedCreature(Creature* pSummon) override { summons.summon(pSummon); }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!_isInCombat())
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        scriptEvents.updateEvents(diff, 0);
        onCombatTick(diff);

        if (_isCasting())
            return;

        if (uint32_t const eventId = scriptEvents.getFinishedEvent())
            onEvent(eventId);
    }

protected:
    virtual void onEngage() {}
    virtual void onReset() {}
    virtual void onCombatTick(uint32_t /*diff*/) {}
    virtual void onEvent(uint32_t /*eventId*/) {}

    Unit* anyPlayer() { return selectUnitTarget(FilterArgs(TargetFilter_Player)); }

    // anyone but the tank, the tank when alone
    Unit* offTankPlayer()
    {
        FilterArgs args(TargetFilter_Player);
        args.addFilter(TargetFilter_NotCurrent);
        if (Unit* pTarget = selectUnitTarget(args))
            return pTarget;

        return anyPlayer();
    }

    void castAt(Unit* pTarget, uint32_t spellId, bool triggered = false)
    {
        if (pTarget != nullptr)
            castSpell(pTarget, spellId, triggered);
    }

    Creature* callAdd(uint32_t entry, LocationVector const& position)
    {
        Creature* pAdd = summonCreature(entry, position, CORPSE_DESPAWN);
        if (pAdd != nullptr)
            setZoneWideCombat(pAdd);

        return pAdd;
    }

    // energy counter: grows each second, true once full
    bool chargeEnergy(uint32_t diff, uint32_t perSecond)
    {
        mEnergyClock += diff;
        while (mEnergyClock >= 1000)
        {
            mEnergyClock -= 1000;
            mEnergy = mEnergy + perSecond > 100 ? 100 : mEnergy + perSecond;
            showEnergy(mEnergy);
        }

        return mEnergy >= 100;
    }

    void setEnergy(uint32_t value)
    {
        mEnergy = value;
        mEnergyClock = 0;
        showEnergy(value);
    }

    void showEnergy(uint32_t value)
    {
        if (getCreature()->getMaxPower(POWER_TYPE_ENERGY) > 0)
            getCreature()->setPower(POWER_TYPE_ENERGY, value);
    }

    void warn(char const* text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }

    uint32_t mEnergy = 0;
    uint32_t mEnergyClock = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame and boss state

class DawnOfTheInfiniteBossAI : public DawnOfTheInfiniteCombatAI
{
public:
    DawnOfTheInfiniteBossAI(Creature* pCreature, uint32_t dataId) : DawnOfTheInfiniteCombatAI(pCreature), mDataId(dataId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mDataId, EncounterStates::InProgress);
        }

        DawnOfTheInfiniteCombatAI::OnCombatStart(pTarget);
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

        DawnOfTheInfiniteCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        onDefeat();

        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
            pInstance->setBossState(mDataId, EncounterStates::Performed);
        }

        scriptEvents.resetEvents();
        summons.despawnAll();
    }

protected:
    virtual void onDefeat() {}

    uint32_t mDataId;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Chronikar

class DawnOfTheInfiniteChronikarAI : public DawnOfTheInfiniteBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DawnOfTheInfiniteChronikarAI(c); }
    explicit DawnOfTheInfiniteChronikarAI(Creature* pCreature) : DawnOfTheInfiniteBossAI(pCreature, DATA_CHRONIKAR) {}

protected:
    void onEngage() override
    {
        setEnergy(0);
        scriptEvents.addEvent(EVENT_CHRONIKAR_CHRONOSHEAR, 7000);
        scriptEvents.addEvent(EVENT_CHRONIKAR_SAND_STOMP, 12000);
    }

    void onReset() override { setEnergy(0); }

    void onCombatTick(uint32_t diff) override
    {
        if (!chargeEnergy(diff, ChronikarEnergyPerSecond) || _isCasting())
            return;

        setEnergy(0);
        warn("Chronikar leaps with Eon Shatter!");
        castAt(anyPlayer(), SPELL_CHRONIKAR_EON_SHATTER);
        scriptEvents.addEvent(EVENT_CHRONIKAR_EON_FRAGMENTS, 500);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_CHRONIKAR_CHRONOSHEAR:
                castSpellOnVictim(SPELL_CHRONIKAR_CHRONOSHEAR);
                scriptEvents.addEvent(EVENT_CHRONIKAR_CHRONOSHEAR, 15000);
                break;
            case EVENT_CHRONIKAR_SAND_STOMP:
                castSpellAOE(SPELL_CHRONIKAR_SAND_STOMP);
                scriptEvents.addEvent(EVENT_CHRONIKAR_SAND_STOMP, 20000);
                break;
            case EVENT_CHRONIKAR_EON_FRAGMENTS:
            {
                // shards of the shattered timespace scatter over the room
                uint32_t hit = 0;
                for (Unit* pPlayer : playersAround(getCreature(), 60.0f))
                {
                    if (hit++ >= ChronikarFragmentTargets)
                        break;

                    castSpell(pPlayer, SPELL_CHRONIKAR_EON_FRAGMENTS, true);
                }
                break;
            }
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Manifested Timeways

class DawnOfTheInfiniteTimewaysAI : public DawnOfTheInfiniteBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DawnOfTheInfiniteTimewaysAI(c); }
    explicit DawnOfTheInfiniteTimewaysAI(Creature* pCreature) : DawnOfTheInfiniteBossAI(pCreature, DATA_MANIFESTED_TIMEWAYS) {}

protected:
    void onEngage() override
    {
        mReachClock = 0;
        scriptEvents.addEvent(EVENT_TIMEWAYS_CONFLUX, 5000);
        scriptEvents.addEvent(EVENT_TIMEWAYS_CHRONO_FADED, 8000);
        scriptEvents.addEvent(EVENT_TIMEWAYS_FRAGMENTS_OF_TIME, 12000);
    }

    void onReset() override
    {
        _removeAuraOnPlayers(SPELL_TIMEWAYS_DECAYING_TIME);
        _removeAuraOnPlayers(SPELL_TIMEWAYS_ACCELERATING_TIME);
    }

    void onDefeat() override { onReset(); }

    void onCombatTick(uint32_t diff) override
    {
        // nobody in reach, it lashes out at everyone
        mReachClock += diff;
        if (mReachClock < 3000 || _isCasting())
            return;

        mReachClock = 0;
        if (playersAround(getCreature(), TimewaysMeleeReach).empty())
            castSpellAOE(SPELL_TIMEWAYS_RADIANT);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_TIMEWAYS_CONFLUX:
                // the fields shift, one player slows down and another speeds up
                castAt(anyPlayer(), SPELL_TIMEWAYS_DECAYING_TIME, true);
                castAt(offTankPlayer(), SPELL_TIMEWAYS_ACCELERATING_TIME, true);
                scriptEvents.addEvent(EVENT_TIMEWAYS_CONFLUX, 20000);
                break;
            case EVENT_TIMEWAYS_CHRONO_FADED:
                castAt(offTankPlayer(), SPELL_TIMEWAYS_CHRONO_FADED);
                scriptEvents.addEvent(EVENT_TIMEWAYS_CHRONO_FADED, 18000);
                break;
            case EVENT_TIMEWAYS_FRAGMENTS_OF_TIME:
                castSpellAOE(SPELL_TIMEWAYS_FRAGMENTS_OF_TIME);
                scriptEvents.addEvent(EVENT_TIMEWAYS_FRAGMENTS_OF_TIME, 15000);
                break;
            default:
                break;
        }
    }

private:
    uint32_t mReachClock = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Blight of Galakrond, takes new hosts as it weakens

class DawnOfTheInfiniteBlightAI : public DawnOfTheInfiniteBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DawnOfTheInfiniteBlightAI(c); }
    explicit DawnOfTheInfiniteBlightAI(Creature* pCreature) : DawnOfTheInfiniteBossAI(pCreature, DATA_BLIGHT_OF_GALAKROND) {}

protected:
    void onEngage() override
    {
        mHosts = 0;
        castSpellOnSelf(SPELL_BLIGHT_BLIGHT_SEEP, true);
        scriptEvents.addEvent(EVENT_BLIGHT_CORROSIVE_INFUSION, 6000);
        scriptEvents.addEvent(EVENT_BLIGHT_RECLAMATION, 18000);
    }

    void onReset() override
    {
        _removeAura(SPELL_BLIGHT_BLIGHT_SEEP);
        _removeAura(SPELL_BLIGHT_ESSENCE_CONNECTION);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (_isCasting())
            return;

        int32_t const health = _getHealthPercent();
        if (mHosts == 0 && health <= BlightAhnzonPct)
        {
            mHosts = 1;
            warn("The Blight of Galakrond seizes Ahnzon and gains Necrotic Winds!");
            scriptEvents.addEvent(EVENT_BLIGHT_NECROTIC_WINDS, 4000);
        }
        else if (mHosts == 1 && health <= BlightPairPct)
        {
            mHosts = 2;
            raiseLoszkelethAndDazhak();
        }
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_BLIGHT_CORROSIVE_INFUSION:
                castAt(anyPlayer(), SPELL_BLIGHT_CORROSIVE_INFUSION);
                scriptEvents.addEvent(EVENT_BLIGHT_CORROSIVE_INFUSION, 14000);
                break;
            case EVENT_BLIGHT_RECLAMATION:
                castSpellOnVictim(SPELL_BLIGHT_BLIGHT_RECLAMATION);
                scriptEvents.addEvent(EVENT_BLIGHT_RECLAMATION, 25000);
                break;
            case EVENT_BLIGHT_NECROTIC_WINDS:
                castSpellOnSelf(SPELL_BLIGHT_NECROTIC_WINDS);
                scriptEvents.addEvent(EVENT_BLIGHT_NECROTIC_WINDS, 22000);
                break;
            default:
                break;
        }
    }

private:
    void raiseLoszkelethAndDazhak()
    {
        warn("The Blight of Galakrond reanimates Loszkeleth and Dazhak!");
        castSpellOnSelf(SPELL_BLIGHT_ESSENCE_CONNECTION, true);
        callAdd(NPC_LOSZKELETH, pointNear(getCreature()->GetPosition(), 6.0f, 10.0f));
        callAdd(NPC_DAZHAK, pointNear(getCreature()->GetPosition(), 6.0f, 10.0f));
    }

    uint32_t mHosts = 0;
};

class DawnOfTheInfiniteLoszkelethAI : public DawnOfTheInfiniteCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DawnOfTheInfiniteLoszkelethAI(c); }
    explicit DawnOfTheInfiniteLoszkelethAI(Creature* pCreature) : DawnOfTheInfiniteCombatAI(pCreature) {}

protected:
    void onEngage() override { scriptEvents.addEvent(EVENT_LOSZKELETH_NECROFROST, 5000); }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_LOSZKELETH_NECROFROST)
            return;

        castAt(anyPlayer(), SPELL_LOSZKELETH_NECROFROST);
        scriptEvents.addEvent(EVENT_LOSZKELETH_NECROFROST, 15000);
    }
};

// supports Loszkeleth with breath and ejections
class DawnOfTheInfiniteDazhakAI : public DawnOfTheInfiniteCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DawnOfTheInfiniteDazhakAI(c); }
    explicit DawnOfTheInfiniteDazhakAI(Creature* pCreature) : DawnOfTheInfiniteCombatAI(pCreature) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_DAZHAK_NOXIOUS_EJECTION, 4000);
        scriptEvents.addEvent(EVENT_DAZHAK_BLIGHTBREATH, 10000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_DAZHAK_NOXIOUS_EJECTION:
                castAt(anyPlayer(), SPELL_DAZHAK_NOXIOUS_EJECTION);
                scriptEvents.addEvent(EVENT_DAZHAK_NOXIOUS_EJECTION, 12000);
                break;
            case EVENT_DAZHAK_BLIGHTBREATH:
                castSpellOnSelf(SPELL_DAZHAK_INCINERATING_BLIGHTBREATH);
                scriptEvents.addEvent(EVENT_DAZHAK_BLIGHTBREATH, 18000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Iridikron the Stonescaled

class DawnOfTheInfiniteIridikronAI : public DawnOfTheInfiniteBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DawnOfTheInfiniteIridikronAI(c); }
    explicit DawnOfTheInfiniteIridikronAI(Creature* pCreature) : DawnOfTheInfiniteBossAI(pCreature, DATA_IRIDIKRON) {}

protected:
    void onEngage() override
    {
        setEnergy(0);
        castSpellOnSelf(SPELL_IRIDIKRON_PATIENT_TACTICIAN, true);
        castSpellOnSelf(SPELL_IRIDIKRON_CRUSHING_ONSLAUGHT, true);
        scriptEvents.addEvent(EVENT_IRIDIKRON_PULVERIZING_EXHALATION, 14000);
        scriptEvents.addEvent(EVENT_IRIDIKRON_EARTHSURGE, 24000);
    }

    void onReset() override
    {
        setEnergy(0);
        _removeAura(SPELL_IRIDIKRON_PATIENT_TACTICIAN);
        _removeAura(SPELL_IRIDIKRON_CRUSHING_ONSLAUGHT);
    }

    // the patient tactician strikes once his energy is full
    void onCombatTick(uint32_t diff) override
    {
        if (!chargeEnergy(diff, IridikronEnergyPerSecond) || _isCasting())
            return;

        setEnergy(0);
        warn("Iridikron unleashes Cataclysmic Obliteration!");
        castSpellAOE(SPELL_IRIDIKRON_CATACLYSMIC_OBLITERATION);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_IRIDIKRON_PULVERIZING_EXHALATION:
                castSpellOnVictim(SPELL_IRIDIKRON_PULVERIZING_EXHALATION);
                scriptEvents.addEvent(EVENT_IRIDIKRON_PULVERIZING_EXHALATION, 30000);
                break;
            case EVENT_IRIDIKRON_EARTHSURGE:
                castSpellOnSelf(SPELL_IRIDIKRON_EARTHSURGE);
                scriptEvents.addEvent(EVENT_IRIDIKRON_EARTHSURGE, 40000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Tyr, the Infinite Keeper, spends his energy and refills it at the Oathstone

class DawnOfTheInfiniteTyrAI : public DawnOfTheInfiniteBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DawnOfTheInfiniteTyrAI(c); }
    explicit DawnOfTheInfiniteTyrAI(Creature* pCreature) : DawnOfTheInfiniteBossAI(pCreature, DATA_TYR_INFINITE_KEEPER) {}

protected:
    void onEngage() override
    {
        setEnergy(100);
        mTechnique = 0;
        scriptEvents.addEvent(EVENT_TYR_HAND_TECHNIQUE, 5000);
        scriptEvents.addEvent(EVENT_TYR_SPARK_OF_TYR, 15000);
    }

    void onReset() override
    {
        setEnergy(100);
        _removeAura(SPELL_TYR_TEMPORAL_ESSENCE);
        _removeAura(SPELL_TYR_RADIANT_BARRIER);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_TYR_HAND_TECHNIQUE:
                useTechnique();
                break;
            case EVENT_TYR_SPARK_OF_TYR:
                castSpellAOE(SPELL_TYR_SPARK_OF_TYR);
                scriptEvents.addEvent(EVENT_TYR_SPARK_OF_TYR, 20000);
                break;
            case EVENT_TYR_RECHARGED:
                _removeAura(SPELL_TYR_TEMPORAL_ESSENCE);
                setEnergy(100);
                scriptEvents.addEvent(EVENT_TYR_HAND_TECHNIQUE, 3000);
                break;
            default:
                break;
        }
    }

private:
    // blow, strike and annihilation in turn, each one costs energy
    void useTechnique()
    {
        switch (mTechnique++ % 3)
        {
            case 0:
                castSpellOnVictim(SPELL_TYR_TITANIC_BLOW);
                break;
            case 1:
                castSpellOnVictim(SPELL_TYR_DIVIDING_STRIKE);
                break;
            default:
                castSpellAOE(SPELL_TYR_INFINITE_ANNIHILATION);
                break;
        }

        setEnergy(mEnergy > TyrTechniqueCost ? mEnergy - TyrTechniqueCost : 0);
        if (mEnergy > 0)
        {
            scriptEvents.addEvent(EVENT_TYR_HAND_TECHNIQUE, 9000);
            return;
        }

        warn("Tyr siphons the Infinite Oathstone and shields himself with a Radiant Barrier!");
        castSpellOnSelf(SPELL_TYR_SIPHON_OATHSTONE);
        castSpellOnSelf(SPELL_TYR_RADIANT_BARRIER, true);
        castSpellOnSelf(SPELL_TYR_TEMPORAL_ESSENCE, true);
        scriptEvents.addEvent(EVENT_TYR_RECHARGED, TyrSiphonMs);
    }

    uint32_t mTechnique = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Morchie

class DawnOfTheInfiniteMorchieAI : public DawnOfTheInfiniteBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DawnOfTheInfiniteMorchieAI(c); }
    explicit DawnOfTheInfiniteMorchieAI(Creature* pCreature) : DawnOfTheInfiniteBossAI(pCreature, DATA_MORCHIE) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_MORCHIE_SAND_BLAST, 8000);
        scriptEvents.addEvent(EVENT_MORCHIE_TIME_TRAPS, 14000);
        scriptEvents.addEvent(EVENT_MORCHIE_FAMILIAR_FACES, 25000);
        scriptEvents.addEvent(EVENT_MORCHIE_MORE_PROBLEMS, 35000);
    }

    void onReset() override { _removeAuraOnPlayers(SPELL_TRAP_TIME_STASIS); }

    void onDefeat() override { _removeAuraOnPlayers(SPELL_TRAP_TIME_STASIS); }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_MORCHIE_SAND_BLAST:
                castSpellOnVictim(SPELL_MORCHIE_SAND_BLAST);
                scriptEvents.addEvent(EVENT_MORCHIE_SAND_BLAST, 15000);
                break;
            case EVENT_MORCHIE_TIME_TRAPS:
                castSpellOnSelf(SPELL_MORCHIE_TIME_TRAPS);
                setTimeTraps();
                scriptEvents.addEvent(EVENT_MORCHIE_TIME_TRAPS, 30000);
                break;
            case EVENT_MORCHIE_FAMILIAR_FACES:
                castSpellAOE(SPELL_MORCHIE_FAMILIAR_FACES);
                familiarFaces();
                scriptEvents.addEvent(EVENT_MORCHIE_FAMILIAR_FACES, 45000);
                break;
            case EVENT_MORCHIE_MORE_PROBLEMS:
                moreProblems();
                scriptEvents.addEvent(EVENT_MORCHIE_DRAGONS_BREATH, 1000);
                scriptEvents.addEvent(EVENT_MORCHIE_MORE_PROBLEMS, 45000);
                break;
            case EVENT_MORCHIE_DRAGONS_BREATH:
                castSpellAOE(SPELL_MORCHIE_DRAGONS_BREATH);
                break;
            default:
                break;
        }
    }

private:
    void setTimeTraps()
    {
        for (uint32_t i = 0; i < MorchieTrapsPerCast; ++i)
        {
            if (Unit* pTarget = anyPlayer())
                summonCreature(NPC_TIME_TRAP, pointNear(pTarget->GetPosition(), 3.0f, 7.0f), TIMED_DESPAWN, MorchieTrapLifeMs);
        }
    }

    // faces from other timelines, each one hunting its own player
    void familiarFaces()
    {
        std::vector<Creature*> const faces = livingAround(this, NPC_FAMILIAR_FACE, 100.0f);
        if (!faces.empty())
        {
            warn("Morchie restores her Familiar Faces!");
            for (Creature* pFace : faces)
            {
                pFace->setHealth(pFace->getMaxHealth());
                pFace->removeAllNegativeAuras();
            }
            return;
        }

        warn("Morchie brings Familiar Faces from other timelines!");
        uint32_t called = 0;
        for (Unit* pPlayer : playersAround(getCreature(), 100.0f))
        {
            if (called++ >= MorchieFaceLimit)
                break;

            Creature* pFace = summonCreature(NPC_FAMILIAR_FACE, pointNear(pPlayer->GetPosition(), 3.0f, 5.0f), CORPSE_DESPAWN);
            if (pFace != nullptr && pFace->GetScript() != nullptr)
                pFace->GetScript()->SetCreatureData64(FACE_DATA_MIRRORED, pPlayer->getGuid());
        }
    }

    void moreProblems()
    {
        warn("Morchie brings More Problems! Find the real Morchie!");
        castSpellOnSelf(SPELL_MORCHIE_MORE_PROBLEMS);

        for (uint32_t i = 0; i < MorchieAlternateCount; ++i)
            summonCreature(NPC_MORCHIE_ALTERNATE, ringPoint(getCreature()->GetPosition(), 8.0f, i * DawnOfTheInfiniteTwoPi / MorchieAlternateCount), TIMED_DESPAWN, MorchieAlternateLifeMs);
    }
};

// a Morchie from another timeline, sent back before her breath lands
class DawnOfTheInfiniteMorchieAlternateAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DawnOfTheInfiniteMorchieAlternateAI(c); }
    explicit DawnOfTheInfiniteMorchieAlternateAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        setRooted(true);
        _setMeleeDisabled(true);
    }
};

class DawnOfTheInfiniteFamiliarFaceAI : public DawnOfTheInfiniteCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DawnOfTheInfiniteFamiliarFaceAI(c); }
    explicit DawnOfTheInfiniteFamiliarFaceAI(Creature* pCreature) : DawnOfTheInfiniteCombatAI(pCreature) {}

    void SetCreatureData64(uint32_t type, uint64_t data) override
    {
        if (type != FACE_DATA_MIRRORED)
            return;

        mMirroredGuid = data;
        Unit* pMirrored = getCreature()->getWorldMapUnit(mMirroredGuid);
        if (pMirrored == nullptr || !pMirrored->isAlive())
            return;

        castSpell(pMirrored, SPELL_FACE_FIXATE, true);
        attackStart(pMirrored);
    }

    // only the mirrored player counts as a foe
    bool canAttackTarget(Unit* pTarget) override
    {
        return mMirroredGuid == 0 || pTarget == nullptr || pTarget->getGuid() == mMirroredGuid;
    }

protected:
    void onEngage() override { castSpellOnSelf(SPELL_FACE_ANACHRONISTIC_DECAY, true); }

    void onReset() override { _removeAura(SPELL_FACE_ANACHRONISTIC_DECAY); }

private:
    uint64_t mMirroredGuid = 0;
};

// roots whoever steps on it and lashes the whole party
class DawnOfTheInfiniteTimeTrapAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DawnOfTheInfiniteTimeTrapAI(c); }
    explicit DawnOfTheInfiniteTimeTrapAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        setRooted(true);
        _setMeleeDisabled(true);
        setCanEnterCombat(false);
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (mSprung || !isAlive())
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (mCheckClock > diff)
        {
            mCheckClock -= diff;
            return;
        }

        mCheckClock = 500;
        std::vector<Unit*> const caught = playersAround(getCreature(), MorchieTrapReach);
        if (caught.empty())
            return;

        mSprung = true;
        for (Unit* pPlayer : caught)
            castSpell(pPlayer, SPELL_TRAP_TIME_STASIS, true);

        castSpellAOE(SPELL_TRAP_TEMPORAL_BACKLASH, true);
        despawn(1000, 0);
    }

private:
    uint32_t mCheckClock = 1500;
    bool mSprung = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Time-Lost Battlefield, the opposing faction's commander

class DawnOfTheInfiniteCommanderAI : public DawnOfTheInfiniteBossAI
{
public:
    static CreatureAIScript* CreateGrommash(Creature* c) { return new DawnOfTheInfiniteCommanderAI(c, GrommashKit); }
    static CreatureAIScript* CreateLothar(Creature* c) { return new DawnOfTheInfiniteCommanderAI(c, LotharKit); }

    DawnOfTheInfiniteCommanderAI(Creature* pCreature, BattlefieldCommanderKit const& kit) : DawnOfTheInfiniteBossAI(pCreature, DATA_TIME_LOST_BATTLEFIELD), mKit(kit) {}

protected:
    void onEngage() override
    {
        mShockwavesLeft = 0;
        castSpellOnSelf(mKit.battleInstinct, true);
        scriptEvents.addEvent(EVENT_COMMANDER_HEAVY_STRIKE, 6000);
        scriptEvents.addEvent(EVENT_COMMANDER_RALLY, 10000);
        scriptEvents.addEvent(EVENT_COMMANDER_SHOCKWAVE, 15000);
        scriptEvents.addEvent(EVENT_COMMANDER_BLADESTORM, 22000);
        scriptEvents.addEvent(EVENT_COMMANDER_SHOUT, 30000);
    }

    void onReset() override { _removeAura(mKit.battleInstinct); }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_COMMANDER_HEAVY_STRIKE:
                castSpellOnVictim(mKit.heavyStrike);
                scriptEvents.addEvent(EVENT_COMMANDER_HEAVY_STRIKE, 14000);
                break;
            case EVENT_COMMANDER_SHOCKWAVE:
                // three waves in quick succession
                mShockwavesLeft = CommanderShockwaveCount;
                scriptEvents.addEvent(EVENT_COMMANDER_SHOCKWAVE_WAVE, 100);
                scriptEvents.addEvent(EVENT_COMMANDER_SHOCKWAVE, 28000);
                break;
            case EVENT_COMMANDER_SHOCKWAVE_WAVE:
                castSpellOnVictim(mKit.shockwave);
                if (mShockwavesLeft > 0 && --mShockwavesLeft > 0)
                    scriptEvents.addEvent(EVENT_COMMANDER_SHOCKWAVE_WAVE, 2000);
                break;
            case EVENT_COMMANDER_BLADESTORM:
                warn("A Bladestorm pursues a marked player!");
                castAt(offTankPlayer(), SPELL_COMMANDER_BLADESTORM);
                scriptEvents.addEvent(EVENT_COMMANDER_BLADESTORM, 35000);
                break;
            case EVENT_COMMANDER_RALLY:
                warn(mKit.rallyText);
                castSpellOnSelf(mKit.rally);
                for (uint32_t troop : mKit.troops)
                    callAdd(troop, pointNear(getCreature()->GetPosition(), 6.0f, 12.0f));
                scriptEvents.addEvent(EVENT_COMMANDER_RALLY, 40000);
                break;
            case EVENT_COMMANDER_SHOUT:
                castSpellAOE(mKit.shout);
                scriptEvents.addEvent(EVENT_COMMANDER_SHOUT, 30000);
                break;
            default:
                break;
        }
    }

private:
    BattlefieldCommanderKit const& mKit;
    uint32_t mShockwavesLeft = 0;
};

class DawnOfTheInfiniteTrooperAI : public DawnOfTheInfiniteCombatAI
{
public:
    static CreatureAIScript* CreateAxeThrower(Creature* c) { return new DawnOfTheInfiniteTrooperAI(c, AxeThrowerKit); }
    static CreatureAIScript* CreateWarlock(Creature* c) { return new DawnOfTheInfiniteTrooperAI(c, WarlockKit); }
    static CreatureAIScript* CreateArcher(Creature* c) { return new DawnOfTheInfiniteTrooperAI(c, ArcherKit); }
    static CreatureAIScript* CreateConjuror(Creature* c) { return new DawnOfTheInfiniteTrooperAI(c, ConjurorKit); }

    DawnOfTheInfiniteTrooperAI(Creature* pCreature, BattlefieldTrooperKit const& kit) : DawnOfTheInfiniteCombatAI(pCreature), mKit(kit) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_TROOPER_VOLLEY, 1500);
        scriptEvents.addEvent(EVENT_TROOPER_SPECIAL, 6000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_TROOPER_VOLLEY:
                castAt(anyPlayer(), mKit.volley);
                scriptEvents.addEvent(EVENT_TROOPER_VOLLEY, mKit.volleyMs);
                break;
            case EVENT_TROOPER_SPECIAL:
                castAt(anyPlayer(), mKit.special);
                scriptEvents.addEvent(EVENT_TROOPER_SPECIAL, mKit.specialMs);
                break;
            default:
                break;
        }
    }

private:
    BattlefieldTrooperKit const& mKit;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Chrono-Lord Deios, rises above the rifts once his keepers stand

class DawnOfTheInfiniteDeiosAI : public DawnOfTheInfiniteBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DawnOfTheInfiniteDeiosAI(c); }
    explicit DawnOfTheInfiniteDeiosAI(Creature* pCreature) : DawnOfTheInfiniteBossAI(pCreature, DATA_CHRONO_LORD_DEIOS) {}

    // a slain keeper collapses its rift and wounds Deios
    void OnSummonDies(Creature* pSummon, Unit* /*pKiller*/) override
    {
        if (pSummon == nullptr || pSummon->getEntry() != NPC_INFINITE_KEEPER || mKeepersAlive == 0)
            return;

        --mKeepersAlive;
        if (!mAloft)
            return;

        Creature* pSelf = getCreature();
        uint32_t const wound = pSelf->getMaxHealth() / 100 * DeiosRiftCollapsePct;
        pSelf->setHealth(pSelf->getHealth() > wound ? pSelf->getHealth() - wound : 1);
        warn("A Time Rift collapses and wounds Chrono-Lord Deios!");

        if (mKeepersAlive == 0)
            land(true);
    }

protected:
    void onEngage() override
    {
        mKeepersCalled = 0;
        mKeepersAlive = 0;
        mAloft = false;
        scriptEvents.addEvent(EVENT_DEIOS_SUMMON_KEEPER, 5000);
        scriptEvents.addEvent(EVENT_DEIOS_TEMPORAL_BREATH, 10000);
        scriptEvents.addEvent(EVENT_DEIOS_INFINITY_ORB, 14000);
    }

    void onReset() override
    {
        if (mAloft)
            land(false);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_DEIOS_SUMMON_KEEPER:
                castSpellOnSelf(SPELL_DEIOS_SUMMON_INFINITE_KEEPER);
                scriptEvents.addEvent(EVENT_DEIOS_KEEPER_ARRIVES, 500);
                break;
            case EVENT_DEIOS_KEEPER_ARRIVES:
                callKeeper();
                break;
            case EVENT_DEIOS_TEMPORAL_BREATH:
                if (!mAloft)
                    castSpellOnVictim(SPELL_DEIOS_TEMPORAL_BREATH);
                scriptEvents.addEvent(EVENT_DEIOS_TEMPORAL_BREATH, 20000);
                break;
            case EVENT_DEIOS_INFINITY_ORB:
                castSpellAOE(SPELL_DEIOS_INFINITY_ORB);
                scriptEvents.addEvent(EVENT_DEIOS_INFINITY_ORB, mAloft ? 12000 : 18000);
                break;
            default:
                break;
        }
    }

private:
    void callKeeper()
    {
        float const angle = mKeepersCalled * DawnOfTheInfiniteTwoPi / DeiosKeeperCount;
        ++mKeepersCalled;
        if (callAdd(NPC_INFINITE_KEEPER, ringPoint(getCreature()->GetPosition(), 15.0f, angle)) != nullptr)
            ++mKeepersAlive;

        if (mKeepersCalled < DeiosKeeperCount)
        {
            scriptEvents.addEvent(EVENT_DEIOS_SUMMON_KEEPER, 12000);
            return;
        }

        // nothing left to guard the rifts, he stays down
        if (mKeepersAlive > 0)
            takeToTheSkies();
    }

    void takeToTheSkies()
    {
        mAloft = true;
        warn("Chrono-Lord Deios takes to the skies! Defeat the Infinite Keepers!");
        castSpellOnSelf(SPELL_DEIOS_COLLAPSING_TIME_RIFT, true);
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        setRooted(true);
        _setMeleeDisabled(true);
    }

    void land(bool announce)
    {
        mAloft = false;
        _removeAura(SPELL_DEIOS_COLLAPSING_TIME_RIFT);
        removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        setRooted(false);
        _setMeleeDisabled(false);

        if (announce)
            warn("Chrono-Lord Deios is forced back to the ground!");
    }

    uint32_t mKeepersCalled = 0;
    uint32_t mKeepersAlive = 0;
    bool mAloft = false;
};

// holds a rift open and lets troopers through
class DawnOfTheInfiniteInfiniteKeeperAI : public DawnOfTheInfiniteCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DawnOfTheInfiniteInfiniteKeeperAI(c); }
    explicit DawnOfTheInfiniteInfiniteKeeperAI(Creature* pCreature) : DawnOfTheInfiniteCombatAI(pCreature) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_KEEPER_INFINITE_BLAST, 3000);
        scriptEvents.addEvent(EVENT_KEEPER_TIME_RIFT, 6000);
        scriptEvents.addEvent(EVENT_KEEPER_CHRONAL_BURN, 8000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_KEEPER_INFINITE_BLAST:
                castSpellOnVictim(SPELL_KEEPER_INFINITE_BLAST);
                scriptEvents.addEvent(EVENT_KEEPER_INFINITE_BLAST, 6000);
                break;
            case EVENT_KEEPER_CHRONAL_BURN:
                castAt(anyPlayer(), SPELL_KEEPER_CHRONAL_BURN);
                scriptEvents.addEvent(EVENT_KEEPER_CHRONAL_BURN, 12000);
                break;
            case EVENT_KEEPER_TIME_RIFT:
                callAdd(NPC_TIME_DISPLACED_TROOPER, pointNear(getCreature()->GetPosition(), 3.0f, 6.0f));
                scriptEvents.addEvent(EVENT_KEEPER_TIME_RIFT, 15000);
                break;
            default:
                break;
        }
    }
};

class DawnOfTheInfiniteTimeDisplacedTrooperAI : public DawnOfTheInfiniteCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DawnOfTheInfiniteTimeDisplacedTrooperAI(c); }
    explicit DawnOfTheInfiniteTimeDisplacedTrooperAI(Creature* pCreature) : DawnOfTheInfiniteCombatAI(pCreature) {}

protected:
    void onEngage() override
    {
        if (!hasAura(SPELL_TROOPER_TIME_DISPLACEMENT))
            castSpellOnSelf(SPELL_TROOPER_TIME_DISPLACEMENT, true);
    }
};

void SetupDawnOfTheInfinite(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_DAWN_OF_THE_INFINITE, &DawnOfTheInfiniteInstanceScript::Create);

    mgr->register_creature_script(BOSS_CHRONIKAR, &DawnOfTheInfiniteChronikarAI::Create);

    mgr->register_creature_script(BOSS_MANIFESTED_TIMEWAYS, &DawnOfTheInfiniteTimewaysAI::Create);
    mgr->register_creature_script(BOSS_MANIFESTED_TIMEWAYS_ALT, &DawnOfTheInfiniteTimewaysAI::Create);

    mgr->register_creature_script(BOSS_BLIGHT_OF_GALAKROND, &DawnOfTheInfiniteBlightAI::Create);
    mgr->register_creature_script(BOSS_BLIGHT_OF_GALAKROND_ALT, &DawnOfTheInfiniteBlightAI::Create);
    mgr->register_creature_script(BOSS_BLIGHT_OF_GALAKROND_ALT2, &DawnOfTheInfiniteBlightAI::Create);
    mgr->register_creature_script(NPC_LOSZKELETH, &DawnOfTheInfiniteLoszkelethAI::Create);
    mgr->register_creature_script(NPC_DAZHAK, &DawnOfTheInfiniteDazhakAI::Create);

    mgr->register_creature_script(BOSS_IRIDIKRON, &DawnOfTheInfiniteIridikronAI::Create);

    mgr->register_creature_script(BOSS_TYR_INFINITE_KEEPER, &DawnOfTheInfiniteTyrAI::Create);

    mgr->register_creature_script(BOSS_MORCHIE, &DawnOfTheInfiniteMorchieAI::Create);
    mgr->register_creature_script(BOSS_MORCHIE_ALT, &DawnOfTheInfiniteMorchieAI::Create);
    mgr->register_creature_script(NPC_MORCHIE_ALTERNATE, &DawnOfTheInfiniteMorchieAlternateAI::Create);
    mgr->register_creature_script(NPC_FAMILIAR_FACE, &DawnOfTheInfiniteFamiliarFaceAI::Create);
    mgr->register_creature_script(NPC_TIME_TRAP, &DawnOfTheInfiniteTimeTrapAI::Create);

    mgr->register_creature_script(BOSS_GROMMASH_HELLSCREAM, &DawnOfTheInfiniteCommanderAI::CreateGrommash);
    mgr->register_creature_script(BOSS_GROMMASH_HELLSCREAM_ALT, &DawnOfTheInfiniteCommanderAI::CreateGrommash);
    mgr->register_creature_script(BOSS_ANDUIN_LOTHAR, &DawnOfTheInfiniteCommanderAI::CreateLothar);
    mgr->register_creature_script(BOSS_ANDUIN_LOTHAR_ALT, &DawnOfTheInfiniteCommanderAI::CreateLothar);
    mgr->register_creature_script(NPC_HORDE_AXE_THROWER, &DawnOfTheInfiniteTrooperAI::CreateAxeThrower);
    mgr->register_creature_script(NPC_HORDE_WARLOCK, &DawnOfTheInfiniteTrooperAI::CreateWarlock);
    mgr->register_creature_script(NPC_ALLIANCE_ARCHER, &DawnOfTheInfiniteTrooperAI::CreateArcher);
    mgr->register_creature_script(NPC_ALLIANCE_CONJUROR, &DawnOfTheInfiniteTrooperAI::CreateConjuror);

    mgr->register_creature_script(BOSS_CHRONO_LORD_DEIOS, &DawnOfTheInfiniteDeiosAI::Create);
    mgr->register_creature_script(BOSS_CHRONO_LORD_DEIOS_ALT, &DawnOfTheInfiniteDeiosAI::Create);
    mgr->register_creature_script(NPC_INFINITE_KEEPER, &DawnOfTheInfiniteInfiniteKeeperAI::Create);
    mgr->register_creature_script(NPC_TIME_DISPLACED_TROOPER, &DawnOfTheInfiniteTimeDisplacedTrooperAI::Create);
}
