/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Uldaman_Legacy_Of_Tyr.hpp"

#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <cmath>
#include <map>
#include <set>
#include <vector>

enum UldamanLegacyOfTyrEvents
{
    EVENT_BAELOG_HEAVY_ARROW = 1,
    EVENT_BAELOG_WILD_CLEAVE,
    EVENT_ERIC_SKULLCRACKER,
    EVENT_ERIC_DAGGER_THROW,
    EVENT_OLAF_RICOCHETING_SHIELD,
    EVENT_OLAF_DEFENSIVE_BULWARK,

    EVENT_BROMACH_THUNDERING_SLAM,
    EVENT_BROMACH_CALL_OF_THE_DEEP,
    EVENT_BROMACH_QUAKING_TOTEM,
    EVENT_BROMACH_BLOODLUST,

    EVENT_AMBUSHER_AMBUSH,
    EVENT_GEOMANCER_STONE_SPIKE,
    EVENT_GEOMANCER_CHAIN_LIGHTNING,

    EVENT_TALONDRAS_RESONATING_ORB,
    EVENT_TALONDRAS_EARTHEN_SHARDS,
    EVENT_TALONDRAS_CRUSHING_STOMP,

    EVENT_EMBERON_SEARING_CLAP,
    EVENT_EMBERON_UNSTABLE_EMBERS,

    EVENT_DEIOS_ETERNITY_ORB,
    EVENT_DEIOS_WING_BUFFET,
    EVENT_DEIOS_SAND_BREATH,
    EVENT_DEIOS_TIME_SINK
};

namespace
{
    // members of the encounters fought as one
    struct UldamanGroupSlot
    {
        uint32_t dataId;
        uint32_t entry;
    };

    UldamanGroupSlot const UldamanGroupSlots[] =
    {
        { DATA_THE_LOST_DWARVES, BOSS_BAELOG },
        { DATA_THE_LOST_DWARVES, BOSS_ERIC_THE_SWIFT },
        { DATA_THE_LOST_DWARVES, BOSS_OLAF }
    };

    // room doors shut during the fight, passages open after the kill
    struct UldamanDoor
    {
        uint32_t entry;
        uint32_t dataId;
        bool passage;
    };

    UldamanDoor const UldamanDoors[] =
    {
        { GO_PREHISTORIA_VAULT_DOOR,  DATA_BROMACH,            true },
        { GO_TALONDRAS_ENTRANCE_DOOR, DATA_SENTINEL_TALONDRAS, false },
        { GO_TALONDRAS_EXIT_DOOR,     DATA_SENTINEL_TALONDRAS, true },
        { GO_EMBERON_ENTRANCE_DOOR,   DATA_EMBERON,            false },
        { GO_EMBERON_EXIT_DOOR,       DATA_EMBERON,            true },
        { GO_DEIOS_ENTRANCE_DOOR,     DATA_CHRONO_LORD_DEIOS,  false }
    };

    UldamanDoor const* findUldamanDoor(uint32_t entry)
    {
        for (UldamanDoor const& door : UldamanDoors)
        {
            if (door.entry == entry)
                return &door;
        }

        return nullptr;
    }

    // energy bars, time from empty to full
    uint32_t const LongboatRaidMs = 45000;
    uint32_t const DynamoChargeMs = 40000;
    uint32_t const EmberonPurgeMs = 50000;

    // a dwarf leaves for the longboat at this health
    uint32_t const DwarfRetreatPct = 10;

    uint32_t const LongboatVolleyShots = 4;
    uint32_t const LongboatShotsPerCrew = 2;
    uint32_t const LongboatCrewFireMs = 9000;

    uint32_t const EmberonPurgeLimitMs = 60000;
    uint32_t const KeepersPerActivation = 2;

    uint32_t const DeiosOrbCharges = 5;

    LocationVector uldamanRingPoint(LocationVector const& origin, float minRange, float maxRange)
    {
        float const angle = Util::getRandomFloat(6.2831853f);
        float const range = Util::getRandomFloat(minRange, maxRange);
        return LocationVector(origin.x + range * std::cos(angle), origin.y + range * std::sin(angle), origin.z, angle);
    }

    // random living player around a creature without a threat list
    Unit* uldamanPlayerAround(Creature* pCreature, float range)
    {
        std::vector<Unit*> players;
        for (Object* pObject : pCreature->getInRangePlayersSet())
        {
            Unit* pPlayer = static_cast<Unit*>(pObject);
            if (pPlayer != nullptr && pPlayer->isAlive() && pCreature->getDistance(pPlayer) <= range)
                players.push_back(pPlayer);
        }

        if (players.empty())
            return nullptr;

        return players[Util::getRandomUInt(static_cast<uint32_t>(players.size() - 1))];
    }

    uint32_t uldamanPercentOf(uint32_t value, uint32_t pct)
    {
        return static_cast<uint32_t>(static_cast<uint64_t>(value) * pct / 100);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class UldamanLegacyInstanceScript : public InstanceScript
{
public:
    explicit UldamanLegacyInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(UldamanLegacyOfTyrEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new UldamanLegacyInstanceScript(pMapMgr); }

    // the crew is done only when every dwarf has fallen or left for the longboat
    bool setBossState(uint32_t id, EncounterStates state) override
    {
        if (state == EncounterStates::Performed && getBossState(id) == EncounterStates::InProgress && !isGroupDown(id, nullptr))
            return false;

        return InstanceScript::setBossState(id, state);
    }

    void OnCreaturePushToWorld(Creature* pCreature) override
    {
        uint32_t const entry = pCreature->getEntry();
        if (entry == NPC_VAULT_KEEPER)
        {
            mKeeperGuids.insert(pCreature->GetNewGUID());
            return;
        }

        if (entry == NPC_LONGBOAT_RAID || entry == BOSS_EMBERON)
        {
            mTrackedGuids[entry] = pCreature->GetNewGUID();
            return;
        }

        for (UldamanGroupSlot const& slot : UldamanGroupSlots)
        {
            if (slot.entry == entry)
            {
                mTrackedGuids[entry] = pCreature->GetNewGUID();
                return;
            }
        }
    }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        if (findUldamanDoor(pGameObject->getEntry()) == nullptr)
            return;

        mDoorGuids.push_back(pGameObject->GetNewGUID());
        applyDoorState(pGameObject);
    }

    void OnEncounterStateChange(uint32_t id, uint32_t state) override
    {
        for (WoWGuid const& guid : mDoorGuids)
        {
            if (GameObject* pDoor = getGameObjectByGuid(guid))
                applyDoorState(pDoor);
        }

        if (id == DATA_THE_LOST_DWARVES)
            onDwarvesStateChange(state);
        else if (id == DATA_EMBERON && state != EncounterStates::InProgress)
            onKeepersIdle(state == EncounterStates::Failed);
    }

    Creature* getTracked(uint32_t entry)
    {
        auto const itr = mTrackedGuids.find(entry);
        return itr != mTrackedGuids.end() ? getCreatureByGuid(itr->second) : nullptr;
    }

    std::vector<Creature*> getGroup(uint32_t dataId)
    {
        std::vector<Creature*> group;
        for (UldamanGroupSlot const& slot : UldamanGroupSlots)
        {
            if (slot.dataId != dataId)
                continue;

            if (Creature* pMember = getTracked(slot.entry))
                group.push_back(pMember);
        }

        return group;
    }

    // a dwarf aboard the longboat counts as defeated
    bool isGroupDown(uint32_t dataId, Creature* pExcept)
    {
        for (Creature* pMember : getGroup(dataId))
        {
            if (pMember != pExcept && pMember->isAlive() && !isRetreated(pMember->getEntry()))
                return false;
        }

        return true;
    }

    bool isGroupFighting(uint32_t dataId, Creature* pExcept)
    {
        for (Creature* pMember : getGroup(dataId))
        {
            if (pMember != pExcept && pMember->isAlive() && pMember->isInCombat() && !isRetreated(pMember->getEntry()))
                return true;
        }

        return false;
    }

    // pulling one member pulls the rest
    void rallyGroup(uint32_t dataId, Creature* pPuller)
    {
        if (mRallying)
            return;

        mRallying = true;
        for (Creature* pMember : getGroup(dataId))
        {
            if (pMember == pPuller || !pMember->isAlive() || pMember->isInCombat() || pMember->GetScript() == nullptr)
                continue;

            pMember->GetScript()->setZoneWideCombat();
        }
        mRallying = false;
    }

    void sendAction(uint32_t entry, int32_t action)
    {
        Creature* pTarget = getTracked(entry);
        if (pTarget != nullptr && pTarget->isAlive() && pTarget->GetScript() != nullptr)
            pTarget->GetScript()->DoAction(action);
    }

    void markRetreated(uint32_t entry) { mRetreated.insert(entry); }
    bool isRetreated(uint32_t entry) const { return mRetreated.find(entry) != mRetreated.end(); }
    uint32_t retreatedCount() const { return static_cast<uint32_t>(mRetreated.size()); }

    std::vector<Creature*> getKeepers()
    {
        std::vector<Creature*> keepers;
        for (WoWGuid const& guid : mKeeperGuids)
        {
            if (Creature* pKeeper = getCreatureByGuid(guid))
                keepers.push_back(pKeeper);
        }

        return keepers;
    }

private:
    void onDwarvesStateChange(uint32_t state)
    {
        if (state == EncounterStates::Failed)
        {
            // a wiped crew fights again in full
            for (Creature* pDwarf : getGroup(DATA_THE_LOST_DWARVES))
            {
                if (!pDwarf->isAlive() || isRetreated(pDwarf->getEntry()))
                    pDwarf->despawn(1000, 5000);
            }
            mRetreated.clear();
            return;
        }

        if (state != EncounterStates::Performed)
            return;

        // the beaten crew sails off
        for (Creature* pDwarf : getGroup(DATA_THE_LOST_DWARVES))
        {
            if (pDwarf->isAlive())
                pDwarf->despawn(4000, 0);
        }
    }

    // keepers fall dormant, destroyed ones return after a wipe
    void onKeepersIdle(bool restore)
    {
        for (Creature* pKeeper : getKeepers())
        {
            if (!pKeeper->isAlive())
            {
                if (restore)
                    pKeeper->despawn(1000, 5000);
                continue;
            }

            if (pKeeper->GetScript() != nullptr)
                pKeeper->GetScript()->DoAction(ACTION_VAULT_KEEPER_DEACTIVATE);
        }
    }

    void applyDoorState(GameObject* pDoor)
    {
        UldamanDoor const* pRule = findUldamanDoor(pDoor->getEntry());
        if (pRule == nullptr)
            return;

        EncounterStates const state = getBossState(pRule->dataId);
        bool const open = pRule->passage ? state == EncounterStates::Performed : state != EncounterStates::InProgress;
        uint8_t const wanted = open ? GO_STATE_OPEN : GO_STATE_CLOSED;
        if (pDoor->getState() != wanted)
            pDoor->setState(wanted);
    }

    std::map<uint32_t, WoWGuid> mTrackedGuids;
    std::set<WoWGuid> mKeeperGuids;
    std::vector<WoWGuid> mDoorGuids;
    std::set<uint32_t> mRetreated;
    bool mRallying = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event clock, energy stand-in and target helpers

class UldamanLegacyCombatAI : public CreatureAIScript
{
public:
    explicit UldamanLegacyCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        mEnergyMs = 0;
        onEngage();
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        scriptEvents.resetEvents();
        summons.despawnAll();
        mEnergyMs = 0;

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

    UldamanLegacyInstanceScript* getUldaman()
    {
        if (getCreature()->GetMapId() != MAP_ULDAMAN_LEGACY_OF_TYR)
            return nullptr;

        return static_cast<UldamanLegacyInstanceScript*>(getInstanceScript());
    }

    Unit* anyPlayer() { return selectUnitTarget(FilterArgs(TargetFilter_Player)); }

    Unit* castAtPlayer(uint32_t spellId, bool triggered = false)
    {
        Unit* pTarget = anyPlayer();
        if (pTarget != nullptr)
            castSpell(pTarget, spellId, triggered);

        return pTarget;
    }

    // normal difficulty has its own spell versions
    uint32_t byDifficulty(uint32_t heroicSpellId, uint32_t normalSpellId) { return isHeroic() ? heroicSpellId : normalSpellId; }

    Creature* callAdd(uint32_t entry, LocationVector const& spot)
    {
        Creature* pAdd = summonCreature(entry, spot, CORPSE_DESPAWN);
        if (pAdd != nullptr)
            setZoneWideCombat(pAdd);

        return pAdd;
    }

    LocationVector aroundSelf(float minRange, float maxRange) { return uldamanRingPoint(getCreature()->GetPosition(), minRange, maxRange); }

    void warn(char const* text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }

    // energy stand-in: true once the bar is full and nothing is being cast
    bool energyFull(uint32_t diff, uint32_t fullAfterMs)
    {
        if (mEnergyMs < fullAfterMs)
            mEnergyMs += diff;

        if (mEnergyMs < fullAfterMs || _isCasting())
            return false;

        mEnergyMs = 0;
        return true;
    }

    uint32_t mEnergyMs = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame, boss state and group pull

class UldamanLegacyBossAI : public UldamanLegacyCombatAI
{
public:
    UldamanLegacyBossAI(Creature* pCreature, uint32_t dataId) : UldamanLegacyCombatAI(pCreature), mDataId(dataId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (UldamanLegacyInstanceScript* pUldaman = getUldaman())
        {
            pUldaman->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pUldaman->setBossState(mDataId, EncounterStates::InProgress);
            pUldaman->rallyGroup(mDataId, getCreature());
        }

        UldamanLegacyCombatAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        if (isAlive())
        {
            if (UldamanLegacyInstanceScript* pUldaman = getUldaman())
            {
                pUldaman->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

                // the wipe counts once the last partner has left the fight
                if (!pUldaman->isGroupFighting(mDataId, getCreature()))
                    pUldaman->setBossState(mDataId, EncounterStates::Failed);
            }
        }

        UldamanLegacyCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (UldamanLegacyInstanceScript* pUldaman = getUldaman())
        {
            pUldaman->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

            if (pUldaman->isGroupDown(mDataId, getCreature()))
                pUldaman->setBossState(mDataId, EncounterStates::Performed);
        }

        scriptEvents.resetEvents();
        summons.despawnAll();
    }

protected:
    uint32_t mDataId;
};

//////////////////////////////////////////////////////////////////////////////////////////
// The Lost Dwarves, each one gives up at low health and crews the longboat

class UldamanLegacyLostDwarfAI : public UldamanLegacyBossAI
{
public:
    UldamanLegacyLostDwarfAI(Creature* pCreature, char const* retreatText) : UldamanLegacyBossAI(pCreature, DATA_THE_LOST_DWARVES), mRetreatText(retreatText) {}

    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage == nullptr)
            return;

        if (mRetreated)
        {
            *damage = 0;
            return;
        }

        if (!_isInCombat())
            return;

        Creature* pSelf = getCreature();
        uint32_t const health = pSelf->getHealth();
        uint32_t const retreatHealth = uldamanPercentOf(pSelf->getMaxHealth(), DwarfRetreatPct);
        if (health > *damage && health - *damage > retreatHealth)
            return;

        *damage = health > retreatHealth ? health - retreatHealth : 0;
        retreat();
    }

protected:
    void onCombatTick(uint32_t diff) override
    {
        if (mRetreated || !energyFull(diff, LongboatRaidMs))
            return;

        // every dwarf calls, the boat answers once per raid
        if (UldamanLegacyInstanceScript* pUldaman = getUldaman())
            pUldaman->sendAction(NPC_LONGBOAT_RAID, ACTION_LONGBOAT_RAID);
    }

    // a retreated dwarf stays aboard until the encounter is decided
    void onReset() override
    {
        if (!mRetreated)
            return;

        UldamanLegacyInstanceScript* pUldaman = getUldaman();
        if (pUldaman != nullptr && pUldaman->getBossState(DATA_THE_LOST_DWARVES) == EncounterStates::InProgress)
            return;

        mRetreated = false;
        getCreature()->removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        setReactState(REACT_AGGRESSIVE);
        _setMeleeDisabled(false);
        _removeAura(SPELL_DWARF_RECOVERING);
    }

private:
    void retreat()
    {
        mRetreated = true;
        scriptEvents.resetEvents();

        Creature* pSelf = getCreature();
        pSelf->addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        setReactState(REACT_PASSIVE);
        attackStop();
        _setMeleeDisabled(true);
        castSpellOnSelf(SPELL_DWARF_RECOVERING, true);
        warn(mRetreatText);

        UldamanLegacyInstanceScript* pUldaman = getUldaman();
        if (pUldaman == nullptr)
            return;

        if (Creature* pBoat = pUldaman->getTracked(NPC_LONGBOAT_RAID))
            moveTo(pBoat->GetPositionX(), pBoat->GetPositionY(), pBoat->GetPositionZ());

        pUldaman->sendUnitEncounter(EncounterFrameDisengaged, pSelf);
        pUldaman->markRetreated(pSelf->getEntry());

        // the last one to give up ends the encounter
        if (pUldaman->isGroupDown(DATA_THE_LOST_DWARVES, nullptr))
            pUldaman->setBossState(DATA_THE_LOST_DWARVES, EncounterStates::Performed);
    }

    char const* mRetreatText;
    bool mRetreated = false;
};

class UldamanLegacyBaelogAI : public UldamanLegacyLostDwarfAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new UldamanLegacyBaelogAI(c); }
    explicit UldamanLegacyBaelogAI(Creature* pCreature) : UldamanLegacyLostDwarfAI(pCreature, "Baelog retreats to the longboat!") {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_BAELOG_WILD_CLEAVE, 5000);
        scriptEvents.addEvent(EVENT_BAELOG_HEAVY_ARROW, 9000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_BAELOG_HEAVY_ARROW:
                warn("Baelog takes aim with a Heavy Arrow!");
                castAtPlayer(byDifficulty(SPELL_BAELOG_HEAVY_ARROW, SPELL_BAELOG_HEAVY_ARROW_NORMAL));
                scriptEvents.addEvent(EVENT_BAELOG_HEAVY_ARROW, 20000);
                break;
            case EVENT_BAELOG_WILD_CLEAVE:
                castSpellOnVictim(SPELL_BAELOG_WILD_CLEAVE);
                scriptEvents.addEvent(EVENT_BAELOG_WILD_CLEAVE, 12000);
                break;
            default:
                break;
        }
    }
};

class UldamanLegacyEricAI : public UldamanLegacyLostDwarfAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new UldamanLegacyEricAI(c); }
    explicit UldamanLegacyEricAI(Creature* pCreature) : UldamanLegacyLostDwarfAI(pCreature, "Eric \"The Swift\" retreats to the longboat!") {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_ERIC_DAGGER_THROW, 4000);
        scriptEvents.addEvent(EVENT_ERIC_SKULLCRACKER, 10000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ERIC_SKULLCRACKER:
                castAtPlayer(SPELL_ERIC_SKULLCRACKER);
                scriptEvents.addEvent(EVENT_ERIC_SKULLCRACKER, 18000);
                break;
            case EVENT_ERIC_DAGGER_THROW:
                castAtPlayer(SPELL_ERIC_DAGGER_THROW);
                scriptEvents.addEvent(EVENT_ERIC_DAGGER_THROW, 9000);
                break;
            default:
                break;
        }
    }
};

class UldamanLegacyOlafAI : public UldamanLegacyLostDwarfAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new UldamanLegacyOlafAI(c); }
    explicit UldamanLegacyOlafAI(Creature* pCreature) : UldamanLegacyLostDwarfAI(pCreature, "Olaf retreats to the longboat!") {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_OLAF_RICOCHETING_SHIELD, 7000);
        scriptEvents.addEvent(EVENT_OLAF_DEFENSIVE_BULWARK, 20000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_OLAF_RICOCHETING_SHIELD:
                castAtPlayer(SPELL_OLAF_RICOCHETING_SHIELD);
                scriptEvents.addEvent(EVENT_OLAF_RICOCHETING_SHIELD, 16000);
                break;
            case EVENT_OLAF_DEFENSIVE_BULWARK:
                warn("Olaf raises a Defensive Bulwark!");
                castSpellOnSelf(SPELL_OLAF_DEFENSIVE_BULWARK);
                scriptEvents.addEvent(EVENT_OLAF_DEFENSIVE_BULWARK, 35000);
                break;
            default:
                break;
        }
    }
};

// fires while the encounter runs: raids on call, steady fire from the crew aboard
class UldamanLegacyLongboatAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new UldamanLegacyLongboatAI(c); }
    explicit UldamanLegacyLongboatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setRooted(true);
        _setMeleeDisabled(true);
    }

    void DoAction(int32_t action) override
    {
        if (action != ACTION_LONGBOAT_RAID || mVolleyLeft > 0)
            return;

        UldamanLegacyInstanceScript* pUldaman = getUldaman();
        if (pUldaman == nullptr)
            return;

        // every dwarf aboard adds to the barrage
        mVolleyLeft = LongboatVolleyShots + LongboatShotsPerCrew * pUldaman->retreatedCount();
        mVolleyClockMs = 1000;
        sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "The Lost Dwarves launch a Longboat Raid!");

        if (isHeroic())
            castSpellOnSelf(SPELL_LONGBOAT_HASTY_REARMING, true);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        UldamanLegacyInstanceScript* pUldaman = getUldaman();
        if (pUldaman == nullptr || pUldaman->getBossState(DATA_THE_LOST_DWARVES) != EncounterStates::InProgress)
        {
            mVolleyLeft = 0;
            mCrewClockMs = 0;
            return;
        }

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (mVolleyLeft > 0)
        {
            tickVolley(diff);
            return;
        }

        uint32_t const crew = pUldaman->retreatedCount();
        if (crew == 0)
            return;

        mCrewClockMs += diff;
        if (mCrewClockMs < LongboatCrewFireMs / crew)
            return;

        mCrewClockMs = 0;
        fireAt(SPELL_LONGBOAT_SEARING_CANNONFIRE);
    }

private:
    UldamanLegacyInstanceScript* getUldaman()
    {
        if (getCreature()->GetMapId() != MAP_ULDAMAN_LEGACY_OF_TYR)
            return nullptr;

        return static_cast<UldamanLegacyInstanceScript*>(getInstanceScript());
    }

    void tickVolley(uint32_t diff)
    {
        if (mVolleyClockMs > diff)
        {
            mVolleyClockMs -= diff;
            return;
        }

        fireAt(SPELL_LONGBOAT_SEARING_CANNONFIRE);
        mVolleyClockMs = isHeroic() ? 1000 : 1500;
        if (--mVolleyLeft > 0)
            return;

        // the raid ends in a fiery surge and burning pitch
        castSpellAOE(SPELL_LONGBOAT_FIERY_SURGE, true);
        fireAt(SPELL_LONGBOAT_BURNING_PITCH);
    }

    void fireAt(uint32_t spellId)
    {
        if (Unit* pTarget = uldamanPlayerAround(getCreature(), 80.0f))
            castSpell(pTarget, spellId, true);
    }

    uint32_t mVolleyLeft = 0;
    uint32_t mVolleyClockMs = 0;
    uint32_t mCrewClockMs = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Bromach

class UldamanLegacyBromachAI : public UldamanLegacyBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new UldamanLegacyBromachAI(c); }
    explicit UldamanLegacyBromachAI(Creature* pCreature) : UldamanLegacyBossAI(pCreature, DATA_BROMACH) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_BROMACH_THUNDERING_SLAM, 8000);
        scriptEvents.addEvent(EVENT_BROMACH_CALL_OF_THE_DEEP, 15000);
        scriptEvents.addEvent(EVENT_BROMACH_QUAKING_TOTEM, 20000);
        scriptEvents.addEvent(EVENT_BROMACH_BLOODLUST, 30000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_BROMACH_THUNDERING_SLAM:
                castSpellAOE(SPELL_BROMACH_THUNDERING_SLAM);
                scriptEvents.addEvent(EVENT_BROMACH_THUNDERING_SLAM, 18000);
                break;
            case EVENT_BROMACH_CALL_OF_THE_DEEP:
                warn("Bromach calls the troggs from the deep!");
                castSpellAOE(SPELL_BROMACH_CALL_OF_THE_DEEP);
                for (uint8_t i = 0; i < 2; ++i)
                    callAdd(NPC_STONEVAULT_AMBUSHER, aroundSelf(10.0f, 16.0f));
                callAdd(NPC_STONEVAULT_GEOMANCER, aroundSelf(10.0f, 16.0f));
                scriptEvents.addEvent(EVENT_BROMACH_CALL_OF_THE_DEEP, 40000);
                break;
            case EVENT_BROMACH_QUAKING_TOTEM:
                castSpellAOE(SPELL_BROMACH_QUAKING_TOTEM);
                summonCreature(NPC_QUAKING_TOTEM, aroundSelf(8.0f, 14.0f), CORPSE_DESPAWN);
                scriptEvents.addEvent(EVENT_BROMACH_QUAKING_TOTEM, 35000);
                break;
            case EVENT_BROMACH_BLOODLUST:
                warn("Bromach bolsters the troggs with Bloodlust!");
                castSpellAOE(SPELL_BROMACH_BLOODLUST);
                scriptEvents.addEvent(EVENT_BROMACH_BLOODLUST, 45000);
                break;
            default:
                break;
        }
    }
};

// shakes the ground while it stands, a last tremor when destroyed
class UldamanLegacyQuakingTotemAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new UldamanLegacyQuakingTotemAI(c); }
    explicit UldamanLegacyQuakingTotemAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        setRooted(true);
        _setMeleeDisabled(true);
    }

    void OnDied(Unit* /*pKiller*/) override { castSpellOnSelf(SPELL_QUAKING_TOTEM_TREMOR, true); }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!isAlive())
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (mQuakeClockMs > diff)
        {
            mQuakeClockMs -= diff;
            return;
        }

        mQuakeClockMs = 3000;
        castSpellAOE(SPELL_QUAKING_TOTEM_SHOCKING_QUAKE, true);
    }

private:
    uint32_t mQuakeClockMs = 2000;
};

class UldamanLegacyStonevaultAmbusherAI : public UldamanLegacyCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new UldamanLegacyStonevaultAmbusherAI(c); }
    explicit UldamanLegacyStonevaultAmbusherAI(Creature* pCreature) : UldamanLegacyCombatAI(pCreature) {}

protected:
    void onEngage() override { scriptEvents.addEvent(EVENT_AMBUSHER_AMBUSH, 1000); }

    void onEvent(uint32_t eventId) override
    {
        if (eventId == EVENT_AMBUSHER_AMBUSH)
            castAtPlayer(SPELL_AMBUSHER_AMBUSH);
    }
};

class UldamanLegacyStonevaultGeomancerAI : public UldamanLegacyCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new UldamanLegacyStonevaultGeomancerAI(c); }
    explicit UldamanLegacyStonevaultGeomancerAI(Creature* pCreature) : UldamanLegacyCombatAI(pCreature) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_GEOMANCER_STONE_SPIKE, 3000);

        if (isHeroic())
            scriptEvents.addEvent(EVENT_GEOMANCER_CHAIN_LIGHTNING, 8000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_GEOMANCER_STONE_SPIKE:
                castAtPlayer(SPELL_GEOMANCER_STONE_SPIKE);
                scriptEvents.addEvent(EVENT_GEOMANCER_STONE_SPIKE, 7000);
                break;
            case EVENT_GEOMANCER_CHAIN_LIGHTNING:
                castAtPlayer(SPELL_GEOMANCER_CHAIN_LIGHTNING);
                scriptEvents.addEvent(EVENT_GEOMANCER_CHAIN_LIGHTNING, 14000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Sentinel Talondras

class UldamanLegacyTalondrasAI : public UldamanLegacyBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new UldamanLegacyTalondrasAI(c); }
    explicit UldamanLegacyTalondrasAI(Creature* pCreature) : UldamanLegacyBossAI(pCreature, DATA_SENTINEL_TALONDRAS) {}

protected:
    void onEngage() override
    {
        // two charges against stuns
        castSpellOnSelf(SPELL_TALONDRAS_INEXORABLE, true);
        castSpellOnSelf(SPELL_TALONDRAS_INEXORABLE, true);

        if (isHeroic())
            castSpellOnSelf(SPELL_TALONDRAS_ANCIENT_DYNAMO, true);

        scriptEvents.addEvent(EVENT_TALONDRAS_RESONATING_ORB, 5000);
        scriptEvents.addEvent(EVENT_TALONDRAS_EARTHEN_SHARDS, 9000);
        scriptEvents.addEvent(EVENT_TALONDRAS_CRUSHING_STOMP, 14000);
    }

    void onReset() override
    {
        _removeAura(SPELL_TALONDRAS_TITANIC_EMPOWERMENT);
        _removeAura(SPELL_TALONDRAS_ANCIENT_DYNAMO);
    }

    // heroic: the dynamo charges unless she is stunned
    void onCombatTick(uint32_t diff) override
    {
        if (!isHeroic())
            return;

        if (getCreature()->hasUnitStateFlag(UNIT_STATE_STUNNED))
        {
            mEnergyMs = 0;
            return;
        }

        if (!energyFull(diff, DynamoChargeMs))
            return;

        warn("The Ancient Dynamo empowers Sentinel Talondras!");
        castSpellOnSelf(SPELL_TALONDRAS_TITANIC_EMPOWERMENT, true);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_TALONDRAS_RESONATING_ORB:
                castAtPlayer(SPELL_TALONDRAS_RESONATING_ORB);
                scriptEvents.addEvent(EVENT_TALONDRAS_RESONATING_ORB, 22000);
                break;
            case EVENT_TALONDRAS_EARTHEN_SHARDS:
                castAtPlayer(SPELL_TALONDRAS_EARTHEN_SHARDS);
                scriptEvents.addEvent(EVENT_TALONDRAS_EARTHEN_SHARDS, 15000);
                break;
            case EVENT_TALONDRAS_CRUSHING_STOMP:
                castSpellAOE(SPELL_TALONDRAS_CRUSHING_STOMP);
                scriptEvents.addEvent(EVENT_TALONDRAS_CRUSHING_STOMP, 24000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Emberon

class UldamanLegacyEmberonAI : public UldamanLegacyBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new UldamanLegacyEmberonAI(c); }
    explicit UldamanLegacyEmberonAI(Creature* pCreature) : UldamanLegacyBossAI(pCreature, DATA_EMBERON) {}

    // Sacred Barrier: nothing gets through while he purges
    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage != nullptr && mPurging)
            *damage = 0;
    }

protected:
    void onEngage() override
    {
        mPurging = false;
        mPurgeMs = 0;
        addRegularEvents();

        if (isHeroic())
            activateKeepers();
    }

    void onReset() override
    {
        if (mPurging)
            stopPurging();
    }

    void onCombatTick(uint32_t diff) override
    {
        if (mPurging)
        {
            mPurgeMs += diff;
            if (mPurgeMs >= EmberonPurgeLimitMs || !anyKeeperActive())
                endPurging();
            return;
        }

        if (energyFull(diff, EmberonPurgeMs))
            startPurging();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_EMBERON_SEARING_CLAP:
                castSpellOnVictim(SPELL_EMBERON_SEARING_CLAP);
                scriptEvents.addEvent(EVENT_EMBERON_SEARING_CLAP, 16000);
                break;
            case EVENT_EMBERON_UNSTABLE_EMBERS:
                castAtPlayer(SPELL_EMBERON_UNSTABLE_EMBERS);
                scriptEvents.addEvent(EVENT_EMBERON_UNSTABLE_EMBERS, 22000);
                break;
            default:
                break;
        }
    }

private:
    void addRegularEvents()
    {
        scriptEvents.addEvent(EVENT_EMBERON_SEARING_CLAP, 6000);
        scriptEvents.addEvent(EVENT_EMBERON_UNSTABLE_EMBERS, 10000);
    }

    // heroic: barrier holds until the woken keepers are destroyed
    void startPurging()
    {
        warn("Emberon channels Purging Flames!");
        if (!isHeroic())
        {
            castSpellOnSelf(SPELL_EMBERON_PURGING_FLAMES_NORMAL);
            return;
        }

        if (!anyKeeperActive())
            activateKeepers();

        mPurging = true;
        mPurgeMs = 0;
        scriptEvents.resetEvents();
        setRooted(true);
        castSpellOnSelf(SPELL_EMBERON_SACRED_BARRIER, true);
        castSpellOnSelf(SPELL_EMBERON_PURGING_FLAMES);
    }

    void stopPurging()
    {
        mPurging = false;
        getCreature()->interruptSpell();
        _removeAura(SPELL_EMBERON_PURGING_FLAMES);
        _removeAura(SPELL_EMBERON_SACRED_BARRIER);
        setRooted(false);
    }

    void endPurging()
    {
        stopPurging();
        warn("Emberon's Sacred Barrier fades!");
        addRegularEvents();
        activateKeepers();
    }

    void activateKeepers()
    {
        UldamanLegacyInstanceScript* pUldaman = getUldaman();
        if (pUldaman == nullptr)
            return;

        castSpellAOE(SPELL_EMBERON_ACTIVATE_KEEPERS, true);

        uint32_t woken = 0;
        for (Creature* pKeeper : pUldaman->getKeepers())
        {
            if (woken >= KeepersPerActivation)
                break;

            if (!pKeeper->isAlive() || !pKeeper->hasUnitFlags(UNIT_FLAG_NOT_SELECTABLE) || pKeeper->GetScript() == nullptr)
                continue;

            pKeeper->GetScript()->DoAction(ACTION_VAULT_KEEPER_ACTIVATE);
            ++woken;
        }
    }

    // an active keeper can be targeted, a dormant one cannot
    bool anyKeeperActive()
    {
        UldamanLegacyInstanceScript* pUldaman = getUldaman();
        if (pUldaman == nullptr)
            return false;

        for (Creature* pKeeper : pUldaman->getKeepers())
        {
            if (pKeeper->isAlive() && !pKeeper->hasUnitFlags(UNIT_FLAG_NOT_SELECTABLE))
                return true;
        }

        return false;
    }

    bool mPurging = false;
    uint32_t mPurgeMs = 0;
};

// dormant until Emberon wakes it, then hurls flames until destroyed
class UldamanLegacyVaultKeeperAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new UldamanLegacyVaultKeeperAI(c); }
    explicit UldamanLegacyVaultKeeperAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setRooted(true);
        _setMeleeDisabled(true);
        setReactState(REACT_PASSIVE);
        fallDormant();
    }

    void DoAction(int32_t action) override
    {
        if (!isAlive())
            return;

        if (action == ACTION_VAULT_KEEPER_ACTIVATE)
            wake();
        else if (action == ACTION_VAULT_KEEPER_DEACTIVATE)
            fallDormant();
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!mActive || !isAlive())
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (mFlameClockMs > diff)
        {
            mFlameClockMs -= diff;
            return;
        }

        mFlameClockMs = Util::getRandomUInt(5000, 8000);
        if (Unit* pTarget = uldamanPlayerAround(getCreature(), 60.0f))
            castSpell(pTarget, SPELL_VAULT_KEEPER_SEEKING_FLAME);
    }

private:
    void wake()
    {
        mActive = true;
        mFlameClockMs = 3000;

        Creature* pSelf = getCreature();
        pSelf->setHealth(pSelf->getMaxHealth());
        pSelf->removeUnitFlags(UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_IGNORE_PLAYER_COMBAT);
    }

    void fallDormant()
    {
        mActive = false;
        getCreature()->addUnitFlags(UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
    }

    bool mActive = false;
    uint32_t mFlameClockMs = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Chrono-Lord Deios

class UldamanLegacyDeiosAI : public UldamanLegacyBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new UldamanLegacyDeiosAI(c); }
    explicit UldamanLegacyDeiosAI(Creature* pCreature) : UldamanLegacyBossAI(pCreature, DATA_CHRONO_LORD_DEIOS) {}

protected:
    void onEngage() override
    {
        // starts with a full bar of temporal energy
        mOrbsLeft = DeiosOrbCharges;
        scriptEvents.addEvent(EVENT_DEIOS_ETERNITY_ORB, 6000);
        scriptEvents.addEvent(EVENT_DEIOS_SAND_BREATH, 8000);
        scriptEvents.addEvent(EVENT_DEIOS_WING_BUFFET, 14000);

        if (isHeroic())
            scriptEvents.addEvent(EVENT_DEIOS_TIME_SINK, 12000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_DEIOS_ETERNITY_ORB:
                spendTime();
                break;
            case EVENT_DEIOS_SAND_BREATH:
                castSpellOnVictim(SPELL_DEIOS_SAND_BREATH);
                scriptEvents.addEvent(EVENT_DEIOS_SAND_BREATH, 18000);
                break;
            case EVENT_DEIOS_WING_BUFFET:
                castSpellAOE(SPELL_DEIOS_WING_BUFFET);
                scriptEvents.addEvent(EVENT_DEIOS_WING_BUFFET, 24000);
                break;
            case EVENT_DEIOS_TIME_SINK:
                castAtPlayer(SPELL_DEIOS_TIME_SINK);
                scriptEvents.addEvent(EVENT_DEIOS_TIME_SINK, 25000);
                break;
            default:
                break;
        }
    }

private:
    // every orb costs energy, an empty bar rewinds time
    void spendTime()
    {
        if (mOrbsLeft == 0)
        {
            warn("Chrono-Lord Deios rewinds the flow of time!");
            castSpellAOE(byDifficulty(SPELL_DEIOS_REWIND_TIMEFLOW, SPELL_DEIOS_REWIND_TIMEFLOW_NORMAL));
            mOrbsLeft = DeiosOrbCharges;
            scriptEvents.addEvent(EVENT_DEIOS_ETERNITY_ORB, 12000);
            return;
        }

        castAtPlayer(SPELL_DEIOS_ETERNITY_ORB);
        --mOrbsLeft;
        scriptEvents.addEvent(EVENT_DEIOS_ETERNITY_ORB, 9000);
    }

    uint32_t mOrbsLeft = DeiosOrbCharges;
};

void SetupUldamanLegacyOfTyr(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_ULDAMAN_LEGACY_OF_TYR, &UldamanLegacyInstanceScript::Create);

    mgr->register_creature_script(BOSS_BAELOG, &UldamanLegacyBaelogAI::Create);
    mgr->register_creature_script(BOSS_ERIC_THE_SWIFT, &UldamanLegacyEricAI::Create);
    mgr->register_creature_script(BOSS_OLAF, &UldamanLegacyOlafAI::Create);
    mgr->register_creature_script(NPC_LONGBOAT_RAID, &UldamanLegacyLongboatAI::Create);

    mgr->register_creature_script(BOSS_BROMACH, &UldamanLegacyBromachAI::Create);
    mgr->register_creature_script(NPC_QUAKING_TOTEM, &UldamanLegacyQuakingTotemAI::Create);
    mgr->register_creature_script(NPC_STONEVAULT_AMBUSHER, &UldamanLegacyStonevaultAmbusherAI::Create);
    mgr->register_creature_script(NPC_STONEVAULT_GEOMANCER, &UldamanLegacyStonevaultGeomancerAI::Create);

    mgr->register_creature_script(BOSS_SENTINEL_TALONDRAS, &UldamanLegacyTalondrasAI::Create);

    mgr->register_creature_script(BOSS_EMBERON, &UldamanLegacyEmberonAI::Create);
    mgr->register_creature_script(NPC_VAULT_KEEPER, &UldamanLegacyVaultKeeperAI::Create);

    mgr->register_creature_script(BOSS_CHRONO_LORD_DEIOS, &UldamanLegacyDeiosAI::Create);
}
