/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Grimrail_Depot.hpp"

#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <map>
#include <set>
#include <vector>

enum GrimrailDepotEvents
{
    EVENT_ROCKETSPARK_VX18 = 1,
    EVENT_ROCKETSPARK_X21_BARRAGE,
    EVENT_ROCKETSPARK_NEW_PLAN_MISSILE,

    EVENT_BORKA_SLAM,
    EVENT_BORKA_MAD_DASH,

    EVENT_NITROGG_GRENADE,
    EVENT_NITROGG_MORTAR,
    EVENT_NITROGG_SUPPRESSIVE_FIRE,
    EVENT_NITROGG_SLAG_BLAST,
    EVENT_NITROGG_CALL_GROMKAR,
    EVENT_NITROGG_CALL_INFANTRY,

    EVENT_BOOMER_MORTAR,
    EVENT_GRENADIER_GRENADE,
    EVENT_GUNNER_SHRAPNEL_BLAST,

    EVENT_TOVRA_FREEZING_SNARE,
    EVENT_TOVRA_SPINNING_SPEAR
};

// reinforcement drop points in the assault car
static LocationVector const NitroggGromkarDrop(1628.354f, 1851.177f, 114.154f, 4.650741f);
static LocationVector const NitroggInfantryDrops[2] =
{
    { 1653.266f, 1794.999f, 107.767f, 2.742215f },
    { 1659.568f, 1813.276f, 108.144f, 3.194596f }
};

class GrimrailDepotInstanceScript : public InstanceScript
{
public:
    explicit GrimrailDepotInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(GrimrailDepotEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new GrimrailDepotInstanceScript(pMapMgr); }

    // a credited member falling before its partners must not end the encounter
    bool setBossState(uint32_t id, EncounterStates state) override
    {
        if (state == EncounterStates::Performed && getBossState(id) == EncounterStates::InProgress && id == DATA_ROCKETSPARK_AND_BORKA && !isRailCrewDown(nullptr))
            return false;

        return InstanceScript::setBossState(id, state);
    }

    void OnCreaturePushToWorld(Creature* pCreature) override
    {
        switch (pCreature->getEntry())
        {
            case BOSS_ROCKETSPARK:
            case BOSS_BORKA:
            case BOSS_BORKA_ALT:
                mRailCrewGuids[pCreature->getEntry()] = pCreature->GetNewGUID();
                break;
            default:
                break;
        }
    }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        switch (pGameObject->getEntry())
        {
            case GO_RAILYARD_SPIKED_GATE:
                mRailyardSealGuids.insert(pGameObject->GetNewGUID());
                break;
            case GO_RAILYARD_IRON_GATE:
                trackExitDoor(mRailyardExitGuids, pGameObject, DATA_ROCKETSPARK_AND_BORKA);
                break;
            case GO_NITROGG_ASSAULT_DOOR:
                trackExitDoor(mNitroggExitGuids, pGameObject, DATA_NITROGG);
                break;
            case GO_TOVRA_SPIKED_GATE:
                mTovraSealGuids.insert(pGameObject->GetNewGUID());
                break;
            default:
                break;
        }
    }

    void OnEncounterStateChange(uint32_t id, uint32_t state) override
    {
        switch (id)
        {
            case DATA_ROCKETSPARK_AND_BORKA:
                updateSeal(mRailyardSealGuids, state);
                if (state == EncounterStates::Performed)
                    openDoors(mRailyardExitGuids);
                else if (state == EncounterStates::Failed)
                    bringBackRailCrew();
                break;
            case DATA_NITROGG:
                if (state == EncounterStates::Performed)
                    openDoors(mNitroggExitGuids);
                break;
            case DATA_TOVRA:
                updateSeal(mTovraSealGuids, state);
                break;
            default:
                break;
        }
    }

    std::vector<Creature*> getRailCrew()
    {
        std::vector<Creature*> crew;
        for (auto const& tracked : mRailCrewGuids)
        {
            if (Creature* pMember = getCreatureByGuid(tracked.second))
                crew.push_back(pMember);
        }

        return crew;
    }

    bool isRailCrewDown(Creature* pExcept)
    {
        for (Creature* pMember : getRailCrew())
        {
            if (pMember != pExcept && pMember->isAlive())
                return false;
        }

        return true;
    }

private:
    void trackExitDoor(std::set<WoWGuid>& doorGuids, GameObject* pDoor, uint32_t encounterId)
    {
        if (doorGuids.insert(pDoor->GetNewGUID()).second)
            setDoorOpen(pDoor, getBossState(encounterId) == EncounterStates::Performed);
    }

    void openDoors(std::set<WoWGuid> const& doorGuids)
    {
        for (WoWGuid const& guid : doorGuids)
        {
            if (GameObject* pDoor = getGameObjectByGuid(guid))
                setDoorOpen(pDoor, true);
        }
    }

    // the encounter decides, the spawn state of a door is not reliable
    void setDoorOpen(GameObject* pDoor, bool open)
    {
        uint8_t const state = open ? GO_STATE_OPEN : GO_STATE_CLOSED;
        if (pDoor->getState() != state)
            pDoor->setState(state);
    }

    // seal is closed while the fight runs
    void updateSeal(std::set<WoWGuid> const& doorGuids, uint32_t state)
    {
        for (WoWGuid const& guid : doorGuids)
        {
            if (GameObject* pDoor = getGameObjectByGuid(guid))
                setDoorOpen(pDoor, state != EncounterStates::InProgress);
        }
    }

    void bringBackRailCrew()
    {
        for (Creature* pMember : getRailCrew())
        {
            if (!pMember->isAlive())
                pMember->despawn(1000, 5000);
        }
    }

    std::map<uint32_t, WoWGuid> mRailCrewGuids;
    std::set<WoWGuid> mRailyardSealGuids;
    std::set<WoWGuid> mRailyardExitGuids;
    std::set<WoWGuid> mNitroggExitGuids;
    std::set<WoWGuid> mTovraSealGuids;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event tick while in combat

class GrimrailDepotCombatAI : public CreatureAIScript
{
public:
    explicit GrimrailDepotCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override { onEngage(); }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        scriptEvents.resetEvents();
        summons.despawnAll();
    }

    void onSummonedCreature(Creature* pSummon) override { summons.summon(pSummon); }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!_isInCombat())
            return;

        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        onCombatTick();

        if (_isCasting())
            return;

        if (uint32_t eventId = scriptEvents.getFinishedEvent())
            onEvent(eventId);
    }

protected:
    virtual void onEngage() {}
    virtual void onCombatTick() {}
    virtual void onEvent(uint32_t /*eventId*/) {}

    Unit* randomPlayer() { return selectUnitTarget(FilterArgs(TargetFilter_Player)); }

    void castOnRandomPlayer(uint32_t spellId)
    {
        if (Unit* pTarget = randomPlayer())
            castSpell(pTarget, spellId);
    }

    Creature* callAdd(uint32_t entry, LocationVector const& position)
    {
        Creature* pAdd = summonCreature(entry, position, CORPSE_DESPAWN);
        if (pAdd != nullptr)
            setZoneWideCombat(pAdd);

        return pAdd;
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame and boss state

class GrimrailDepotBossAI : public GrimrailDepotCombatAI
{
public:
    GrimrailDepotBossAI(Creature* pCreature, uint32_t encounterId) : GrimrailDepotCombatAI(pCreature), mEncounterId(encounterId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mEncounterId, EncounterStates::InProgress);
        }

        GrimrailDepotCombatAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
            pInstance->setBossState(mEncounterId, EncounterStates::Failed);
        }

        GrimrailDepotCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
            if (isLastStanding())
                pInstance->setBossState(mEncounterId, EncounterStates::Performed);
        }

        onDefeat();
    }

protected:
    virtual bool isLastStanding() { return true; }
    virtual void onDefeat() {}

    GrimrailDepotInstanceScript* getGrimrail()
    {
        if (getCreature()->GetMapId() != MAP_GRIMRAIL_DEPOT)
            return nullptr;

        return static_cast<GrimrailDepotInstanceScript*>(getInstanceScript());
    }

    uint32_t mEncounterId;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Rail crew base: Rocketspark and Borka share pull, kill credit and wipe

class GrimrailDepotRailCrewAI : public GrimrailDepotBossAI
{
public:
    explicit GrimrailDepotRailCrewAI(Creature* pCreature) : GrimrailDepotBossAI(pCreature, DATA_ROCKETSPARK_AND_BORKA) {}

    void OnCombatStart(Unit* pTarget) override
    {
        GrimrailDepotBossAI::OnCombatStart(pTarget);

        if (GrimrailDepotInstanceScript* pGrimrail = getGrimrail())
        {
            for (Creature* pMember : pGrimrail->getRailCrew())
            {
                if (pMember != getCreature() && pMember->isAlive() && !pMember->isInCombat())
                    setZoneWideCombat(pMember);
            }
        }
    }

protected:
    bool isLastStanding() override
    {
        GrimrailDepotInstanceScript* pGrimrail = getGrimrail();
        return pGrimrail == nullptr || pGrimrail->isRailCrewDown(getCreature());
    }

    void onDefeat() override
    {
        GrimrailDepotInstanceScript* pGrimrail = getGrimrail();
        if (pGrimrail == nullptr)
            return;

        for (Creature* pMember : pGrimrail->getRailCrew())
        {
            if (pMember != getCreature() && pMember->isAlive() && pMember->GetScript() != nullptr)
                pMember->GetScript()->DoAction(ACTION_RAIL_PARTNER_FALLEN);
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Railmaster Rocketspark

class GrimrailDepotRocketsparkAI : public GrimrailDepotRailCrewAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new GrimrailDepotRocketsparkAI(c); }
    explicit GrimrailDepotRocketsparkAI(Creature* pCreature) : GrimrailDepotRailCrewAI(pCreature) {}

    void DoAction(int32_t action) override
    {
        if (action != ACTION_RAIL_PARTNER_FALLEN || !_isInCombat())
            return;

        // Borka is gone, the rocket launcher takes over
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "Time to go to plan B!");
        castSpellOnSelf(SPELL_ROCKETSPARK_NEW_PLAN);
        scriptEvents.addEvent(EVENT_ROCKETSPARK_NEW_PLAN_MISSILE, 5000);
    }

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_ROCKETSPARK_VX18, 5000);
        scriptEvents.addEvent(EVENT_ROCKETSPARK_X21_BARRAGE, 30000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ROCKETSPARK_VX18:
                castOnRandomPlayer(SPELL_ROCKETSPARK_VX18_TARGET_ELIMINATOR);
                scriptEvents.addEvent(EVENT_ROCKETSPARK_VX18, 12000);
                break;
            case EVENT_ROCKETSPARK_X21_BARRAGE:
                sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Railmaster Rocketspark fires the X21-01A Missile Barrage!");
                castSpellOnSelf(SPELL_ROCKETSPARK_X21_MISSILE_BARRAGE);
                scriptEvents.addEvent(EVENT_ROCKETSPARK_X21_BARRAGE, 40000);
                break;
            case EVENT_ROCKETSPARK_NEW_PLAN_MISSILE:
                castOnRandomPlayer(SPELL_ROCKETSPARK_NEW_PLAN_MISSILE);
                scriptEvents.addEvent(EVENT_ROCKETSPARK_NEW_PLAN_MISSILE, 5000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Borka the Brute

class GrimrailDepotBorkaAI : public GrimrailDepotRailCrewAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new GrimrailDepotBorkaAI(c); }
    explicit GrimrailDepotBorkaAI(Creature* pCreature) : GrimrailDepotRailCrewAI(pCreature) {}

    void DoAction(int32_t action) override
    {
        if (action != ACTION_RAIL_PARTNER_FALLEN || !_isInCombat())
            return;

        sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Borka the Brute flies into a rage!");
        castSpellOnSelf(SPELL_BORKA_UNMANAGED_AGGRESSION);
    }

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_BORKA_SLAM, 8000);
        scriptEvents.addEvent(EVENT_BORKA_MAD_DASH, 20000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_BORKA_SLAM:
                castSpellAOE(SPELL_BORKA_SLAM);
                scriptEvents.addEvent(EVENT_BORKA_SLAM, 8000);
                break;
            case EVENT_BORKA_MAD_DASH:
                if (Unit* pTarget = randomPlayer())
                {
                    sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Borka the Brute begins a Mad Dash!");
                    castSpell(pTarget, SPELL_BORKA_MAD_DASH);
                }
                scriptEvents.addEvent(EVENT_BORKA_MAD_DASH, 20000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Nitrogg Thundertower

class GrimrailDepotNitroggAI : public GrimrailDepotBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new GrimrailDepotNitroggAI(c); }
    explicit GrimrailDepotNitroggAI(Creature* pCreature) : GrimrailDepotBossAI(pCreature, DATA_NITROGG) {}

    void OnCombatStop(Unit* pTarget) override
    {
        setRooted(false);
        GrimrailDepotBossAI::OnCombatStop(pTarget);
    }

protected:
    void onEngage() override
    {
        mOnCannon = false;
        scriptEvents.addEvent(EVENT_NITROGG_GRENADE, 8000);
        scriptEvents.addEvent(EVENT_NITROGG_MORTAR, 15000);
    }

    void onCombatTick() override
    {
        if (mOnCannon || _getHealthPercent() > 80)
            return;

        // second stage: he mans the assault cannon and calls the troops
        mOnCannon = true;
        scriptEvents.resetEvents();
        setRooted(true);
        sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Nitrogg Thundertower mans the Assault Cannon and calls for reinforcements!");

        scriptEvents.addEvent(EVENT_NITROGG_SUPPRESSIVE_FIRE, 8000);
        scriptEvents.addEvent(EVENT_NITROGG_CALL_INFANTRY, 20000);
        scriptEvents.addEvent(EVENT_NITROGG_SLAG_BLAST, 20000);
        scriptEvents.addEvent(EVENT_NITROGG_CALL_GROMKAR, 30000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_NITROGG_GRENADE:
                castOnRandomPlayer(SPELL_NITROGG_BLACKROCK_GRENADE);
                scriptEvents.addEvent(EVENT_NITROGG_GRENADE, 10000);
                break;
            case EVENT_NITROGG_MORTAR:
                castOnRandomPlayer(SPELL_NITROGG_BLACKROCK_MORTAR);
                scriptEvents.addEvent(EVENT_NITROGG_MORTAR, 25000);
                break;
            case EVENT_NITROGG_SUPPRESSIVE_FIRE:
                castOnRandomPlayer(SPELL_NITROGG_SUPPRESSIVE_FIRE);
                scriptEvents.addEvent(EVENT_NITROGG_SUPPRESSIVE_FIRE, 15000);
                break;
            case EVENT_NITROGG_SLAG_BLAST:
                castOnRandomPlayer(SPELL_NITROGG_SLAG_BLAST);
                scriptEvents.addEvent(EVENT_NITROGG_SLAG_BLAST, 30000);
                break;
            case EVENT_NITROGG_CALL_GROMKAR:
                callAdd(NPC_GROMKAR_BOOMER, NitroggGromkarDrop);
                callAdd(NPC_GROMKAR_GRENADIER, NitroggGromkarDrop);
                callAdd(NPC_GROMKAR_GUNNER, NitroggGromkarDrop);
                scriptEvents.addEvent(EVENT_NITROGG_CALL_GROMKAR, 25000);
                break;
            case EVENT_NITROGG_CALL_INFANTRY:
            {
                LocationVector const& drop = NitroggInfantryDrops[Util::getRandomUInt(1)];
                callAdd(NPC_IRON_INFANTRY, drop);
                callAdd(NPC_IRON_INFANTRY, drop);
                scriptEvents.addEvent(EVENT_NITROGG_CALL_INFANTRY, 20000);
                break;
            }
            default:
                break;
        }
    }

private:
    bool mOnCannon = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Nitrogg reinforcements

class GrimrailDepotGromkarBoomerAI : public GrimrailDepotCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new GrimrailDepotGromkarBoomerAI(c); }
    explicit GrimrailDepotGromkarBoomerAI(Creature* pCreature) : GrimrailDepotCombatAI(pCreature) {}

protected:
    void onEngage() override { scriptEvents.addEvent(EVENT_BOOMER_MORTAR, 8000); }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_BOOMER_MORTAR)
            return;

        castOnRandomPlayer(SPELL_BOOMER_BLACKROCK_MORTAR);
        scriptEvents.addEvent(EVENT_BOOMER_MORTAR, 25000);
    }
};

class GrimrailDepotGromkarGrenadierAI : public GrimrailDepotCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new GrimrailDepotGromkarGrenadierAI(c); }
    explicit GrimrailDepotGromkarGrenadierAI(Creature* pCreature) : GrimrailDepotCombatAI(pCreature) {}

protected:
    void onEngage() override { scriptEvents.addEvent(EVENT_GRENADIER_GRENADE, 4000); }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_GRENADIER_GRENADE)
            return;

        castOnRandomPlayer(SPELL_GRENADIER_BLACKROCK_GRENADE);
        scriptEvents.addEvent(EVENT_GRENADIER_GRENADE, 10000);
    }
};

class GrimrailDepotGromkarGunnerAI : public GrimrailDepotCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new GrimrailDepotGromkarGunnerAI(c); }
    explicit GrimrailDepotGromkarGunnerAI(Creature* pCreature) : GrimrailDepotCombatAI(pCreature) {}

protected:
    void onEngage() override { scriptEvents.addEvent(EVENT_GUNNER_SHRAPNEL_BLAST, 1000); }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_GUNNER_SHRAPNEL_BLAST)
            return;

        castOnRandomPlayer(SPELL_GUNNER_SHRAPNEL_BLAST);
        scriptEvents.addEvent(EVENT_GUNNER_SHRAPNEL_BLAST, 4000);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Skylord Tovra

class GrimrailDepotTovraAI : public GrimrailDepotBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new GrimrailDepotTovraAI(c); }
    explicit GrimrailDepotTovraAI(Creature* pCreature) : GrimrailDepotBossAI(pCreature, DATA_TOVRA) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_TOVRA_FREEZING_SNARE, 8000);
        scriptEvents.addEvent(EVENT_TOVRA_SPINNING_SPEAR, 12000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_TOVRA_FREEZING_SNARE:
                castOnRandomPlayer(SPELL_TOVRA_FREEZING_SNARE);
                scriptEvents.addEvent(EVENT_TOVRA_FREEZING_SNARE, 8000);
                break;
            case EVENT_TOVRA_SPINNING_SPEAR:
                castOnRandomPlayer(SPELL_TOVRA_SPINNING_SPEAR);
                scriptEvents.addEvent(EVENT_TOVRA_SPINNING_SPEAR, 12000);
                break;
            default:
                break;
        }
    }
};

void SetupGrimrailDepot(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_GRIMRAIL_DEPOT, &GrimrailDepotInstanceScript::Create);

    mgr->register_creature_script(BOSS_ROCKETSPARK, &GrimrailDepotRocketsparkAI::Create);
    mgr->register_creature_script(BOSS_BORKA, &GrimrailDepotBorkaAI::Create);
    mgr->register_creature_script(BOSS_BORKA_ALT, &GrimrailDepotBorkaAI::Create);
    mgr->register_creature_script(BOSS_NITROGG, &GrimrailDepotNitroggAI::Create);
    mgr->register_creature_script(BOSS_TOVRA, &GrimrailDepotTovraAI::Create);

    mgr->register_creature_script(NPC_GROMKAR_BOOMER, &GrimrailDepotGromkarBoomerAI::Create);
    mgr->register_creature_script(NPC_GROMKAR_GRENADIER, &GrimrailDepotGromkarGrenadierAI::Create);
    mgr->register_creature_script(NPC_GROMKAR_GUNNER, &GrimrailDepotGromkarGunnerAI::Create);
}
