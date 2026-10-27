/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Ara_Kara.hpp"

#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <cmath>
#include <vector>

enum AraKaraEvents
{
    EVENT_KIKATAL_ERUPTING_WEBS = 1,
    EVENT_KIKATAL_VENOM_VOLLEY,
    EVENT_KIKATAL_BLOODWORKER,

    EVENT_ANUBZEKT_IMPALE,
    EVENT_ANUBZEKT_BURROW_CHARGE,
    EVENT_ANUBZEKT_INFESTATION,
    EVENT_ANUBZEKT_SWARM_ENDS,

    EVENT_AVANOXX_VORACIOUS_BITE,
    EVENT_AVANOXX_ALERTING_SHRILL,
    EVENT_AVANOXX_GOSSAMER_ONSLAUGHT
};

namespace
{
    // room doors shut while their encounter runs
    struct AraKaraDoor
    {
        uint32_t entry;
        uint32_t dataId;
    };

    AraKaraDoor const AraKaraDoors[] =
    {
        { GO_ANUBZEKT_NORTH_SPIKE_WALL, DATA_ANUBZEKT },
        { GO_ANUBZEKT_SOUTH_SPIKE_WALL, DATA_ANUBZEKT }
    };

    AraKaraDoor const* findAraKaraDoor(uint32_t entry)
    {
        for (AraKaraDoor const& door : AraKaraDoors)
        {
            if (door.entry == entry)
                return &door;
        }

        return nullptr;
    }

    float const AraKaraTwoPi = 6.28318531f;

    // drones come in from the hauling tunnel
    LocationVector const KikatalBloodworkerEntry(-522.2f, 1115.5f, 92.3f);
    uint32_t const KikatalEnergyPerSecond = 2;
    uint32_t const BloodworkerLifetimeMs = 40000;
    uint32_t const BlackBloodLifetimeMs = 45000;
    float const BlackBloodReach = 3.5f;
    float const HarvesterSearchRange = 150.0f;

    uint32_t const AnubzektEnergyPerSecond = 2;
    uint32_t const EyeOfTheSwarmMs = 25000;

    float const CrawlerFixateThreat = 1000000.0f;

    // living players within range of a creature
    std::vector<Unit*> araKaraPlayersNear(Creature* pCreature, float range)
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

    LocationVector araKaraRingPoint(LocationVector const& origin, float minRange, float maxRange)
    {
        float const angle = Util::getRandomFloat(AraKaraTwoPi);
        float const range = Util::getRandomFloat(minRange, maxRange);
        return LocationVector(origin.x + range * std::cos(angle), origin.y + range * std::sin(angle), origin.z, angle);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class AraKaraInstanceScript : public InstanceScript
{
public:
    explicit AraKaraInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(AraKaraEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new AraKaraInstanceScript(pMapMgr); }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        if (findAraKaraDoor(pGameObject->getEntry()) == nullptr)
            return;

        mDoorGuids.push_back(pGameObject->GetNewGUID());
        applyDoorState(pGameObject);
    }

    void OnEncounterStateChange(uint32_t /*id*/, uint32_t /*state*/) override
    {
        for (WoWGuid const& guid : mDoorGuids)
        {
            if (GameObject* pDoor = getGameObjectByGuid(guid))
                applyDoorState(pDoor);
        }
    }

private:
    void applyDoorState(GameObject* pDoor)
    {
        AraKaraDoor const* pRule = findAraKaraDoor(pDoor->getEntry());
        if (pRule == nullptr)
            return;

        uint8_t const wanted = getBossState(pRule->dataId) != EncounterStates::InProgress ? GO_STATE_OPEN : GO_STATE_CLOSED;
        if (pDoor->getState() != wanted)
            pDoor->setState(wanted);
    }

    std::vector<WoWGuid> mDoorGuids;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event clock, energy and target helpers

class AraKaraCombatAI : public CreatureAIScript
{
public:
    explicit AraKaraCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        mEnergy = 0;
        mEnergyClock = 0;
        onEngage();
    }

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

    InstanceScript* araKaraScript()
    {
        if (getCreature()->GetMapId() != MAP_ARA_KARA)
            return nullptr;

        return getInstanceScript();
    }

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

    LocationVector aroundSelf(float minRange, float maxRange) { return araKaraRingPoint(getCreature()->GetPosition(), minRange, maxRange); }

    void warn(char const* text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }

    // true once the bar is full
    bool fillEnergy(uint32_t diff, uint32_t perSecond)
    {
        mEnergyClock += diff;
        while (mEnergyClock >= 1000)
        {
            mEnergyClock -= 1000;
            mEnergy += perSecond;
        }

        if (mEnergy > 100)
            mEnergy = 100;

        return mEnergy >= 100;
    }

    void emptyEnergy()
    {
        mEnergy = 0;
        mEnergyClock = 0;
    }

private:
    uint32_t mEnergy = 0;
    uint32_t mEnergyClock = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame and boss state

class AraKaraBossAI : public AraKaraCombatAI
{
public:
    AraKaraBossAI(Creature* pCreature, uint32_t dataId) : AraKaraCombatAI(pCreature), mDataId(dataId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = araKaraScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mDataId, EncounterStates::InProgress);
        }

        AraKaraCombatAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        if (isAlive())
        {
            if (InstanceScript* pInstance = araKaraScript())
            {
                pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
                pInstance->setBossState(mDataId, EncounterStates::Failed);
            }
        }

        AraKaraCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (InstanceScript* pInstance = araKaraScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
            pInstance->setBossState(mDataId, EncounterStates::Performed);
        }

        scriptEvents.resetEvents();
        summons.despawnAll();
    }

protected:
    uint32_t mDataId;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Ki'katal the Harvester

class AraKaraKikatalAI : public AraKaraBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AraKaraKikatalAI(c); }
    explicit AraKaraKikatalAI(Creature* pCreature) : AraKaraBossAI(pCreature, DATA_KIKATAL_THE_HARVESTER) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_KIKATAL_BLOODWORKER, 3000);
        scriptEvents.addEvent(EVENT_KIKATAL_ERUPTING_WEBS, 8000);
        scriptEvents.addEvent(EVENT_KIKATAL_VENOM_VOLLEY, 12000);
    }

    // full energy pulls everyone into the singularity
    void onCombatTick(uint32_t diff) override
    {
        if (!fillEnergy(diff, KikatalEnergyPerSecond) || _isCasting())
            return;

        emptyEnergy();
        warn("Ki'katal the Harvester manifests a Cosmic Singularity! Hold on in the Black Blood!");
        castSpellAOE(SPELL_KIKATAL_COSMIC_SINGULARITY);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_KIKATAL_ERUPTING_WEBS:
                castSpellAOE(SPELL_KIKATAL_ERUPTING_WEBS);
                scriptEvents.addEvent(EVENT_KIKATAL_ERUPTING_WEBS, 18000);
                break;
            case EVENT_KIKATAL_VENOM_VOLLEY:
                castSpellAOE(SPELL_KIKATAL_VENOM_VOLLEY);
                scriptEvents.addEvent(EVENT_KIKATAL_VENOM_VOLLEY, 22000);
                break;
            case EVENT_KIKATAL_BLOODWORKER:
                sendBloodworker();
                scriptEvents.addEvent(EVENT_KIKATAL_BLOODWORKER, 20000);
                break;
            default:
                break;
        }
    }

private:
    // a drone hauls its Black Blood to the harvester
    void sendBloodworker()
    {
        Creature* pWorker = summonCreature(NPC_BLOODWORKER, KikatalBloodworkerEntry, TIMED_OR_CORPSE_DESPAWN, BloodworkerLifetimeMs);
        if (pWorker == nullptr || pWorker->GetScript() == nullptr)
            return;

        LocationVector const drop = aroundSelf(3.0f, 6.0f);
        pWorker->GetScript()->moveTo(drop.x, drop.y, drop.z, false);
    }
};

// drops its cargo and flees once struck
class AraKaraBloodworkerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AraKaraBloodworkerAI(c); }
    explicit AraKaraBloodworkerAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override { setReactState(REACT_PASSIVE); }

    void OnDamageTaken(Unit* /*pAttacker*/, uint32_t /*amount*/) override
    {
        if (mSpilled || !isAlive())
            return;

        mSpilled = true;
        spillBlood();
        despawn(3000, 30000);
    }

private:
    // the pool belongs to the harvester so a wipe clears it
    void spillBlood()
    {
        Creature* pHarvester = findNearestCreature(BOSS_KIKATAL_THE_HARVESTER, HarvesterSearchRange);
        if (pHarvester == nullptr)
            pHarvester = findNearestCreature(BOSS_KIKATAL_THE_HARVESTER_ALT, HarvesterSearchRange);

        if (pHarvester == nullptr || !pHarvester->isAlive() || !pHarvester->isInCombat() || pHarvester->GetScript() == nullptr)
            return;

        pHarvester->GetScript()->summonCreature(NPC_BLACK_BLOOD, getCreature()->GetPosition(), TIMED_DESPAWN, BlackBloodLifetimeMs);
    }

    bool mSpilled = false;
};

// players stepping in are held by Grasping Blood
class AraKaraBlackBloodAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AraKaraBlackBloodAI(c); }
    explicit AraKaraBlackBloodAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        mClock += static_cast<uint32_t>(time_passed);
        if (mClock < 1000)
            return;

        mClock = 0;
        if (!mSettled)
        {
            mSettled = true;
            setRooted(true);
        }

        for (Unit* pPlayer : araKaraPlayersNear(getCreature(), BlackBloodReach))
        {
            if (!pPlayer->hasAurasWithId(SPELL_KIKATAL_GRASPING_BLOOD))
                castSpell(pPlayer, SPELL_KIKATAL_GRASPING_BLOOD, true);
        }
    }

private:
    uint32_t mClock = 0;
    bool mSettled = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Anub'zekt, calls the whole brood into the Eye of the Swarm at full energy

class AraKaraAnubzektAI : public AraKaraBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AraKaraAnubzektAI(c); }
    explicit AraKaraAnubzektAI(Creature* pCreature) : AraKaraBossAI(pCreature, DATA_ANUBZEKT) {}

protected:
    void onEngage() override
    {
        mInSwarm = false;
        scheduleGroundAttacks();
    }

    void onCombatTick(uint32_t diff) override
    {
        if (mInSwarm || !fillEnergy(diff, AnubzektEnergyPerSecond) || _isCasting())
            return;

        mInSwarm = true;
        scriptEvents.resetEvents();
        warn("Anub'zekt drags his foes into the Eye of the Swarm!");
        castSpellAOE(SPELL_ANUBZEKT_EYE_OF_THE_SWARM);

        scriptEvents.addEvent(EVENT_ANUBZEKT_INFESTATION, 2000);
        scriptEvents.addEvent(EVENT_ANUBZEKT_SWARM_ENDS, EyeOfTheSwarmMs);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ANUBZEKT_IMPALE:
                castSpellOnVictim(SPELL_ANUBZEKT_IMPALE);
                scriptEvents.addEvent(EVENT_ANUBZEKT_IMPALE, 12000);
                break;
            case EVENT_ANUBZEKT_BURROW_CHARGE:
                warn("Anub'zekt burrows and charges!");
                castAt(anyPlayer(), SPELL_ANUBZEKT_BURROW_CHARGE);
                scriptEvents.addEvent(EVENT_ANUBZEKT_BURROW_CHARGE, 22000);
                break;
            case EVENT_ANUBZEKT_INFESTATION:
                castAt(offTankPlayer(), SPELL_ANUBZEKT_INFESTATION);
                scriptEvents.addEvent(EVENT_ANUBZEKT_INFESTATION, mInSwarm ? 7000 : 20000);
                break;
            case EVENT_ANUBZEKT_SWARM_ENDS:
                mInSwarm = false;
                emptyEnergy();
                scriptEvents.resetEvents();
                scheduleGroundAttacks();
                break;
            default:
                break;
        }
    }

private:
    void scheduleGroundAttacks()
    {
        scriptEvents.addEvent(EVENT_ANUBZEKT_IMPALE, 5000);
        scriptEvents.addEvent(EVENT_ANUBZEKT_INFESTATION, 10000);
        scriptEvents.addEvent(EVENT_ANUBZEKT_BURROW_CHARGE, 15000);
    }

    bool mInSwarm = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Avanoxx

class AraKaraAvanoxxAI : public AraKaraBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AraKaraAvanoxxAI(c); }
    explicit AraKaraAvanoxxAI(Creature* pCreature) : AraKaraBossAI(pCreature, DATA_AVANOXX) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_AVANOXX_VORACIOUS_BITE, 5000);
        scriptEvents.addEvent(EVENT_AVANOXX_ALERTING_SHRILL, 10000);
        scriptEvents.addEvent(EVENT_AVANOXX_GOSSAMER_ONSLAUGHT, 30000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_AVANOXX_VORACIOUS_BITE:
                castSpellOnVictim(SPELL_AVANOXX_VORACIOUS_BITE);
                scriptEvents.addEvent(EVENT_AVANOXX_VORACIOUS_BITE, 12000);
                break;
            case EVENT_AVANOXX_ALERTING_SHRILL:
            {
                warn("Avanoxx lets out an Alerting Shrill! Starved Crawlers answer the call!");
                castSpellAOE(SPELL_AVANOXX_ALERTING_SHRILL);
                uint32_t const count = isHeroic() ? 3 : 2;
                for (uint32_t i = 0; i < count; ++i)
                    callAdd(NPC_STARVED_CRAWLER, aroundSelf(10.0f, 18.0f));
                scriptEvents.addEvent(EVENT_AVANOXX_ALERTING_SHRILL, 40000);
                break;
            }
            case EVENT_AVANOXX_GOSSAMER_ONSLAUGHT:
                warn("Avanoxx unleashes a Gossamer Onslaught!");
                castSpellAOE(SPELL_AVANOXX_GOSSAMER_ONSLAUGHT);
                scriptEvents.addEvent(EVENT_AVANOXX_GOSSAMER_ONSLAUGHT, 40000);
                break;
            default:
                break;
        }
    }
};

// grows hungrier over time and fixates a random player
class AraKaraStarvedCrawlerAI : public AraKaraCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AraKaraStarvedCrawlerAI(c); }
    explicit AraKaraStarvedCrawlerAI(Creature* pCreature) : AraKaraCombatAI(pCreature) {}

protected:
    void onEngage() override
    {
        castSpellOnSelf(SPELL_CRAWLER_HUNGER, true);

        if (Unit* pPrey = offTankPlayer())
        {
            _clearHateList();
            addThreat(pPrey, CrawlerFixateThreat);
        }
    }
};

void SetupAraKara(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_ARA_KARA, &AraKaraInstanceScript::Create);

    mgr->register_creature_script(BOSS_KIKATAL_THE_HARVESTER, &AraKaraKikatalAI::Create);
    mgr->register_creature_script(BOSS_KIKATAL_THE_HARVESTER_ALT, &AraKaraKikatalAI::Create);
    mgr->register_creature_script(NPC_BLOODWORKER, &AraKaraBloodworkerAI::Create);
    mgr->register_creature_script(NPC_BLACK_BLOOD, &AraKaraBlackBloodAI::Create);

    mgr->register_creature_script(BOSS_ANUBZEKT, &AraKaraAnubzektAI::Create);

    mgr->register_creature_script(BOSS_AVANOXX, &AraKaraAvanoxxAI::Create);
    mgr->register_creature_script(NPC_STARVED_CRAWLER, &AraKaraStarvedCrawlerAI::Create);
}
