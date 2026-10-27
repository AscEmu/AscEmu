/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Ruby_Life_Pools.hpp"

#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <cmath>
#include <initializer_list>
#include <map>
#include <vector>

enum RubyLifePoolsEvents
{
    EVENT_KOKIA_NEXT_STRIKE = 1,
    EVENT_KOKIA_CALL_BLAZEBOUND,

    EVENT_BLAZEBOUND_BLAZE_VOLLEY,
    EVENT_BLAZEBOUND_INFERNO,
    EVENT_BLAZEBOUND_BURNOUT,

    EVENT_MELIDRUSSA_FRIGID_SHARD,
    EVENT_MELIDRUSSA_CHILLSTORM,
    EVENT_MELIDRUSSA_HAILBURST,

    EVENT_KYRAKKA_INFERNO_SPIT,
    EVENT_KYRAKKA_ROARING_FIREBREATH,

    EVENT_ERKHART_STORMSLAM,
    EVENT_ERKHART_WINDS_OF_CHANGE,
    EVENT_ERKHART_INTERRUPTING_CLOUDBURST
};

namespace
{
    float const RubyLifePoolsTwoPi = 6.28318531f;

    uint32_t const KokiaEnergyPerSecond = 3;
    uint32_t const KokiaStrikeIntervalMs = 9000;

    // Melidrussa wakes her whelps at these health marks
    int32_t const MelidrussaWhelpMarks[2] = { 70, 40 };
    uint32_t const MelidrussaWhelpsPerWave = 4;

    // either rider or drake this low brings Kyrakka down
    int32_t const KyrakkaLandingPct = 50;

    // members of the encounter fought as one
    struct RubyLifePoolsGroupSlot
    {
        uint32_t dataId;
        uint32_t entry;
    };

    RubyLifePoolsGroupSlot const RubyLifePoolsGroupSlots[] =
    {
        { DATA_KYRAKKA_AND_ERKHART, BOSS_KYRAKKA },
        { DATA_KYRAKKA_AND_ERKHART, BOSS_KYRAKKA_ALT },
        { DATA_KYRAKKA_AND_ERKHART, BOSS_ERKHART_STORMVEIN }
    };

    // room doors shut during the fight, passages open after the kill
    struct RubyLifePoolsDoor
    {
        uint32_t entry;
        uint32_t dataId;
        bool passage;
    };

    RubyLifePoolsDoor const RubyLifePoolsDoors[] =
    {
        { GO_KOKIA_FIRE_WALL, DATA_KOKIA_BLAZEHOOF, true }
    };

    RubyLifePoolsDoor const* findRubyLifePoolsDoor(uint32_t entry)
    {
        for (RubyLifePoolsDoor const& door : RubyLifePoolsDoors)
        {
            if (door.entry == entry)
                return &door;
        }

        return nullptr;
    }

    LocationVector pointNear(LocationVector const& origin, float minRange, float maxRange)
    {
        float const angle = Util::getRandomFloat(RubyLifePoolsTwoPi);
        float const range = Util::getRandomFloat(minRange, maxRange);
        return LocationVector(origin.x + range * std::cos(angle), origin.y + range * std::sin(angle), origin.z, angle);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class RubyLifePoolsInstanceScript : public InstanceScript
{
public:
    explicit RubyLifePoolsInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(RubyLifePoolsEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new RubyLifePoolsInstanceScript(pMapMgr); }

    // a fallen partner must not end the encounter while the other still fights
    bool setBossState(uint32_t id, EncounterStates state) override
    {
        if (state == EncounterStates::Performed && getBossState(id) == EncounterStates::InProgress && !isGroupDown(id, nullptr))
            return false;

        return InstanceScript::setBossState(id, state);
    }

    void OnCreaturePushToWorld(Creature* pCreature) override
    {
        for (RubyLifePoolsGroupSlot const& slot : RubyLifePoolsGroupSlots)
        {
            if (slot.entry == pCreature->getEntry())
            {
                mMemberGuids[slot.entry] = pCreature->GetNewGUID();
                return;
            }
        }
    }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        if (findRubyLifePoolsDoor(pGameObject->getEntry()) == nullptr)
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

        if (state != EncounterStates::Failed)
            return;

        // a wiped pair fights again in full
        for (Creature* pMember : getGroup(id))
        {
            if (!pMember->isAlive())
                pMember->despawn(1000, 5000);
        }
    }

    Creature* getMember(uint32_t entry)
    {
        auto const itr = mMemberGuids.find(entry);
        return itr != mMemberGuids.end() ? getCreatureByGuid(itr->second) : nullptr;
    }

    std::vector<Creature*> getGroup(uint32_t dataId)
    {
        std::vector<Creature*> group;
        for (RubyLifePoolsGroupSlot const& slot : RubyLifePoolsGroupSlots)
        {
            if (slot.dataId != dataId)
                continue;

            if (Creature* pMember = getMember(slot.entry))
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

    bool isGroupFighting(uint32_t dataId, Creature* pExcept)
    {
        for (Creature* pMember : getGroup(dataId))
        {
            if (pMember != pExcept && pMember->isAlive() && pMember->isInCombat())
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

    // the drake is the one living Kyrakka of the pair
    Creature* getKyrakka()
    {
        for (uint32_t entry : { BOSS_KYRAKKA, BOSS_KYRAKKA_ALT })
        {
            Creature* pKyrakka = getMember(entry);
            if (pKyrakka != nullptr && pKyrakka->isAlive())
                return pKyrakka;
        }

        return nullptr;
    }

    void sendAction(Creature* pTarget, int32_t action)
    {
        if (pTarget != nullptr && pTarget->isAlive() && pTarget->GetScript() != nullptr)
            pTarget->GetScript()->DoAction(action);
    }

private:
    void applyDoorState(GameObject* pDoor)
    {
        RubyLifePoolsDoor const* pRule = findRubyLifePoolsDoor(pDoor->getEntry());
        if (pRule == nullptr)
            return;

        EncounterStates const state = getBossState(pRule->dataId);
        bool const open = pRule->passage ? state == EncounterStates::Performed : state != EncounterStates::InProgress;
        uint8_t const wanted = open ? GO_STATE_OPEN : GO_STATE_CLOSED;
        if (pDoor->getState() != wanted)
            pDoor->setState(wanted);
    }

    std::map<uint32_t, WoWGuid> mMemberGuids;
    std::vector<WoWGuid> mDoorGuids;
    bool mRallying = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event clock while in combat

class RubyLifePoolsCombatAI : public CreatureAIScript
{
public:
    explicit RubyLifePoolsCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

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

    RubyLifePoolsInstanceScript* getPools()
    {
        if (getCreature()->GetMapId() != MAP_RUBY_LIFE_POOLS)
            return nullptr;

        return static_cast<RubyLifePoolsInstanceScript*>(getInstanceScript());
    }

    Unit* anyPlayer() { return selectUnitTarget(FilterArgs(TargetFilter_Player)); }

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

    void showEnergy(uint32_t value)
    {
        if (getCreature()->getMaxPower(POWER_TYPE_ENERGY) > 0)
            getCreature()->setPower(POWER_TYPE_ENERGY, value);
    }

    void warn(char const* text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame, boss state and group pull

class RubyLifePoolsBossAI : public RubyLifePoolsCombatAI
{
public:
    RubyLifePoolsBossAI(Creature* pCreature, uint32_t dataId) : RubyLifePoolsCombatAI(pCreature), mDataId(dataId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mDataId, EncounterStates::InProgress);
        }

        onPull();

        if (RubyLifePoolsInstanceScript* pPools = getPools())
            pPools->rallyGroup(mDataId, getCreature());

        RubyLifePoolsCombatAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        if (isAlive())
        {
            if (InstanceScript* pInstance = getInstanceScript())
            {
                pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

                // the wipe counts once the last partner has left the fight
                RubyLifePoolsInstanceScript* pPools = getPools();
                if (pPools == nullptr || !pPools->isGroupFighting(mDataId, getCreature()))
                    pInstance->setBossState(mDataId, EncounterStates::Failed);
            }
        }

        RubyLifePoolsCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        onDefeat();

        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

            RubyLifePoolsInstanceScript* pPools = getPools();
            if (pPools == nullptr || pPools->isGroupDown(mDataId, getCreature()))
                pInstance->setBossState(mDataId, EncounterStates::Performed);
        }

        scriptEvents.resetEvents();
        summons.despawnAll();
    }

protected:
    virtual void onPull() {}
    virtual void onDefeat() {}

    uint32_t mDataId;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Kokia Blazehoof

class RubyLifePoolsKokiaAI : public RubyLifePoolsBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new RubyLifePoolsKokiaAI(c); }
    explicit RubyLifePoolsKokiaAI(Creature* pCreature) : RubyLifePoolsBossAI(pCreature, DATA_KOKIA_BLAZEHOOF) {}

protected:
    void onEngage() override
    {
        mEnergy = 0;
        mEnergyClock = 0;
        mBoulderNext = true;
        showEnergy(0);
        scriptEvents.addEvent(EVENT_KOKIA_NEXT_STRIKE, 6000);
    }

    void onReset() override { showEnergy(0); }

    void onCombatTick(uint32_t diff) override
    {
        mEnergyClock += diff;
        while (mEnergyClock >= 1000)
        {
            mEnergyClock -= 1000;
            mEnergy = mEnergy + KokiaEnergyPerSecond > 100 ? 100 : mEnergy + KokiaEnergyPerSecond;
            showEnergy(mEnergy);
        }

        if (mEnergy < 100 || _isCasting())
            return;

        mEnergy = 0;
        showEnergy(0);
        warn("Kokia Blazehoof begins a Ritual of Blazebinding!");
        castAt(anyPlayer(), SPELL_KOKIA_RITUAL_OF_BLAZEBINDING);
        scriptEvents.addEvent(EVENT_KOKIA_CALL_BLAZEBOUND, 500);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_KOKIA_NEXT_STRIKE:
                // boulders and blows take turns
                if (mBoulderNext)
                    castAt(anyPlayer(), SPELL_KOKIA_MOLTEN_BOULDER);
                else
                    castSpellOnVictim(SPELL_KOKIA_SEARING_BLOWS);
                mBoulderNext = !mBoulderNext;
                scriptEvents.addEvent(EVENT_KOKIA_NEXT_STRIKE, KokiaStrikeIntervalMs);
                break;
            case EVENT_KOKIA_CALL_BLAZEBOUND:
                callAdd(NPC_BLAZEBOUND_FIRE_STORM, pointNear(getCreature()->GetPosition(), 6.0f, 12.0f));
                break;
            default:
                break;
        }
    }

private:
    uint32_t mEnergy = 0;
    uint32_t mEnergyClock = 0;
    bool mBoulderNext = true;
};

class RubyLifePoolsBlazeboundAI : public RubyLifePoolsCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new RubyLifePoolsBlazeboundAI(c); }
    explicit RubyLifePoolsBlazeboundAI(Creature* pCreature) : RubyLifePoolsCombatAI(pCreature) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_BLAZEBOUND_BLAZE_VOLLEY, 4000);
        scriptEvents.addEvent(EVENT_BLAZEBOUND_INFERNO, 10000);
        scriptEvents.addEvent(EVENT_BLAZEBOUND_BURNOUT, 16000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_BLAZEBOUND_BLAZE_VOLLEY:
                castSpellAOE(SPELL_BLAZEBOUND_BLAZE_VOLLEY);
                scriptEvents.addEvent(EVENT_BLAZEBOUND_BLAZE_VOLLEY, 12000);
                break;
            case EVENT_BLAZEBOUND_INFERNO:
                castSpellAOE(SPELL_BLAZEBOUND_INFERNO);
                scriptEvents.addEvent(EVENT_BLAZEBOUND_INFERNO, 20000);
                break;
            case EVENT_BLAZEBOUND_BURNOUT:
                castSpellAOE(SPELL_BLAZEBOUND_BURNOUT);
                scriptEvents.addEvent(EVENT_BLAZEBOUND_BURNOUT, 24000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Melidrussa Chillworn

class RubyLifePoolsMelidrussaAI : public RubyLifePoolsBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new RubyLifePoolsMelidrussaAI(c); }
    explicit RubyLifePoolsMelidrussaAI(Creature* pCreature) : RubyLifePoolsBossAI(pCreature, DATA_MELIDRUSSA_CHILLWORN) {}

protected:
    void onEngage() override
    {
        mWavesCalled = 0;
        scriptEvents.addEvent(EVENT_MELIDRUSSA_FRIGID_SHARD, 3000);
        scriptEvents.addEvent(EVENT_MELIDRUSSA_CHILLSTORM, 12000);
        scriptEvents.addEvent(EVENT_MELIDRUSSA_HAILBURST, 18000);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mWavesCalled < 2 && _getHealthPercent() <= MelidrussaWhelpMarks[mWavesCalled] && !_isCasting())
        {
            ++mWavesCalled;
            awakenWhelps();
        }
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_MELIDRUSSA_FRIGID_SHARD:
                castAt(anyPlayer(), SPELL_MELIDRUSSA_FRIGID_SHARD);
                scriptEvents.addEvent(EVENT_MELIDRUSSA_FRIGID_SHARD, 7000);
                break;
            case EVENT_MELIDRUSSA_CHILLSTORM:
                castAt(anyPlayer(), SPELL_MELIDRUSSA_CHILLSTORM);
                scriptEvents.addEvent(EVENT_MELIDRUSSA_CHILLSTORM, 25000);
                break;
            case EVENT_MELIDRUSSA_HAILBURST:
                castSpellAOE(SPELL_MELIDRUSSA_HAILBURST);
                scriptEvents.addEvent(EVENT_MELIDRUSSA_HAILBURST, 22000);
                break;
            default:
                break;
        }
    }

private:
    void awakenWhelps()
    {
        warn("Melidrussa Chillworn awakens her Infused Whelps!");
        castSpellOnSelf(SPELL_MELIDRUSSA_AWAKEN_WHELPS, true);

        for (uint32_t i = 0; i < MelidrussaWhelpsPerWave; ++i)
            callAdd(NPC_INFUSED_WHELP, pointNear(getCreature()->GetPosition(), 6.0f, 12.0f));
    }

    uint32_t mWavesCalled = 0;
};

// every claw strike chills its target
class RubyLifePoolsInfusedWhelpAI : public RubyLifePoolsCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new RubyLifePoolsInfusedWhelpAI(c); }
    explicit RubyLifePoolsInfusedWhelpAI(Creature* pCreature) : RubyLifePoolsCombatAI(pCreature) {}

protected:
    void onEngage() override
    {
        if (!hasAura(SPELL_WHELP_COLD_CLAWS))
            castSpellOnSelf(SPELL_WHELP_COLD_CLAWS, true);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Kyrakka and Erkhart Stormvein

// airborne at first, lands once either partner is worn down
class RubyLifePoolsKyrakkaAI : public RubyLifePoolsBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new RubyLifePoolsKyrakkaAI(c); }
    explicit RubyLifePoolsKyrakkaAI(Creature* pCreature) : RubyLifePoolsBossAI(pCreature, DATA_KYRAKKA_AND_ERKHART) {}

    void DoAction(int32_t action) override
    {
        if (action == ACTION_KYRAKKA_LAND && mAirborne && _isInCombat())
            land();
    }

protected:
    // the rider jumps down to meet the intruders, he is called in when missing
    void onPull() override
    {
        RubyLifePoolsInstanceScript* pPools = getPools();
        if (pPools == nullptr)
            return;

        Creature* pErkhart = pPools->getMember(BOSS_ERKHART_STORMVEIN);
        if (pErkhart != nullptr && pErkhart->isAlive())
            return;

        LocationVector const& self = getCreature()->GetPosition();
        spawnCreature(BOSS_ERKHART_STORMVEIN, LocationVector(self.x + 3.0f, self.y + 3.0f, self.z, self.o));
    }

    void onEngage() override
    {
        mAirborne = true;
        setRooted(true);
        _setMeleeDisabled(true);

        scriptEvents.addEvent(EVENT_KYRAKKA_INFERNO_SPIT, 6000);
        scriptEvents.addEvent(EVENT_KYRAKKA_ROARING_FIREBREATH, 14000);
    }

    void onReset() override
    {
        mAirborne = false;
        setRooted(false);
        _setMeleeDisabled(false);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mAirborne && _getHealthPercent() <= KyrakkaLandingPct)
            land();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_KYRAKKA_INFERNO_SPIT:
                castAt(anyPlayer(), SPELL_KYRAKKA_INFERNO_SPIT);
                scriptEvents.addEvent(EVENT_KYRAKKA_INFERNO_SPIT, 12000);
                break;
            case EVENT_KYRAKKA_ROARING_FIREBREATH:
                castAt(anyPlayer(), SPELL_KYRAKKA_ROARING_FIREBREATH);
                scriptEvents.addEvent(EVENT_KYRAKKA_ROARING_FIREBREATH, 18000);
                break;
            default:
                break;
        }
    }

private:
    void land()
    {
        mAirborne = false;
        setRooted(false);
        _setMeleeDisabled(false);
        warn("Kyrakka lands and Erkhart Stormvein mounts up again!");

        if (RubyLifePoolsInstanceScript* pPools = getPools())
            pPools->sendAction(pPools->getMember(BOSS_ERKHART_STORMVEIN), ACTION_ERKHART_REMOUNT);
    }

    bool mAirborne = false;
};

class RubyLifePoolsErkhartAI : public RubyLifePoolsBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new RubyLifePoolsErkhartAI(c); }
    explicit RubyLifePoolsErkhartAI(Creature* pCreature) : RubyLifePoolsBossAI(pCreature, DATA_KYRAKKA_AND_ERKHART) {}

    void DoAction(int32_t action) override
    {
        if (action != ACTION_ERKHART_REMOUNT || !_isInCombat())
            return;

        // back at his drake's side
        mWornDown = true;
        RubyLifePoolsInstanceScript* pPools = getPools();
        Creature* pKyrakka = pPools != nullptr ? pPools->getKyrakka() : nullptr;
        if (pKyrakka != nullptr)
            moveJump(pointNear(pKyrakka->GetPosition(), 2.0f, 4.0f), 20.0f, 8.0f);
    }

protected:
    void onEngage() override
    {
        mWornDown = false;
        scriptEvents.addEvent(EVENT_ERKHART_STORMSLAM, 5000);
        scriptEvents.addEvent(EVENT_ERKHART_INTERRUPTING_CLOUDBURST, 9000);
        scriptEvents.addEvent(EVENT_ERKHART_WINDS_OF_CHANGE, 14000);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mWornDown || _getHealthPercent() > KyrakkaLandingPct)
            return;

        mWornDown = true;
        if (RubyLifePoolsInstanceScript* pPools = getPools())
            pPools->sendAction(pPools->getKyrakka(), ACTION_KYRAKKA_LAND);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ERKHART_STORMSLAM:
                castSpellOnVictim(SPELL_ERKHART_STORMSLAM);
                scriptEvents.addEvent(EVENT_ERKHART_STORMSLAM, 11000);
                break;
            case EVENT_ERKHART_INTERRUPTING_CLOUDBURST:
                castSpellAOE(SPELL_ERKHART_INTERRUPTING_CLOUDBURST);
                scriptEvents.addEvent(EVENT_ERKHART_INTERRUPTING_CLOUDBURST, 18000);
                break;
            case EVENT_ERKHART_WINDS_OF_CHANGE:
                warn("Erkhart Stormvein calls the Winds of Change!");
                castSpellAOE(SPELL_ERKHART_WINDS_OF_CHANGE);
                scriptEvents.addEvent(EVENT_ERKHART_WINDS_OF_CHANGE, 20000);
                break;
            default:
                break;
        }
    }

private:
    bool mWornDown = false;
};

void SetupRubyLifePools(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_RUBY_LIFE_POOLS, &RubyLifePoolsInstanceScript::Create);

    mgr->register_creature_script(BOSS_KOKIA_BLAZEHOOF, &RubyLifePoolsKokiaAI::Create);
    mgr->register_creature_script(NPC_BLAZEBOUND_FIRE_STORM, &RubyLifePoolsBlazeboundAI::Create);

    mgr->register_creature_script(BOSS_MELIDRUSSA_CHILLWORN, &RubyLifePoolsMelidrussaAI::Create);
    mgr->register_creature_script(NPC_INFUSED_WHELP, &RubyLifePoolsInfusedWhelpAI::Create);

    mgr->register_creature_script(BOSS_KYRAKKA, &RubyLifePoolsKyrakkaAI::Create);
    mgr->register_creature_script(BOSS_KYRAKKA_ALT, &RubyLifePoolsKyrakkaAI::Create);
    mgr->register_creature_script(BOSS_ERKHART_STORMVEIN, &RubyLifePoolsErkhartAI::Create);
}
