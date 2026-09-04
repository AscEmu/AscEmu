/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

// Throne of the Tides - four bosses (verified against wowhead), hand-ported into AscEmu's own
// CreatureAIScript / InstanceScript framework.
//
// Simplifications versus the original fight design:
//  - Mindbender Gursha carries no hand-scripted rotation in the reference implementation
//    either (it relies on database-driven spellcasting) - kept the same way here.
//  - Lady Naz'jar's "waterspout shield" invulnerability add-phase (kill 3 adds to break it)
//    is dropped; she runs her core rotation (Geyser / Fungal Spores / Shock Blast) throughout.
//  - Ozumat is, in the original design, an elaborate gossip-triggered escort/defense event
//    spanning several rooms with wave after wave of adds - not a stand-up fight. It's ported
//    here as a straightforward boss encounter using its own listed abilities instead.

#include "Setup.h"
#include "Instance_Throne_Of_Tides.hpp"

#include "Objects/GameObject.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

enum ThroneOfTidesEvents
{
    EVENT_NAZJAR_SUMMON_GEYSER = 1,
    EVENT_NAZJAR_FUNGAL_SPORES,
    EVENT_NAZJAR_SHOCK_BLAST,

    EVENT_ULTHOK_SQUEEZE,
    EVENT_ULTHOK_ENRAGE,
    EVENT_ULTHOK_CURSE_OF_FATIGUE,
    EVENT_ULTHOK_DARK_FISSURE,

    EVENT_OZUMAT_GLOBE_IMPACT,
    EVENT_OZUMAT_BLIGHT,

    EVENT_SENTINEL_RANDOM,
    EVENT_SENTINEL_VICTIM,

    EVENT_SPIRITMENDER_VICTIM,
    EVENT_SPIRITMENDER_RANDOM,

    EVENT_TAINTED_SENTRY_SWELL,

    EVENT_UNSTABLE_CORRUPTION_GROWTH,

    EVENT_GURSHA_EARTHFURY,
    EVENT_GURSHA_STORMFLURRY_TOTEM,
    EVENT_GURSHA_FLAME_SHOCK,
    EVENT_GURSHA_TERRIFYING_VISION,
    EVENT_GURSHA_MIND_ROT
};

class ThroneOfTidesInstanceScript : public InstanceScript
{
public:
    explicit ThroneOfTidesInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(ThroneOfTidesEncounterCount);
        mUlthokDoor1Guid = 0;
        mUlthokDoor2Guid = 0;
        mOzumatDoorGuid = 0;
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new ThroneOfTidesInstanceScript(pMapMgr); }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        switch (pGameObject->getEntry())
        {
            case GO_ABYSSAL_MAW_DOOR_1:
                mUlthokDoor1Guid = pGameObject->getGuidLow();
                break;
            case GO_ABYSSAL_MAW_DOOR_2:
                mUlthokDoor2Guid = pGameObject->getGuidLow();
                break;
            case GO_ABYSSAL_MAW_DOOR_4:
                mOzumatDoorGuid = pGameObject->getGuidLow();
                break;
            default:
                break;
        }
    }

    void OpenUlthokDoors()
    {
        if (GameObject* pDoor1 = GetGameObjectByGuid(mUlthokDoor1Guid))
            useDoorOrButton(pDoor1);
        if (GameObject* pDoor2 = GetGameObjectByGuid(mUlthokDoor2Guid))
            useDoorOrButton(pDoor2);
    }

    void OpenOzumatDoor()
    {
        if (GameObject* pDoor = GetGameObjectByGuid(mOzumatDoorGuid))
            useDoorOrButton(pDoor);
    }

private:
    uint32_t mUlthokDoor1Guid;
    uint32_t mUlthokDoor2Guid;
    uint32_t mOzumatDoorGuid;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Lady Naz'jar

class LadyNazjarAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new LadyNazjarAI(c); }
    explicit LadyNazjarAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 18886, "You have interfered with our plans for the last time, mortals!");
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_LADY_NAZJAR, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_NAZJAR_SUMMON_GEYSER, 11000);
        scriptEvents.addEvent(EVENT_NAZJAR_FUNGAL_SPORES, 13000);
        scriptEvents.addEvent(EVENT_NAZJAR_SHOCK_BLAST, 13000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_LADY_NAZJAR, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    // Her real reference death line calls out to Commander Ulthok ("Ulthok, stop them...",
    // sound 18889) - kept here rather than moved to him since he has no matching "answer" line
    // in the reference and this dungeon doesn't model a scripted hand-off between the two.
    void OnDied(Unit* /*pKiller*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 18889, "Ulthok, stop them...");
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_LADY_NAZJAR, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_NAZJAR_SUMMON_GEYSER:
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 18892, "Take arms, minions! Rise from the icy depths!");
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_NAZJAR_SUMMON_GEYSER);
                scriptEvents.addEvent(EVENT_NAZJAR_SUMMON_GEYSER, 13000);
                break;
            case EVENT_NAZJAR_FUNGAL_SPORES:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_NAZJAR_FUNGAL_SPORES);
                scriptEvents.addEvent(EVENT_NAZJAR_FUNGAL_SPORES, 16000);
                break;
            case EVENT_NAZJAR_SHOCK_BLAST:
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 18893, "Destroy these intruders! Leave them for the great dark beyond!");
                castSpellOnVictim(getRaidModeValue(SPELL_NAZJAR_SHOCK_BLAST_10, SPELL_NAZJAR_SHOCK_BLAST_25, SPELL_NAZJAR_SHOCK_BLAST_10, SPELL_NAZJAR_SHOCK_BLAST_25));
                scriptEvents.addEvent(EVENT_NAZJAR_SHOCK_BLAST, 15000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Commander Ulthok

class CommanderUlthokAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CommanderUlthokAI(c); }
    explicit CommanderUlthokAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 18543, "Iilth vwah, uhn'agth fhssh za.");
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_COMMANDER_ULTHOK, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_ULTHOK_SQUEEZE, 13200);
        scriptEvents.addEvent(EVENT_ULTHOK_ENRAGE, 32000);
        scriptEvents.addEvent(EVENT_ULTHOK_CURSE_OF_FATIGUE, 13000);
        scriptEvents.addEvent(EVENT_ULTHOK_DARK_FISSURE, 7000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_COMMANDER_ULTHOK, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_COMMANDER_ULTHOK, EncounterStates::Performed);
        static_cast<ThroneOfTidesInstanceScript*>(getInstanceScript())->OpenUlthokDoors();
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_ULTHOK_SQUEEZE:
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 18543, "Where one falls, many shall take its place...");
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, getRaidModeValue(SPELL_ULTHOK_SQUEEZE_10, SPELL_ULTHOK_SQUEEZE_25, SPELL_ULTHOK_SQUEEZE_10, SPELL_ULTHOK_SQUEEZE_25));
                scriptEvents.addEvent(EVENT_ULTHOK_SQUEEZE, 20000);
                break;
            case EVENT_ULTHOK_ENRAGE:
                castSpellOnSelf(SPELL_ULTHOK_ENRAGE);
                scriptEvents.addEvent(EVENT_ULTHOK_ENRAGE, 32000);
                break;
            case EVENT_ULTHOK_CURSE_OF_FATIGUE:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_ULTHOK_CURSE_OF_FATIGUE);
                scriptEvents.addEvent(EVENT_ULTHOK_CURSE_OF_FATIGUE, 18000);
                break;
            case EVENT_ULTHOK_DARK_FISSURE:
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 18542, "Ywaq maq oou.");
                castSpellOnSelf(SPELL_ULTHOK_DARK_FISSURE);
                scriptEvents.addEvent(EVENT_ULTHOK_DARK_FISSURE, 20000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Mindbender Gursha - see the MindbenderGurshaSpells comment in the header for why this
// combines both fight-halves' abilities into one continuous rotation.

class MindbenderGurshaAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MindbenderGurshaAI(c); }
    explicit MindbenderGurshaAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 18860, "A new host must be found.");
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_MINDBENDER_GURSHA, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_GURSHA_EARTHFURY, 6000);
        scriptEvents.addEvent(EVENT_GURSHA_STORMFLURRY_TOTEM, 10000);
        scriptEvents.addEvent(EVENT_GURSHA_FLAME_SHOCK, 4000);
        scriptEvents.addEvent(EVENT_GURSHA_TERRIFYING_VISION, 20000);
        scriptEvents.addEvent(EVENT_GURSHA_MIND_ROT, 14000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_MINDBENDER_GURSHA, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_MINDBENDER_GURSHA, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_GURSHA_EARTHFURY:
                castSpellAOE(SPELL_GURSHA_EARTHFURY);
                scriptEvents.addEvent(EVENT_GURSHA_EARTHFURY, 16000);
                break;
            case EVENT_GURSHA_STORMFLURRY_TOTEM:
                castSpellOnSelf(SPELL_GURSHA_STORMFLURRY_TOTEM);
                scriptEvents.addEvent(EVENT_GURSHA_STORMFLURRY_TOTEM, 30000);
                break;
            case EVENT_GURSHA_FLAME_SHOCK:
                castSpellOnVictim(SPELL_GURSHA_FLAME_SHOCK);
                scriptEvents.addEvent(EVENT_GURSHA_FLAME_SHOCK, 15000);
                break;
            case EVENT_GURSHA_TERRIFYING_VISION:
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 18867, "Is. This. Reality?");
                castSpellAOE(SPELL_GURSHA_TERRIFYING_VISION);
                scriptEvents.addEvent(EVENT_GURSHA_TERRIFYING_VISION, 30000);
                break;
            case EVENT_GURSHA_MIND_ROT:
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 18863, "They are outside the cycle...");
                castSpellAOE(SPELL_GURSHA_MIND_ROT);
                scriptEvents.addEvent(EVENT_GURSHA_MIND_ROT, 18000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Ozumat (simplified - see file header comment)

class OzumatAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new OzumatAI(c); }
    explicit OzumatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_OZUMAT, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_OZUMAT_GLOBE_IMPACT, 2500);
        scriptEvents.addEvent(EVENT_OZUMAT_BLIGHT, 3500);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_OZUMAT, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_OZUMAT, EncounterStates::Performed);
        static_cast<ThroneOfTidesInstanceScript*>(getInstanceScript())->OpenOzumatDoor();
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_OZUMAT_GLOBE_IMPACT:
                castSpellOnSelf(SPELL_OZUMAT_GLOBE_IMPACT_PERIODIC);
                scriptEvents.addEvent(EVENT_OZUMAT_GLOBE_IMPACT, 20000);
                break;
            case EVENT_OZUMAT_BLIGHT:
                castSpellOnSelf(SPELL_OZUMAT_BLIGHT_OF_OZUMAT);
                scriptEvents.addEvent(EVENT_OZUMAT_BLIGHT, 25000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Trash

class NazjarSentinelAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NazjarSentinelAI(c); }
    explicit NazjarSentinelAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_SENTINEL_RANDOM, 4000);
        scriptEvents.addEvent(EVENT_SENTINEL_VICTIM, 13000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_SENTINEL_RANDOM:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_SENTINEL_RANDOM);
                scriptEvents.addEvent(EVENT_SENTINEL_RANDOM, 5000);
                break;
            case EVENT_SENTINEL_VICTIM:
                castSpellOnVictim(SPELL_SENTINEL_VICTIM);
                scriptEvents.addEvent(EVENT_SENTINEL_VICTIM, 13500);
                break;
            default:
                break;
        }
    }
};

class NazjarInvaderAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NazjarInvaderAI(c); }
    explicit NazjarInvaderAI(Creature* pCreature) : CreatureAIScript(pCreature), mLowHpCast(false) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_SPIRITMENDER_VICTIM, 5000);
        mLowHpCast = false;
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (!mLowHpCast && _getHealthPercent() <= 50)
        {
            castSpellOnSelf(SPELL_INVADER_LOW_HP);
            mLowHpCast = true;
        }

        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_SPIRITMENDER_VICTIM)
        {
            castSpellOnVictim(SPELL_INVADER_VICTIM);
            scriptEvents.addEvent(EVENT_SPIRITMENDER_VICTIM, 6000);
        }
    }

private:
    bool mLowHpCast;
};

class NazjarSpiritmenderAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NazjarSpiritmenderAI(c); }
    explicit NazjarSpiritmenderAI(Creature* pCreature) : CreatureAIScript(pCreature), mLowHpCast(false) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_SPIRITMENDER_VICTIM, 1500);
        scriptEvents.addEvent(EVENT_SPIRITMENDER_RANDOM, 22000);
        mLowHpCast = false;
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (!mLowHpCast && _getHealthPercent() <= 50)
        {
            castSpellOnSelf(SPELL_SPIRITMENDER_LOW_HP);
            mLowHpCast = true;
        }

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_SPIRITMENDER_VICTIM:
                castSpellOnVictim(SPELL_SPIRITMENDER_VICTIM);
                scriptEvents.addEvent(EVENT_SPIRITMENDER_VICTIM, 2400);
                break;
            case EVENT_SPIRITMENDER_RANDOM:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_SPIRITMENDER_RANDOM);
                scriptEvents.addEvent(EVENT_SPIRITMENDER_RANDOM, 14000);
                break;
            default:
                break;
        }
    }

private:
    bool mLowHpCast;
};

class TaintedSentryAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TaintedSentryAI(c); }
    explicit TaintedSentryAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_TAINTED_SENTRY_SWELL, 18000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void OnDied(Unit* /*pKiller*/) override
    {
        castSpellOnSelf(SPELL_TAINTED_SENTRY_DEATH, true);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_TAINTED_SENTRY_SWELL)
        {
            castSpellOnSelf(SPELL_TAINTED_SENTRY_SWELL);
            scriptEvents.addEvent(EVENT_TAINTED_SENTRY_SWELL, 19000);
        }
    }
};

// Unstable Corruption is passive and grows on its own timer regardless of combat state, then
// explodes on death.
class UnstableCorruptionAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new UnstableCorruptionAI(c); }
    explicit UnstableCorruptionAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        setReactState(REACT_PASSIVE);
        mGrowthTimerId = _addTimer(2000);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        castSpellOnSelf(SPELL_UNSTABLE_CORRUPTION_DEATH, true);
    }

    void AIUpdate(unsigned long /*time_passed*/) override
    {
        if (mGrowthTimerId != 0 && _isTimerFinished(mGrowthTimerId))
        {
            castSpellOnSelf(SPELL_UNSTABLE_CORRUPTION_GROWTH, true);
            _resetTimer(mGrowthTimerId, 2000);
        }
    }

private:
    uint32_t mGrowthTimerId;
};

void SetupThroneOfTides(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_THRONE_OF_THE_TIDES, &ThroneOfTidesInstanceScript::Create);

    mgr->register_creature_script(BOSS_LADY_NAZJAR, &LadyNazjarAI::Create);
    mgr->register_creature_script(BOSS_COMMANDER_ULTHOK, &CommanderUlthokAI::Create);
    mgr->register_creature_script(BOSS_MINDBENDER_GURSHA, &MindbenderGurshaAI::Create);
    mgr->register_creature_script(BOSS_OZUMAT, &OzumatAI::Create);

    mgr->register_creature_script(NPC_NAZJAR_SENTINEL, &NazjarSentinelAI::Create);
    mgr->register_creature_script(NPC_NAZJAR_INVADER, &NazjarInvaderAI::Create);
    mgr->register_creature_script(NPC_NAZJAR_SPIRITMENDER, &NazjarSpiritmenderAI::Create);
    mgr->register_creature_script(NPC_TAINTED_SENTRY, &TaintedSentryAI::Create);
    mgr->register_creature_script(NPC_UNSTABLE_CORRUPTION, &UnstableCorruptionAI::Create);
}
