/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

// Halls of Origination - seven bosses (verified against wowhead), hand-ported into AscEmu's
// own CreatureAIScript / InstanceScript framework.
//
// Simplifications versus the original fight design: this dungeon is heavily phase- and
// event-scripted (Anhuur's shield/beacon phase, Ptah's earthstorm phase, Anraphet's
// activation intro, Isiset's rotating elemental "state" mechanic, Setesh's summon-wave
// portal phase, Rajh's vehicle-based Inferno Leap). Every boss here runs a simplified but
// functional continuous rotation built from its core abilities instead.

#include "Setup.h"
#include "Instance_Halls_Of_Origination.hpp"

#include "Objects/GameObject.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

enum HallsOfOriginationEvents
{
    EVENT_ANHUUR_DIVINE_RECKONING = 1,
    EVENT_ANHUUR_BURNING_LIGHT,

    EVENT_PTAH_RAGING_SMASH,
    EVENT_PTAH_FLAME_BOLT,

    EVENT_ANRAPHET_NEMESIS_STRIKE,
    EVENT_ANRAPHET_ALPHA_BEAMS,
    EVENT_ANRAPHET_OMEGA_STANCE,

    EVENT_ISISET_SUPERNOVA,
    EVENT_ISISET_ASTRAL_RAIN,
    EVENT_ISISET_ASTRAL_FAMILIAR,
    EVENT_ISISET_VEIL_OF_SKY,

    EVENT_AMMUNAE_WITHER,
    EVENT_AMMUNAE_SUMMON_SEEDLING_POD,
    EVENT_AMMUNAE_CONSUME_LIFE_ENERGY,

    EVENT_SETESH_CHAOS_BOLT,
    EVENT_SETESH_SEED_OF_CHAOS,
    EVENT_SETESH_REIGN_OF_CHAOS,

    EVENT_RAJH_SUN_STRIKE,
    EVENT_RAJH_SUMMON_SUN_ORB,
    EVENT_RAJH_BLAZING_INFERNO,

    EVENT_SUN_TOUCHED_SEARING_FLAMES,

    EVENT_SPEAKER_OUT_OF_COMBAT,
    EVENT_SPEAKER_RANDOM,
    EVENT_PIT_VIPER_VICTIM,
    EVENT_TORMENTOR_CURSE_OF_EXHAUSTION,
    EVENT_TORMENTOR_SHADOW_BOLT,
    EVENT_DUSTBONE_SMASH,
    EVENT_SWIFTSTALKER_SHOOT,
    EVENT_SWIFTSTALKER_CHARGED_SHOT,
    EVENT_RUNECASTER_RUNIC_CLEAVE,
    EVENT_RUNECASTER_CURSE,
    EVENT_SHADOWLANCER_SHADOWLANCE,
    EVENT_SHADOWLANCER_PACT_OF_DARKNESS,
    EVENT_FIRESHAPER_FIREBALL,
    EVENT_FIRESHAPER_MOLTEN_BARRIER,
    EVENT_FIRESHAPER_METEOR
};

class HallsOfOriginationInstanceScript : public InstanceScript
{
public:
    explicit HallsOfOriginationInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(HallsOfOriginationEncounterCount);
        mAnhuurDoorGuid = 0;
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new HallsOfOriginationInstanceScript(pMapMgr); }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        if (pGameObject->getEntry() == GO_ANHUUR_DOOR)
            mAnhuurDoorGuid = pGameObject->getGuidLow();
    }

    void OpenAnhuurDoor()
    {
        if (GameObject* pDoor = GetGameObjectByGuid(mAnhuurDoorGuid))
            useDoorOrButton(pDoor);
    }

private:
    uint32_t mAnhuurDoorGuid;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Temple Guardian Anhuur

class TempleGuardianAnhuurAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TempleGuardianAnhuurAI(c); }
    explicit TempleGuardianAnhuurAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    // Reference dialogue data for entry 39425 also has SAY_SHIELD ("Beacons of light, bestow
    // upon me your aegis!", sound 18581), EMOTE_SHIELD ("%s becomes protected by his defense
    // beacons! Disable them by pulling the levers below!"), a matching shield-dropped emote,
    // and an unlabeled "I regret nothing." line (sound 18582) - all tied to the beacon/lever
    // shield phase the header comment already notes isn't ported, so they have no cast point
    // to attach to here.
    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 18580, "Turn back, intruders! These halls must not be disturbed!");
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_TEMPLE_GUARDIAN_ANHUUR, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_ANHUUR_DIVINE_RECKONING, 10000);
        scriptEvents.addEvent(EVENT_ANHUUR_BURNING_LIGHT, 12000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_TEMPLE_GUARDIAN_ANHUUR, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 18579, "What... have you... done?");
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_TEMPLE_GUARDIAN_ANHUUR, EncounterStates::Performed);
        static_cast<HallsOfOriginationInstanceScript*>(getInstanceScript())->OpenAnhuurDoor();
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_ANHUUR_DIVINE_RECKONING:
                castSpellOnVictim(SPELL_ANHUUR_DIVINE_RECKONING);
                scriptEvents.addEvent(EVENT_ANHUUR_DIVINE_RECKONING, 10000);
                break;
            case EVENT_ANHUUR_BURNING_LIGHT:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_ANHUUR_BURNING_LIGHT);
                scriptEvents.addEvent(EVENT_ANHUUR_BURNING_LIGHT, 12000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Earthrager Ptah

class EarthragerPtahAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EarthragerPtahAI(c); }
    explicit EarthragerPtahAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 18906, "More carrion for the swarm...");
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_EARTHRAGER_PTAH, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_PTAH_RAGING_SMASH, 7000);
        scriptEvents.addEvent(EVENT_PTAH_FLAME_BOLT, 8000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_EARTHRAGER_PTAH, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 18905, "Ptah... is... no more...");
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_EARTHRAGER_PTAH, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_PTAH_RAGING_SMASH:
                castSpellOnVictim(SPELL_PTAH_RAGING_SMASH);
                scriptEvents.addEvent(EVENT_PTAH_RAGING_SMASH, 14000);
                break;
            case EVENT_PTAH_FLAME_BOLT:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_PTAH_FLAME_BOLT);
                scriptEvents.addEvent(EVENT_PTAH_FLAME_BOLT, 16000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Anraphet

class AnraphetAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AnraphetAI(c); }
    explicit AnraphetAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    // Reference dialogue data for entry 39788 also has a long SAY_INTRO activation speech
    // ("This unit has been activated outside normal operating protocols...", sound 20857),
    // played once when the room's puzzle/activation sequence completes rather than on combat
    // start - no such pre-combat activation event exists in this simplified port, so it has
    // no cast point to attach to here.
    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 20862, "Purge of unauthorized entities commencing.");
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_ANRAPHET, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_ANRAPHET_NEMESIS_STRIKE, 8000);
        scriptEvents.addEvent(EVENT_ANRAPHET_ALPHA_BEAMS, 10000);
        scriptEvents.addEvent(EVENT_ANRAPHET_OMEGA_STANCE, 35000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_ANRAPHET, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnTargetDied(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 20859, "Purge Complete.");
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 20856, "Anraphet unit shutting down...");
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_ANRAPHET, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_ANRAPHET_NEMESIS_STRIKE:
                castSpellOnVictim(SPELL_ANRAPHET_NEMESIS_STRIKE);
                scriptEvents.addEvent(EVENT_ANRAPHET_NEMESIS_STRIKE, 21000);
                break;
            case EVENT_ANRAPHET_ALPHA_BEAMS:
                castSpellAOE(SPELL_ANRAPHET_ALPHA_BEAMS);
                scriptEvents.addEvent(EVENT_ANRAPHET_ALPHA_BEAMS, 42000);
                break;
            case EVENT_ANRAPHET_OMEGA_STANCE:
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 20861, "Omega Stance activated. Annihilation of foreign unit is now imminent.");
                castSpellOnSelf(SPELL_ANRAPHET_OMEGA_STANCE);
                scriptEvents.addEvent(EVENT_ANRAPHET_OMEGA_STANCE, 47000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Isiset

class IsisetAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new IsisetAI(c); }
    explicit IsisetAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_ISISET, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_ISISET_ASTRAL_RAIN, 8000);
        scriptEvents.addEvent(EVENT_ISISET_ASTRAL_FAMILIAR, 16000);
        scriptEvents.addEvent(EVENT_ISISET_VEIL_OF_SKY, 24000);
        scriptEvents.addEvent(EVENT_ISISET_SUPERNOVA, 32000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_ISISET, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_ISISET, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_ISISET_ASTRAL_RAIN:
                castSpellAOE(SPELL_ISISET_ASTRAL_RAIN_CONTROLLER);
                scriptEvents.addEvent(EVENT_ISISET_ASTRAL_RAIN, 32000);
                break;
            case EVENT_ISISET_ASTRAL_FAMILIAR:
                castSpellOnSelf(SPELL_ISISET_ASTRAL_FAMILIAR_CONTROLLER);
                scriptEvents.addEvent(EVENT_ISISET_ASTRAL_FAMILIAR, 32000);
                break;
            case EVENT_ISISET_VEIL_OF_SKY:
                castSpellAOE(SPELL_ISISET_VEIL_OF_SKY_DAMAGE);
                scriptEvents.addEvent(EVENT_ISISET_VEIL_OF_SKY, 32000);
                break;
            case EVENT_ISISET_SUPERNOVA:
                castSpellOnSelf(SPELL_ISISET_SUPERNOVA);
                scriptEvents.addEvent(EVENT_ISISET_SUPERNOVA, 32000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Ammunae

class AmmunaeAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AmmunaeAI(c); }
    explicit AmmunaeAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "This chamber will flourish with your life energy!");
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_AMMUNAE, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_AMMUNAE_SUMMON_SEEDLING_POD, 7000);
        scriptEvents.addEvent(EVENT_AMMUNAE_WITHER, 7000);
        scriptEvents.addEvent(EVENT_AMMUNAE_CONSUME_LIFE_ENERGY, 21000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_AMMUNAE, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "The cycle continues...");
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_AMMUNAE, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_AMMUNAE_SUMMON_SEEDLING_POD:
                castSpellOnSelf(SPELL_AMMUNAE_SUMMON_SEEDLING_POD);
                scriptEvents.addEvent(EVENT_AMMUNAE_SUMMON_SEEDLING_POD, 15000);
                break;
            case EVENT_AMMUNAE_WITHER:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_AMMUNAE_WITHER);
                scriptEvents.addEvent(EVENT_AMMUNAE_WITHER, 8000);
                break;
            case EVENT_AMMUNAE_CONSUME_LIFE_ENERGY:
                castSpellOnVictim(SPELL_AMMUNAE_CONSUME_LIFE_ENERGY);
                scriptEvents.addEvent(EVENT_AMMUNAE_CONSUME_LIFE_ENERGY, 21000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Setesh

class SeteshAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SeteshAI(c); }
    explicit SeteshAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_SETESH, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_SETESH_CHAOS_BOLT, 5000);
        scriptEvents.addEvent(EVENT_SETESH_SEED_OF_CHAOS, 20000);
        scriptEvents.addEvent(EVENT_SETESH_REIGN_OF_CHAOS, 30000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_SETESH, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_SETESH, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_SETESH_CHAOS_BOLT:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_SETESH_CHAOS_BOLT);
                scriptEvents.addEvent(EVENT_SETESH_CHAOS_BOLT, 6000);
                break;
            case EVENT_SETESH_SEED_OF_CHAOS:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_SETESH_SEED_OF_CHAOS);
                scriptEvents.addEvent(EVENT_SETESH_SEED_OF_CHAOS, 20000);
                break;
            case EVENT_SETESH_REIGN_OF_CHAOS:
                castSpellAOE(SPELL_SETESH_REIGN_OF_CHAOS);
                scriptEvents.addEvent(EVENT_SETESH_REIGN_OF_CHAOS, 30000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Rajh

class RajhAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new RajhAI(c); }
    explicit RajhAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "Defilers! Wretches! Fiends! Begone from here!");
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_RAJH, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_RAJH_SUMMON_SUN_ORB, 10000);
        scriptEvents.addEvent(EVENT_RAJH_SUN_STRIKE, 20000);
        scriptEvents.addEvent(EVENT_RAJH_BLAZING_INFERNO, 14000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_RAJH, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "Blazing rays of light, take me!");
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_RAJH, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_RAJH_SUMMON_SUN_ORB:
                castSpellOnSelf(SPELL_RAJH_SUMMON_SUN_ORB);
                scriptEvents.addEvent(EVENT_RAJH_SUMMON_SUN_ORB, 25000);
                break;
            case EVENT_RAJH_SUN_STRIKE:
                castSpellOnVictim(SPELL_RAJH_SUN_STRIKE);
                scriptEvents.addEvent(EVENT_RAJH_SUN_STRIKE, 18000);
                break;
            case EVENT_RAJH_BLAZING_INFERNO:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_RAJH_BLAZING_INFERNO);
                scriptEvents.addEvent(EVENT_RAJH_BLAZING_INFERNO, 20000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Trash: Sun-Touched Servant / Sprite / Spriteling all share the same combat kit.

class SunTouchedServantAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SunTouchedServantAI(c); }
    explicit SunTouchedServantAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_SUN_TOUCHED_SEARING_FLAMES, 5000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_SUN_TOUCHED_SEARING_FLAMES)
        {
            castSpellOnVictim(SPELL_SUN_TOUCHED_SEARING_FLAMES);
            scriptEvents.addEvent(EVENT_SUN_TOUCHED_SEARING_FLAMES, 6000);
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// More trash

class SunTouchedSpeakerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SunTouchedSpeakerAI(c); }
    explicit SunTouchedSpeakerAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.resetEvents();
        scriptEvents.addEvent(EVENT_SPEAKER_RANDOM, 4000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        scriptEvents.resetEvents();
        scriptEvents.addEvent(EVENT_SPEAKER_OUT_OF_COMBAT, 3000);
    }

    void OnLoad() override
    {
        scriptEvents.addEvent(EVENT_SPEAKER_OUT_OF_COMBAT, 3000);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_SPEAKER_OUT_OF_COMBAT:
                castSpellOnSelf(SPELL_SPEAKER_OUT_OF_COMBAT);
                scriptEvents.addEvent(EVENT_SPEAKER_OUT_OF_COMBAT, 12000);
                break;
            case EVENT_SPEAKER_RANDOM:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_SPEAKER_RANDOM);
                scriptEvents.addEvent(EVENT_SPEAKER_RANDOM, 4000);
                break;
            default:
                break;
        }
    }
};

class PitViperAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new PitViperAI(c); }
    explicit PitViperAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override { scriptEvents.addEvent(EVENT_PIT_VIPER_VICTIM, 9000); }
    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_PIT_VIPER_VICTIM)
        {
            castSpellOnVictim(SPELL_PIT_VIPER_VICTIM);
            scriptEvents.addEvent(EVENT_PIT_VIPER_VICTIM, 8000);
        }
    }
};

class DustboneTormentorAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DustboneTormentorAI(c); }
    explicit DustboneTormentorAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_TORMENTOR_CURSE_OF_EXHAUSTION, 6000);
        scriptEvents.addEvent(EVENT_TORMENTOR_SHADOW_BOLT, 7000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_TORMENTOR_CURSE_OF_EXHAUSTION:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_TORMENTOR_CURSE_OF_EXHAUSTION);
                scriptEvents.addEvent(EVENT_TORMENTOR_CURSE_OF_EXHAUSTION, 44000);
                break;
            case EVENT_TORMENTOR_SHADOW_BOLT:
                castSpellOnVictim(SPELL_TORMENTOR_SHADOW_BOLT);
                scriptEvents.addEvent(EVENT_TORMENTOR_SHADOW_BOLT, 2400);
                break;
            default:
                break;
        }
    }
};

// Simple Dustbone Horror - just melees and smashes.
class DustboneHorrorSimpleAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DustboneHorrorSimpleAI(c); }
    explicit DustboneHorrorSimpleAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override { scriptEvents.addEvent(EVENT_DUSTBONE_SMASH, 10000); }
    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_DUSTBONE_SMASH)
        {
            castSpellOnVictim(SPELL_DUSTBONE_SMASH);
            scriptEvents.addEvent(EVENT_DUSTBONE_SMASH, 12000);
        }
    }
};

// This Dustbone Horror submerges and flees once it drops below 20% health instead of dying
// normally, matching the reference's "stop attacking, go passive, submerge, despawn" sequence.
class DustboneHorrorSubmergingAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DustboneHorrorSubmergingAI(c); }
    explicit DustboneHorrorSubmergingAI(Creature* pCreature) : CreatureAIScript(pCreature), mSubmerged(false) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_DUSTBONE_SMASH, 4000);
        mSubmerged = false;
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (!mSubmerged && _getHealthPercent() <= 20)
        {
            attackStop();
            setReactState(REACT_PASSIVE);
            castSpellOnSelf(SPELL_DUSTBONE_SUBMERGE, true);
            mSubmerged = true;
            despawn(3000, 0);
            return;
        }

        if (_isCasting() || mSubmerged)
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_DUSTBONE_SMASH)
        {
            castSpellOnVictim(SPELL_DUSTBONE_SMASH);
            scriptEvents.addEvent(EVENT_DUSTBONE_SMASH, 10000);
        }
    }

private:
    bool mSubmerged;
};

class TempleSwiftstalkerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TempleSwiftstalkerAI(c); }
    explicit TempleSwiftstalkerAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_SWIFTSTALKER_SHOOT, 1000);
        scriptEvents.addEvent(EVENT_SWIFTSTALKER_CHARGED_SHOT, 6000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_SWIFTSTALKER_SHOOT:
                castSpellOnVictim(SPELL_SWIFTSTALKER_SHOOT);
                scriptEvents.addEvent(EVENT_SWIFTSTALKER_SHOOT, 2400);
                break;
            case EVENT_SWIFTSTALKER_CHARGED_SHOT:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_SWIFTSTALKER_CHARGED_SHOT);
                scriptEvents.addEvent(EVENT_SWIFTSTALKER_CHARGED_SHOT, 26000);
                break;
            default:
                break;
        }
    }
};

class TempleRunecasterAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TempleRunecasterAI(c); }
    explicit TempleRunecasterAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_RUNECASTER_RUNIC_CLEAVE, 5000);
        scriptEvents.addEvent(EVENT_RUNECASTER_CURSE, 9000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_RUNECASTER_RUNIC_CLEAVE:
                castSpellOnVictim(SPELL_RUNECASTER_RUNIC_CLEAVE);
                scriptEvents.addEvent(EVENT_RUNECASTER_RUNIC_CLEAVE, 10000);
                break;
            case EVENT_RUNECASTER_CURSE:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_RUNECASTER_CURSE);
                scriptEvents.addEvent(EVENT_RUNECASTER_CURSE, 34000);
                break;
            default:
                break;
        }
    }
};

class TempleShadowlancerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TempleShadowlancerAI(c); }
    explicit TempleShadowlancerAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_SHADOWLANCER_SHADOWLANCE, 7000);
        scriptEvents.addEvent(EVENT_SHADOWLANCER_PACT_OF_DARKNESS, 8000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_SHADOWLANCER_SHADOWLANCE:
                castSpellOnSelf(SPELL_SHADOWLANCER_SHADOWLANCE);
                scriptEvents.addEvent(EVENT_SHADOWLANCER_SHADOWLANCE, 20000);
                break;
            case EVENT_SHADOWLANCER_PACT_OF_DARKNESS:
                castSpellOnSelf(SPELL_SHADOWLANCER_PACT_OF_DARKNESS);
                scriptEvents.addEvent(EVENT_SHADOWLANCER_PACT_OF_DARKNESS, 24000);
                break;
            default:
                break;
        }
    }
};

class TempleFireshaperAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TempleFireshaperAI(c); }
    explicit TempleFireshaperAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_FIRESHAPER_FIREBALL, 1);
        scriptEvents.addEvent(EVENT_FIRESHAPER_MOLTEN_BARRIER, 9000);
        scriptEvents.addEvent(EVENT_FIRESHAPER_METEOR, 15000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_FIRESHAPER_FIREBALL:
                castSpellOnVictim(SPELL_FIRESHAPER_FIREBALL);
                scriptEvents.addEvent(EVENT_FIRESHAPER_FIREBALL, 1600);
                break;
            case EVENT_FIRESHAPER_MOLTEN_BARRIER:
                castSpellOnSelf(SPELL_FIRESHAPER_MOLTEN_BARRIER);
                scriptEvents.addEvent(EVENT_FIRESHAPER_MOLTEN_BARRIER, 30000);
                break;
            case EVENT_FIRESHAPER_METEOR:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_FIRESHAPER_METEOR);
                scriptEvents.addEvent(EVENT_FIRESHAPER_METEOR, 25000);
                break;
            default:
                break;
        }
    }
};

void SetupHallsOfOrigination(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_HALLS_OF_ORIGINATION, &HallsOfOriginationInstanceScript::Create);

    mgr->register_creature_script(BOSS_TEMPLE_GUARDIAN_ANHUUR, &TempleGuardianAnhuurAI::Create);
    mgr->register_creature_script(BOSS_EARTHRAGER_PTAH, &EarthragerPtahAI::Create);
    mgr->register_creature_script(BOSS_ANRAPHET, &AnraphetAI::Create);
    mgr->register_creature_script(BOSS_ISISET, &IsisetAI::Create);
    mgr->register_creature_script(BOSS_AMMUNAE, &AmmunaeAI::Create);
    mgr->register_creature_script(BOSS_SETESH, &SeteshAI::Create);
    mgr->register_creature_script(BOSS_RAJH, &RajhAI::Create);

    mgr->register_creature_script(NPC_SUN_TOUCHED_SERVANT, &SunTouchedServantAI::Create);
    mgr->register_creature_script(NPC_SUN_TOUCHED_SPRITE, &SunTouchedServantAI::Create);
    mgr->register_creature_script(NPC_SUN_TOUCHED_SPRITELING, &SunTouchedServantAI::Create);

    mgr->register_creature_script(NPC_SUN_TOUCHED_SPEAKER, &SunTouchedSpeakerAI::Create);
    mgr->register_creature_script(NPC_PIT_VIPER, &PitViperAI::Create);
    mgr->register_creature_script(NPC_DUSTBONE_TORMENTOR, &DustboneTormentorAI::Create);
    mgr->register_creature_script(NPC_DUSTBONE_HORROR_SIMPLE, &DustboneHorrorSimpleAI::Create);
    mgr->register_creature_script(NPC_DUSTBONE_HORROR_SUBMERGING, &DustboneHorrorSubmergingAI::Create);
    mgr->register_creature_script(NPC_TEMPLE_SWIFTSTALKER, &TempleSwiftstalkerAI::Create);
    mgr->register_creature_script(NPC_TEMPLE_RUNECASTER, &TempleRunecasterAI::Create);
    mgr->register_creature_script(NPC_TEMPLE_SHADOWLANCER, &TempleShadowlancerAI::Create);
    mgr->register_creature_script(NPC_TEMPLE_FIRESHAPER, &TempleFireshaperAI::Create);
}
