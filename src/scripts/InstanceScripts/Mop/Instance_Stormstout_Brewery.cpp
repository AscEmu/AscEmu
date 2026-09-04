/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

// Stormstout Brewery - three bosses, hand-ported into AscEmu's own CreatureAIScript /
// InstanceScript framework. See the header comment for what was simplified/dropped.

#include "Setup.h"
#include "Instance_Stormstout_Brewery.hpp"

#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

enum StormstoutBreweryEvents
{
    EVENT_OOKOOK_GROUND_POUND = 1,
    EVENT_OOKOOK_GOING_BANANAS,

    EVENT_HOPTALLUS_FURLWIND,
    EVENT_HOPTALLUS_CARROT_BREATH,
    EVENT_HOPTALLUS_SCREECH,

    EVENT_YANZHU_BLOAT,
    EVENT_YANZHU_BLACKOUT_BREW,
    EVENT_YANZHU_BREW_BOLT,
    EVENT_YANZHU_WALL_OF_SUDS,

    EVENT_HOZEN_PARTY_ANIMAL_CLEAVE,
    EVENT_HOZEN_PARTY_ANIMAL_ENRAGE,

    EVENT_BREWMASTER_LESSER_HEAL,
    EVENT_BREWMASTER_RENEW
};

class StormstoutBreweryInstanceScript : public InstanceScript
{
public:
    explicit StormstoutBreweryInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(StormstoutBreweryEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new StormstoutBreweryInstanceScript(pMapMgr); }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        if (pGameObject->getEntry() == GO_PANDA_BREWERY_DOOR)
            mBreweryDoorGuids.push_back(pGameObject->getGuidLow());
    }

    void OpenBreweryDoor()
    {
        for (uint32_t guid : mBreweryDoorGuids)
        {
            if (GameObject* pDoor = GetGameObjectByGuid(guid))
                useDoorOrButton(pDoor);
        }
    }

private:
    std::vector<uint32_t> mBreweryDoorGuids;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Ook-Ook

class OokOokAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new OokOokAI(c); }
    explicit OokOokAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 28796, "Me gonna ook you in the dooker!");
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_OOK_OOK, EncounterStates::InProgress);

        mBananaTalkIndex = 0;
        scriptEvents.addEvent(EVENT_OOKOOK_GROUND_POUND, 12000);
        scriptEvents.addEvent(EVENT_OOKOOK_GOING_BANANAS, 9000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_OOK_OOK, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 28797, "Ook! Oooook!!");
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_OOK_OOK, EncounterStates::Performed);
        static_cast<StormstoutBreweryInstanceScript*>(getInstanceScript())->OpenBreweryDoor();
    }

    void OnTargetDied(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 28799, "In the dooker!");
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_OOKOOK_GROUND_POUND:
                castSpellOnVictim(SPELL_OOKOOK_GROUND_POUND);
                scriptEvents.addEvent(EVENT_OOKOOK_GROUND_POUND, 13000);
                break;
            case EVENT_OOKOOK_GOING_BANANAS:
            {
                castSpellOnSelf(SPELL_OOKOOK_GOING_BANANAS);
                static const char* bananaLines[3] = { "Get Ooking party started!", "Come on and get your Ook on!", "We gonna Ook all night!" };
                static const uint32_t bananaSounds[3] = { 28800, 28801, 28802 };
                if (mBananaTalkIndex < 3)
                {
                    sendChatMessage(CHAT_MSG_MONSTER_YELL, bananaSounds[mBananaTalkIndex], bananaLines[mBananaTalkIndex]);
                    ++mBananaTalkIndex;
                }
                scriptEvents.addEvent(EVENT_OOKOOK_GOING_BANANAS, 9500);
                break;
            }
            default:
                break;
        }
    }

private:
    int mBananaTalkIndex = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Hoptallus

class HoptallusAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HoptallusAI(c); }
    explicit HoptallusAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_HOPTALLUS, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_HOPTALLUS_FURLWIND, 16000);
        scriptEvents.addEvent(EVENT_HOPTALLUS_CARROT_BREATH, 30000);
        scriptEvents.addEvent(EVENT_HOPTALLUS_SCREECH, 30000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_HOPTALLUS, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_HOPTALLUS, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_HOPTALLUS_FURLWIND:
                castSpellOnVictim(SPELL_HOPTALLUS_FURLWIND);
                scriptEvents.addEvent(EVENT_HOPTALLUS_FURLWIND, 45000);
                break;
            case EVENT_HOPTALLUS_CARROT_BREATH:
                castSpellOnVictim(SPELL_HOPTALLUS_CARROT_BREATH);
                scriptEvents.addEvent(EVENT_HOPTALLUS_CARROT_BREATH, 26000);
                break;
            case EVENT_HOPTALLUS_SCREECH:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Hoptallus screeches, calling more of the warren to his side!");
                castSpellOnSelf(SPELL_HOPTALLUS_SCREECH);
                summonCreature(NPC_GOLDEN_HOPLING, getCreature()->GetPositionX() + 5.0f, getCreature()->GetPositionY(), getCreature()->GetPositionZ(), 0.0f, TIMED_DESPAWN, 15000);
                summonCreature(NPC_GOLDEN_HOPLING, getCreature()->GetPositionX() - 5.0f, getCreature()->GetPositionY(), getCreature()->GetPositionZ(), 0.0f, TIMED_DESPAWN, 15000);
                summonCreature(NPC_GOLDEN_HOPLING, getCreature()->GetPositionX(), getCreature()->GetPositionY() + 5.0f, getCreature()->GetPositionZ(), 0.0f, TIMED_DESPAWN, 15000);
                scriptEvents.addEvent(EVENT_HOPTALLUS_SCREECH, 30000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Yan-Zhu the Uncasked

class YanZhuAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new YanZhuAI(c); }
    explicit YanZhuAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "You dare disturb my brew?!");
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_YAN_ZHU, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_YANZHU_BLOAT, 18000);
        scriptEvents.addEvent(EVENT_YANZHU_BLACKOUT_BREW, 6000);
        scriptEvents.addEvent(EVENT_YANZHU_BREW_BOLT, 4000);
        scriptEvents.addEvent(EVENT_YANZHU_WALL_OF_SUDS, 24000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_YAN_ZHU, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "The brew... spoils...");
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_YAN_ZHU, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_YANZHU_BLOAT:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Yan-Zhu inflates one of his foes with brew!");
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_YANZHU_BLOAT);
                scriptEvents.addEvent(EVENT_YANZHU_BLOAT, 30000);
                break;
            case EVENT_YANZHU_BLACKOUT_BREW:
                castSpellOnVictim(SPELL_YANZHU_BLACKOUT_BREW);
                scriptEvents.addEvent(EVENT_YANZHU_BLACKOUT_BREW, 3000);
                break;
            case EVENT_YANZHU_BREW_BOLT:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_YANZHU_BREW_BOLT);
                scriptEvents.addEvent(EVENT_YANZHU_BREW_BOLT, 8000);
                break;
            case EVENT_YANZHU_WALL_OF_SUDS:
                castSpellAOE(SPELL_YANZHU_WALL_OF_SUDS);
                scriptEvents.addEvent(EVENT_YANZHU_WALL_OF_SUDS, 28000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Trash - Hozen Party Animal

class HozenPartyAnimalAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HozenPartyAnimalAI(c); }
    explicit HozenPartyAnimalAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_HOZEN_PARTY_ANIMAL_CLEAVE, 6000);
        scriptEvents.addEvent(EVENT_HOZEN_PARTY_ANIMAL_ENRAGE, 14000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_HOZEN_PARTY_ANIMAL_CLEAVE:
                castSpellOnVictim(SPELL_HOZEN_PARTY_ANIMAL_CLEAVE);
                scriptEvents.addEvent(EVENT_HOZEN_PARTY_ANIMAL_CLEAVE, 12000);
                break;
            case EVENT_HOZEN_PARTY_ANIMAL_ENRAGE:
                castSpellOnSelf(SPELL_HOZEN_PARTY_ANIMAL_ENRAGE);
                scriptEvents.addEvent(EVENT_HOZEN_PARTY_ANIMAL_ENRAGE, 24000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Trash - Ancestral Brewmaster

class AncestralBrewmasterAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AncestralBrewmasterAI(c); }
    explicit AncestralBrewmasterAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_BREWMASTER_LESSER_HEAL, 8000);
        scriptEvents.addEvent(EVENT_BREWMASTER_RENEW, 4000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_BREWMASTER_LESSER_HEAL:
                castSpellOnSelf(SPELL_BREWMASTER_LESSER_HEAL);
                scriptEvents.addEvent(EVENT_BREWMASTER_LESSER_HEAL, 16000);
                break;
            case EVENT_BREWMASTER_RENEW:
                castSpellOnSelf(SPELL_BREWMASTER_RENEW);
                scriptEvents.addEvent(EVENT_BREWMASTER_RENEW, 20000);
                break;
            default:
                break;
        }
    }
};

void SetupStormstoutBrewery(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_STORMSTOUT_BREWERY, &StormstoutBreweryInstanceScript::Create);

    mgr->register_creature_script(BOSS_OOK_OOK, &OokOokAI::Create);
    mgr->register_creature_script(BOSS_HOPTALLUS, &HoptallusAI::Create);
    mgr->register_creature_script(BOSS_YAN_ZHU, &YanZhuAI::Create);

    mgr->register_creature_script(NPC_HOZEN_PARTY_ANIMAL, &HozenPartyAnimalAI::Create);
    mgr->register_creature_script(NPC_ANCESTRAL_BREWMASTER, &AncestralBrewmasterAI::Create);
}
