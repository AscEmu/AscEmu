/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

// The Vortex Pinnacle - three independent bosses (verified against wowhead), hand-ported into
// AscEmu's own CreatureAIScript / InstanceScript framework.
//
// Simplifications versus the original fight design: all three encounters here rely heavily on
// elaborate positional/environmental puzzles (Ertan's orbiting "Storm's Edge" safe-zone adds,
// Altairus' air-current knockback/safe-zone stalkers, Asaad's grounding-field safe-triangle).
// None of those are ported - each boss instead runs its core damage rotation (rotation timers
// and spell IDs match the original), so the dungeon is fully clearable but without those
// signature environmental mechanics.

#include "Setup.h"
#include "Instance_Vortex_Pinnacle.hpp"

#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

enum VortexPinnacleEvents
{
    EVENT_ERTAN_STORMS_EDGE = 1,
    EVENT_ERTAN_STORMS_EDGE_AURA,
    EVENT_ERTAN_END_STORMS_EDGE,
    EVENT_ERTAN_LIGHTNING_BOLT,
    EVENT_ERTAN_SUMMON_TEMPEST,

    EVENT_ALTAIRUS_CALL_THE_WIND,
    EVENT_ALTAIRUS_CHILLING_BREATH,
    EVENT_ALTAIRUS_TWISTING_WINDS,

    EVENT_ASAAD_CHAIN_LIGHTNING,
    EVENT_ASAAD_STATIC_CLING,
    EVENT_ASAAD_SUMMON_SKYFALL_STAR,

    EVENT_GUST_SOLDIER_WIND_BLAST,
    EVENT_GUST_SOLDIER_AIR_NOVA,

    EVENT_WILD_VORTEX_LIGHTNING_BOLT,

    EVENT_ARMORED_MISTRAL_GALE_STRIKE,
    EVENT_ARMORED_MISTRAL_STORM_SURGE,

    EVENT_CLOUD_PRINCE_STARFALL,
    EVENT_CLOUD_PRINCE_TYPHOON,

    EVENT_EMPYREAN_ASSASSIN_VAPOR_FORM,

    EVENT_TURBULENT_SQUALL_ASPHYXIATE,
    EVENT_TURBULENT_SQUALL_CLOUDBURST,
    EVENT_TURBULENT_SQUALL_HURRICANE,

    EVENT_SERVANT_OF_ASAAD_CRUSADER_STRIKE,
    EVENT_SERVANT_OF_ASAAD_DIVINE_STORM,

    EVENT_EXECUTOR_DEVASTATE,
    EVENT_EXECUTOR_SHOCKWAVE,

    EVENT_MINISTER_OF_AIR_LIGHTNING_NOVA,

    EVENT_TEMPLE_ADEPT_HOLY_SMITE,
    EVENT_TEMPLE_ADEPT_GREATER_HEAL
};

class VortexPinnacleInstanceScript : public InstanceScript
{
public:
    explicit VortexPinnacleInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(VortexPinnacleEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new VortexPinnacleInstanceScript(pMapMgr); }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Grand Vizier Ertan

class GrandVizierErtanAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new GrandVizierErtanAI(c); }
    explicit GrandVizierErtanAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "Filthy beasts! Your presence in Skywall will not be tolerated!");
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_GRAND_VIZIER_ERTAN, EncounterStates::InProgress);

        castSpellOnSelf(SPELL_ERTAN_STORMS_EDGE_PERIODIC);
        scriptEvents.addEvent(EVENT_ERTAN_STORMS_EDGE, 24000);
        scriptEvents.addEvent(EVENT_ERTAN_LIGHTNING_BOLT, 1);
        if (isHeroic())
            scriptEvents.addEvent(EVENT_ERTAN_SUMMON_TEMPEST, 16000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_GRAND_VIZIER_ERTAN, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_GRAND_VIZIER_ERTAN, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_ERTAN_STORMS_EDGE:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Grand Vizier Ertan retracts his cyclone shield!");
                castSpellOnSelf(SPELL_ERTAN_STORMS_EDGE_VISUAL);
                scriptEvents.addEvent(EVENT_ERTAN_STORMS_EDGE_AURA, 3000);
                scriptEvents.addEvent(EVENT_ERTAN_STORMS_EDGE, 31000);
                break;
            case EVENT_ERTAN_STORMS_EDGE_AURA:
                _removeAura(SPELL_ERTAN_STORMS_EDGE_PERIODIC);
                castSpellOnSelf(SPELL_ERTAN_STORMS_EDGE_PERIODIC_2);
                scriptEvents.addEvent(EVENT_ERTAN_END_STORMS_EDGE, 6000);
                break;
            case EVENT_ERTAN_END_STORMS_EDGE:
                _removeAura(SPELL_ERTAN_STORMS_EDGE_PERIODIC_2);
                castSpellOnSelf(SPELL_ERTAN_STORMS_EDGE_PERIODIC);
                break;
            case EVENT_ERTAN_LIGHTNING_BOLT:
                castSpellOnVictim(SPELL_ERTAN_LIGHTNING_BOLT);
                scriptEvents.addEvent(EVENT_ERTAN_LIGHTNING_BOLT, 2400);
                break;
            case EVENT_ERTAN_SUMMON_TEMPEST:
                castSpellOnSelf(SPELL_ERTAN_SUMMON_TEMPEST);
                scriptEvents.addEvent(EVENT_ERTAN_SUMMON_TEMPEST, 17000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Altairus

class AltairusAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AltairusAI(c); }
    explicit AltairusAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_ALTAIRUS, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_ALTAIRUS_CALL_THE_WIND, 6000);
        scriptEvents.addEvent(EVENT_ALTAIRUS_CHILLING_BREATH, 12000);
        scriptEvents.addEvent(EVENT_ALTAIRUS_TWISTING_WINDS, 18000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_ALTAIRUS, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_ALTAIRUS, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_ALTAIRUS_CALL_THE_WIND:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "The wind abruptly shifts direction at Altairus' command!");
                castSpellAOE(SPELL_ALTAIRUS_CALL_THE_WIND);
                if (Unit* pUpwind = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pUpwind, SPELL_ALTAIRUS_UPWIND);
                if (Unit* pDownwind = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pDownwind, SPELL_ALTAIRUS_DOWNWIND);
                scriptEvents.addEvent(EVENT_ALTAIRUS_CALL_THE_WIND, 22000);
                break;
            case EVENT_ALTAIRUS_CHILLING_BREATH:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_ALTAIRUS_CHILLING_BREATH);
                scriptEvents.addEvent(EVENT_ALTAIRUS_CHILLING_BREATH, 12000);
                break;
            case EVENT_ALTAIRUS_TWISTING_WINDS:
                castSpellAOE(SPELL_ALTAIRUS_TWISTING_WINDS);
                scriptEvents.addEvent(EVENT_ALTAIRUS_TWISTING_WINDS, 24000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Asaad

class AsaadAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AsaadAI(c); }
    explicit AsaadAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "YOU tread upon the sacrosanct! Mortals have no place amidst the clouds.");
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_ASAAD, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_ASAAD_SUMMON_SKYFALL_STAR, 10000);
        if (isHeroic())
            scriptEvents.addEvent(EVENT_ASAAD_STATIC_CLING, 10000);
        scriptEvents.addEvent(EVENT_ASAAD_CHAIN_LIGHTNING, 14000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_ASAAD, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "The winds take me!");
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_ASAAD, EncounterStates::Performed);
    }

    void onSummonedCreature(Creature* summon) override
    {
        if (summon->getEntry() == NPC_VP_SKYFALL_STAR)
            summon->castSpell(summon, SPELL_ASAAD_ARCANE_BARRAGE, true);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_ASAAD_SUMMON_SKYFALL_STAR:
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "Al'Akir, your servant calls for aid!");
                castSpellOnSelf(SPELL_ASAAD_SUMMON_SKYFALL_STAR);
                scriptEvents.addEvent(EVENT_ASAAD_SUMMON_SKYFALL_STAR, 12000);
                break;
            case EVENT_ASAAD_STATIC_CLING:
                castSpellAOE(SPELL_ASAAD_STATIC_CLING);
                scriptEvents.addEvent(EVENT_ASAAD_STATIC_CLING, 17000);
                break;
            case EVENT_ASAAD_CHAIN_LIGHTNING:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_ASAAD_CHAIN_LIGHTNING);
                scriptEvents.addEvent(EVENT_ASAAD_CHAIN_LIGHTNING, 25000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Trash

class GustSoldierAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new GustSoldierAI(c); }
    explicit GustSoldierAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        castSpellOnVictim(SPELL_GUST_SOLDIER_CHARGE);
        scriptEvents.addEvent(EVENT_GUST_SOLDIER_WIND_BLAST, 8000);
        scriptEvents.addEvent(EVENT_GUST_SOLDIER_AIR_NOVA, 20000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_GUST_SOLDIER_WIND_BLAST:
                castSpellOnSelf(SPELL_GUST_SOLDIER_WIND_BLAST);
                scriptEvents.addEvent(EVENT_GUST_SOLDIER_WIND_BLAST, 10000);
                break;
            case EVENT_GUST_SOLDIER_AIR_NOVA:
                castSpellOnSelf(SPELL_GUST_SOLDIER_AIR_NOVA);
                scriptEvents.addEvent(EVENT_GUST_SOLDIER_AIR_NOVA, 22000);
                break;
            default:
                break;
        }
    }
};

class WildVortexAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new WildVortexAI(c); }
    explicit WildVortexAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_WILD_VORTEX_LIGHTNING_BOLT, 1600);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_WILD_VORTEX_LIGHTNING_BOLT)
        {
            castSpellOnVictim(SPELL_WILD_VORTEX_LIGHTNING_BOLT);
            scriptEvents.addEvent(EVENT_WILD_VORTEX_LIGHTNING_BOLT, 1600);
        }
    }
};

class ArmoredMistralAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ArmoredMistralAI(c); }
    explicit ArmoredMistralAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        castSpellOnSelf(SPELL_ARMORED_MISTRAL_RISING_WINDS);
        scriptEvents.addEvent(EVENT_ARMORED_MISTRAL_GALE_STRIKE, 10000);
        scriptEvents.addEvent(EVENT_ARMORED_MISTRAL_STORM_SURGE, 12000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_ARMORED_MISTRAL_GALE_STRIKE:
                castSpellOnSelf(SPELL_ARMORED_MISTRAL_GALE_STRIKE);
                scriptEvents.addEvent(EVENT_ARMORED_MISTRAL_GALE_STRIKE, 11000);
                break;
            case EVENT_ARMORED_MISTRAL_STORM_SURGE:
                castSpellOnSelf(SPELL_ARMORED_MISTRAL_STORM_SURGE);
                scriptEvents.addEvent(EVENT_ARMORED_MISTRAL_STORM_SURGE, 13000);
                break;
            default:
                break;
        }
    }
};

// Cloud Prince's Whipping Winds add-summon/threat-transfer mechanic is not ported - it keeps
// its Starfall/Typhoon damage rotation.
class CloudPrinceAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CloudPrinceAI(c); }
    explicit CloudPrinceAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        castSpellOnSelf(SPELL_CLOUD_PRINCE_WHIPPING_WINDS);
        scriptEvents.addEvent(EVENT_CLOUD_PRINCE_STARFALL, 5000);
        scriptEvents.addEvent(EVENT_CLOUD_PRINCE_TYPHOON, 10000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_CLOUD_PRINCE_STARFALL:
                castSpellAOE(SPELL_CLOUD_PRINCE_STARFALL);
                scriptEvents.addEvent(EVENT_CLOUD_PRINCE_STARFALL, 8000);
                break;
            case EVENT_CLOUD_PRINCE_TYPHOON:
                castSpellAOE(SPELL_CLOUD_PRINCE_TYPHOON);
                scriptEvents.addEvent(EVENT_CLOUD_PRINCE_TYPHOON, 13000);
                break;
            default:
                break;
        }
    }
};

class EmpyreanAssassinAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EmpyreanAssassinAI(c); }
    explicit EmpyreanAssassinAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_EMPYREAN_ASSASSIN_VAPOR_FORM, 20000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_EMPYREAN_ASSASSIN_VAPOR_FORM)
        {
            castSpellOnSelf(SPELL_EMPYREAN_ASSASSIN_VAPOR_FORM);
            scriptEvents.addEvent(EVENT_EMPYREAN_ASSASSIN_VAPOR_FORM, 20000);
        }
    }
};

class TurbulentSquallAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TurbulentSquallAI(c); }
    explicit TurbulentSquallAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_TURBULENT_SQUALL_ASPHYXIATE, 5000);
        scriptEvents.addEvent(EVENT_TURBULENT_SQUALL_CLOUDBURST, 10000);
        scriptEvents.addEvent(EVENT_TURBULENT_SQUALL_HURRICANE, 5000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_TURBULENT_SQUALL_ASPHYXIATE:
                castSpellOnVictim(SPELL_TURBULENT_SQUALL_ASPHYXIATE);
                scriptEvents.addEvent(EVENT_TURBULENT_SQUALL_ASPHYXIATE, 12000);
                break;
            case EVENT_TURBULENT_SQUALL_CLOUDBURST:
                castSpellOnSelf(SPELL_TURBULENT_SQUALL_CLOUDBURST);
                scriptEvents.addEvent(EVENT_TURBULENT_SQUALL_CLOUDBURST, 14000);
                break;
            case EVENT_TURBULENT_SQUALL_HURRICANE:
                castSpellAOE(SPELL_TURBULENT_SQUALL_HURRICANE);
                scriptEvents.addEvent(EVENT_TURBULENT_SQUALL_HURRICANE, 18000);
                break;
            default:
                break;
        }
    }
};

class ServantOfAsaadAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ServantOfAsaadAI(c); }
    explicit ServantOfAsaadAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_SERVANT_OF_ASAAD_CRUSADER_STRIKE, 6000);
        scriptEvents.addEvent(EVENT_SERVANT_OF_ASAAD_DIVINE_STORM, 14000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_SERVANT_OF_ASAAD_CRUSADER_STRIKE:
                castSpellOnVictim(SPELL_SERVANT_OF_ASAAD_CRUSADER_STRIKE);
                scriptEvents.addEvent(EVENT_SERVANT_OF_ASAAD_CRUSADER_STRIKE, 6000);
                break;
            case EVENT_SERVANT_OF_ASAAD_DIVINE_STORM:
                castSpellOnSelf(SPELL_SERVANT_OF_ASAAD_DIVINE_STORM);
                scriptEvents.addEvent(EVENT_SERVANT_OF_ASAAD_DIVINE_STORM, 14000);
                break;
            default:
                break;
        }
    }
};

class ExecutorOfTheCaliphAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ExecutorOfTheCaliphAI(c); }
    explicit ExecutorOfTheCaliphAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_EXECUTOR_DEVASTATE, 6000);
        scriptEvents.addEvent(EVENT_EXECUTOR_SHOCKWAVE, 15000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_EXECUTOR_DEVASTATE:
                castSpellOnVictim(SPELL_EXECUTOR_DEVASTATE);
                scriptEvents.addEvent(EVENT_EXECUTOR_DEVASTATE, 6000);
                break;
            case EVENT_EXECUTOR_SHOCKWAVE:
                castSpellOnSelf(SPELL_EXECUTOR_SHOCKWAVE);
                scriptEvents.addEvent(EVENT_EXECUTOR_SHOCKWAVE, 18000);
                break;
            default:
                break;
        }
    }
};

class MinisterOfAirAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MinisterOfAirAI(c); }
    explicit MinisterOfAirAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        castSpellOnVictim(SPELL_MINISTER_OF_AIR_LIGHTNING_LASH);
        scriptEvents.addEvent(EVENT_MINISTER_OF_AIR_LIGHTNING_NOVA, 12000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_MINISTER_OF_AIR_LIGHTNING_NOVA)
        {
            castSpellOnSelf(SPELL_MINISTER_OF_AIR_LIGHTNING_NOVA);
            scriptEvents.addEvent(EVENT_MINISTER_OF_AIR_LIGHTNING_NOVA, 18000);
        }
    }
};

class TempleAdeptAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TempleAdeptAI(c); }
    explicit TempleAdeptAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_TEMPLE_ADEPT_HOLY_SMITE, 500);
        scriptEvents.addEvent(EVENT_TEMPLE_ADEPT_GREATER_HEAL, 3000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_TEMPLE_ADEPT_HOLY_SMITE:
                castSpellOnVictim(SPELL_TEMPLE_ADEPT_HOLY_SMITE);
                scriptEvents.addEvent(EVENT_TEMPLE_ADEPT_HOLY_SMITE, 3000);
                break;
            case EVENT_TEMPLE_ADEPT_GREATER_HEAL:
                castSpellOnSelf(SPELL_TEMPLE_ADEPT_GREATER_HEAL);
                scriptEvents.addEvent(EVENT_TEMPLE_ADEPT_GREATER_HEAL, 10000);
                break;
            default:
                break;
        }
    }
};

void SetupVortexPinnacle(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_THE_VORTEX_PINNACLE, &VortexPinnacleInstanceScript::Create);

    mgr->register_creature_script(BOSS_GRAND_VIZIER_ERTAN, &GrandVizierErtanAI::Create);
    mgr->register_creature_script(BOSS_ALTAIRUS, &AltairusAI::Create);
    mgr->register_creature_script(BOSS_ASAAD, &AsaadAI::Create);

    mgr->register_creature_script(NPC_GUST_SOLDIER, &GustSoldierAI::Create);
    mgr->register_creature_script(NPC_WILD_VORTEX, &WildVortexAI::Create);
    mgr->register_creature_script(NPC_ARMORED_MISTRAL, &ArmoredMistralAI::Create);
    mgr->register_creature_script(NPC_CLOUD_PRINCE, &CloudPrinceAI::Create);
    mgr->register_creature_script(NPC_EMPYREAN_ASSASSIN, &EmpyreanAssassinAI::Create);
    mgr->register_creature_script(NPC_TURBULENT_SQUALL, &TurbulentSquallAI::Create);
    mgr->register_creature_script(NPC_SERVANT_OF_ASAAD, &ServantOfAsaadAI::Create);
    mgr->register_creature_script(NPC_EXECUTOR_OF_THE_CALIPH, &ExecutorOfTheCaliphAI::Create);
    mgr->register_creature_script(NPC_MINISTER_OF_AIR, &MinisterOfAirAI::Create);
    mgr->register_creature_script(NPC_TEMPLE_ADEPT, &TempleAdeptAI::Create);
}
