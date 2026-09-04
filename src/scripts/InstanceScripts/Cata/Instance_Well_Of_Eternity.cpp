/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Well_Of_Eternity.hpp"

#include "Objects/GameObject.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

class WellOfEternityInstanceScript : public InstanceScript
{
public:
    explicit WellOfEternityInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(WellOfEternityEncounterCount);
        mLargeFirewallDoorGuid = 0;
        mSmallFirewallDoorGuid = 0;
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new WellOfEternityInstanceScript(pMapMgr); }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        switch (pGameObject->getEntry())
        {
            case GO_LARGE_FIREWALL_DOOR:
                mLargeFirewallDoorGuid = pGameObject->getGuidLow();
                break;
            case GO_SMALL_FIREWALL_DOOR:
                mSmallFirewallDoorGuid = pGameObject->getGuidLow();
                break;
            default:
                break;
        }
    }

    void OpenPerotharnDoors()
    {
        if (GameObject* pDoor1 = GetGameObjectByGuid(mLargeFirewallDoorGuid))
            useDoorOrButton(pDoor1);
        if (GameObject* pDoor2 = GetGameObjectByGuid(mSmallFirewallDoorGuid))
            useDoorOrButton(pDoor2);
    }

private:
    uint32_t mLargeFirewallDoorGuid;
    uint32_t mSmallFirewallDoorGuid;
};

enum WellOfEternityEvents
{
    EVENT_DOOMGUARD_CLEAVE = 1,
    EVENT_DOOMGUARD_MORTAL_STRIKE,

    EVENT_DREADLORD_SHADOW_BOLT,
    EVENT_DREADLORD_FEAR
};

class PerotharnAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new PerotharnAI(c); }
    explicit PerotharnAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_PEROTHARN, EncounterStates::InProgress);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_PEROTHARN, EncounterStates::Failed);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_PEROTHARN, EncounterStates::Performed);
        static_cast<WellOfEternityInstanceScript*>(getInstanceScript())->OpenPerotharnDoors();
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Trash

class DoomguardAnnihilatorAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DoomguardAnnihilatorAI(c); }
    explicit DoomguardAnnihilatorAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_DOOMGUARD_CLEAVE, 4000);
        scriptEvents.addEvent(EVENT_DOOMGUARD_MORTAL_STRIKE, 10000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        scriptEvents.resetEvents();
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_DOOMGUARD_CLEAVE:
                castSpellOnVictim(SPELL_DOOMGUARD_CLEAVE);
                scriptEvents.addEvent(EVENT_DOOMGUARD_CLEAVE, 8000);
                break;
            case EVENT_DOOMGUARD_MORTAL_STRIKE:
                castSpellOnVictim(SPELL_DOOMGUARD_MORTAL_STRIKE);
                scriptEvents.addEvent(EVENT_DOOMGUARD_MORTAL_STRIKE, 16000);
                break;
            default:
                break;
        }
    }
};

class DreadlordDefenderAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DreadlordDefenderAI(c); }
    explicit DreadlordDefenderAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_DREADLORD_SHADOW_BOLT, 2500);
        scriptEvents.addEvent(EVENT_DREADLORD_FEAR, 12000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        scriptEvents.resetEvents();
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_DREADLORD_SHADOW_BOLT:
                castSpellOnVictim(SPELL_DREADLORD_SHADOW_BOLT);
                scriptEvents.addEvent(EVENT_DREADLORD_SHADOW_BOLT, 9000);
                break;
            case EVENT_DREADLORD_FEAR:
                castSpellOnVictim(SPELL_DREADLORD_FEAR);
                scriptEvents.addEvent(EVENT_DREADLORD_FEAR, 22000);
                break;
            default:
                break;
        }
    }
};

void SetupWellOfEternity(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_WELL_OF_ETERNITY, &WellOfEternityInstanceScript::Create);

    mgr->register_creature_script(BOSS_PEROTHARN, &PerotharnAI::Create);

    mgr->register_creature_script(NPC_DOOMGUARD_ANNIHILATOR, &DoomguardAnnihilatorAI::Create);
    mgr->register_creature_script(NPC_DREADLORD_DEFENDER, &DreadlordDefenderAI::Create);
}
