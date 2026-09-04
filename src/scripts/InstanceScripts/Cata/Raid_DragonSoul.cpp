/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

// Dragon Soul - eight encounters. Morchok, Warlord Zon'ozz, Yor'sahj, Hagara, Ultraxion and
// Warmaster Blackhorn have no reference C++ script to port at all (see header comment) - their
// simplified rotations here are built directly from wowhead's ability listings rather than a
// ported script. Madness of Deathwing is a real but extremely elaborate multi-platform,
// quicktime-style finale; only its core damage-dealing spells are kept, everything phase- and
// movement-related is dropped. Spine of Deathwing has no fightable boss creature in the data
// (an add/tendon-based platform-assault phase) and isn't scripted - only its slot is reserved
// so the encounter count/save-data layout matches the real instance. Doors/encounter gating
// are not tracked.

#include "Setup.h"
#include "Raid_DragonSoul.hpp"

#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

enum DragonSoulEvents
{
    EVENT_MORCHOK_STOMP = 1,
    EVENT_MORCHOK_CRUSH_ARMOR,
    EVENT_MORCHOK_BLACK_BLOOD_OF_THE_EARTH,

    EVENT_ZONOZZ_PSYCHIC_DRAIN,
    EVENT_ZONOZZ_DISRUPTING_SHADOWS,
    EVENT_ZONOZZ_FOCUSED_ANGER,

    EVENT_YORSAHJ_VOID_BOLT,
    EVENT_YORSAHJ_SEARING_BLOOD,

    EVENT_HAGARA_ICE_LANCE,
    EVENT_HAGARA_FROZEN_TEMPEST,
    EVENT_HAGARA_LIGHTNING_STORM,

    EVENT_ULTRAXION_FADING_LIGHT,
    EVENT_ULTRAXION_HOUR_OF_TWILIGHT,

    EVENT_BLACKHORN_DEVASTATE,
    EVENT_BLACKHORN_DISRUPTING_ROAR,
    EVENT_BLACKHORN_SHOCKWAVE,

    EVENT_DEATHWING_CATACLYSM,
    EVENT_DEATHWING_ELEMENTIUM_BOLT,

    EVENT_WYRMREST_PROTECTOR_TAIL_SWEEP,
    EVENT_WYRMREST_PROTECTOR_CLEAVE,
    EVENT_WYRMREST_PROTECTOR_GORE,
    EVENT_WYRMREST_PROTECTOR_REND,
    EVENT_EARTHEN_DESTROYER_ROCK_BLAST,
    EVENT_EARTHEN_DESTROYER_QUAKE,
    EVENT_EARTHEN_STONE_SPIKES,
    EVENT_EARTHEN_SOLDIER_SHIELD_SLAM,
    EVENT_WATER_LORD_FROST_BOLT,
    EVENT_WATER_LORD_TIDAL_WAVE,
    EVENT_SIEGE_CAPTAIN_CLEAVE
};

class DragonSoulInstanceScript : public InstanceScript
{
public:
    explicit DragonSoulInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(DragonSoulEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new DragonSoulInstanceScript(pMapMgr); }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Morchok

class MorchokAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MorchokAI(c); }
    explicit MorchokAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_MORCHOK, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_MORCHOK_STOMP, 15000);
        scriptEvents.addEvent(EVENT_MORCHOK_CRUSH_ARMOR, 10000);
        scriptEvents.addEvent(EVENT_MORCHOK_BLACK_BLOOD_OF_THE_EARTH, 25000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_MORCHOK, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_MORCHOK, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_MORCHOK_STOMP:
                castSpellAOE(SPELL_MORCHOK_STOMP);
                scriptEvents.addEvent(EVENT_MORCHOK_STOMP, 20000);
                break;
            case EVENT_MORCHOK_CRUSH_ARMOR:
                castSpellOnVictim(SPELL_MORCHOK_CRUSH_ARMOR);
                scriptEvents.addEvent(EVENT_MORCHOK_CRUSH_ARMOR, 12000);
                break;
            case EVENT_MORCHOK_BLACK_BLOOD_OF_THE_EARTH:
                castSpellAOE(SPELL_MORCHOK_BLACK_BLOOD_OF_THE_EARTH);
                scriptEvents.addEvent(EVENT_MORCHOK_BLACK_BLOOD_OF_THE_EARTH, 30000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Warlord Zon'ozz

class WarlordZonozzAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new WarlordZonozzAI(c); }
    explicit WarlordZonozzAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_WARLORD_ZONOZZ, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_ZONOZZ_PSYCHIC_DRAIN, 10000);
        scriptEvents.addEvent(EVENT_ZONOZZ_DISRUPTING_SHADOWS, 16000);
        scriptEvents.addEvent(EVENT_ZONOZZ_FOCUSED_ANGER, 22000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_WARLORD_ZONOZZ, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_WARLORD_ZONOZZ, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_ZONOZZ_PSYCHIC_DRAIN:
                castSpellOnVictim(SPELL_ZONOZZ_PSYCHIC_DRAIN);
                scriptEvents.addEvent(EVENT_ZONOZZ_PSYCHIC_DRAIN, 24000);
                break;
            case EVENT_ZONOZZ_DISRUPTING_SHADOWS:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_ZONOZZ_DISRUPTING_SHADOWS);
                scriptEvents.addEvent(EVENT_ZONOZZ_DISRUPTING_SHADOWS, 18000);
                break;
            case EVENT_ZONOZZ_FOCUSED_ANGER:
                castSpellOnSelf(SPELL_ZONOZZ_FOCUSED_ANGER);
                scriptEvents.addEvent(EVENT_ZONOZZ_FOCUSED_ANGER, 26000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Yor'sahj the Unsleeping

class YorsahjTheUnsleepingAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new YorsahjTheUnsleepingAI(c); }
    explicit YorsahjTheUnsleepingAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_YORSAHJ_THE_UNSLEEPING, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_YORSAHJ_VOID_BOLT, 8000);
        scriptEvents.addEvent(EVENT_YORSAHJ_SEARING_BLOOD, 14000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_YORSAHJ_THE_UNSLEEPING, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_YORSAHJ_THE_UNSLEEPING, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_YORSAHJ_VOID_BOLT:
                castSpellOnVictim(SPELL_YORSAHJ_VOID_BOLT);
                scriptEvents.addEvent(EVENT_YORSAHJ_VOID_BOLT, 16000);
                break;
            case EVENT_YORSAHJ_SEARING_BLOOD:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_YORSAHJ_SEARING_BLOOD);
                scriptEvents.addEvent(EVENT_YORSAHJ_SEARING_BLOOD, 18000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Hagara the Stormbinder

class HagaraTheStormbinderAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HagaraTheStormbinderAI(c); }
    explicit HagaraTheStormbinderAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_HAGARA_THE_STORMBINDER, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_HAGARA_ICE_LANCE, 8000);
        scriptEvents.addEvent(EVENT_HAGARA_FROZEN_TEMPEST, 18000);
        scriptEvents.addEvent(EVENT_HAGARA_LIGHTNING_STORM, 26000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_HAGARA_THE_STORMBINDER, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_HAGARA_THE_STORMBINDER, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_HAGARA_ICE_LANCE:
                castSpellOnVictim(SPELL_HAGARA_ICE_LANCE);
                scriptEvents.addEvent(EVENT_HAGARA_ICE_LANCE, 14000);
                break;
            case EVENT_HAGARA_FROZEN_TEMPEST:
                castSpellAOE(SPELL_HAGARA_FROZEN_TEMPEST);
                scriptEvents.addEvent(EVENT_HAGARA_FROZEN_TEMPEST, 24000);
                break;
            case EVENT_HAGARA_LIGHTNING_STORM:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_HAGARA_LIGHTNING_STORM);
                scriptEvents.addEvent(EVENT_HAGARA_LIGHTNING_STORM, 28000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Ultraxion

class UltraxionAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new UltraxionAI(c); }
    explicit UltraxionAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_ULTRAXION, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_ULTRAXION_FADING_LIGHT, 12000);
        scriptEvents.addEvent(EVENT_ULTRAXION_HOUR_OF_TWILIGHT, 20000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_ULTRAXION, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_ULTRAXION, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_ULTRAXION_FADING_LIGHT:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_ULTRAXION_FADING_LIGHT);
                scriptEvents.addEvent(EVENT_ULTRAXION_FADING_LIGHT, 20000);
                break;
            case EVENT_ULTRAXION_HOUR_OF_TWILIGHT:
                castSpellAOE(SPELL_ULTRAXION_HOUR_OF_TWILIGHT);
                scriptEvents.addEvent(EVENT_ULTRAXION_HOUR_OF_TWILIGHT, 30000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Warmaster Blackhorn

class WarmasterBlackhornAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new WarmasterBlackhornAI(c); }
    explicit WarmasterBlackhornAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_WARMASTER_BLACKHORN, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_BLACKHORN_DEVASTATE, 8000);
        scriptEvents.addEvent(EVENT_BLACKHORN_DISRUPTING_ROAR, 16000);
        scriptEvents.addEvent(EVENT_BLACKHORN_SHOCKWAVE, 22000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_WARMASTER_BLACKHORN, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_WARMASTER_BLACKHORN, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_BLACKHORN_DEVASTATE:
                castSpellOnVictim(SPELL_BLACKHORN_DEVASTATE);
                scriptEvents.addEvent(EVENT_BLACKHORN_DEVASTATE, 14000);
                break;
            case EVENT_BLACKHORN_DISRUPTING_ROAR:
                castSpellAOE(SPELL_BLACKHORN_DISRUPTING_ROAR);
                scriptEvents.addEvent(EVENT_BLACKHORN_DISRUPTING_ROAR, 20000);
                break;
            case EVENT_BLACKHORN_SHOCKWAVE:
                castSpellAOE(SPELL_BLACKHORN_SHOCKWAVE);
                scriptEvents.addEvent(EVENT_BLACKHORN_SHOCKWAVE, 24000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Madness of Deathwing

class MadnessOfDeathwingAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MadnessOfDeathwingAI(c); }
    explicit MadnessOfDeathwingAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "You have done NOTHING. I will tear your world APART.");
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_MADNESS_OF_DEATHWING, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_DEATHWING_CATACLYSM, 15000);
        scriptEvents.addEvent(EVENT_DEATHWING_ELEMENTIUM_BOLT, 8000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_MADNESS_OF_DEATHWING, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_MADNESS_OF_DEATHWING, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_DEATHWING_CATACLYSM:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Deathwing begins to cast Cataclysm! Stop Him!");
                castSpellAOE(SPELL_DEATHWING_CATACLYSM);
                scriptEvents.addEvent(EVENT_DEATHWING_CATACLYSM, 26000);
                break;
            case EVENT_DEATHWING_ELEMENTIUM_BOLT:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Deathwing begins to cast Elementium Bolt!");
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_DEATHWING_ELEMENTIUM_BOLT);
                scriptEvents.addEvent(EVENT_DEATHWING_ELEMENTIUM_BOLT, 16000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Trash

class WyrmrestProtectorAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new WyrmrestProtectorAI(c); }
    explicit WyrmrestProtectorAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_WYRMREST_PROTECTOR_TAIL_SWEEP, 8000);
        scriptEvents.addEvent(EVENT_WYRMREST_PROTECTOR_CLEAVE, 5000);
        scriptEvents.addEvent(EVENT_WYRMREST_PROTECTOR_GORE, 7000);
        scriptEvents.addEvent(EVENT_WYRMREST_PROTECTOR_REND, 9000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_WYRMREST_PROTECTOR_TAIL_SWEEP:
                castSpellOnVictim(SPELL_WYRMREST_PROTECTOR_TAIL_SWEEP);
                scriptEvents.addEvent(EVENT_WYRMREST_PROTECTOR_TAIL_SWEEP, 8000);
                break;
            case EVENT_WYRMREST_PROTECTOR_CLEAVE:
                castSpellOnVictim(SPELL_WYRMREST_PROTECTOR_CLEAVE);
                scriptEvents.addEvent(EVENT_WYRMREST_PROTECTOR_CLEAVE, 7000);
                break;
            case EVENT_WYRMREST_PROTECTOR_GORE:
                castSpellOnVictim(SPELL_WYRMREST_PROTECTOR_GORE);
                scriptEvents.addEvent(EVENT_WYRMREST_PROTECTOR_GORE, 7000);
                break;
            case EVENT_WYRMREST_PROTECTOR_REND:
                castSpellOnVictim(SPELL_WYRMREST_PROTECTOR_REND);
                scriptEvents.addEvent(EVENT_WYRMREST_PROTECTOR_REND, 12000);
                break;
            default:
                break;
        }
    }
};

class EarthenDestroyerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EarthenDestroyerAI(c); }
    explicit EarthenDestroyerAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_EARTHEN_DESTROYER_ROCK_BLAST, 2000);
        scriptEvents.addEvent(EVENT_EARTHEN_DESTROYER_QUAKE, 7000);
        scriptEvents.addEvent(EVENT_EARTHEN_STONE_SPIKES, 3400);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_EARTHEN_DESTROYER_ROCK_BLAST:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_EARTHEN_DESTROYER_ROCK_BLAST);
                scriptEvents.addEvent(EVENT_EARTHEN_DESTROYER_ROCK_BLAST, 12000);
                break;
            case EVENT_EARTHEN_DESTROYER_QUAKE:
                castSpellOnSelf(SPELL_EARTHEN_DESTROYER_QUAKE);
                scriptEvents.addEvent(EVENT_EARTHEN_DESTROYER_QUAKE, 22000);
                break;
            case EVENT_EARTHEN_STONE_SPIKES:
                castSpellOnVictim(SPELL_EARTHEN_STONE_SPIKES);
                scriptEvents.addEvent(EVENT_EARTHEN_STONE_SPIKES, 3400);
                break;
            default:
                break;
        }
    }
};

class EarthenSoldierAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EarthenSoldierAI(c); }
    explicit EarthenSoldierAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        mEnrageCast = false;
        scriptEvents.addEvent(EVENT_EARTHEN_STONE_SPIKES, 3400);
        scriptEvents.addEvent(EVENT_EARTHEN_SOLDIER_SHIELD_SLAM, 8000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (!mEnrageCast && _getHealthPercent() <= 30)
        {
            mEnrageCast = true;
            castSpellOnSelf(SPELL_EARTHEN_SOLDIER_ENRAGE);
        }

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_EARTHEN_STONE_SPIKES:
                castSpellOnVictim(SPELL_EARTHEN_STONE_SPIKES);
                scriptEvents.addEvent(EVENT_EARTHEN_STONE_SPIKES, 3400);
                break;
            case EVENT_EARTHEN_SOLDIER_SHIELD_SLAM:
                castSpellOnSelf(SPELL_EARTHEN_SOLDIER_SHIELD_SLAM);
                scriptEvents.addEvent(EVENT_EARTHEN_SOLDIER_SHIELD_SLAM, 24000);
                break;
            default:
                break;
        }
    }

private:
    bool mEnrageCast = false;
};

class AncientWaterLordAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AncientWaterLordAI(c); }
    explicit AncientWaterLordAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_WATER_LORD_FROST_BOLT, 4000);
        scriptEvents.addEvent(EVENT_WATER_LORD_TIDAL_WAVE, 7000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_WATER_LORD_FROST_BOLT:
                castSpellOnVictim(SPELL_WATER_LORD_FROST_BOLT);
                scriptEvents.addEvent(EVENT_WATER_LORD_FROST_BOLT, 13500);
                break;
            case EVENT_WATER_LORD_TIDAL_WAVE:
                castSpellOnVictim(SPELL_WATER_LORD_TIDAL_WAVE);
                scriptEvents.addEvent(EVENT_WATER_LORD_TIDAL_WAVE, 25000);
                break;
            default:
                break;
        }
    }
};

class TwilightSiegeCaptainAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TwilightSiegeCaptainAI(c); }
    explicit TwilightSiegeCaptainAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_SIEGE_CAPTAIN_CLEAVE, 2000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_SIEGE_CAPTAIN_CLEAVE)
        {
            castSpellOnSelf(SPELL_SIEGE_CAPTAIN_CLEAVE);
            scriptEvents.addEvent(EVENT_SIEGE_CAPTAIN_CLEAVE, 20000);
        }
    }
};

void SetupDragonSoul(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_DRAGON_SOUL, &DragonSoulInstanceScript::Create);

    mgr->register_creature_script(BOSS_MORCHOK, &MorchokAI::Create);
    mgr->register_creature_script(BOSS_WARLORD_ZONOZZ, &WarlordZonozzAI::Create);
    mgr->register_creature_script(BOSS_YORSAHJ_THE_UNSLEEPING, &YorsahjTheUnsleepingAI::Create);
    mgr->register_creature_script(BOSS_HAGARA_THE_STORMBINDER, &HagaraTheStormbinderAI::Create);
    mgr->register_creature_script(BOSS_ULTRAXION, &UltraxionAI::Create);
    mgr->register_creature_script(BOSS_WARMASTER_BLACKHORN, &WarmasterBlackhornAI::Create);
    mgr->register_creature_script(BOSS_MADNESS_OF_DEATHWING, &MadnessOfDeathwingAI::Create);

    mgr->register_creature_script(NPC_WYRMREST_PROTECTOR, &WyrmrestProtectorAI::Create);
    mgr->register_creature_script(NPC_EARTHEN_DESTROYER, &EarthenDestroyerAI::Create);
    mgr->register_creature_script(NPC_EARTHEN_SOLDIER, &EarthenSoldierAI::Create);
    mgr->register_creature_script(NPC_ANCIENT_WATER_LORD, &AncientWaterLordAI::Create);
    mgr->register_creature_script(NPC_TWILIGHT_SIEGE_CAPTAIN, &TwilightSiegeCaptainAI::Create);
}
