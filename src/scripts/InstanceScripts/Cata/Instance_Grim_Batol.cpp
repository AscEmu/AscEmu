/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

// Grim Batol - four bosses (verified against wowhead), hand-ported into AscEmu's own
// CreatureAIScript / InstanceScript framework.
//
// Simplifications versus the original fight design: several summon-trigger chains (Umbriss'
// Blitz/Ground Siege stalkers, Erudax's Shadow Gale trigger) are collapsed into direct casts,
// and the weapon-swapping phase gimmick on Forgemaster Throngus and the ground/vehicle phase
// split on Drahga Shadowburner (mounted on the captive dragon Valiona) are dropped in favor of
// a single continuous rotation built from each phase's real abilities.

#include "Setup.h"
#include "Instance_Grim_Batol.hpp"

#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

enum GrimBatolEvents
{
    EVENT_UMBRISS_BLEEDING_WOUND = 1,
    EVENT_UMBRISS_BLITZ,
    EVENT_UMBRISS_GROUND_SIEGE,
    EVENT_UMBRISS_SUMMON_SKARDYN,

    EVENT_THRONGUS_MIGHTY_STOMP,
    EVENT_THRONGUS_DISORIENTING_ROAR,
    EVENT_THRONGUS_IMPALING_SLAM,
    EVENT_THRONGUS_FLAMING_SHIELD,

    EVENT_DRAHGA_BURNING_SHADOWBOLT,
    EVENT_DRAHGA_INVOCATION_OF_FLAME,
    EVENT_DRAHGA_DEVOURING_FLAMES,

    EVENT_ERUDAX_BINDING_SHADOWS,
    EVENT_ERUDAX_ENFEEBLING_BLOW,
    EVENT_ERUDAX_SHADOW_GALE,
    EVENT_ERUDAX_SUMMON_FACELESS_CORRUPTOR,

    EVENT_BEGUILER_DECEITFUL_BLAST,
    EVENT_BEGUILER_CHAINED_MIND,

    EVENT_TROGG_DWELLER_CLAW_PUNCTURE,
    EVENT_TWILIGHT_ENFORCER_MEAT_GRINDER,
    EVENT_EARTHSHAPER_EARTH_SPIKE,
    EVENT_ARMSMASTER_MORTAL_STRIKE,
    EVENT_ARMSMASTER_FLURRY_OF_BLOWS,
    EVENT_AZUREBORNE_GUARDIAN_CURSE,
    EVENT_AZUREBORNE_GUARDIAN_ARCANE_INFUSION,
    EVENT_STORMBREAKER_WATER_BOLT,
    EVENT_CRIMSONBORNE_GUARDIAN_SHOCKWAVE,
    EVENT_AZUREBORNE_SEER_TWILIGHT_BOLT,
    EVENT_ENSLAVED_GRONN_BRUTE_SMASH
};

class GrimBatolInstanceScript : public InstanceScript
{
public:
    explicit GrimBatolInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(GrimBatolEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new GrimBatolInstanceScript(pMapMgr); }
};

//////////////////////////////////////////////////////////////////////////////////////////
// General Umbriss

class GeneralUmbrissAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new GeneralUmbrissAI(c); }
    explicit GeneralUmbrissAI(Creature* pCreature) : CreatureAIScript(pCreature), mFrenzyCast(false) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "Attack you cowardly vermin!");
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_GENERAL_UMBRISS, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_UMBRISS_SUMMON_SKARDYN, 6000);
        scriptEvents.addEvent(EVENT_UMBRISS_BLEEDING_WOUND, 11000);
        scriptEvents.addEvent(EVENT_UMBRISS_GROUND_SIEGE, 25500);
        scriptEvents.addEvent(EVENT_UMBRISS_BLITZ, 16000);
        mFrenzyCast = false;
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_GENERAL_UMBRISS, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "Messy...");
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_GENERAL_UMBRISS, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (!mFrenzyCast && _getHealthPercent() <= 30)
        {
            sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "General Umbriss goes into a frenzy!");
            castSpellOnSelf(SPELL_UMBRISS_FRENZY, true);
            mFrenzyCast = true;
        }

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_UMBRISS_BLEEDING_WOUND:
                castSpellOnVictim(SPELL_UMBRISS_BLEEDING_WOUND);
                scriptEvents.addEvent(EVENT_UMBRISS_BLEEDING_WOUND, 14000);
                break;
            case EVENT_UMBRISS_BLITZ:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "General Umbriss sets his eyes on you and begins to cast Blitz!");
                castSpellAOE(SPELL_UMBRISS_BLITZ);
                scriptEvents.addEvent(EVENT_UMBRISS_BLITZ, 20000);
                break;
            case EVENT_UMBRISS_GROUND_SIEGE:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "General Umbriss begins to cast Ground Siege!");
                castSpellAOE(SPELL_UMBRISS_GROUND_SIEGE);
                scriptEvents.addEvent(EVENT_UMBRISS_GROUND_SIEGE, 25000);
                break;
            case EVENT_UMBRISS_SUMMON_SKARDYN:
                castSpellOnSelf(SPELL_UMBRISS_SUMMON_SKARDYN);
                scriptEvents.addEvent(EVENT_UMBRISS_SUMMON_SKARDYN, 30000);
                break;
            default:
                break;
        }
    }

private:
    bool mFrenzyCast;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Forgemaster Throngus

class ForgemasterThrongusAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ForgemasterThrongusAI(c); }
    explicit ForgemasterThrongusAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "You not get through defenses!");
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_FORGEMASTER_THRONGUS, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_THRONGUS_MIGHTY_STOMP, 7000);
        scriptEvents.addEvent(EVENT_THRONGUS_DISORIENTING_ROAR, 17000);
        scriptEvents.addEvent(EVENT_THRONGUS_IMPALING_SLAM, 24000);
        scriptEvents.addEvent(EVENT_THRONGUS_FLAMING_SHIELD, 13000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_FORGEMASTER_THRONGUS, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "Death... Good choice. Not best choice maybe, but better than fail and live.");
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_FORGEMASTER_THRONGUS, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_THRONGUS_MIGHTY_STOMP:
                castSpellAOE(SPELL_THRONGUS_MIGHTY_STOMP);
                scriptEvents.addEvent(EVENT_THRONGUS_MIGHTY_STOMP, 18000);
                break;
            case EVENT_THRONGUS_DISORIENTING_ROAR:
                castSpellAOE(SPELL_THRONGUS_DISORIENTING_ROAR);
                scriptEvents.addEvent(EVENT_THRONGUS_DISORIENTING_ROAR, 25000);
                break;
            case EVENT_THRONGUS_IMPALING_SLAM:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "You are impaled!");
                castSpellOnVictim(SPELL_THRONGUS_IMPALING_SLAM);
                scriptEvents.addEvent(EVENT_THRONGUS_IMPALING_SLAM, 22000);
                break;
            case EVENT_THRONGUS_FLAMING_SHIELD:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_THRONGUS_FLAMING_SHIELD);
                scriptEvents.addEvent(EVENT_THRONGUS_FLAMING_SHIELD, 20000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Drahga Shadowburner

class DrahgaShadowburnerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DrahgaShadowburnerAI(c); }
    explicit DrahgaShadowburnerAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "I will burn you from the inside out!");
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_DRAHGA_SHADOWBURNER, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_DRAHGA_BURNING_SHADOWBOLT, 3000);
        scriptEvents.addEvent(EVENT_DRAHGA_INVOCATION_OF_FLAME, 10000);
        scriptEvents.addEvent(EVENT_DRAHGA_DEVOURING_FLAMES, 18000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_DRAHGA_SHADOWBURNER, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_DRAHGA_SHADOWBURNER, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_DRAHGA_BURNING_SHADOWBOLT:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_DRAHGA_BURNING_SHADOWBOLT);
                scriptEvents.addEvent(EVENT_DRAHGA_BURNING_SHADOWBOLT, 4000);
                break;
            case EVENT_DRAHGA_INVOCATION_OF_FLAME:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Drahga Shadowburner summons an Invocation of Flame!");
                castSpellOnSelf(SPELL_DRAHGA_INVOCATION_OF_FLAME);
                scriptEvents.addEvent(EVENT_DRAHGA_INVOCATION_OF_FLAME, 20000);
                break;
            case EVENT_DRAHGA_DEVOURING_FLAMES:
                castSpellAOE(SPELL_DRAHGA_DEVOURING_FLAMES_AOE);
                scriptEvents.addEvent(EVENT_DRAHGA_DEVOURING_FLAMES, 22000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Erudax

class ErudaxAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ErudaxAI(c); }
    explicit ErudaxAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "The darkest days are still ahead!");
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_ERUDAX, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_ERUDAX_BINDING_SHADOWS, 10500);
        scriptEvents.addEvent(EVENT_ERUDAX_ENFEEBLING_BLOW, 19000);
        scriptEvents.addEvent(EVENT_ERUDAX_SHADOW_GALE, 26000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_ERUDAX, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_ERUDAX, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_ERUDAX_BINDING_SHADOWS:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_ERUDAX_BINDING_SHADOWS);
                scriptEvents.addEvent(EVENT_ERUDAX_BINDING_SHADOWS, 20000);
                break;
            case EVENT_ERUDAX_ENFEEBLING_BLOW:
                castSpellOnVictim(getRaidModeValue(SPELL_ERUDAX_ENFEEBLING_BLOW_10, SPELL_ERUDAX_ENFEEBLING_BLOW_25, SPELL_ERUDAX_ENFEEBLING_BLOW_10, SPELL_ERUDAX_ENFEEBLING_BLOW_25));
                scriptEvents.addEvent(EVENT_ERUDAX_ENFEEBLING_BLOW, 21000);
                break;
            case EVENT_ERUDAX_SHADOW_GALE:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Erudax begins to cast Shadow Gale!");
                castSpellAOE(SPELL_ERUDAX_SHADOW_GALE);
                scriptEvents.addEvent(EVENT_ERUDAX_SUMMON_FACELESS_CORRUPTOR, 18000);
                scriptEvents.addEvent(EVENT_ERUDAX_SHADOW_GALE, 30000);
                break;
            case EVENT_ERUDAX_SUMMON_FACELESS_CORRUPTOR:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Erudax summons a Faceless Guardian!");
                castSpellOnSelf(SPELL_ERUDAX_SUMMON_FACELESS_CORRUPTOR);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Trash: Twilight Beguiler (found near the enslaved gronn packs on the way to Forgemaster
// Throngus). The gronn-enslaving cosmetic sequence isn't ported - it's given a plain combat
// rotation instead.

class TwilightBeguilerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TwilightBeguilerAI(c); }
    explicit TwilightBeguilerAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_BEGUILER_DECEITFUL_BLAST, 7000);
        scriptEvents.addEvent(EVENT_BEGUILER_CHAINED_MIND, 7000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        scriptEvents.resetEvents();
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_BEGUILER_DECEITFUL_BLAST:
                castSpellOnVictim(SPELL_BEGUILER_DECEITFUL_BLAST);
                scriptEvents.addEvent(EVENT_BEGUILER_DECEITFUL_BLAST, 16000);
                break;
            case EVENT_BEGUILER_CHAINED_MIND:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_BEGUILER_CHAINED_MIND);
                scriptEvents.addEvent(EVENT_BEGUILER_CHAINED_MIND, 12000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// More trash

class TroggDwellerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TroggDwellerAI(c); }
    explicit TroggDwellerAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override { scriptEvents.addEvent(EVENT_TROGG_DWELLER_CLAW_PUNCTURE, 2000); }
    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_TROGG_DWELLER_CLAW_PUNCTURE)
        {
            castSpellOnVictim(SPELL_TROGG_DWELLER_CLAW_PUNCTURE);
            scriptEvents.addEvent(EVENT_TROGG_DWELLER_CLAW_PUNCTURE, 3000);
        }
    }
};

class TwilightEnforcerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TwilightEnforcerAI(c); }
    explicit TwilightEnforcerAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override { scriptEvents.addEvent(EVENT_TWILIGHT_ENFORCER_MEAT_GRINDER, 4000); }
    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_TWILIGHT_ENFORCER_MEAT_GRINDER)
        {
            castSpellOnSelf(SPELL_TWILIGHT_ENFORCER_MEAT_GRINDER);
            scriptEvents.addEvent(EVENT_TWILIGHT_ENFORCER_MEAT_GRINDER, 12000);
        }
    }
};

class TwilightEarthshaperAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TwilightEarthshaperAI(c); }
    explicit TwilightEarthshaperAI(Creature* pCreature) : CreatureAIScript(pCreature), mStoneSkinCast(false) {}

    void OnLoad() override
    {
        castSpellOnSelf(SPELL_EARTHSHAPER_SUMMON_ROCK_ELEMENTAL, true);
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_EARTHSHAPER_EARTH_SPIKE, 2000);
        mStoneSkinCast = false;
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (!mStoneSkinCast && _getHealthPercent() <= 55)
        {
            castSpellOnSelf(SPELL_EARTHSHAPER_STONE_SKIN);
            mStoneSkinCast = true;
        }

        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_EARTHSHAPER_EARTH_SPIKE)
        {
            castSpellOnVictim(SPELL_EARTHSHAPER_EARTH_SPIKE);
            scriptEvents.addEvent(EVENT_EARTHSHAPER_EARTH_SPIKE, 3000);
        }
    }

private:
    bool mStoneSkinCast;
};

class TwilightArmsmasterAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TwilightArmsmasterAI(c); }
    explicit TwilightArmsmasterAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_ARMSMASTER_MORTAL_STRIKE, 4000);
        scriptEvents.addEvent(EVENT_ARMSMASTER_FLURRY_OF_BLOWS, 5000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_ARMSMASTER_MORTAL_STRIKE:
                castSpellOnVictim(SPELL_ARMSMASTER_MORTAL_STRIKE);
                scriptEvents.addEvent(EVENT_ARMSMASTER_MORTAL_STRIKE, 14000);
                break;
            case EVENT_ARMSMASTER_FLURRY_OF_BLOWS:
                castSpellOnVictim(SPELL_ARMSMASTER_FLURRY_OF_BLOWS);
                scriptEvents.addEvent(EVENT_ARMSMASTER_FLURRY_OF_BLOWS, 6000);
                break;
            default:
                break;
        }
    }
};

class AzureborneGuardianAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AzureborneGuardianAI(c); }
    explicit AzureborneGuardianAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_AZUREBORNE_GUARDIAN_CURSE, 2000);
        scriptEvents.addEvent(EVENT_AZUREBORNE_GUARDIAN_ARCANE_INFUSION, 5000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_AZUREBORNE_GUARDIAN_CURSE:
                castSpellOnVictim(SPELL_AZUREBORNE_GUARDIAN_CURSE);
                scriptEvents.addEvent(EVENT_AZUREBORNE_GUARDIAN_CURSE, 3000);
                break;
            case EVENT_AZUREBORNE_GUARDIAN_ARCANE_INFUSION:
                castSpellOnSelf(SPELL_AZUREBORNE_GUARDIAN_ARCANE_INFUSION);
                scriptEvents.addEvent(EVENT_AZUREBORNE_GUARDIAN_ARCANE_INFUSION, 8000);
                break;
            default:
                break;
        }
    }
};

class TwilightStormbreakerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TwilightStormbreakerAI(c); }
    explicit TwilightStormbreakerAI(Creature* pCreature) : CreatureAIScript(pCreature), mShellCast(false) {}

    void OnLoad() override
    {
        castSpellOnSelf(SPELL_STORMBREAKER_SUMMON_WATER_SPIRIT, true);
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_STORMBREAKER_WATER_BOLT, 1);
        mShellCast = false;
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (!mShellCast && _getHealthPercent() <= 55)
        {
            castSpellOnSelf(SPELL_STORMBREAKER_WATER_SHELL);
            mShellCast = true;
        }

        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_STORMBREAKER_WATER_BOLT)
        {
            castSpellOnVictim(SPELL_STORMBREAKER_WATER_BOLT);
            scriptEvents.addEvent(EVENT_STORMBREAKER_WATER_BOLT, 3000);
        }
    }

private:
    bool mShellCast;
};

class CrimsonborneGuardianAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CrimsonborneGuardianAI(c); }
    explicit CrimsonborneGuardianAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        castSpellOnVictim(SPELL_CRIMSONBORNE_GUARDIAN_CHARGE);
        scriptEvents.addEvent(EVENT_CRIMSONBORNE_GUARDIAN_SHOCKWAVE, 4000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_CRIMSONBORNE_GUARDIAN_SHOCKWAVE)
        {
            castSpellOnVictim(SPELL_CRIMSONBORNE_GUARDIAN_SHOCKWAVE);
            scriptEvents.addEvent(EVENT_CRIMSONBORNE_GUARDIAN_SHOCKWAVE, 5000);
        }
    }
};

class AzureborneSeerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AzureborneSeerAI(c); }
    explicit AzureborneSeerAI(Creature* pCreature) : CreatureAIScript(pCreature), mWarpedCast(false), mArcaneCast(false) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_AZUREBORNE_SEER_TWILIGHT_BOLT, 2000);
        mWarpedCast = false;
        mArcaneCast = false;
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (!mWarpedCast && _getHealthPercent() <= 20)
        {
            castSpellOnVictim(SPELL_AZUREBORNE_SEER_WARPED_TWILIGHT);
            mWarpedCast = true;
        }
        if (!mArcaneCast && _getHealthPercent() <= 40)
        {
            castSpellOnSelf(SPELL_AZUREBORNE_SEER_TWISTED_ARCANE);
            mArcaneCast = true;
        }

        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_AZUREBORNE_SEER_TWILIGHT_BOLT)
        {
            castSpellOnVictim(SPELL_AZUREBORNE_SEER_TWILIGHT_BOLT);
            scriptEvents.addEvent(EVENT_AZUREBORNE_SEER_TWILIGHT_BOLT, 3000);
        }
    }

private:
    bool mWarpedCast;
    bool mArcaneCast;
};

class EnslavedGronnBruteAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EnslavedGronnBruteAI(c); }
    explicit EnslavedGronnBruteAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override { scriptEvents.addEvent(EVENT_ENSLAVED_GRONN_BRUTE_SMASH, 4000); }
    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_ENSLAVED_GRONN_BRUTE_SMASH)
        {
            castSpellOnVictim(SPELL_ENSLAVED_GRONN_BRUTE_SMASH);
            scriptEvents.addEvent(EVENT_ENSLAVED_GRONN_BRUTE_SMASH, 11000);
        }
    }
};

void SetupGrimBatol(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_GRIM_BATOL, &GrimBatolInstanceScript::Create);

    mgr->register_creature_script(BOSS_GENERAL_UMBRISS, &GeneralUmbrissAI::Create);
    mgr->register_creature_script(BOSS_FORGEMASTER_THRONGUS, &ForgemasterThrongusAI::Create);
    mgr->register_creature_script(BOSS_DRAHGA_SHADOWBURNER, &DrahgaShadowburnerAI::Create);
    mgr->register_creature_script(BOSS_ERUDAX, &ErudaxAI::Create);
    mgr->register_creature_script(NPC_TWILIGHT_BEGUILER, &TwilightBeguilerAI::Create);

    mgr->register_creature_script(NPC_TROGG_DWELLER, &TroggDwellerAI::Create);
    mgr->register_creature_script(NPC_TWILIGHT_ENFORCER, &TwilightEnforcerAI::Create);
    mgr->register_creature_script(NPC_TWILIGHT_EARTHSHAPER, &TwilightEarthshaperAI::Create);
    mgr->register_creature_script(NPC_TWILIGHT_ARMSMASTER, &TwilightArmsmasterAI::Create);
    mgr->register_creature_script(NPC_AZUREBORNE_GUARDIAN, &AzureborneGuardianAI::Create);
    mgr->register_creature_script(NPC_TWILIGHT_STORMBREAKER, &TwilightStormbreakerAI::Create);
    mgr->register_creature_script(NPC_CRIMSONBORNE_GUARDIAN, &CrimsonborneGuardianAI::Create);
    mgr->register_creature_script(NPC_AZUREBORNE_SEER, &AzureborneSeerAI::Create);
    mgr->register_creature_script(NPC_ENSLAVED_GRONN_BRUTE, &EnslavedGronnBruteAI::Create);
}
