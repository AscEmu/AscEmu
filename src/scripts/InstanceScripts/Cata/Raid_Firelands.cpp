/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

// Firelands - seven bosses (verified against wowhead), hand-ported into AscEmu's own
// CreatureAIScript / InstanceScript framework.
//
// Every fight here carries an elaborate signature mechanic that isn't ported: Beth'tilac's
// web-climbing/ground-phase split, Lord Rhyolith's shoot-the-feet steering puzzle, Shannox's
// companion-pet (Riplimb/Rageface) trap gauntlet, Alysrazor's flight/feather-collection phase,
// Baleroc's weapon-swap Torment mechanic, Majordomo Staghelm's cat/scorpion stance dance, and
// Ragnaros' multi-phase platform/meteor finale. Each boss instead runs a simplified but
// functional core rotation from its real kit. Progression gating (per wowhead: Beth'tilac +
// Shannox unlock Rhyolith + Alysrazor, which unlock Baleroc, which unlocks Majordomo
// Staghelm, who unlocks Ragnaros) is tracked via the Baleroc Door, the fire walls leading to
// Staghelm's platform, and the Sulfuron Keep door.

#include "Setup.h"
#include "Raid_Firelands.hpp"

#include "Objects/GameObject.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

enum FirelandsEvents
{
    EVENT_BETHTILAC_EMBER_FLARE = 1,
    EVENT_BETHTILAC_VENOM_RAIN,
    EVENT_BETHTILAC_SMOLDERING_DEVASTATION,

    EVENT_SHANNOX_IMMOLATION_TRAP,
    EVENT_SHANNOX_ARCING_SLASH,
    EVENT_SHANNOX_HURL_SPEAR,

    EVENT_RHYOLITH_CONCUSSIVE_STOMP,
    EVENT_RHYOLITH_MOLTEN_SPEW,

    EVENT_ALYSRAZOR_FIRE_IT_UP,
    EVENT_ALYSRAZOR_FIEROCLAST_BARRAGE,

    EVENT_BALEROC_TORMENT,
    EVENT_BALEROC_SHARDS_OF_TORMENT,
    EVENT_BALEROC_BLAZE_OF_GLORY,
    EVENT_BALEROC_BERSERK,

    EVENT_STAGHELM_FLAME_SCYTHE,
    EVENT_STAGHELM_FIERY_CYCLONE,
    EVENT_STAGHELM_BURNING_ORBS,

    EVENT_RAGNAROS_WRATH_OF_RAGNAROS,
    EVENT_RAGNAROS_MAGMA_BLAST,
    EVENT_RAGNAROS_SULFURAS_SMASH,
    EVENT_RAGNAROS_LIVING_METEOR,

    EVENT_MOLTEN_LORD_STRIKE,
    EVENT_MOLTEN_LORD_STOMP,
    EVENT_MOLTEN_LORD_ERUPTION,
    EVENT_MOLTEN_LORD_MOLTEN_ARMOR,
    EVENT_FLAMEWAKER_ANIMATOR_ANIMATE,
    EVENT_CINDERWEB_DRONE_POISON_SPIT,
    EVENT_CINDERWEB_DRONE_WEB_SPRAY,
    EVENT_CINDERWEB_DRONE_SHELL_ARMOR,
    EVENT_FLAMEWAKER_CAUTERIZER_CAUTERIZE,
    EVENT_CINDERWEB_SPINNER_SPIT
};

class FirelandsInstanceScript : public InstanceScript
{
public:
    explicit FirelandsInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(FirelandsEncounterCount);
        mBalerocDoorGuid = 0;
        mStaghelmFireWall1Guid = 0;
        mStaghelmFireWall2Guid = 0;
        mSulfuronKeepDoorGuid = 0;
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new FirelandsInstanceScript(pMapMgr); }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        switch (pGameObject->getEntry())
        {
            case GO_BALEROC_DOOR:
                mBalerocDoorGuid = pGameObject->getGuidLow();
                break;
            case GO_STAGHELM_FIRE_WALL_1:
                mStaghelmFireWall1Guid = pGameObject->getGuidLow();
                break;
            case GO_STAGHELM_FIRE_WALL_2:
                mStaghelmFireWall2Guid = pGameObject->getGuidLow();
                break;
            case GO_SULFURON_KEEP_DOOR:
                mSulfuronKeepDoorGuid = pGameObject->getGuidLow();
                break;
            default:
                break;
        }
    }

    // Opens once both first-tier bosses (Beth'tilac and Shannox, either order) and both
    // second-tier bosses (Lord Rhyolith and Alysrazor, either order) are down.
    void TryOpenBalerocDoor()
    {
        if (getBossState(DATA_BETHTILAC) != EncounterStates::Performed)
            return;
        if (getBossState(DATA_SHANNOX) != EncounterStates::Performed)
            return;
        if (getBossState(DATA_LORD_RHYOLITH) != EncounterStates::Performed)
            return;
        if (getBossState(DATA_ALYSRAZOR) != EncounterStates::Performed)
            return;

        if (GameObject* pDoor = GetGameObjectByGuid(mBalerocDoorGuid))
            useDoorOrButton(pDoor);
    }

    // Opens the path toward Majordomo Staghelm's platform after Baleroc.
    void OpenStaghelmFireWalls()
    {
        if (GameObject* pWall = GetGameObjectByGuid(mStaghelmFireWall1Guid))
            useDoorOrButton(pWall);
        if (GameObject* pWall = GetGameObjectByGuid(mStaghelmFireWall2Guid))
            useDoorOrButton(pWall);
    }

    // Opens the way into Ragnaros' Sulfuron Keep chamber after Majordomo Staghelm.
    void OpenSulfuronKeepDoor()
    {
        if (GameObject* pDoor = GetGameObjectByGuid(mSulfuronKeepDoorGuid))
            useDoorOrButton(pDoor);
    }

private:
    uint32_t mBalerocDoorGuid;
    uint32_t mStaghelmFireWall1Guid;
    uint32_t mStaghelmFireWall2Guid;
    uint32_t mSulfuronKeepDoorGuid;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Beth'tilac

class BethtilacAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BethtilacAI(c); }
    explicit BethtilacAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_BETHTILAC, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_BETHTILAC_EMBER_FLARE, 10000);
        scriptEvents.addEvent(EVENT_BETHTILAC_VENOM_RAIN, 16000);
        scriptEvents.addEvent(EVENT_BETHTILAC_SMOLDERING_DEVASTATION, 40000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_BETHTILAC, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_BETHTILAC, EncounterStates::Performed);
        static_cast<FirelandsInstanceScript*>(getInstanceScript())->TryOpenBalerocDoor();
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_BETHTILAC_EMBER_FLARE:
                castSpellAOE(SPELL_BETHTILAC_EMBER_FLARE);
                scriptEvents.addEvent(EVENT_BETHTILAC_EMBER_FLARE, 22000);
                break;
            case EVENT_BETHTILAC_VENOM_RAIN:
                castSpellAOE(SPELL_BETHTILAC_VENOM_RAIN);
                scriptEvents.addEvent(EVENT_BETHTILAC_VENOM_RAIN, 20000);
                break;
            case EVENT_BETHTILAC_SMOLDERING_DEVASTATION:
                castSpellOnSelf(SPELL_BETHTILAC_SMOLDERING_DEVASTATION);
                scriptEvents.addEvent(EVENT_BETHTILAC_SMOLDERING_DEVASTATION, 45000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shannox

class ShannoxAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ShannoxAI(c); }
    explicit ShannoxAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_SHANNOX, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_SHANNOX_IMMOLATION_TRAP, 8000);
        scriptEvents.addEvent(EVENT_SHANNOX_ARCING_SLASH, 14000);
        scriptEvents.addEvent(EVENT_SHANNOX_HURL_SPEAR, 20000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_SHANNOX, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_SHANNOX, EncounterStates::Performed);
        static_cast<FirelandsInstanceScript*>(getInstanceScript())->TryOpenBalerocDoor();
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_SHANNOX_IMMOLATION_TRAP:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_SHANNOX_IMMOLATION_TRAP);
                scriptEvents.addEvent(EVENT_SHANNOX_IMMOLATION_TRAP, 20000);
                break;
            case EVENT_SHANNOX_ARCING_SLASH:
                castSpellAOE(SPELL_SHANNOX_ARCING_SLASH);
                scriptEvents.addEvent(EVENT_SHANNOX_ARCING_SLASH, 18000);
                break;
            case EVENT_SHANNOX_HURL_SPEAR:
                castSpellOnVictim(SPELL_SHANNOX_HURL_SPEAR);
                scriptEvents.addEvent(EVENT_SHANNOX_HURL_SPEAR, 22000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Lord Rhyolith

class LordRhyolithAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new LordRhyolithAI(c); }
    explicit LordRhyolithAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "Hah? Hruumph? Soft little fleshy-things? Here? Nuisances, nuisances!");
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_LORD_RHYOLITH, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_RHYOLITH_CONCUSSIVE_STOMP, 12000);
        scriptEvents.addEvent(EVENT_RHYOLITH_MOLTEN_SPEW, 8000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_LORD_RHYOLITH, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "Finished.");
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_LORD_RHYOLITH, EncounterStates::Performed);
        static_cast<FirelandsInstanceScript*>(getInstanceScript())->TryOpenBalerocDoor();
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_RHYOLITH_CONCUSSIVE_STOMP:
                castSpellAOE(SPELL_RHYOLITH_CONCUSSIVE_STOMP);
                scriptEvents.addEvent(EVENT_RHYOLITH_CONCUSSIVE_STOMP, 20000);
                break;
            case EVENT_RHYOLITH_MOLTEN_SPEW:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_RHYOLITH_MOLTEN_SPEW);
                scriptEvents.addEvent(EVENT_RHYOLITH_MOLTEN_SPEW, 16000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Alysrazor

class AlysrazorAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AlysrazorAI(c); }
    explicit AlysrazorAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_ALYSRAZOR, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_ALYSRAZOR_FIRE_IT_UP, 10000);
        scriptEvents.addEvent(EVENT_ALYSRAZOR_FIEROCLAST_BARRAGE, 16000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_ALYSRAZOR, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_ALYSRAZOR, EncounterStates::Performed);
        static_cast<FirelandsInstanceScript*>(getInstanceScript())->TryOpenBalerocDoor();
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_ALYSRAZOR_FIRE_IT_UP:
                castSpellOnSelf(SPELL_ALYSRAZOR_FIRE_IT_UP);
                scriptEvents.addEvent(EVENT_ALYSRAZOR_FIRE_IT_UP, 24000);
                break;
            case EVENT_ALYSRAZOR_FIEROCLAST_BARRAGE:
                castSpellAOE(SPELL_ALYSRAZOR_FIEROCLAST_BARRAGE);
                scriptEvents.addEvent(EVENT_ALYSRAZOR_FIEROCLAST_BARRAGE, 20000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Baleroc

class BalerocAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BalerocAI(c); }
    explicit BalerocAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 24441, "You are forbidden from my master's domain, mortals.");
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_BALEROC, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_BALEROC_TORMENT, 5000);
        scriptEvents.addEvent(EVENT_BALEROC_SHARDS_OF_TORMENT, 5000);
        scriptEvents.addEvent(EVENT_BALEROC_BLAZE_OF_GLORY, 30500);
        scriptEvents.addEvent(EVENT_BALEROC_BERSERK, 360000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_BALEROC, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnTargetDied(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 24449, "You have been judged.");
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 24444, "Mortal filth... the master's keep is forbidden....");
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_BALEROC, EncounterStates::Performed);
        static_cast<FirelandsInstanceScript*>(getInstanceScript())->OpenStaghelmFireWalls();
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_BALEROC_TORMENT:
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 24447, "By the Firelord's command, you, too, shall perish!");
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_BALEROC_TORMENT);
                scriptEvents.addEvent(EVENT_BALEROC_TORMENT, 26000);
                break;
            case EVENT_BALEROC_SHARDS_OF_TORMENT:
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 24446, "Fool mortals. Hurl yourselves into your own demise!");
                castSpellAOE(SPELL_BALEROC_SHARDS_OF_TORMENT);
                scriptEvents.addEvent(EVENT_BALEROC_SHARDS_OF_TORMENT, 22000);
                break;
            case EVENT_BALEROC_BLAZE_OF_GLORY:
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 24459, "Burn beneath my molten fury!");
                castSpellOnSelf(SPELL_BALEROC_BLAZE_OF_GLORY);
                scriptEvents.addEvent(EVENT_BALEROC_BLAZE_OF_GLORY, 30500);
                break;
            case EVENT_BALEROC_BERSERK:
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 24450, "Your flesh is forfeit to the fires of this realm.");
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Baleroc goes into a berserker rage!");
                castSpellOnSelf(SPELL_BALEROC_BERSERK, true);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Majordomo Staghelm

class MajordomoStaghelmAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MajordomoStaghelmAI(c); }
    explicit MajordomoStaghelmAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "Behold the rage of the Firelands!");
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_MAJORDOMO_STAGHELM, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_STAGHELM_FLAME_SCYTHE, 8000);
        scriptEvents.addEvent(EVENT_STAGHELM_FIERY_CYCLONE, 20000);
        scriptEvents.addEvent(EVENT_STAGHELM_BURNING_ORBS, 14000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_MAJORDOMO_STAGHELM, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "My studies... had only just begun...");
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_MAJORDOMO_STAGHELM, EncounterStates::Performed);
        static_cast<FirelandsInstanceScript*>(getInstanceScript())->OpenSulfuronKeepDoor();
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_STAGHELM_FLAME_SCYTHE:
                castSpellOnVictim(SPELL_STAGHELM_FLAME_SCYTHE);
                scriptEvents.addEvent(EVENT_STAGHELM_FLAME_SCYTHE, 14000);
                break;
            case EVENT_STAGHELM_FIERY_CYCLONE:
                castSpellAOE(SPELL_STAGHELM_FIERY_CYCLONE);
                scriptEvents.addEvent(EVENT_STAGHELM_FIERY_CYCLONE, 30000);
                break;
            case EVENT_STAGHELM_BURNING_ORBS:
                castSpellOnSelf(SPELL_STAGHELM_BURNING_ORBS);
                scriptEvents.addEvent(EVENT_STAGHELM_BURNING_ORBS, 26000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Ragnaros

class RagnarosAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new RagnarosAI(c); }
    explicit RagnarosAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_RAGNAROS, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_RAGNAROS_WRATH_OF_RAGNAROS, 12000);
        scriptEvents.addEvent(EVENT_RAGNAROS_MAGMA_BLAST, 8000);
        scriptEvents.addEvent(EVENT_RAGNAROS_SULFURAS_SMASH, 20000);
        scriptEvents.addEvent(EVENT_RAGNAROS_LIVING_METEOR, 16000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_RAGNAROS, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_RAGNAROS, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_RAGNAROS_WRATH_OF_RAGNAROS:
                castSpellAOE(SPELL_RAGNAROS_WRATH_OF_RAGNAROS);
                scriptEvents.addEvent(EVENT_RAGNAROS_WRATH_OF_RAGNAROS, 24000);
                break;
            case EVENT_RAGNAROS_MAGMA_BLAST:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_RAGNAROS_MAGMA_BLAST);
                scriptEvents.addEvent(EVENT_RAGNAROS_MAGMA_BLAST, 18000);
                break;
            case EVENT_RAGNAROS_SULFURAS_SMASH:
                castSpellOnSelf(SPELL_RAGNAROS_SULFURAS_SMASH);
                scriptEvents.addEvent(EVENT_RAGNAROS_SULFURAS_SMASH, 30000);
                break;
            case EVENT_RAGNAROS_LIVING_METEOR:
                castSpellAOE(SPELL_RAGNAROS_LIVING_METEOR);
                scriptEvents.addEvent(EVENT_RAGNAROS_LIVING_METEOR, 22000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Trash

class MoltenLordAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MoltenLordAI(c); }
    explicit MoltenLordAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_MOLTEN_LORD_STRIKE, 1000);
        scriptEvents.addEvent(EVENT_MOLTEN_LORD_STOMP, 12000);
        scriptEvents.addEvent(EVENT_MOLTEN_LORD_ERUPTION, 20000);
        scriptEvents.addEvent(EVENT_MOLTEN_LORD_MOLTEN_ARMOR, 5000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_MOLTEN_LORD_STRIKE:
                castSpellOnVictim(SPELL_MOLTEN_LORD_STRIKE);
                scriptEvents.addEvent(EVENT_MOLTEN_LORD_STRIKE, 4000);
                break;
            case EVENT_MOLTEN_LORD_STOMP:
                castSpellOnVictim(SPELL_MOLTEN_LORD_STOMP);
                scriptEvents.addEvent(EVENT_MOLTEN_LORD_STOMP, 12000);
                break;
            case EVENT_MOLTEN_LORD_ERUPTION:
                castSpellOnVictim(SPELL_MOLTEN_LORD_ERUPTION);
                scriptEvents.addEvent(EVENT_MOLTEN_LORD_ERUPTION, 20000);
                break;
            case EVENT_MOLTEN_LORD_MOLTEN_ARMOR:
                castSpellOnSelf(SPELL_MOLTEN_LORD_MOLTEN_ARMOR);
                scriptEvents.addEvent(EVENT_MOLTEN_LORD_MOLTEN_ARMOR, 5000);
                break;
            default:
                break;
        }
    }
};

// Reference drives this mob's animate/summon-link behavior through AI scripting data events
// we have no equivalent for (link/reset triggers); ported as a periodic self-cast only.
class FlamewakerAnimatorAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new FlamewakerAnimatorAI(c); }
    explicit FlamewakerAnimatorAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_FLAMEWAKER_ANIMATOR_ANIMATE, 1000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_FLAMEWAKER_ANIMATOR_ANIMATE)
        {
            castSpellOnSelf(SPELL_FLAMEWAKER_ANIMATOR_ANIMATE);
            scriptEvents.addEvent(EVENT_FLAMEWAKER_ANIMATOR_ANIMATE, 5000);
        }
    }
};

class FlamewakerSubjugatorAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new FlamewakerSubjugatorAI(c); }
    explicit FlamewakerSubjugatorAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override { mEmergencyCast = false; }

    void AIUpdate(unsigned long /*time_passed*/) override
    {
        if (!mEmergencyCast && _getHealthPercent() <= 25)
        {
            mEmergencyCast = true;
            castSpellOnSelf(SPELL_FLAMEWAKER_SUBJUGATOR_EMERGENCY);
        }
    }

private:
    bool mEmergencyCast = false;
};

class CinderwebDroneAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CinderwebDroneAI(c); }
    explicit CinderwebDroneAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_CINDERWEB_DRONE_POISON_SPIT, 10000);
        scriptEvents.addEvent(EVENT_CINDERWEB_DRONE_WEB_SPRAY, 10000);
        scriptEvents.addEvent(EVENT_CINDERWEB_DRONE_SHELL_ARMOR, 10000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_CINDERWEB_DRONE_POISON_SPIT:
                castSpellOnVictim(SPELL_CINDERWEB_DRONE_POISON_SPIT);
                scriptEvents.addEvent(EVENT_CINDERWEB_DRONE_POISON_SPIT, 25000);
                break;
            case EVENT_CINDERWEB_DRONE_WEB_SPRAY:
                castSpellOnVictim(SPELL_CINDERWEB_DRONE_WEB_SPRAY);
                scriptEvents.addEvent(EVENT_CINDERWEB_DRONE_WEB_SPRAY, 25000);
                break;
            case EVENT_CINDERWEB_DRONE_SHELL_ARMOR:
                castSpellOnSelf(SPELL_CINDERWEB_DRONE_SHELL_ARMOR);
                scriptEvents.addEvent(EVENT_CINDERWEB_DRONE_SHELL_ARMOR, 25000);
                break;
            default:
                break;
        }
    }
};

class FlamewakerCauterizerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new FlamewakerCauterizerAI(c); }
    explicit FlamewakerCauterizerAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_FLAMEWAKER_CAUTERIZER_CAUTERIZE, 10000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_FLAMEWAKER_CAUTERIZER_CAUTERIZE)
        {
            castSpellOnSelf(SPELL_FLAMEWAKER_CAUTERIZER_CAUTERIZE);
            scriptEvents.addEvent(EVENT_FLAMEWAKER_CAUTERIZER_CAUTERIZE, 25000);
        }
    }
};

class CinderwebSpinnerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CinderwebSpinnerAI(c); }
    explicit CinderwebSpinnerAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_CINDERWEB_SPINNER_SPIT, 3000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_CINDERWEB_SPINNER_SPIT)
        {
            castSpellOnVictim(SPELL_CINDERWEB_SPINNER_SPIT);
            scriptEvents.addEvent(EVENT_CINDERWEB_SPINNER_SPIT, 3000);
        }
    }
};

void SetupFirelands(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_FIRELANDS, &FirelandsInstanceScript::Create);

    mgr->register_creature_script(BOSS_BETHTILAC, &BethtilacAI::Create);
    mgr->register_creature_script(BOSS_SHANNOX, &ShannoxAI::Create);
    mgr->register_creature_script(BOSS_LORD_RHYOLITH, &LordRhyolithAI::Create);
    mgr->register_creature_script(BOSS_ALYSRAZOR, &AlysrazorAI::Create);
    mgr->register_creature_script(BOSS_BALEROC, &BalerocAI::Create);
    mgr->register_creature_script(BOSS_MAJORDOMO_STAGHELM, &MajordomoStaghelmAI::Create);
    mgr->register_creature_script(BOSS_RAGNAROS, &RagnarosAI::Create);

    mgr->register_creature_script(NPC_MOLTEN_LORD, &MoltenLordAI::Create);
    mgr->register_creature_script(NPC_FLAMEWAKER_ANIMATOR, &FlamewakerAnimatorAI::Create);
    mgr->register_creature_script(NPC_FLAMEWAKER_SUBJUGATOR, &FlamewakerSubjugatorAI::Create);
    mgr->register_creature_script(NPC_CINDERWEB_DRONE, &CinderwebDroneAI::Create);
    mgr->register_creature_script(NPC_FLAMEWAKER_CAUTERIZER, &FlamewakerCauterizerAI::Create);
    mgr->register_creature_script(NPC_CINDERWEB_SPINNER, &CinderwebSpinnerAI::Create);
}
