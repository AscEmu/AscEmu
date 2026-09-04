/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Hour_Of_Twilight.hpp"

#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

enum HourOfTwilightEvents
{
    EVENT_TORUNSCAR_EARTHEN_VORTEX = 1,
    EVENT_WINTERBLUFF_FROST_BEACON,
    EVENT_DIMBLAZE_WILDFIRE,
    EVENT_JALARA_STATIC_CLING,

    EVENT_ASIRA_SHADOW_BLADES,
    EVENT_ASIRA_VOID_BLAST,
    EVENT_ASIRA_DARK_MENDING,

    EVENT_BENEDICTUS_TWILIGHT_BARRAGE,
    EVENT_BENEDICTUS_DOMINATE_MIND,
    EVENT_BENEDICTUS_HOLY_FIRE,

    EVENT_MAMMOTH_BULL_TRAMPLE,
    EVENT_JORMUNGAR_ACID_SPIT,
    EVENT_MAGNATAUR_PATRIARCH_SMASH,
    EVENT_WASTES_DIGGER_CLAW,
    EVENT_SNOWPLAIN_DISCIPLE_STRIKE,
    EVENT_SNOWPLAIN_DISCIPLE_HEAL,
    EVENT_SNOWPLAIN_SHAMAN_LIGHTNING
};

class HourOfTwilightInstanceScript : public InstanceScript
{
public:
    explicit HourOfTwilightInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(HourOfTwilightEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new HourOfTwilightInstanceScript(pMapMgr); }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Council of Twilight - four casters fought together as a single encounter/wipe unit; each
// runs its own rotation and reports into the shared DATA_COUNCIL_OF_TWILIGHT boss state.

class CouncilOfTwilightMemberAI : public CreatureAIScript
{
public:
    explicit CouncilOfTwilightMemberAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->setBossState(DATA_COUNCIL_OF_TWILIGHT, EncounterStates::InProgress);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        scriptEvents.resetEvents();
        getInstanceScript()->setBossState(DATA_COUNCIL_OF_TWILIGHT, EncounterStates::Failed);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->setBossState(DATA_COUNCIL_OF_TWILIGHT, EncounterStates::Performed);
    }
};

class EarthcallerTorunscarAI : public CouncilOfTwilightMemberAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EarthcallerTorunscarAI(c); }
    explicit EarthcallerTorunscarAI(Creature* pCreature) : CouncilOfTwilightMemberAI(pCreature) {}

    void OnCombatStart(Unit* pTarget) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "The earth itself rejects you!");
        CouncilOfTwilightMemberAI::OnCombatStart(pTarget);
        scriptEvents.addEvent(EVENT_TORUNSCAR_EARTHEN_VORTEX, 9000);
    }

    void OnDied(Unit* pKiller) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "The mountain... crumbles...");
        CouncilOfTwilightMemberAI::OnDied(pKiller);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_TORUNSCAR_EARTHEN_VORTEX)
        {
            castSpellAOE(SPELL_TORUNSCAR_EARTHEN_VORTEX);
            scriptEvents.addEvent(EVENT_TORUNSCAR_EARTHEN_VORTEX, 18000);
        }
    }
};

class TawnWinterbluffAI : public CouncilOfTwilightMemberAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TawnWinterbluffAI(c); }
    explicit TawnWinterbluffAI(Creature* pCreature) : CouncilOfTwilightMemberAI(pCreature) {}

    void OnCombatStart(Unit* pTarget) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "Feel the chill of the void!");
        CouncilOfTwilightMemberAI::OnCombatStart(pTarget);
        scriptEvents.addEvent(EVENT_WINTERBLUFF_FROST_BEACON, 12000);
    }

    void OnDied(Unit* pKiller) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "So... cold...");
        CouncilOfTwilightMemberAI::OnDied(pKiller);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_WINTERBLUFF_FROST_BEACON)
        {
            if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                castSpell(pTarget, SPELL_WINTERBLUFF_FROST_BEACON);
            scriptEvents.addEvent(EVENT_WINTERBLUFF_FROST_BEACON, 20000);
        }
    }
};

class HargothDimblazeAI : public CouncilOfTwilightMemberAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HargothDimblazeAI(c); }
    explicit HargothDimblazeAI(Creature* pCreature) : CouncilOfTwilightMemberAI(pCreature) {}

    void OnCombatStart(Unit* pTarget) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "Burn, little ones!");
        CouncilOfTwilightMemberAI::OnCombatStart(pTarget);
        scriptEvents.addEvent(EVENT_DIMBLAZE_WILDFIRE, 10000);
    }

    void OnDied(Unit* pKiller) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "The flame... gutters out...");
        CouncilOfTwilightMemberAI::OnDied(pKiller);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_DIMBLAZE_WILDFIRE)
        {
            castSpellOnVictim(SPELL_DIMBLAZE_WILDFIRE);
            scriptEvents.addEvent(EVENT_DIMBLAZE_WILDFIRE, 16000);
        }
    }
};

class StormcallerJalaraAI : public CouncilOfTwilightMemberAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new StormcallerJalaraAI(c); }
    explicit StormcallerJalaraAI(Creature* pCreature) : CouncilOfTwilightMemberAI(pCreature) {}

    void OnCombatStart(Unit* pTarget) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "The storm answers my call!");
        CouncilOfTwilightMemberAI::OnCombatStart(pTarget);
        scriptEvents.addEvent(EVENT_JALARA_STATIC_CLING, 14000);
    }

    void OnDied(Unit* pKiller) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "The winds... fall silent...");
        CouncilOfTwilightMemberAI::OnDied(pKiller);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_JALARA_STATIC_CLING)
        {
            if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                castSpell(pTarget, SPELL_JALARA_STATIC_CLING);
            scriptEvents.addEvent(EVENT_JALARA_STATIC_CLING, 20000);
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Asira Dawnslayer

class AsiraDawnslayerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AsiraDawnslayerAI(c); }
    explicit AsiraDawnslayerAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "You will not leave this hall alive!");
        getInstanceScript()->setBossState(DATA_ASIRA_DAWNSLAYER, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_ASIRA_SHADOW_BLADES, 8000);
        scriptEvents.addEvent(EVENT_ASIRA_VOID_BLAST, 14000);
        scriptEvents.addEvent(EVENT_ASIRA_DARK_MENDING, 22000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        scriptEvents.resetEvents();
        getInstanceScript()->setBossState(DATA_ASIRA_DAWNSLAYER, EncounterStates::Failed);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "The Twilight's Hammer... will avenge me...");
        getInstanceScript()->setBossState(DATA_ASIRA_DAWNSLAYER, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (!mMendCast && _getHealthPercent() <= 40)
        {
            mMendCast = true;
            sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Asira Dawnslayer channels dark mending magic!");
            castSpellOnSelf(SPELL_ASIRA_DARK_MENDING);
        }

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_ASIRA_SHADOW_BLADES:
                castSpellOnVictim(SPELL_ASIRA_SHADOW_BLADES);
                scriptEvents.addEvent(EVENT_ASIRA_SHADOW_BLADES, 12000);
                break;
            case EVENT_ASIRA_VOID_BLAST:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_ASIRA_VOID_BLAST);
                scriptEvents.addEvent(EVENT_ASIRA_VOID_BLAST, 18000);
                break;
            default:
                break;
        }
    }

private:
    bool mMendCast = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Archbishop Benedictus

class ArchbishopBenedictusAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ArchbishopBenedictusAI(c); }
    explicit ArchbishopBenedictusAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "The Twilight Father demands your suffering!");
        getInstanceScript()->setBossState(DATA_ARCHBISHOP_BENEDICTUS, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_BENEDICTUS_TWILIGHT_BARRAGE, 10000);
        scriptEvents.addEvent(EVENT_BENEDICTUS_DOMINATE_MIND, 20000);
        scriptEvents.addEvent(EVENT_BENEDICTUS_HOLY_FIRE, 6000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        scriptEvents.resetEvents();
        getInstanceScript()->setBossState(DATA_ARCHBISHOP_BENEDICTUS, EncounterStates::Failed);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "It is... not... over...");
        getInstanceScript()->setBossState(DATA_ARCHBISHOP_BENEDICTUS, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (!mBarrageAnnounced && _getHealthPercent() <= 50)
        {
            mBarrageAnnounced = true;
            sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "Embrace the Twilight!");
        }

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_BENEDICTUS_TWILIGHT_BARRAGE:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Archbishop Benedictus begins to cast Twilight Barrage!");
                castSpellAOE(SPELL_BENEDICTUS_TWILIGHT_BARRAGE);
                scriptEvents.addEvent(EVENT_BENEDICTUS_TWILIGHT_BARRAGE, 24000);
                break;
            case EVENT_BENEDICTUS_DOMINATE_MIND:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_BENEDICTUS_DOMINATE_MIND);
                scriptEvents.addEvent(EVENT_BENEDICTUS_DOMINATE_MIND, 30000);
                break;
            case EVENT_BENEDICTUS_HOLY_FIRE:
                castSpellOnVictim(SPELL_BENEDICTUS_HOLY_FIRE);
                scriptEvents.addEvent(EVENT_BENEDICTUS_HOLY_FIRE, 9000);
                break;
            default:
                break;
        }
    }

private:
    bool mBarrageAnnounced = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Trash

class EmaciatedMammothBullAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EmaciatedMammothBullAI(c); }
    explicit EmaciatedMammothBullAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_MAMMOTH_BULL_TRAMPLE, 5000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_MAMMOTH_BULL_TRAMPLE)
        {
            castSpellOnVictim(SPELL_MAMMOTH_BULL_TRAMPLE);
            scriptEvents.addEvent(EVENT_MAMMOTH_BULL_TRAMPLE, 5000);
        }
    }
};

// Reference also drives an action-list-linked ability at low health (source ids 2629300/
// 2629301) with no direct spell cast in AI scripting data; not ported.
class HulkingJormungarAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HulkingJormungarAI(c); }
    explicit HulkingJormungarAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_JORMUNGAR_ACID_SPIT, 6000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_JORMUNGAR_ACID_SPIT)
        {
            sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "The Hulking Jormungar falters for a moment, opening its mouth wide.");
            castSpellOnVictim(SPELL_JORMUNGAR_ACID_SPIT);
            scriptEvents.addEvent(EVENT_JORMUNGAR_ACID_SPIT, 11000);
        }
    }
};

class MagnataurPatriarchAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MagnataurPatriarchAI(c); }
    explicit MagnataurPatriarchAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        castSpellOnVictim(SPELL_MAGNATAUR_PATRIARCH_SMASH);
        scriptEvents.addEvent(EVENT_MAGNATAUR_PATRIARCH_SMASH, 2300);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_MAGNATAUR_PATRIARCH_SMASH)
        {
            castSpellOnVictim(SPELL_MAGNATAUR_PATRIARCH_SMASH);
            scriptEvents.addEvent(EVENT_MAGNATAUR_PATRIARCH_SMASH, 2300);
        }
    }
};

class DragonboneCondorAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DragonboneCondorAI(c); }
    explicit DragonboneCondorAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override { mScreechCast = false; }

    void AIUpdate(unsigned long /*time_passed*/) override
    {
        if (!mScreechCast && _getHealthPercent() <= 30)
        {
            mScreechCast = true;
            castSpellOnSelf(SPELL_DRAGONBONE_CONDOR_SCREECH);
        }
    }

private:
    bool mScreechCast = false;
};

class WastesDiggerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new WastesDiggerAI(c); }
    explicit WastesDiggerAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        castSpellOnSelf(SPELL_WASTES_DIGGER_BURROW);
        scriptEvents.addEvent(EVENT_WASTES_DIGGER_CLAW, 3000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_WASTES_DIGGER_CLAW)
        {
            castSpellOnVictim(SPELL_WASTES_DIGGER_CLAW);
            scriptEvents.addEvent(EVENT_WASTES_DIGGER_CLAW, 6000);
        }
    }
};

class SnowplainDiscipleAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SnowplainDiscipleAI(c); }
    explicit SnowplainDiscipleAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        castSpellOnSelf(SPELL_SNOWPLAIN_DISCIPLE_STRIKE);
        scriptEvents.addEvent(EVENT_SNOWPLAIN_DISCIPLE_STRIKE, 4000);
        scriptEvents.addEvent(EVENT_SNOWPLAIN_DISCIPLE_HEAL, 13000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_SNOWPLAIN_DISCIPLE_STRIKE:
                castSpellOnVictim(SPELL_SNOWPLAIN_DISCIPLE_STRIKE);
                scriptEvents.addEvent(EVENT_SNOWPLAIN_DISCIPLE_STRIKE, 4000);
                break;
            case EVENT_SNOWPLAIN_DISCIPLE_HEAL:
                castSpellOnSelf(SPELL_SNOWPLAIN_DISCIPLE_HEAL);
                scriptEvents.addEvent(EVENT_SNOWPLAIN_DISCIPLE_HEAL, 13000);
                break;
            default:
                break;
        }
    }
};

class SnowplainShamanAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SnowplainShamanAI(c); }
    explicit SnowplainShamanAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_SNOWPLAIN_SHAMAN_LIGHTNING, 6000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_SNOWPLAIN_SHAMAN_LIGHTNING)
        {
            castSpellOnSelf(SPELL_SNOWPLAIN_SHAMAN_LIGHTNING);
            scriptEvents.addEvent(EVENT_SNOWPLAIN_SHAMAN_LIGHTNING, 25000);
        }
    }
};

void SetupHourOfTwilight(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_HOUR_OF_TWILIGHT, &HourOfTwilightInstanceScript::Create);

    mgr->register_creature_script(BOSS_EARTHCALLER_TORUNSCAR, &EarthcallerTorunscarAI::Create);
    mgr->register_creature_script(BOSS_TAWN_WINTERBLUFF, &TawnWinterbluffAI::Create);
    mgr->register_creature_script(BOSS_HARGOTH_DIMBLAZE, &HargothDimblazeAI::Create);
    mgr->register_creature_script(BOSS_STORMCALLER_JALARA, &StormcallerJalaraAI::Create);
    mgr->register_creature_script(BOSS_ASIRA_DAWNSLAYER, &AsiraDawnslayerAI::Create);
    mgr->register_creature_script(BOSS_ARCHBISHOP_BENEDICTUS, &ArchbishopBenedictusAI::Create);

    mgr->register_creature_script(NPC_EMACIATED_MAMMOTH_BULL, &EmaciatedMammothBullAI::Create);
    mgr->register_creature_script(NPC_HULKING_JORMUNGAR, &HulkingJormungarAI::Create);
    mgr->register_creature_script(NPC_MAGNATAUR_PATRIARCH, &MagnataurPatriarchAI::Create);
    mgr->register_creature_script(NPC_DRAGONBONE_CONDOR, &DragonboneCondorAI::Create);
    mgr->register_creature_script(NPC_WASTES_DIGGER, &WastesDiggerAI::Create);
    mgr->register_creature_script(NPC_SNOWPLAIN_DISCIPLE, &SnowplainDiscipleAI::Create);
    mgr->register_creature_script(NPC_SNOWPLAIN_SHAMAN, &SnowplainShamanAI::Create);
}
