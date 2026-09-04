/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

// Blackwing Descent - six bosses (verified against wowhead), hand-ported into AscEmu's own
// CreatureAIScript / InstanceScript framework.
//
// Every fight here carries an elaborate signature mechanic that isn't ported: Magmaw's
// vehicle head-exposure grapple, the Omnotron robots' shared-health/recharge cycle,
// Chimaeron's Feud/Massacre/sleep-phase cycle, Atramedes' sound-meter stealth mechanic,
// Maloriak's potion-throw phase transforms, and Nefarian's End's full Onyxia-then-Nefarian
// platform sequence. Each boss (or, for Omnotron and the Nefarian/Onyxia pair, each
// participant) instead runs a simplified but functional core rotation from its real kit.
// The Athenaeum Door (Chimaeron -> Atramedes/Maloriak wing) and Inner Chamber Door
// (Atramedes + Maloriak -> Nefarian's End) are tracked; Magmaw and Omnotron have no gating
// door objects in the reference data.

#include "Setup.h"
#include "Raid_Blackwing_Descent.hpp"

#include "Objects/GameObject.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

enum BlackwingDescentEvents
{
    EVENT_MAGMAW_LAVA_SPEW = 1,
    EVENT_MAGMAW_MANGLE,
    EVENT_MAGMAW_PILLAR_OF_FLAME,

    EVENT_ELECTRON_STATIC_SHOCK,
    EVENT_MAGMATRON_INCINERATION,
    EVENT_ARCANOTRON_POWER_GENERATOR,
    EVENT_TOXITRON_CHEMICAL_BOMB,

    EVENT_CHIMAERON_CAUSTIC_SLIME,
    EVENT_CHIMAERON_BREAK,
    EVENT_CHIMAERON_MASSACRE,

    EVENT_ATRAMEDES_ROARING_BREATH,
    EVENT_ATRAMEDES_SEARING_FLAME,
    EVENT_ATRAMEDES_SONIC_BREATH,

    EVENT_MALORIAK_ARCANE_STORM,
    EVENT_MALORIAK_THROW_BOTTLE,

    EVENT_NEFARIAN_SHADOWFLAME_BARRAGE,
    EVENT_NEFARIAN_ELECTRICAL_CHARGE,
    EVENT_NEFARIAN_BERSERK,

    EVENT_ONYXIA_ELECTRICAL_CHARGE,
    EVENT_ONYXIA_TAIL_LASH,
    EVENT_ONYXIA_SHADOWFLAME_BREATH,

    EVENT_GOLEM_SENTRY_1,
    EVENT_GOLEM_SENTRY_2
};

class BlackwingDescentInstanceScript : public InstanceScript
{
public:
    explicit BlackwingDescentInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(BlackwingDescentEncounterCount);
        mAthenaeumDoorGuid = 0;
        mInnerChamberDoorGuid = 0;
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new BlackwingDescentInstanceScript(pMapMgr); }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        switch (pGameObject->getEntry())
        {
            case GO_ATHENAEUM_DOOR:
                mAthenaeumDoorGuid = pGameObject->getGuidLow();
                break;
            case GO_INNER_CHAMBER_DOOR:
                mInnerChamberDoorGuid = pGameObject->getGuidLow();
                break;
            default:
                break;
        }
    }

    // Opens after Chimaeron, granting access to the Atramedes/Maloriak wing.
    void OpenAthenaeumDoor()
    {
        if (GameObject* pDoor = GetGameObjectByGuid(mAthenaeumDoorGuid))
            useDoorOrButton(pDoor);
    }

    // Opens once both side bosses (Atramedes and Maloriak, doable in either order) are down,
    // granting access to Nefarian's End.
    void TryOpenInnerChamberDoor()
    {
        if (getBossState(DATA_ATRAMEDES) != EncounterStates::Performed)
            return;
        if (getBossState(DATA_MALORIAK) != EncounterStates::Performed)
            return;

        if (GameObject* pDoor = GetGameObjectByGuid(mInnerChamberDoorGuid))
            useDoorOrButton(pDoor);
    }

private:
    uint32_t mAthenaeumDoorGuid;
    uint32_t mInnerChamberDoorGuid;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Magmaw

class MagmawAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MagmawAI(c); }
    explicit MagmawAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_MAGMAW, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_MAGMAW_LAVA_SPEW, 12000);
        scriptEvents.addEvent(EVENT_MAGMAW_MANGLE, 8000);
        scriptEvents.addEvent(EVENT_MAGMAW_PILLAR_OF_FLAME, 20000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_MAGMAW, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_MAGMAW, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_MAGMAW_LAVA_SPEW:
                castSpellAOE(SPELL_MAGMAW_LAVA_SPEW);
                scriptEvents.addEvent(EVENT_MAGMAW_LAVA_SPEW, 18000);
                break;
            case EVENT_MAGMAW_MANGLE:
                castSpellOnVictim(SPELL_MAGMAW_MANGLE);
                scriptEvents.addEvent(EVENT_MAGMAW_MANGLE, 14000);
                break;
            case EVENT_MAGMAW_PILLAR_OF_FLAME:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_MAGMAW_PILLAR_OF_FLAME);
                scriptEvents.addEvent(EVENT_MAGMAW_PILLAR_OF_FLAME, 22000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Omnotron Defense System - Electron / Magmatron / Arcanotron / Toxitron, fought together.

class OmnotronMemberAI : public CreatureAIScript
{
public:
    explicit OmnotronMemberAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_OMNOTRON_DEFENSE_SYSTEM, EncounterStates::InProgress);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_OMNOTRON_DEFENSE_SYSTEM, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
    }
};

class ElectronAI : public OmnotronMemberAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ElectronAI(c); }
    explicit ElectronAI(Creature* pCreature) : OmnotronMemberAI(pCreature) {}

    void OnCombatStart(Unit* pTarget) override
    {
        OmnotronMemberAI::OnCombatStart(pTarget);
        scriptEvents.addEvent(EVENT_ELECTRON_STATIC_SHOCK, 8000);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_ELECTRON_STATIC_SHOCK)
        {
            if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                castSpell(pTarget, SPELL_ELECTRON_STATIC_SHOCK);
            scriptEvents.addEvent(EVENT_ELECTRON_STATIC_SHOCK, 16000);
        }
    }
};

class MagmatronAI : public OmnotronMemberAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MagmatronAI(c); }
    explicit MagmatronAI(Creature* pCreature) : OmnotronMemberAI(pCreature) {}

    void OnCombatStart(Unit* pTarget) override
    {
        OmnotronMemberAI::OnCombatStart(pTarget);
        scriptEvents.addEvent(EVENT_MAGMATRON_INCINERATION, 10000);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_MAGMATRON_INCINERATION)
        {
            castSpellAOE(SPELL_MAGMATRON_INCINERATION);
            scriptEvents.addEvent(EVENT_MAGMATRON_INCINERATION, 18000);
        }
    }
};

class ArcanotronAI : public OmnotronMemberAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ArcanotronAI(c); }
    explicit ArcanotronAI(Creature* pCreature) : OmnotronMemberAI(pCreature) {}

    void OnCombatStart(Unit* pTarget) override
    {
        OmnotronMemberAI::OnCombatStart(pTarget);
        scriptEvents.addEvent(EVENT_ARCANOTRON_POWER_GENERATOR, 12000);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_ARCANOTRON_POWER_GENERATOR)
        {
            castSpellOnSelf(SPELL_ARCANOTRON_POWER_GENERATOR);
            scriptEvents.addEvent(EVENT_ARCANOTRON_POWER_GENERATOR, 20000);
        }
    }
};

class ToxitronAI : public OmnotronMemberAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ToxitronAI(c); }
    explicit ToxitronAI(Creature* pCreature) : OmnotronMemberAI(pCreature) {}

    void OnCombatStart(Unit* pTarget) override
    {
        OmnotronMemberAI::OnCombatStart(pTarget);
        scriptEvents.addEvent(EVENT_TOXITRON_CHEMICAL_BOMB, 14000);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_TOXITRON_CHEMICAL_BOMB)
        {
            if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                castSpell(pTarget, SPELL_TOXITRON_CHEMICAL_BOMB);
            scriptEvents.addEvent(EVENT_TOXITRON_CHEMICAL_BOMB, 22000);
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Chimaeron

class ChimaeronAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ChimaeronAI(c); }
    explicit ChimaeronAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_CHIMAERON, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_CHIMAERON_CAUSTIC_SLIME, 5000);
        scriptEvents.addEvent(EVENT_CHIMAERON_BREAK, 5000);
        scriptEvents.addEvent(EVENT_CHIMAERON_MASSACRE, 26000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_CHIMAERON, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_CHIMAERON, EncounterStates::Performed);
        static_cast<BlackwingDescentInstanceScript*>(getInstanceScript())->OpenAthenaeumDoor();
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_CHIMAERON_CAUSTIC_SLIME:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_CHIMAERON_CAUSTIC_SLIME);
                scriptEvents.addEvent(EVENT_CHIMAERON_CAUSTIC_SLIME, 20000);
                break;
            case EVENT_CHIMAERON_BREAK:
                castSpellOnVictim(SPELL_CHIMAERON_BREAK);
                scriptEvents.addEvent(EVENT_CHIMAERON_BREAK, 18000);
                break;
            case EVENT_CHIMAERON_MASSACRE:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Chimaeron prepares to massacre his foes!");
                castSpellAOE(SPELL_CHIMAERON_MASSACRE);
                scriptEvents.addEvent(EVENT_CHIMAERON_MASSACRE, 30000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Atramedes

class AtramedesAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AtramedesAI(c); }
    explicit AtramedesAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "I have no need for eyes to see my enemies. Your clumsy footsteps and foul stench give you away!");
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_ATRAMEDES, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_ATRAMEDES_ROARING_BREATH, 10000);
        scriptEvents.addEvent(EVENT_ATRAMEDES_SEARING_FLAME, 16000);
        scriptEvents.addEvent(EVENT_ATRAMEDES_SONIC_BREATH, 24000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_ATRAMEDES, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "This miserable existence finally ends.");
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_ATRAMEDES, EncounterStates::Performed);
        static_cast<BlackwingDescentInstanceScript*>(getInstanceScript())->TryOpenInnerChamberDoor();
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_ATRAMEDES_ROARING_BREATH:
                castSpellAOE(SPELL_ATRAMEDES_ROARING_BREATH);
                scriptEvents.addEvent(EVENT_ATRAMEDES_ROARING_BREATH, 22000);
                break;
            case EVENT_ATRAMEDES_SEARING_FLAME:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Atramedes rears back and casts Searing Flame!");
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_ATRAMEDES_SEARING_FLAME);
                scriptEvents.addEvent(EVENT_ATRAMEDES_SEARING_FLAME, 18000);
                break;
            case EVENT_ATRAMEDES_SONIC_BREATH:
                castSpellAOE(SPELL_ATRAMEDES_SONIC_BREATH);
                scriptEvents.addEvent(EVENT_ATRAMEDES_SONIC_BREATH, 26000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Maloriak

class MaloriakAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MaloriakAI(c); }
    explicit MaloriakAI(Creature* pCreature) : CreatureAIScript(pCreature), mBottleIndex(0) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "There can be no disruptions! Mustn't keep the master waiting! Mustn't fail again!");
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_MALORIAK, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_MALORIAK_ARCANE_STORM, 12000);
        scriptEvents.addEvent(EVENT_MALORIAK_THROW_BOTTLE, 8000);
        mBottleIndex = 0;
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_MALORIAK, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "There will never be another like me...");
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_MALORIAK, EncounterStates::Performed);
        static_cast<BlackwingDescentInstanceScript*>(getInstanceScript())->TryOpenInnerChamberDoor();
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_MALORIAK_ARCANE_STORM:
                castSpellAOE(SPELL_MALORIAK_ARCANE_STORM);
                scriptEvents.addEvent(EVENT_MALORIAK_ARCANE_STORM, 20000);
                break;
            case EVENT_MALORIAK_THROW_BOTTLE:
            {
                static const uint32_t bottles[3] = { SPELL_MALORIAK_THROW_RED_BOTTLE, SPELL_MALORIAK_THROW_BLUE_BOTTLE, SPELL_MALORIAK_THROW_GREEN_BOTTLE };
                static const char* bottleText[3] = { "Maloriak throws a red vial into the cauldron!", "Maloriak throws a blue vial into the cauldron!", "Maloriak throws a green vial into the cauldron!" };
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, bottleText[mBottleIndex % 3]);
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, bottles[mBottleIndex % 3]);
                ++mBottleIndex;
                scriptEvents.addEvent(EVENT_MALORIAK_THROW_BOTTLE, 12000);
                break;
            }
            default:
                break;
        }
    }

private:
    uint8_t mBottleIndex;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Nefarian's End - Onyxia, then Nefarian.

class NefarianEndAI : public CreatureAIScript
{
public:
    explicit NefarianEndAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_NEFARIANS_END, EncounterStates::InProgress);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_NEFARIANS_END, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }
};

class OnyxiaEndAI : public NefarianEndAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new OnyxiaEndAI(c); }
    explicit OnyxiaEndAI(Creature* pCreature) : NefarianEndAI(pCreature) {}

    void OnCombatStart(Unit* pTarget) override
    {
        NefarianEndAI::OnCombatStart(pTarget);
        scriptEvents.addEvent(EVENT_ONYXIA_TAIL_LASH, 10000);
        scriptEvents.addEvent(EVENT_ONYXIA_SHADOWFLAME_BREATH, 16000);
        scriptEvents.addEvent(EVENT_ONYXIA_ELECTRICAL_CHARGE, 22000);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_ONYXIA_TAIL_LASH:
                castSpellAOE(SPELL_ONYXIA_TAIL_LASH);
                scriptEvents.addEvent(EVENT_ONYXIA_TAIL_LASH, 20000);
                break;
            case EVENT_ONYXIA_SHADOWFLAME_BREATH:
                castSpellOnVictim(SPELL_ONYXIA_SHADOWFLAME_BREATH);
                scriptEvents.addEvent(EVENT_ONYXIA_SHADOWFLAME_BREATH, 18000);
                break;
            case EVENT_ONYXIA_ELECTRICAL_CHARGE:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Energy begins to arc across Onyxia's body as her electrical charge increases.");
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_ONYXIA_ELECTRICAL_CHARGE);
                scriptEvents.addEvent(EVENT_ONYXIA_ELECTRICAL_CHARGE, 24000);
                break;
            default:
                break;
        }
    }
};

class NefarianAI : public NefarianEndAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NefarianAI(c); }
    explicit NefarianAI(Creature* pCreature) : NefarianEndAI(pCreature) {}

    void OnCombatStart(Unit* pTarget) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "Curse you, mortals! Such a callous disregard for one's possessions must be met with extreme force!");
        NefarianEndAI::OnCombatStart(pTarget);
        scriptEvents.addEvent(EVENT_NEFARIAN_SHADOWFLAME_BARRAGE, 12000);
        scriptEvents.addEvent(EVENT_NEFARIAN_ELECTRICAL_CHARGE, 20000);
        scriptEvents.addEvent(EVENT_NEFARIAN_BERSERK, 600000);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "Defeat has never tasted so bitter...");
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_NEFARIANS_END, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_NEFARIAN_SHADOWFLAME_BARRAGE:
                castSpellAOE(SPELL_NEFARIAN_SHADOWFLAME_BARRAGE);
                scriptEvents.addEvent(EVENT_NEFARIAN_SHADOWFLAME_BARRAGE, 22000);
                break;
            case EVENT_NEFARIAN_ELECTRICAL_CHARGE:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "The air crackles with electricity!");
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_NEFARIAN_ELECTRICAL_CHARGE);
                scriptEvents.addEvent(EVENT_NEFARIAN_ELECTRICAL_CHARGE, 26000);
                break;
            case EVENT_NEFARIAN_BERSERK:
                castSpellOnSelf(SPELL_NEFARIAN_BERSERK, true);
                break;
            default:
                break;
        }
    }
};

class GolemSentryAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new GolemSentryAI(c); }
    explicit GolemSentryAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_GOLEM_SENTRY_1, 9000);
        scriptEvents.addEvent(EVENT_GOLEM_SENTRY_2, 9000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_GOLEM_SENTRY_1:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_GOLEM_SENTRY_1);
                scriptEvents.addEvent(EVENT_GOLEM_SENTRY_1, 18000);
                break;
            case EVENT_GOLEM_SENTRY_2:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_GOLEM_SENTRY_2);
                scriptEvents.addEvent(EVENT_GOLEM_SENTRY_2, 7000);
                break;
            default:
                break;
        }
    }
};

void SetupBlackwingDescent(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_BLACKWING_DESCENT, &BlackwingDescentInstanceScript::Create);

    mgr->register_creature_script(BOSS_MAGMAW, &MagmawAI::Create);
    mgr->register_creature_script(NPC_ELECTRON, &ElectronAI::Create);
    mgr->register_creature_script(NPC_MAGMATRON, &MagmatronAI::Create);
    mgr->register_creature_script(NPC_ARCANOTRON, &ArcanotronAI::Create);
    mgr->register_creature_script(NPC_TOXITRON, &ToxitronAI::Create);
    mgr->register_creature_script(BOSS_CHIMAERON, &ChimaeronAI::Create);
    mgr->register_creature_script(BOSS_ATRAMEDES, &AtramedesAI::Create);
    mgr->register_creature_script(BOSS_MALORIAK, &MaloriakAI::Create);
    mgr->register_creature_script(NPC_ONYXIA, &OnyxiaEndAI::Create);
    mgr->register_creature_script(BOSS_NEFARIAN, &NefarianAI::Create);

    mgr->register_creature_script(NPC_GOLEM_SENTRY, &GolemSentryAI::Create);
}
