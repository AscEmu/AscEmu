/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

// Lost City of the Tolvir - four encounters (verified against wowhead), hand-ported into
// AscEmu's own CreatureAIScript / InstanceScript framework.
//
// Simplifications versus the original fight design: each of these four fights has a
// multi-phase or vehicle-grab gimmick (Husam's landmine field and "throw player" grab,
// Lockmaw/Augh's stealth-and-ambush phase transition, Barim's Repentance soul-fragment
// phase, Siamat's "unleashed" platform transformation). All four are ported as a single
// continuous rotation built from each fight's real abilities, dropping the phase gimmick.

#include "Setup.h"
#include "Instance_Lost_City_Of_Tolvir.hpp"

#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

enum LostCityOfTolvirEvents
{
    EVENT_HUSAM_HAMMER_FIST = 1,
    EVENT_HUSAM_MYSTIC_TRAP,
    EVENT_HUSAM_BAD_INTENTIONS,
    EVENT_HUSAM_SHOCKWAVE,

    EVENT_LOCKMAW_VISCOUS_POISON,
    EVENT_LOCKMAW_SCENT_OF_BLOOD,
    EVENT_LOCKMAW_DUST_FLAIL,

    EVENT_AUGH_PARALYTIC_BLOW_DART,
    EVENT_AUGH_WHIRLWIND,
    EVENT_AUGH_DRAGONS_BREATH,

    EVENT_BARIM_FIFTY_LASHINGS,
    EVENT_BARIM_PLAGUE_OF_AGES,
    EVENT_BARIM_HEAVENS_FURY,

    EVENT_SIAMAT_STORM_BOLT,
    EVENT_SIAMAT_DEFLECTING_WINDS,
    EVENT_SIAMAT_CLOUD_BURST,
    EVENT_SIAMAT_CALL_OF_SKY,

    EVENT_PYGMY_SCOUT_PULSE,
    EVENT_PYGMY_FIREBREATHER_BREATH,
    EVENT_PYGMY_FIREBREATHER_FLAME_JET,
    EVENT_AXEMASTER_CLEAVE,
    EVENT_AXEMASTER_REND,
    EVENT_MYRMIDON_SHIELD_BASH,
    EVENT_OATHSWORN_BLEED_PULSE,
    EVENT_WANDERER_PIERCING_STAB,
    EVENT_PATHFINDER_AIMED_SHOT,
    EVENT_PLAGUEBRINGER_DISEASE_CLOUD,
    EVENT_PLAGUEBRINGER_INFECTED_BITE,
    EVENT_TORTURER_WHIP,
    EVENT_TORTURER_MUTILATE,
    EVENT_THEURGIST_LIGHTNING_BOLT,
    EVENT_THEURGIST_CHAIN_LIGHTNING,
    EVENT_THEURGIST_STATIC_CHARGE,
    EVENT_SKINNER_SKIN,
    EVENT_SKINNER_GUT_HOOK,
    EVENT_DARKCASTER_SHADOW_BOLT_PULSE,
    EVENT_DARKCASTER_CURSE,
    EVENT_DARKCASTER_DRAIN_LIFE,
    EVENT_SCORPID_KEEPER_MEND_PET,
    EVENT_SCORPID_KEEPER_ENVENOM,
    EVENT_TOLVIR_MERCHANT_GUARD,
    EVENT_CAPTAIN_BATTLE_SHOUT,
    EVENT_CAPTAIN_MORTAL_STRIKE
};

class LostCityOfTolvirInstanceScript : public InstanceScript
{
public:
    explicit LostCityOfTolvirInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(LostCityOfTolvirEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new LostCityOfTolvirInstanceScript(pMapMgr); }
};

//////////////////////////////////////////////////////////////////////////////////////////
// General Husam

class GeneralHusamAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new GeneralHusamAI(c); }
    explicit GeneralHusamAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "Invaders, you shall go no further!");
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_GENERAL_HUSAM, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_HUSAM_HAMMER_FIST, 7000);
        scriptEvents.addEvent(EVENT_HUSAM_MYSTIC_TRAP, 7000);
        scriptEvents.addEvent(EVENT_HUSAM_BAD_INTENTIONS, 12000);
        scriptEvents.addEvent(EVENT_HUSAM_SHOCKWAVE, isHeroic() ? 15000 : 18000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_GENERAL_HUSAM, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnTargetDied(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "Insolent rat!");
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "Siamat must not be freed! Turn back before it is too late!");
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_GENERAL_HUSAM, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_HUSAM_HAMMER_FIST:
                castSpellOnVictim(SPELL_HUSAM_HAMMER_FIST);
                scriptEvents.addEvent(EVENT_HUSAM_HAMMER_FIST, 11000);
                break;
            case EVENT_HUSAM_MYSTIC_TRAP:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_HUSAM_MYSTIC_TRAP);
                scriptEvents.addEvent(EVENT_HUSAM_MYSTIC_TRAP, 9000);
                break;
            case EVENT_HUSAM_BAD_INTENTIONS:
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "Tread Lightly.");
                castSpellAOE(SPELL_HUSAM_BAD_INTENTIONS);
                scriptEvents.addEvent(EVENT_HUSAM_BAD_INTENTIONS, 17000);
                break;
            case EVENT_HUSAM_SHOCKWAVE:
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "Murkash!");
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "General Husam begins to cast Shockwave!");
                castSpellOnSelf(SPELL_HUSAM_SHOCKWAVE_VISUAL);
                castSpellAOE(SPELL_HUSAM_SHOCKWAVE_VISUAL);
                scriptEvents.addEvent(EVENT_HUSAM_SHOCKWAVE, isHeroic() ? 20000 : 24000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Lockmaw & Augh

class LockmawAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new LockmawAI(c); }
    explicit LockmawAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_LOCKMAW_AND_AUGH, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_LOCKMAW_VISCOUS_POISON, 6000);
        scriptEvents.addEvent(EVENT_LOCKMAW_SCENT_OF_BLOOD, 6000);
        scriptEvents.addEvent(EVENT_LOCKMAW_DUST_FLAIL, 8000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_LOCKMAW_AND_AUGH, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_LOCKMAW_AND_AUGH, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_LOCKMAW_VISCOUS_POISON:
                castSpellOnVictim(SPELL_LOCKMAW_VISCOUS_POISON);
                scriptEvents.addEvent(EVENT_LOCKMAW_VISCOUS_POISON, 14000);
                break;
            case EVENT_LOCKMAW_SCENT_OF_BLOOD:
                castSpellOnSelf(getRaidModeValue(SPELL_LOCKMAW_SCENT_OF_BLOOD_10, SPELL_LOCKMAW_SCENT_OF_BLOOD_25, SPELL_LOCKMAW_SCENT_OF_BLOOD_10, SPELL_LOCKMAW_SCENT_OF_BLOOD_25));
                scriptEvents.addEvent(EVENT_LOCKMAW_SCENT_OF_BLOOD, 30000);
                break;
            case EVENT_LOCKMAW_DUST_FLAIL:
                castSpellAOE(SPELL_LOCKMAW_DUST_FLAIL);
                scriptEvents.addEvent(EVENT_LOCKMAW_DUST_FLAIL, 16000);
                break;
            default:
                break;
        }
    }
};

class AughAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AughAI(c); }
    explicit AughAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "Augh da boss! Oh yeah!");
        scriptEvents.addEvent(EVENT_AUGH_PARALYTIC_BLOW_DART, 8000);
        scriptEvents.addEvent(EVENT_AUGH_WHIRLWIND, 9000);
        scriptEvents.addEvent(EVENT_AUGH_DRAGONS_BREATH, 6000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        scriptEvents.resetEvents();
    }

    // Reference dialogue data for entry 49045 also has an "Augh appears from the distance!"
    // intro emote (no sound) played on his wandering-rare spawn cue, which has no matching
    // hook in this simplified port.
    void OnDied(Unit* /*pKiller*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "GAAAH! How you kill croc?!");
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_AUGH_PARALYTIC_BLOW_DART:
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "Augh smart! Augh already steal treasure while you no looking!");
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_AUGH_PARALYTIC_BLOW_DART);
                scriptEvents.addEvent(EVENT_AUGH_PARALYTIC_BLOW_DART, 18000);
                break;
            case EVENT_AUGH_WHIRLWIND:
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "Who bad?! Augh bad!! Ugn!");
                castSpellAOE(SPELL_AUGH_WHIRLWIND);
                scriptEvents.addEvent(EVENT_AUGH_WHIRLWIND, 20000);
                break;
            case EVENT_AUGH_DRAGONS_BREATH:
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "Augh steal your treasure. Uhn uhn uhnnn!");
                castSpellAOE(SPELL_AUGH_DRAGONS_BREATH);
                scriptEvents.addEvent(EVENT_AUGH_DRAGONS_BREATH, 22000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// High Prophet Barim

class HighProphetBarimAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HighProphetBarimAI(c); }
    explicit HighProphetBarimAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "Begone infidels, you are not welcome here!");
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_HIGH_PROPHET_BARIM, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_BARIM_FIFTY_LASHINGS, 8000);
        scriptEvents.addEvent(EVENT_BARIM_PLAGUE_OF_AGES, 8000);
        scriptEvents.addEvent(EVENT_BARIM_HEAVENS_FURY, 10000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_HIGH_PROPHET_BARIM, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "The heavens take you!");
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_HIGH_PROPHET_BARIM, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_BARIM_FIFTY_LASHINGS:
                castSpellOnVictim(SPELL_BARIM_FIFTY_LASHINGS);
                scriptEvents.addEvent(EVENT_BARIM_FIFTY_LASHINGS, 16000);
                break;
            case EVENT_BARIM_PLAGUE_OF_AGES:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_BARIM_PLAGUE_OF_AGES);
                scriptEvents.addEvent(EVENT_BARIM_PLAGUE_OF_AGES, 18000);
                break;
            case EVENT_BARIM_HEAVENS_FURY:
                castSpellAOE(SPELL_BARIM_HEAVENS_FURY);
                scriptEvents.addEvent(EVENT_BARIM_HEAVENS_FURY, 20000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Siamat

class SiamatAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SiamatAI(c); }
    explicit SiamatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "I. AM. UNLEASHED!");
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_SIAMAT, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_SIAMAT_DEFLECTING_WINDS, 5000);
        scriptEvents.addEvent(EVENT_SIAMAT_STORM_BOLT, 1500);
        scriptEvents.addEvent(EVENT_SIAMAT_CLOUD_BURST, 11000);
        scriptEvents.addEvent(EVENT_SIAMAT_CALL_OF_SKY, 13000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_SIAMAT, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "Nothing more than dust in the wind.");
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_SIAMAT, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_SIAMAT_DEFLECTING_WINDS:
                castSpellOnSelf(SPELL_SIAMAT_DEFLECTING_WINDS);
                scriptEvents.addEvent(EVENT_SIAMAT_DEFLECTING_WINDS, 25000);
                break;
            case EVENT_SIAMAT_STORM_BOLT:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_SIAMAT_STORM_BOLT);
                scriptEvents.addEvent(EVENT_SIAMAT_STORM_BOLT, 19000);
                break;
            case EVENT_SIAMAT_CLOUD_BURST:
                castSpellAOE(SPELL_SIAMAT_CLOUD_BURST);
                scriptEvents.addEvent(EVENT_SIAMAT_CLOUD_BURST, 20000);
                break;
            case EVENT_SIAMAT_CALL_OF_SKY:
                castSpellAOE(SPELL_SIAMAT_CALL_OF_SKY);
                scriptEvents.addEvent(EVENT_SIAMAT_CALL_OF_SKY, 24000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Trash

class PygmyScoutAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new PygmyScoutAI(c); }
    explicit PygmyScoutAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_PYGMY_SCOUT_PULSE, 2700);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_PYGMY_SCOUT_PULSE)
        {
            castSpellOnSelf(SPELL_PYGMY_SCOUT_PULSE);
            scriptEvents.addEvent(EVENT_PYGMY_SCOUT_PULSE, 2700);
        }
    }
};

class PygmyFirebreatherAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new PygmyFirebreatherAI(c); }
    explicit PygmyFirebreatherAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_PYGMY_FIREBREATHER_BREATH, 5000);
        scriptEvents.addEvent(EVENT_PYGMY_FIREBREATHER_FLAME_JET, 6000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_PYGMY_FIREBREATHER_BREATH:
                castSpellOnSelf(SPELL_PYGMY_FIREBREATHER_BREATH);
                scriptEvents.addEvent(EVENT_PYGMY_FIREBREATHER_BREATH, 18000);
                break;
            case EVENT_PYGMY_FIREBREATHER_FLAME_JET:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_PYGMY_FIREBREATHER_FLAME_JET);
                scriptEvents.addEvent(EVENT_PYGMY_FIREBREATHER_FLAME_JET, 6000);
                break;
            default:
                break;
        }
    }
};

class OathswornAxemasterAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new OathswornAxemasterAI(c); }
    explicit OathswornAxemasterAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_AXEMASTER_CLEAVE, 6000);
        scriptEvents.addEvent(EVENT_AXEMASTER_REND, 7200);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_AXEMASTER_CLEAVE:
                castSpellOnVictim(SPELL_AXEMASTER_CLEAVE);
                scriptEvents.addEvent(EVENT_AXEMASTER_CLEAVE, 13000);
                break;
            case EVENT_AXEMASTER_REND:
                castSpellOnVictim(SPELL_AXEMASTER_REND);
                scriptEvents.addEvent(EVENT_AXEMASTER_REND, 11000);
                break;
            default:
                break;
        }
    }
};

class OathswornMyrmidonAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new OathswornMyrmidonAI(c); }
    explicit OathswornMyrmidonAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_MYRMIDON_SHIELD_BASH, 6400);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_MYRMIDON_SHIELD_BASH)
        {
            castSpellOnSelf(SPELL_MYRMIDON_SHIELD_BASH);
            scriptEvents.addEvent(EVENT_MYRMIDON_SHIELD_BASH, 16000);
        }
    }
};

class OathswornWandererAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new OathswornWandererAI(c); }
    explicit OathswornWandererAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_OATHSWORN_BLEED_PULSE, 2000);
        scriptEvents.addEvent(EVENT_WANDERER_PIERCING_STAB, 7000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_OATHSWORN_BLEED_PULSE:
                castSpellOnVictim(SPELL_OATHSWORN_BLEED_PULSE);
                scriptEvents.addEvent(EVENT_OATHSWORN_BLEED_PULSE, 2000);
                break;
            case EVENT_WANDERER_PIERCING_STAB:
                castSpellOnVictim(SPELL_WANDERER_PIERCING_STAB);
                scriptEvents.addEvent(EVENT_WANDERER_PIERCING_STAB, 16000);
                break;
            default:
                break;
        }
    }
};

class OathswornPathfinderAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new OathswornPathfinderAI(c); }
    explicit OathswornPathfinderAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_OATHSWORN_BLEED_PULSE, 2000);
        scriptEvents.addEvent(EVENT_PATHFINDER_AIMED_SHOT, 9000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_OATHSWORN_BLEED_PULSE:
                castSpellOnVictim(SPELL_OATHSWORN_BLEED_PULSE);
                scriptEvents.addEvent(EVENT_OATHSWORN_BLEED_PULSE, 2000);
                break;
            case EVENT_PATHFINDER_AIMED_SHOT:
                castSpellOnVictim(SPELL_PATHFINDER_AIMED_SHOT);
                scriptEvents.addEvent(EVENT_PATHFINDER_AIMED_SHOT, 12000);
                break;
            default:
                break;
        }
    }
};

class NefersetPlaguebringerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NefersetPlaguebringerAI(c); }
    explicit NefersetPlaguebringerAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_PLAGUEBRINGER_DISEASE_CLOUD, 6800);
        scriptEvents.addEvent(EVENT_PLAGUEBRINGER_INFECTED_BITE, 8000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_PLAGUEBRINGER_DISEASE_CLOUD:
                castSpellOnSelf(SPELL_PLAGUEBRINGER_DISEASE_CLOUD);
                scriptEvents.addEvent(EVENT_PLAGUEBRINGER_DISEASE_CLOUD, 23000);
                break;
            case EVENT_PLAGUEBRINGER_INFECTED_BITE:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_PLAGUEBRINGER_INFECTED_BITE);
                scriptEvents.addEvent(EVENT_PLAGUEBRINGER_INFECTED_BITE, 15500);
                break;
            default:
                break;
        }
    }
};

class NefersetTorturerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NefersetTorturerAI(c); }
    explicit NefersetTorturerAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_TORTURER_WHIP, 5500);
        scriptEvents.addEvent(EVENT_TORTURER_MUTILATE, 7000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_TORTURER_WHIP:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_TORTURER_WHIP);
                scriptEvents.addEvent(EVENT_TORTURER_WHIP, 30000);
                break;
            case EVENT_TORTURER_MUTILATE:
                castSpellOnVictim(SPELL_TORTURER_MUTILATE);
                scriptEvents.addEvent(EVENT_TORTURER_MUTILATE, 25000);
                break;
            default:
                break;
        }
    }
};

class NefersetTheurgistAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NefersetTheurgistAI(c); }
    explicit NefersetTheurgistAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_THEURGIST_LIGHTNING_BOLT, 6000);
        scriptEvents.addEvent(EVENT_THEURGIST_CHAIN_LIGHTNING, 14000);
        scriptEvents.addEvent(EVENT_THEURGIST_STATIC_CHARGE, 23000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_THEURGIST_LIGHTNING_BOLT:
                castSpellOnVictim(SPELL_THEURGIST_LIGHTNING_BOLT);
                scriptEvents.addEvent(EVENT_THEURGIST_LIGHTNING_BOLT, 16000);
                break;
            case EVENT_THEURGIST_CHAIN_LIGHTNING:
                castSpellOnSelf(SPELL_THEURGIST_CHAIN_LIGHTNING);
                scriptEvents.addEvent(EVENT_THEURGIST_CHAIN_LIGHTNING, 16000);
                break;
            case EVENT_THEURGIST_STATIC_CHARGE:
                castSpellOnSelf(SPELL_THEURGIST_STATIC_CHARGE);
                scriptEvents.addEvent(EVENT_THEURGIST_STATIC_CHARGE, 16000);
                break;
            default:
                break;
        }
    }
};

class OathswornSkinnerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new OathswornSkinnerAI(c); }
    explicit OathswornSkinnerAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_SKINNER_SKIN, 5000);
        scriptEvents.addEvent(EVENT_SKINNER_GUT_HOOK, 10000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_SKINNER_SKIN:
                castSpellOnSelf(SPELL_SKINNER_SKIN);
                scriptEvents.addEvent(EVENT_SKINNER_SKIN, 15500);
                break;
            case EVENT_SKINNER_GUT_HOOK:
                castSpellOnVictim(SPELL_SKINNER_GUT_HOOK);
                scriptEvents.addEvent(EVENT_SKINNER_GUT_HOOK, 7000);
                break;
            default:
                break;
        }
    }
};

class NefersetDarkcasterAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NefersetDarkcasterAI(c); }
    explicit NefersetDarkcasterAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_DARKCASTER_SHADOW_BOLT_PULSE, 2400);
        scriptEvents.addEvent(EVENT_DARKCASTER_CURSE, 7000);
        scriptEvents.addEvent(EVENT_DARKCASTER_DRAIN_LIFE, 17000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_DARKCASTER_SHADOW_BOLT_PULSE:
                castSpellOnVictim(SPELL_DARKCASTER_SHADOW_BOLT_PULSE);
                scriptEvents.addEvent(EVENT_DARKCASTER_SHADOW_BOLT_PULSE, 2400);
                break;
            case EVENT_DARKCASTER_CURSE:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_DARKCASTER_CURSE);
                scriptEvents.addEvent(EVENT_DARKCASTER_CURSE, 16000);
                break;
            case EVENT_DARKCASTER_DRAIN_LIFE:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_DARKCASTER_DRAIN_LIFE);
                scriptEvents.addEvent(EVENT_DARKCASTER_DRAIN_LIFE, 16000);
                break;
            default:
                break;
        }
    }
};

class OathswornScorpidKeeperAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new OathswornScorpidKeeperAI(c); }
    explicit OathswornScorpidKeeperAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        castSpellOnSelf(SPELL_SCORPID_KEEPER_SUMMON);
        scriptEvents.addEvent(EVENT_SCORPID_KEEPER_MEND_PET, 8000);
        scriptEvents.addEvent(EVENT_SCORPID_KEEPER_ENVENOM, 11000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_SCORPID_KEEPER_MEND_PET:
                castSpellOnSelf(SPELL_SCORPID_KEEPER_MEND_PET);
                scriptEvents.addEvent(EVENT_SCORPID_KEEPER_MEND_PET, 17000);
                break;
            case EVENT_SCORPID_KEEPER_ENVENOM:
                castSpellOnSelf(SPELL_SCORPID_KEEPER_ENVENOM);
                scriptEvents.addEvent(EVENT_SCORPID_KEEPER_ENVENOM, 17000);
                break;
            default:
                break;
        }
    }
};

class TolvirMerchantAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TolvirMerchantAI(c); }
    explicit TolvirMerchantAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_TOLVIR_MERCHANT_GUARD, 1000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_TOLVIR_MERCHANT_GUARD)
        {
            if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                castSpell(pTarget, SPELL_TOLVIR_MERCHANT_GUARD);
            scriptEvents.addEvent(EVENT_TOLVIR_MERCHANT_GUARD, 1000);
        }
    }
};

class OathswornCaptainAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new OathswornCaptainAI(c); }
    explicit OathswornCaptainAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_CAPTAIN_BATTLE_SHOUT, 6000);
        scriptEvents.addEvent(EVENT_CAPTAIN_MORTAL_STRIKE, 6000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_CAPTAIN_BATTLE_SHOUT:
                castSpellOnSelf(SPELL_CAPTAIN_BATTLE_SHOUT);
                scriptEvents.addEvent(EVENT_CAPTAIN_BATTLE_SHOUT, 13000);
                break;
            case EVENT_CAPTAIN_MORTAL_STRIKE:
                castSpellOnVictim(SPELL_CAPTAIN_MORTAL_STRIKE);
                scriptEvents.addEvent(EVENT_CAPTAIN_MORTAL_STRIKE, 13000);
                break;
            default:
                break;
        }
    }
};

void SetupLostCityOfTolvir(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_LOST_CITY_OF_TOLVIR, &LostCityOfTolvirInstanceScript::Create);

    mgr->register_creature_script(BOSS_GENERAL_HUSAM, &GeneralHusamAI::Create);
    mgr->register_creature_script(BOSS_LOCKMAW, &LockmawAI::Create);
    mgr->register_creature_script(BOSS_AUGH, &AughAI::Create);
    mgr->register_creature_script(BOSS_HIGH_PROPHET_BARIM, &HighProphetBarimAI::Create);
    mgr->register_creature_script(BOSS_SIAMAT, &SiamatAI::Create);

    mgr->register_creature_script(NPC_PYGMY_SCOUT, &PygmyScoutAI::Create);
    mgr->register_creature_script(NPC_PYGMY_FIREBREATHER, &PygmyFirebreatherAI::Create);
    mgr->register_creature_script(NPC_OATHSWORN_AXEMASTER, &OathswornAxemasterAI::Create);
    mgr->register_creature_script(NPC_OATHSWORN_MYRMIDON, &OathswornMyrmidonAI::Create);
    mgr->register_creature_script(NPC_OATHSWORN_WANDERER, &OathswornWandererAI::Create);
    mgr->register_creature_script(NPC_OATHSWORN_PATHFINDER, &OathswornPathfinderAI::Create);
    mgr->register_creature_script(NPC_NEFERSET_PLAGUEBRINGER, &NefersetPlaguebringerAI::Create);
    mgr->register_creature_script(NPC_NEFERSET_TORTURER, &NefersetTorturerAI::Create);
    mgr->register_creature_script(NPC_NEFERSET_THEURGIST, &NefersetTheurgistAI::Create);
    mgr->register_creature_script(NPC_OATHSWORN_SKINNER, &OathswornSkinnerAI::Create);
    mgr->register_creature_script(NPC_NEFERSET_DARKCASTER, &NefersetDarkcasterAI::Create);
    mgr->register_creature_script(NPC_OATHSWORN_SCORPID_KEEPER, &OathswornScorpidKeeperAI::Create);
    mgr->register_creature_script(NPC_TOLVIR_MERCHANT, &TolvirMerchantAI::Create);
    mgr->register_creature_script(NPC_OATHSWORN_CAPTAIN, &OathswornCaptainAI::Create);
}
