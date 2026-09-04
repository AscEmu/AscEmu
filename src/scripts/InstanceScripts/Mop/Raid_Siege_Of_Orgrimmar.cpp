/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

// Siege of Orgrimmar - fourteen bosses (see header comment for sourcing/limitations), hand-
// ported into AscEmu's own CreatureAIScript / InstanceScript framework.

#include "Setup.h"
#include "Raid_Siege_Of_Orgrimmar.hpp"

#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

enum SiegeOfOrgrimmarEvents
{
    EVENT_IMMERSEUS_SHA_BOLT = 1,
    EVENT_IMMERSEUS_SWIRL,
    EVENT_IMMERSEUS_CORROSIVE_BLAST,
    EVENT_IMMERSEUS_ERUPTING_SHA,

    EVENT_PROTECTORS_INFERNO_STRIKE,
    EVENT_PROTECTORS_CORRUPTION_SHOCK,
    EVENT_PROTECTORS_CORRUPTED_BREW,
    EVENT_PROTECTORS_CORRUPTION_KICK,

    EVENT_NORUSHEN_CORRUPTION,
    EVENT_NORUSHEN_UNLEASHED_ANGER,
    EVENT_NORUSHEN_LINGERING_CORRUPTION,

    EVENT_PRIDE_MOCKING_BLAST,
    EVENT_PRIDE_LAST_WORD,
    EVENT_PRIDE_WOUNDED_PRIDE,
    EVENT_PRIDE_BURSTING_PRIDE,

    EVENT_GALAKRAS_THUNDER_CLAP,
    EVENT_GALAKRAS_ARCING_SMASH,
    EVENT_GALAKRAS_FLAMESTRIKE,
    EVENT_GALAKRAS_POISON_CLOUD,

    EVENT_JUGGERNAUT_LASER_BURN,
    EVENT_JUGGERNAUT_FLAME_VENTS,
    EVENT_JUGGERNAUT_SHOCK_PULSE,
    EVENT_JUGGERNAUT_MORTAR_BARRAGE,

    EVENT_SHAMAN_FROSTSTORM_BOLT,
    EVENT_SHAMAN_TOXIC_STORM,
    EVENT_SHAMAN_IRON_TOMB,
    EVENT_SHAMAN_FOUL_GEYSER,

    EVENT_NAZGRIM_SUNDERING_BLOW,
    EVENT_NAZGRIM_BONECRACKER,
    EVENT_NAZGRIM_HEROIC_SHOCKWAVE,
    EVENT_NAZGRIM_RAVAGER,

    EVENT_MALKOROK_BREATH_OF_YSHAARJ,
    EVENT_MALKOROK_ARCING_SMASH,
    EVENT_MALKOROK_SEISMIC_SLAM,
    EVENT_MALKOROK_BLOOD_RAGE,

    EVENT_SPOILS_CRUSH,
    EVENT_SPOILS_GUARD_STANCE,

    EVENT_THOK_FEARSOME_ROAR,
    EVENT_THOK_DEAFENING_SCREECH,
    EVENT_THOK_ACID_BREATH,
    EVENT_THOK_WRECKING_BALL,

    EVENT_BLACKFUSE_LAUNCH_SAWBLADE,
    EVENT_BLACKFUSE_SERRATED_SLASH,
    EVENT_BLACKFUSE_ELECTROSTATIC_CHARGE,
    EVENT_BLACKFUSE_DEATH_FROM_ABOVE,

    EVENT_KLAXXI_CLAW,
    EVENT_KLAXXI_SWIPE,
    EVENT_KLAXXI_STING,
    EVENT_KLAXXI_BLOODLETTING,

    EVENT_GARROSH_HAMSTRING,
    EVENT_GARROSH_IRON_STAR_IMPACT,
    EVENT_GARROSH_ANNIHILATE,
    EVENT_GARROSH_CRUSHING_FEAR,

    EVENT_BLOOD_AXE_CLEAVE,
    EVENT_BLOOD_AXE_MORTAL_STRIKE,

    EVENT_GRUNT_WHIRLWIND,
    EVENT_GRUNT_ENRAGE
};

class SiegeOfOrgrimmarInstanceScript : public InstanceScript
{
public:
    explicit SiegeOfOrgrimmarInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(SiegeOfOrgrimmarEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new SiegeOfOrgrimmarInstanceScript(pMapMgr); }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        if (pGameObject->getEntry() == GO_NAZGRIM_ENTRY_DOOR)
            mNazgrimDoorGuids.push_back(pGameObject->getGuidLow());
        else if (pGameObject->getEntry() == GO_GARROSH_ENCOUNTER_SHA_DOOR)
            mGarroshDoorGuids.push_back(pGameObject->getGuidLow());
    }

    void OpenNazgrimDoor()
    {
        for (uint32_t guid : mNazgrimDoorGuids)
        {
            if (GameObject* pDoor = GetGameObjectByGuid(guid))
                useDoorOrButton(pDoor);
        }
    }

    void OpenGarroshDoor()
    {
        for (uint32_t guid : mGarroshDoorGuids)
        {
            if (GameObject* pDoor = GetGameObjectByGuid(guid))
                useDoorOrButton(pDoor);
        }
    }

private:
    std::vector<uint32_t> mNazgrimDoorGuids;
    std::vector<uint32_t> mGarroshDoorGuids;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Immerseus

class ImmerseusAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ImmerseusAI(c); }
    explicit ImmerseusAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_IMMERSEUS, EncounterStates::InProgress);
        scriptEvents.addEvent(EVENT_IMMERSEUS_SHA_BOLT, 5000);
        scriptEvents.addEvent(EVENT_IMMERSEUS_SWIRL, 12000);
        scriptEvents.addEvent(EVENT_IMMERSEUS_CORROSIVE_BLAST, 18000);
        scriptEvents.addEvent(EVENT_IMMERSEUS_ERUPTING_SHA, 24000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_IMMERSEUS, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_IMMERSEUS, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;
        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_IMMERSEUS_SHA_BOLT:
                castSpellOnVictim(SPELL_IMMERSEUS_SHA_BOLT);
                scriptEvents.addEvent(EVENT_IMMERSEUS_SHA_BOLT, 14000);
                break;
            case EVENT_IMMERSEUS_SWIRL:
                castSpellAOE(SPELL_IMMERSEUS_SWIRL);
                scriptEvents.addEvent(EVENT_IMMERSEUS_SWIRL, 22000);
                break;
            case EVENT_IMMERSEUS_CORROSIVE_BLAST:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_IMMERSEUS_CORROSIVE_BLAST);
                scriptEvents.addEvent(EVENT_IMMERSEUS_CORROSIVE_BLAST, 20000);
                break;
            case EVENT_IMMERSEUS_ERUPTING_SHA:
                castSpellOnSelf(SPELL_IMMERSEUS_ERUPTING_SHA);
                scriptEvents.addEvent(EVENT_IMMERSEUS_ERUPTING_SHA, 28000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// The Fallen Protectors

class FallenProtectorsAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new FallenProtectorsAI(c); }
    explicit FallenProtectorsAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_FALLEN_PROTECTORS, EncounterStates::InProgress);
        scriptEvents.addEvent(EVENT_PROTECTORS_INFERNO_STRIKE, 6000);
        scriptEvents.addEvent(EVENT_PROTECTORS_CORRUPTION_SHOCK, 14000);
        scriptEvents.addEvent(EVENT_PROTECTORS_CORRUPTED_BREW, 20000);
        scriptEvents.addEvent(EVENT_PROTECTORS_CORRUPTION_KICK, 10000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_FALLEN_PROTECTORS, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_FALLEN_PROTECTORS, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;
        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_PROTECTORS_INFERNO_STRIKE:
                castSpellOnVictim(SPELL_PROTECTORS_INFERNO_STRIKE);
                scriptEvents.addEvent(EVENT_PROTECTORS_INFERNO_STRIKE, 18000);
                break;
            case EVENT_PROTECTORS_CORRUPTION_SHOCK:
                castSpellAOE(SPELL_PROTECTORS_CORRUPTION_SHOCK);
                scriptEvents.addEvent(EVENT_PROTECTORS_CORRUPTION_SHOCK, 24000);
                break;
            case EVENT_PROTECTORS_CORRUPTED_BREW:
                castSpellOnSelf(SPELL_PROTECTORS_CORRUPTED_BREW);
                scriptEvents.addEvent(EVENT_PROTECTORS_CORRUPTED_BREW, 26000);
                break;
            case EVENT_PROTECTORS_CORRUPTION_KICK:
                castSpellOnVictim(SPELL_PROTECTORS_CORRUPTION_KICK);
                scriptEvents.addEvent(EVENT_PROTECTORS_CORRUPTION_KICK, 16000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Norushen

class NorushenAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NorushenAI(c); }
    explicit NorushenAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_NORUSHEN, EncounterStates::InProgress);
        scriptEvents.addEvent(EVENT_NORUSHEN_CORRUPTION, 8000);
        scriptEvents.addEvent(EVENT_NORUSHEN_UNLEASHED_ANGER, 16000);
        scriptEvents.addEvent(EVENT_NORUSHEN_LINGERING_CORRUPTION, 22000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_NORUSHEN, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_NORUSHEN, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;
        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_NORUSHEN_CORRUPTION:
                castSpellOnVictim(SPELL_NORUSHEN_CORRUPTION);
                scriptEvents.addEvent(EVENT_NORUSHEN_CORRUPTION, 18000);
                break;
            case EVENT_NORUSHEN_UNLEASHED_ANGER:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_NORUSHEN_UNLEASHED_ANGER);
                scriptEvents.addEvent(EVENT_NORUSHEN_UNLEASHED_ANGER, 24000);
                break;
            case EVENT_NORUSHEN_LINGERING_CORRUPTION:
                castSpellAOE(SPELL_NORUSHEN_LINGERING_CORRUPTION);
                scriptEvents.addEvent(EVENT_NORUSHEN_LINGERING_CORRUPTION, 26000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Sha of Pride

class ShaOfPrideAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ShaOfPrideAI(c); }
    explicit ShaOfPrideAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "Your pride feeds me!");
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_SHA_OF_PRIDE, EncounterStates::InProgress);
        scriptEvents.addEvent(EVENT_PRIDE_MOCKING_BLAST, 6000);
        scriptEvents.addEvent(EVENT_PRIDE_LAST_WORD, 14000);
        scriptEvents.addEvent(EVENT_PRIDE_WOUNDED_PRIDE, 20000);
        scriptEvents.addEvent(EVENT_PRIDE_BURSTING_PRIDE, 26000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_SHA_OF_PRIDE, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_SHA_OF_PRIDE, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;
        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_PRIDE_MOCKING_BLAST:
                castSpellOnVictim(SPELL_PRIDE_MOCKING_BLAST);
                scriptEvents.addEvent(EVENT_PRIDE_MOCKING_BLAST, 18000);
                break;
            case EVENT_PRIDE_LAST_WORD:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_PRIDE_LAST_WORD);
                scriptEvents.addEvent(EVENT_PRIDE_LAST_WORD, 22000);
                break;
            case EVENT_PRIDE_WOUNDED_PRIDE:
                castSpellOnVictim(SPELL_PRIDE_WOUNDED_PRIDE);
                scriptEvents.addEvent(EVENT_PRIDE_WOUNDED_PRIDE, 20000);
                break;
            case EVENT_PRIDE_BURSTING_PRIDE:
                castSpellAOE(SPELL_PRIDE_BURSTING_PRIDE);
                scriptEvents.addEvent(EVENT_PRIDE_BURSTING_PRIDE, 28000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Galakras

class GalakrasAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new GalakrasAI(c); }
    explicit GalakrasAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_GALAKRAS, EncounterStates::InProgress);
        scriptEvents.addEvent(EVENT_GALAKRAS_THUNDER_CLAP, 8000);
        scriptEvents.addEvent(EVENT_GALAKRAS_ARCING_SMASH, 16000);
        scriptEvents.addEvent(EVENT_GALAKRAS_FLAMESTRIKE, 22000);
        scriptEvents.addEvent(EVENT_GALAKRAS_POISON_CLOUD, 12000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_GALAKRAS, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_GALAKRAS, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;
        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_GALAKRAS_THUNDER_CLAP:
                castSpellAOE(SPELL_GALAKRAS_THUNDER_CLAP);
                scriptEvents.addEvent(EVENT_GALAKRAS_THUNDER_CLAP, 20000);
                break;
            case EVENT_GALAKRAS_ARCING_SMASH:
                castSpellOnVictim(SPELL_GALAKRAS_ARCING_SMASH);
                scriptEvents.addEvent(EVENT_GALAKRAS_ARCING_SMASH, 18000);
                break;
            case EVENT_GALAKRAS_FLAMESTRIKE:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_GALAKRAS_FLAMESTRIKE);
                scriptEvents.addEvent(EVENT_GALAKRAS_FLAMESTRIKE, 24000);
                break;
            case EVENT_GALAKRAS_POISON_CLOUD:
                castSpellOnVictim(SPELL_GALAKRAS_POISON_CLOUD);
                scriptEvents.addEvent(EVENT_GALAKRAS_POISON_CLOUD, 22000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Iron Juggernaut

class IronJuggernautAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new IronJuggernautAI(c); }
    explicit IronJuggernautAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_IRON_JUGGERNAUT, EncounterStates::InProgress);
        scriptEvents.addEvent(EVENT_JUGGERNAUT_LASER_BURN, 6000);
        scriptEvents.addEvent(EVENT_JUGGERNAUT_FLAME_VENTS, 14000);
        scriptEvents.addEvent(EVENT_JUGGERNAUT_SHOCK_PULSE, 20000);
        scriptEvents.addEvent(EVENT_JUGGERNAUT_MORTAR_BARRAGE, 26000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_IRON_JUGGERNAUT, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_IRON_JUGGERNAUT, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;
        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_JUGGERNAUT_LASER_BURN:
                castSpellOnVictim(SPELL_JUGGERNAUT_LASER_BURN);
                scriptEvents.addEvent(EVENT_JUGGERNAUT_LASER_BURN, 20000);
                break;
            case EVENT_JUGGERNAUT_FLAME_VENTS:
                castSpellAOE(SPELL_JUGGERNAUT_FLAME_VENTS);
                scriptEvents.addEvent(EVENT_JUGGERNAUT_FLAME_VENTS, 24000);
                break;
            case EVENT_JUGGERNAUT_SHOCK_PULSE:
                castSpellOnSelf(SPELL_JUGGERNAUT_SHOCK_PULSE);
                scriptEvents.addEvent(EVENT_JUGGERNAUT_SHOCK_PULSE, 26000);
                break;
            case EVENT_JUGGERNAUT_MORTAR_BARRAGE:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_JUGGERNAUT_MORTAR_BARRAGE);
                scriptEvents.addEvent(EVENT_JUGGERNAUT_MORTAR_BARRAGE, 28000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Kor'kron Dark Shaman

class KorkronDarkShamanAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new KorkronDarkShamanAI(c); }
    explicit KorkronDarkShamanAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_KORKRON_DARK_SHAMAN, EncounterStates::InProgress);
        scriptEvents.addEvent(EVENT_SHAMAN_FROSTSTORM_BOLT, 6000);
        scriptEvents.addEvent(EVENT_SHAMAN_TOXIC_STORM, 14000);
        scriptEvents.addEvent(EVENT_SHAMAN_IRON_TOMB, 20000);
        scriptEvents.addEvent(EVENT_SHAMAN_FOUL_GEYSER, 26000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_KORKRON_DARK_SHAMAN, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_KORKRON_DARK_SHAMAN, EncounterStates::Performed);
        static_cast<SiegeOfOrgrimmarInstanceScript*>(getInstanceScript())->OpenNazgrimDoor();
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;
        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_SHAMAN_FROSTSTORM_BOLT:
                castSpellOnVictim(SPELL_SHAMAN_FROSTSTORM_BOLT);
                scriptEvents.addEvent(EVENT_SHAMAN_FROSTSTORM_BOLT, 18000);
                break;
            case EVENT_SHAMAN_TOXIC_STORM:
                castSpellAOE(SPELL_SHAMAN_TOXIC_STORM);
                scriptEvents.addEvent(EVENT_SHAMAN_TOXIC_STORM, 24000);
                break;
            case EVENT_SHAMAN_IRON_TOMB:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_SHAMAN_IRON_TOMB);
                scriptEvents.addEvent(EVENT_SHAMAN_IRON_TOMB, 22000);
                break;
            case EVENT_SHAMAN_FOUL_GEYSER:
                castSpellOnVictim(SPELL_SHAMAN_FOUL_GEYSER);
                scriptEvents.addEvent(EVENT_SHAMAN_FOUL_GEYSER, 26000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// General Nazgrim

class GeneralNazgrimAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new GeneralNazgrimAI(c); }
    explicit GeneralNazgrimAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "For the Horde!");
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_GENERAL_NAZGRIM, EncounterStates::InProgress);
        scriptEvents.addEvent(EVENT_NAZGRIM_SUNDERING_BLOW, 6000);
        scriptEvents.addEvent(EVENT_NAZGRIM_BONECRACKER, 14000);
        scriptEvents.addEvent(EVENT_NAZGRIM_HEROIC_SHOCKWAVE, 20000);
        scriptEvents.addEvent(EVENT_NAZGRIM_RAVAGER, 26000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_GENERAL_NAZGRIM, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "A warrior's death... at last...");
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_GENERAL_NAZGRIM, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;
        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_NAZGRIM_SUNDERING_BLOW:
                castSpellOnVictim(SPELL_NAZGRIM_SUNDERING_BLOW);
                scriptEvents.addEvent(EVENT_NAZGRIM_SUNDERING_BLOW, 18000);
                break;
            case EVENT_NAZGRIM_BONECRACKER:
                castSpellOnVictim(SPELL_NAZGRIM_BONECRACKER);
                scriptEvents.addEvent(EVENT_NAZGRIM_BONECRACKER, 20000);
                break;
            case EVENT_NAZGRIM_HEROIC_SHOCKWAVE:
                castSpellAOE(SPELL_NAZGRIM_HEROIC_SHOCKWAVE);
                scriptEvents.addEvent(EVENT_NAZGRIM_HEROIC_SHOCKWAVE, 26000);
                break;
            case EVENT_NAZGRIM_RAVAGER:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_NAZGRIM_RAVAGER);
                scriptEvents.addEvent(EVENT_NAZGRIM_RAVAGER, 28000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Malkorok

class MalkorokAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MalkorokAI(c); }
    explicit MalkorokAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "Y'Shaarj's will be done!");
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_MALKOROK, EncounterStates::InProgress);
        scriptEvents.addEvent(EVENT_MALKOROK_BREATH_OF_YSHAARJ, 10000);
        scriptEvents.addEvent(EVENT_MALKOROK_ARCING_SMASH, 6000);
        scriptEvents.addEvent(EVENT_MALKOROK_SEISMIC_SLAM, 16000);
        scriptEvents.addEvent(EVENT_MALKOROK_BLOOD_RAGE, 24000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_MALKOROK, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_MALKOROK, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;
        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_MALKOROK_BREATH_OF_YSHAARJ:
                castSpellAOE(SPELL_MALKOROK_BREATH_OF_YSHAARJ);
                scriptEvents.addEvent(EVENT_MALKOROK_BREATH_OF_YSHAARJ, 26000);
                break;
            case EVENT_MALKOROK_ARCING_SMASH:
                castSpellOnVictim(SPELL_MALKOROK_ARCING_SMASH);
                scriptEvents.addEvent(EVENT_MALKOROK_ARCING_SMASH, 18000);
                break;
            case EVENT_MALKOROK_SEISMIC_SLAM:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_MALKOROK_SEISMIC_SLAM);
                scriptEvents.addEvent(EVENT_MALKOROK_SEISMIC_SLAM, 22000);
                break;
            case EVENT_MALKOROK_BLOOD_RAGE:
                castSpellOnSelf(SPELL_MALKOROK_BLOOD_RAGE);
                scriptEvents.addEvent(EVENT_MALKOROK_BLOOD_RAGE, 30000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Spoils of Pandaria (see header comment - non-combat minigame in the real fight)

class SpoilsOfPandariaAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SpoilsOfPandariaAI(c); }
    explicit SpoilsOfPandariaAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_SPOILS_OF_PANDARIA, EncounterStates::InProgress);
        scriptEvents.addEvent(EVENT_SPOILS_CRUSH, 8000);
        scriptEvents.addEvent(EVENT_SPOILS_GUARD_STANCE, 20000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_SPOILS_OF_PANDARIA, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_SPOILS_OF_PANDARIA, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;
        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_SPOILS_CRUSH:
                castSpellOnVictim(SPELL_SPOILS_CRUSH);
                scriptEvents.addEvent(EVENT_SPOILS_CRUSH, 14000);
                break;
            case EVENT_SPOILS_GUARD_STANCE:
                castSpellOnSelf(SPELL_SPOILS_GUARD_STANCE);
                scriptEvents.addEvent(EVENT_SPOILS_GUARD_STANCE, 26000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Thok the Bloodthirsty

class ThokAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ThokAI(c); }
    explicit ThokAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_THOK, EncounterStates::InProgress);
        scriptEvents.addEvent(EVENT_THOK_FEARSOME_ROAR, 10000);
        scriptEvents.addEvent(EVENT_THOK_DEAFENING_SCREECH, 18000);
        scriptEvents.addEvent(EVENT_THOK_ACID_BREATH, 6000);
        scriptEvents.addEvent(EVENT_THOK_WRECKING_BALL, 24000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_THOK, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_THOK, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;
        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_THOK_FEARSOME_ROAR:
                castSpellAOE(SPELL_THOK_FEARSOME_ROAR);
                scriptEvents.addEvent(EVENT_THOK_FEARSOME_ROAR, 28000);
                break;
            case EVENT_THOK_DEAFENING_SCREECH:
                castSpellOnSelf(SPELL_THOK_DEAFENING_SCREECH);
                scriptEvents.addEvent(EVENT_THOK_DEAFENING_SCREECH, 26000);
                break;
            case EVENT_THOK_ACID_BREATH:
                castSpellOnVictim(SPELL_THOK_ACID_BREATH);
                scriptEvents.addEvent(EVENT_THOK_ACID_BREATH, 18000);
                break;
            case EVENT_THOK_WRECKING_BALL:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_THOK_WRECKING_BALL);
                scriptEvents.addEvent(EVENT_THOK_WRECKING_BALL, 22000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Siegecrafter Blackfuse

class SiegecrafterBlackfuseAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SiegecrafterBlackfuseAI(c); }
    explicit SiegecrafterBlackfuseAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_SIEGECRAFTER_BLACKFUSE, EncounterStates::InProgress);
        scriptEvents.addEvent(EVENT_BLACKFUSE_LAUNCH_SAWBLADE, 8000);
        scriptEvents.addEvent(EVENT_BLACKFUSE_SERRATED_SLASH, 14000);
        scriptEvents.addEvent(EVENT_BLACKFUSE_ELECTROSTATIC_CHARGE, 20000);
        scriptEvents.addEvent(EVENT_BLACKFUSE_DEATH_FROM_ABOVE, 26000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_SIEGECRAFTER_BLACKFUSE, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_SIEGECRAFTER_BLACKFUSE, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;
        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_BLACKFUSE_LAUNCH_SAWBLADE:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_BLACKFUSE_LAUNCH_SAWBLADE);
                scriptEvents.addEvent(EVENT_BLACKFUSE_LAUNCH_SAWBLADE, 20000);
                break;
            case EVENT_BLACKFUSE_SERRATED_SLASH:
                castSpellOnVictim(SPELL_BLACKFUSE_SERRATED_SLASH);
                scriptEvents.addEvent(EVENT_BLACKFUSE_SERRATED_SLASH, 18000);
                break;
            case EVENT_BLACKFUSE_ELECTROSTATIC_CHARGE:
                castSpellAOE(SPELL_BLACKFUSE_ELECTROSTATIC_CHARGE);
                scriptEvents.addEvent(EVENT_BLACKFUSE_ELECTROSTATIC_CHARGE, 24000);
                break;
            case EVENT_BLACKFUSE_DEATH_FROM_ABOVE:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_BLACKFUSE_DEATH_FROM_ABOVE);
                scriptEvents.addEvent(EVENT_BLACKFUSE_DEATH_FROM_ABOVE, 28000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Paragons of the Klaxxi

class ParagonsOfTheKlaxxiAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ParagonsOfTheKlaxxiAI(c); }
    explicit ParagonsOfTheKlaxxiAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_PARAGONS_OF_THE_KLAXXI, EncounterStates::InProgress);
        scriptEvents.addEvent(EVENT_KLAXXI_CLAW, 6000);
        scriptEvents.addEvent(EVENT_KLAXXI_SWIPE, 12000);
        scriptEvents.addEvent(EVENT_KLAXXI_STING, 18000);
        scriptEvents.addEvent(EVENT_KLAXXI_BLOODLETTING, 24000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_PARAGONS_OF_THE_KLAXXI, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_PARAGONS_OF_THE_KLAXXI, EncounterStates::Performed);
        static_cast<SiegeOfOrgrimmarInstanceScript*>(getInstanceScript())->OpenGarroshDoor();
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;
        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_KLAXXI_CLAW:
                castSpellOnVictim(SPELL_KLAXXI_CLAW);
                scriptEvents.addEvent(EVENT_KLAXXI_CLAW, 16000);
                break;
            case EVENT_KLAXXI_SWIPE:
                castSpellAOE(SPELL_KLAXXI_SWIPE);
                scriptEvents.addEvent(EVENT_KLAXXI_SWIPE, 20000);
                break;
            case EVENT_KLAXXI_STING:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_KLAXXI_STING);
                scriptEvents.addEvent(EVENT_KLAXXI_STING, 18000);
                break;
            case EVENT_KLAXXI_BLOODLETTING:
                castSpellOnVictim(SPELL_KLAXXI_BLOODLETTING);
                scriptEvents.addEvent(EVENT_KLAXXI_BLOODLETTING, 22000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Garrosh Hellscream

class GarroshHellscreamAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new GarroshHellscreamAI(c); }
    explicit GarroshHellscreamAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "Mine is the vengeance of Y'Shaarj!");
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_GARROSH_HELLSCREAM, EncounterStates::InProgress);
        scriptEvents.addEvent(EVENT_GARROSH_HAMSTRING, 6000);
        scriptEvents.addEvent(EVENT_GARROSH_IRON_STAR_IMPACT, 14000);
        scriptEvents.addEvent(EVENT_GARROSH_ANNIHILATE, 22000);
        scriptEvents.addEvent(EVENT_GARROSH_CRUSHING_FEAR, 28000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_GARROSH_HELLSCREAM, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "Heh heh heh... heh...");
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_GARROSH_HELLSCREAM, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;
        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_GARROSH_HAMSTRING:
                castSpellOnVictim(SPELL_GARROSH_HAMSTRING);
                scriptEvents.addEvent(EVENT_GARROSH_HAMSTRING, 18000);
                break;
            case EVENT_GARROSH_IRON_STAR_IMPACT:
                castSpellAOE(SPELL_GARROSH_IRON_STAR_IMPACT);
                scriptEvents.addEvent(EVENT_GARROSH_IRON_STAR_IMPACT, 26000);
                break;
            case EVENT_GARROSH_ANNIHILATE:
                castSpellOnVictim(SPELL_GARROSH_ANNIHILATE);
                scriptEvents.addEvent(EVENT_GARROSH_ANNIHILATE, 24000);
                break;
            case EVENT_GARROSH_CRUSHING_FEAR:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_GARROSH_CRUSHING_FEAR);
                scriptEvents.addEvent(EVENT_GARROSH_CRUSHING_FEAR, 30000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Trash - Kor'kron Blood Axe

class KorkronBloodAxeAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new KorkronBloodAxeAI(c); }
    explicit KorkronBloodAxeAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_BLOOD_AXE_CLEAVE, 5000);
        scriptEvents.addEvent(EVENT_BLOOD_AXE_MORTAL_STRIKE, 10000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_BLOOD_AXE_CLEAVE:
                castSpellOnVictim(SPELL_BLOOD_AXE_CLEAVE);
                scriptEvents.addEvent(EVENT_BLOOD_AXE_CLEAVE, 12000);
                break;
            case EVENT_BLOOD_AXE_MORTAL_STRIKE:
                castSpellOnVictim(SPELL_BLOOD_AXE_MORTAL_STRIKE);
                scriptEvents.addEvent(EVENT_BLOOD_AXE_MORTAL_STRIKE, 16000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Trash - Kor'kron Grunt

class KorkronGruntAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new KorkronGruntAI(c); }
    explicit KorkronGruntAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_GRUNT_WHIRLWIND, 8000);
        scriptEvents.addEvent(EVENT_GRUNT_ENRAGE, 16000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_GRUNT_WHIRLWIND:
                castSpellAOE(SPELL_GRUNT_WHIRLWIND);
                scriptEvents.addEvent(EVENT_GRUNT_WHIRLWIND, 18000);
                break;
            case EVENT_GRUNT_ENRAGE:
                castSpellOnSelf(SPELL_GRUNT_ENRAGE);
                scriptEvents.addEvent(EVENT_GRUNT_ENRAGE, 26000);
                break;
            default:
                break;
        }
    }
};

void SetupSiegeOfOrgrimmar(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_SIEGE_OF_ORGRIMMAR, &SiegeOfOrgrimmarInstanceScript::Create);

    mgr->register_creature_script(BOSS_IMMERSEUS, &ImmerseusAI::Create);
    mgr->register_creature_script(BOSS_ROOK_STONETOE, &FallenProtectorsAI::Create);
    mgr->register_creature_script(BOSS_NORUSHEN, &NorushenAI::Create);
    mgr->register_creature_script(BOSS_SHA_OF_PRIDE, &ShaOfPrideAI::Create);
    mgr->register_creature_script(BOSS_GALAKRAS, &GalakrasAI::Create);
    mgr->register_creature_script(BOSS_IRON_JUGGERNAUT, &IronJuggernautAI::Create);
    mgr->register_creature_script(BOSS_EARTHBREAKER_HAROMM, &KorkronDarkShamanAI::Create);
    mgr->register_creature_script(BOSS_GENERAL_NAZGRIM, &GeneralNazgrimAI::Create);
    mgr->register_creature_script(BOSS_MALKOROK, &MalkorokAI::Create);
    mgr->register_creature_script(BOSS_SPOILS_OF_PANDARIA, &SpoilsOfPandariaAI::Create);
    mgr->register_creature_script(BOSS_THOK, &ThokAI::Create);
    mgr->register_creature_script(BOSS_SIEGECRAFTER_BLACKFUSE, &SiegecrafterBlackfuseAI::Create);
    mgr->register_creature_script(BOSS_SKEER_THE_BLOODSEEKER, &ParagonsOfTheKlaxxiAI::Create);
    mgr->register_creature_script(BOSS_GARROSH_HELLSCREAM, &GarroshHellscreamAI::Create);

    mgr->register_creature_script(NPC_KORKRON_BLOOD_AXE, &KorkronBloodAxeAI::Create);
    mgr->register_creature_script(NPC_KORKRON_GRUNT, &KorkronGruntAI::Create);
}
