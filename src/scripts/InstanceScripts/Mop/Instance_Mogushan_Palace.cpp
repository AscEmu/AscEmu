/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

// Mogu'shan Palace - three encounters (verified against wowhead's MoP Classic data), hand-
// ported into AscEmu's own CreatureAIScript / InstanceScript framework. See the header
// comment for the Trial of the Kings simplification.

#include "Setup.h"
#include "Instance_Mogushan_Palace.hpp"

#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

enum MogushanPalaceEvents
{
    EVENT_KUAI_RAVAGE = 1,
    EVENT_KUAI_SHOCKWAVE,

    EVENT_MING_WHIRLING_DERVISH,
    EVENT_MING_MAGNETIC_FIELD,
    EVENT_MING_LIGHTNING_BOLT,

    EVENT_HAIYAN_CONFLAGRATE,
    EVENT_HAIYAN_METEOR,
    EVENT_HAIYAN_TRAUMATIC_BLOW,

    EVENT_GEKKAN_SHANK,
    EVENT_GEKKAN_FIRE_BOLT,
    EVENT_GEKKAN_DARK_BOLT,
    EVENT_GEKKAN_HEX_OF_LETHARGY,
    EVENT_GEKKAN_CLEANSING_FLAME,

    EVENT_XIN_GROUND_SLAM,
    EVENT_XIN_CIRCLE_OF_FLAME,
    EVENT_XIN_INCITING_ROAR,

    EVENT_MOGU_DEFIER_CLEAVE,
    EVENT_MOGU_DEFIER_ENRAGE
};

class MogushanPalaceInstanceScript : public InstanceScript
{
public:
    explicit MogushanPalaceInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(MogushanPalaceEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new MogushanPalaceInstanceScript(pMapMgr); }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        if (pGameObject->getEntry() == GO_ANCIENT_GUOLAI_DOOR)
            mGuoLaiDoorGuids.push_back(pGameObject->getGuidLow());
    }

    void OpenGuoLaiDoor()
    {
        for (uint32_t guid : mGuoLaiDoorGuids)
        {
            if (GameObject* pDoor = GetGameObjectByGuid(guid))
                useDoorOrButton(pDoor);
        }
    }

private:
    std::vector<uint32_t> mGuoLaiDoorGuids;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Trial of the Kings - Kuai the Brute / Ming the Cunning / Haiyan the Unstoppable

class TrialOfTheKingsMemberAI : public CreatureAIScript
{
public:
    explicit TrialOfTheKingsMemberAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_TRIAL_OF_THE_KINGS, EncounterStates::InProgress);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_TRIAL_OF_THE_KINGS, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_TRIAL_OF_THE_KINGS, EncounterStates::Performed);
        static_cast<MogushanPalaceInstanceScript*>(getInstanceScript())->OpenGuoLaiDoor();
    }
};

class KuaiTheBruteAI : public TrialOfTheKingsMemberAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new KuaiTheBruteAI(c); }
    explicit KuaiTheBruteAI(Creature* pCreature) : TrialOfTheKingsMemberAI(pCreature) {}

    void OnCombatStart(Unit* pTarget) override
    {
        TrialOfTheKingsMemberAI::OnCombatStart(pTarget);
        scriptEvents.addEvent(EVENT_KUAI_RAVAGE, 8000);
        scriptEvents.addEvent(EVENT_KUAI_SHOCKWAVE, 14000);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_KUAI_RAVAGE:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_KUAI_RAVAGE);
                scriptEvents.addEvent(EVENT_KUAI_RAVAGE, 22000);
                break;
            case EVENT_KUAI_SHOCKWAVE:
                castSpellOnVictim(SPELL_KUAI_SHOCKWAVE);
                scriptEvents.addEvent(EVENT_KUAI_SHOCKWAVE, 18000);
                break;
            default:
                break;
        }
    }
};

class MingTheCunningAI : public TrialOfTheKingsMemberAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MingTheCunningAI(c); }
    explicit MingTheCunningAI(Creature* pCreature) : TrialOfTheKingsMemberAI(pCreature) {}

    void OnCombatStart(Unit* pTarget) override
    {
        TrialOfTheKingsMemberAI::OnCombatStart(pTarget);
        scriptEvents.addEvent(EVENT_MING_WHIRLING_DERVISH, 10000);
        scriptEvents.addEvent(EVENT_MING_MAGNETIC_FIELD, 18000);
        scriptEvents.addEvent(EVENT_MING_LIGHTNING_BOLT, 4000);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_MING_WHIRLING_DERVISH:
                castSpellAOE(SPELL_MING_WHIRLING_DERVISH);
                scriptEvents.addEvent(EVENT_MING_WHIRLING_DERVISH, 24000);
                break;
            case EVENT_MING_MAGNETIC_FIELD:
                castSpellOnSelf(SPELL_MING_MAGNETIC_FIELD);
                scriptEvents.addEvent(EVENT_MING_MAGNETIC_FIELD, 26000);
                break;
            case EVENT_MING_LIGHTNING_BOLT:
                castSpellOnVictim(SPELL_MING_LIGHTNING_BOLT);
                scriptEvents.addEvent(EVENT_MING_LIGHTNING_BOLT, 8000);
                break;
            default:
                break;
        }
    }
};

class HaiyanTheUnstoppableAI : public TrialOfTheKingsMemberAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HaiyanTheUnstoppableAI(c); }
    explicit HaiyanTheUnstoppableAI(Creature* pCreature) : TrialOfTheKingsMemberAI(pCreature) {}

    void OnCombatStart(Unit* pTarget) override
    {
        TrialOfTheKingsMemberAI::OnCombatStart(pTarget);
        scriptEvents.addEvent(EVENT_HAIYAN_CONFLAGRATE, 9000);
        scriptEvents.addEvent(EVENT_HAIYAN_METEOR, 15000);
        scriptEvents.addEvent(EVENT_HAIYAN_TRAUMATIC_BLOW, 6000);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_HAIYAN_CONFLAGRATE:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_HAIYAN_CONFLAGRATE);
                scriptEvents.addEvent(EVENT_HAIYAN_CONFLAGRATE, 22000);
                break;
            case EVENT_HAIYAN_METEOR:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_HAIYAN_METEOR);
                scriptEvents.addEvent(EVENT_HAIYAN_METEOR, 20000);
                break;
            case EVENT_HAIYAN_TRAUMATIC_BLOW:
                castSpellOnVictim(SPELL_HAIYAN_TRAUMATIC_BLOW);
                scriptEvents.addEvent(EVENT_HAIYAN_TRAUMATIC_BLOW, 16000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Gekkan

class GekkanAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new GekkanAI(c); }
    explicit GekkanAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_GEKKAN, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_GEKKAN_SHANK, 6000);
        scriptEvents.addEvent(EVENT_GEKKAN_FIRE_BOLT, 4000);
        scriptEvents.addEvent(EVENT_GEKKAN_DARK_BOLT, 9000);
        scriptEvents.addEvent(EVENT_GEKKAN_HEX_OF_LETHARGY, 16000);
        scriptEvents.addEvent(EVENT_GEKKAN_CLEANSING_FLAME, 22000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_GEKKAN, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_GEKKAN, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_GEKKAN_SHANK:
                castSpellOnVictim(SPELL_GEKKAN_SHANK);
                scriptEvents.addEvent(EVENT_GEKKAN_SHANK, 14000);
                break;
            case EVENT_GEKKAN_FIRE_BOLT:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_GEKKAN_FIRE_BOLT);
                scriptEvents.addEvent(EVENT_GEKKAN_FIRE_BOLT, 10000);
                break;
            case EVENT_GEKKAN_DARK_BOLT:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_GEKKAN_DARK_BOLT);
                scriptEvents.addEvent(EVENT_GEKKAN_DARK_BOLT, 12000);
                break;
            case EVENT_GEKKAN_HEX_OF_LETHARGY:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_GEKKAN_HEX_OF_LETHARGY);
                scriptEvents.addEvent(EVENT_GEKKAN_HEX_OF_LETHARGY, 24000);
                break;
            case EVENT_GEKKAN_CLEANSING_FLAME:
                castSpellAOE(SPELL_GEKKAN_CLEANSING_FLAME);
                scriptEvents.addEvent(EVENT_GEKKAN_CLEANSING_FLAME, 26000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Xin the Weaponmaster

class XinTheWeaponmasterAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new XinTheWeaponmasterAI(c); }
    explicit XinTheWeaponmasterAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "The clans will unite under my rule, or they will fall!");
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_XIN_THE_WEAPONMASTER, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_XIN_GROUND_SLAM, 9000);
        scriptEvents.addEvent(EVENT_XIN_CIRCLE_OF_FLAME, 15000);
        scriptEvents.addEvent(EVENT_XIN_INCITING_ROAR, 22000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_XIN_THE_WEAPONMASTER, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "The... king... falls...");
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_XIN_THE_WEAPONMASTER, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_XIN_GROUND_SLAM:
                castSpellAOE(SPELL_XIN_GROUND_SLAM);
                scriptEvents.addEvent(EVENT_XIN_GROUND_SLAM, 20000);
                break;
            case EVENT_XIN_CIRCLE_OF_FLAME:
                castSpellOnSelf(SPELL_XIN_CIRCLE_OF_FLAME);
                scriptEvents.addEvent(EVENT_XIN_CIRCLE_OF_FLAME, 24000);
                break;
            case EVENT_XIN_INCITING_ROAR:
                castSpellOnSelf(SPELL_XIN_INCITING_ROAR);
                scriptEvents.addEvent(EVENT_XIN_INCITING_ROAR, 30000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Trash - Mogu Defier

class MoguDefierAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MoguDefierAI(c); }
    explicit MoguDefierAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_MOGU_DEFIER_CLEAVE, 6000);
        scriptEvents.addEvent(EVENT_MOGU_DEFIER_ENRAGE, 16000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_MOGU_DEFIER_CLEAVE:
                castSpellOnVictim(SPELL_MOGU_DEFIER_CLEAVE);
                scriptEvents.addEvent(EVENT_MOGU_DEFIER_CLEAVE, 12000);
                break;
            case EVENT_MOGU_DEFIER_ENRAGE:
                castSpellOnSelf(SPELL_MOGU_DEFIER_ENRAGE);
                scriptEvents.addEvent(EVENT_MOGU_DEFIER_ENRAGE, 26000);
                break;
            default:
                break;
        }
    }
};

void SetupMogushanPalace(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_MOGUSHAN_PALACE, &MogushanPalaceInstanceScript::Create);

    mgr->register_creature_script(BOSS_KUAI_THE_BRUTE, &KuaiTheBruteAI::Create);
    mgr->register_creature_script(BOSS_MING_THE_CUNNING, &MingTheCunningAI::Create);
    mgr->register_creature_script(BOSS_HAIYAN_THE_UNSTOPPABLE, &HaiyanTheUnstoppableAI::Create);
    mgr->register_creature_script(BOSS_GEKKAN, &GekkanAI::Create);
    mgr->register_creature_script(BOSS_XIN_THE_WEAPONMASTER, &XinTheWeaponmasterAI::Create);

    mgr->register_creature_script(NPC_MOGU_DEFIER, &MoguDefierAI::Create);
}
