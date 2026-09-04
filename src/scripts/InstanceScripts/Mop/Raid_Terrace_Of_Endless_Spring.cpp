/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

// Terrace of Endless Spring - four bosses (verified against wowhead's MoP Classic data where
// reachable), hand-ported into AscEmu's own CreatureAIScript / InstanceScript framework.

#include "Setup.h"
#include "Raid_Terrace_Of_Endless_Spring.hpp"

#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

enum TerraceOfEndlessSpringEvents
{
    EVENT_KAOLAN_TOUCH_OF_SHA = 1,
    EVENT_KAOLAN_DEFILED_GROUND,
    EVENT_KAOLAN_OVERWHELMING_CORRUPTION,

    EVENT_TSULONG_DREAD_SHADOWS,
    EVENT_TSULONG_SHADOW_BREATH,
    EVENT_TSULONG_SUNBEAM,
    EVENT_TSULONG_SUN_BREATH,

    EVENT_LEISHI_SPRAY,
    EVENT_LEISHI_SCARY_FOG,

    EVENT_SHAOFFEAR_BREATH_OF_FEAR,
    EVENT_SHAOFFEAR_THRASH,
    EVENT_SHAOFFEAR_OMINOUS_CACKLE,
    EVENT_SHAOFFEAR_DEATH_BLOSSOM,

    EVENT_ANIMATED_PROTECTOR_PROTECT,
    EVENT_ANIMATED_PROTECTOR_CLEAVE,

    EVENT_CORRUPTED_PROTECTOR_DISPERSE,
    EVENT_CORRUPTED_PROTECTOR_CLEAVE
};

class TerraceOfEndlessSpringInstanceScript : public InstanceScript
{
public:
    explicit TerraceOfEndlessSpringInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(TerraceOfEndlessSpringEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new TerraceOfEndlessSpringInstanceScript(pMapMgr); }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        if (pGameObject->getEntry() == GO_CELESTIAL_DOOR)
            mCelestialDoorGuids.push_back(pGameObject->getGuidLow());
    }

    void OpenCelestialDoor()
    {
        for (uint32_t guid : mCelestialDoorGuids)
        {
            if (GameObject* pDoor = GetGameObjectByGuid(guid))
                useDoorOrButton(pDoor);
        }
    }

private:
    std::vector<uint32_t> mCelestialDoorGuids;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Protector Kaolan

class ProtectorKaolanAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ProtectorKaolanAI(c); }
    explicit ProtectorKaolanAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_PROTECTOR_KAOLAN, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_KAOLAN_TOUCH_OF_SHA, 8000);
        scriptEvents.addEvent(EVENT_KAOLAN_DEFILED_GROUND, 14000);
        scriptEvents.addEvent(EVENT_KAOLAN_OVERWHELMING_CORRUPTION, 22000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_PROTECTOR_KAOLAN, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_PROTECTOR_KAOLAN, EncounterStates::Performed);
        static_cast<TerraceOfEndlessSpringInstanceScript*>(getInstanceScript())->OpenCelestialDoor();
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_KAOLAN_TOUCH_OF_SHA:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_KAOLAN_TOUCH_OF_SHA);
                scriptEvents.addEvent(EVENT_KAOLAN_TOUCH_OF_SHA, 18000);
                break;
            case EVENT_KAOLAN_DEFILED_GROUND:
                castSpellAOE(SPELL_KAOLAN_DEFILED_GROUND);
                scriptEvents.addEvent(EVENT_KAOLAN_DEFILED_GROUND, 24000);
                break;
            case EVENT_KAOLAN_OVERWHELMING_CORRUPTION:
                castSpellOnVictim(SPELL_KAOLAN_OVERWHELMING_CORRUPTION);
                scriptEvents.addEvent(EVENT_KAOLAN_OVERWHELMING_CORRUPTION, 26000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Tsulong

class TsulongAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TsulongAI(c); }
    explicit TsulongAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "Night falls upon you!");
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_TSULONG, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_TSULONG_DREAD_SHADOWS, 6000);
        scriptEvents.addEvent(EVENT_TSULONG_SHADOW_BREATH, 12000);
        scriptEvents.addEvent(EVENT_TSULONG_SUNBEAM, 18000);
        scriptEvents.addEvent(EVENT_TSULONG_SUN_BREATH, 24000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_TSULONG, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_TSULONG, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_TSULONG_DREAD_SHADOWS:
                castSpellAOE(SPELL_TSULONG_DREAD_SHADOWS);
                scriptEvents.addEvent(EVENT_TSULONG_DREAD_SHADOWS, 26000);
                break;
            case EVENT_TSULONG_SHADOW_BREATH:
                castSpellOnVictim(SPELL_TSULONG_SHADOW_BREATH);
                scriptEvents.addEvent(EVENT_TSULONG_SHADOW_BREATH, 20000);
                break;
            case EVENT_TSULONG_SUNBEAM:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_TSULONG_SUNBEAM);
                scriptEvents.addEvent(EVENT_TSULONG_SUNBEAM, 22000);
                break;
            case EVENT_TSULONG_SUN_BREATH:
                castSpellOnVictim(SPELL_TSULONG_SUN_BREATH);
                scriptEvents.addEvent(EVENT_TSULONG_SUN_BREATH, 24000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Lei Shi

class LeiShiAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new LeiShiAI(c); }
    explicit LeiShiAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_LEI_SHI, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_LEISHI_SPRAY, 7000);
        scriptEvents.addEvent(EVENT_LEISHI_SCARY_FOG, 16000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_LEI_SHI, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_LEI_SHI, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_LEISHI_SPRAY:
                castSpellAOE(SPELL_LEISHI_SPRAY);
                scriptEvents.addEvent(EVENT_LEISHI_SPRAY, 20000);
                break;
            case EVENT_LEISHI_SCARY_FOG:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_LEISHI_SCARY_FOG);
                scriptEvents.addEvent(EVENT_LEISHI_SCARY_FOG, 24000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Sha of Fear

class ShaOfFearAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ShaOfFearAI(c); }
    explicit ShaOfFearAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "Your fear... feeds me!");
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_SHA_OF_FEAR, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_SHAOFFEAR_BREATH_OF_FEAR, 8000);
        scriptEvents.addEvent(EVENT_SHAOFFEAR_THRASH, 5000);
        scriptEvents.addEvent(EVENT_SHAOFFEAR_OMINOUS_CACKLE, 16000);
        scriptEvents.addEvent(EVENT_SHAOFFEAR_DEATH_BLOSSOM, 24000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_SHA_OF_FEAR, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "This is... not... the end...");
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_SHA_OF_FEAR, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_SHAOFFEAR_BREATH_OF_FEAR:
                castSpellAOE(SPELL_SHAOFFEAR_BREATH_OF_FEAR);
                scriptEvents.addEvent(EVENT_SHAOFFEAR_BREATH_OF_FEAR, 26000);
                break;
            case EVENT_SHAOFFEAR_THRASH:
                castSpellOnVictim(SPELL_SHAOFFEAR_THRASH);
                scriptEvents.addEvent(EVENT_SHAOFFEAR_THRASH, 14000);
                break;
            case EVENT_SHAOFFEAR_OMINOUS_CACKLE:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_SHAOFFEAR_OMINOUS_CACKLE);
                scriptEvents.addEvent(EVENT_SHAOFFEAR_OMINOUS_CACKLE, 22000);
                break;
            case EVENT_SHAOFFEAR_DEATH_BLOSSOM:
                castSpellOnVictim(SPELL_SHAOFFEAR_DEATH_BLOSSOM);
                scriptEvents.addEvent(EVENT_SHAOFFEAR_DEATH_BLOSSOM, 28000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Trash - Animated Protector

class AnimatedProtectorAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AnimatedProtectorAI(c); }
    explicit AnimatedProtectorAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_ANIMATED_PROTECTOR_PROTECT, 6000);
        scriptEvents.addEvent(EVENT_ANIMATED_PROTECTOR_CLEAVE, 10000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_ANIMATED_PROTECTOR_PROTECT:
                castSpellOnSelf(SPELL_ANIMATED_PROTECTOR_PROTECT);
                scriptEvents.addEvent(EVENT_ANIMATED_PROTECTOR_PROTECT, 22000);
                break;
            case EVENT_ANIMATED_PROTECTOR_CLEAVE:
                castSpellOnVictim(SPELL_ANIMATED_PROTECTOR_CLEAVE);
                scriptEvents.addEvent(EVENT_ANIMATED_PROTECTOR_CLEAVE, 12000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Trash - Corrupted Protector

class CorruptedProtectorAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CorruptedProtectorAI(c); }
    explicit CorruptedProtectorAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_CORRUPTED_PROTECTOR_DISPERSE, 8000);
        scriptEvents.addEvent(EVENT_CORRUPTED_PROTECTOR_CLEAVE, 5000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_CORRUPTED_PROTECTOR_DISPERSE:
                castSpellOnSelf(SPELL_CORRUPTED_PROTECTOR_DISPERSE);
                scriptEvents.addEvent(EVENT_CORRUPTED_PROTECTOR_DISPERSE, 24000);
                break;
            case EVENT_CORRUPTED_PROTECTOR_CLEAVE:
                castSpellOnVictim(SPELL_CORRUPTED_PROTECTOR_CLEAVE);
                scriptEvents.addEvent(EVENT_CORRUPTED_PROTECTOR_CLEAVE, 12000);
                break;
            default:
                break;
        }
    }
};

void SetupTerraceOfEndlessSpring(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_TERRACE_OF_ENDLESS_SPRING, &TerraceOfEndlessSpringInstanceScript::Create);

    mgr->register_creature_script(BOSS_PROTECTOR_KAOLAN, &ProtectorKaolanAI::Create);
    mgr->register_creature_script(BOSS_TSULONG, &TsulongAI::Create);
    mgr->register_creature_script(BOSS_LEI_SHI, &LeiShiAI::Create);
    mgr->register_creature_script(BOSS_SHA_OF_FEAR, &ShaOfFearAI::Create);

    mgr->register_creature_script(NPC_ANIMATED_PROTECTOR, &AnimatedProtectorAI::Create);
    mgr->register_creature_script(NPC_CORRUPTED_PROTECTOR, &CorruptedProtectorAI::Create);
}
