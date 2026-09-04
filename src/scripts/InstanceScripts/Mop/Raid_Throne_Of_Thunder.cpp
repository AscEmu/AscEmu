/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

// Throne of Thunder - twelve bosses (see header comment for sourcing/limitations), hand-
// ported into AscEmu's own CreatureAIScript / InstanceScript framework.

#include "Setup.h"
#include "Raid_Throne_Of_Thunder.hpp"

#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

enum ThroneOfThunderEvents
{
    EVENT_JINROKH_THUNDERING_THROW = 1,
    EVENT_JINROKH_STATIC_BURST,
    EVENT_JINROKH_LIGHTNING_STORM,
    EVENT_JINROKH_LIGHTNING_STRIKE,

    EVENT_HORRIDON_TRIPLE_PUNCTURE,
    EVENT_HORRIDON_DOUBLE_SWIPE,
    EVENT_HORRIDON_DIRE_CALL,
    EVENT_HORRIDON_DIRE_FIXATION,

    EVENT_COUNCIL_DARK_POWER,
    EVENT_COUNCIL_FRIGID_ASSAULT,
    EVENT_COUNCIL_SAND_BOLT,
    EVENT_COUNCIL_WRATH_OF_THE_LOA,

    EVENT_TORTOS_SPINNING_SHELL,
    EVENT_TORTOS_KICK_SHELL,
    EVENT_TORTOS_FURIOUS_STONE_BREATH,
    EVENT_TORTOS_ROCKFALL,

    EVENT_MEGAERA_CINDERS,
    EVENT_MEGAERA_TORRENT_OF_ICE,
    EVENT_MEGAERA_ACID_RAIN,
    EVENT_MEGAERA_HYDRA_FRENZY,

    EVENT_JIKUN_TALON_STRIKE,
    EVENT_JIKUN_SCREECH,
    EVENT_JIKUN_TALON_RAKE,
    EVENT_JIKUN_QUILLS,

    EVENT_DURUMU_DISINTEGRATION_BEAM,
    EVENT_DURUMU_GAZE,
    EVENT_DURUMU_BRIGHT_LIGHT,
    EVENT_DURUMU_LIFE_DRAIN,

    EVENT_PRIMORDIUS_PRIMORDIAL_STRIKE,
    EVENT_PRIMORDIUS_MALFORMED_BLOOD,
    EVENT_PRIMORDIUS_MUTAGENIC_POOL,
    EVENT_PRIMORDIUS_BLACK_BLOOD,

    EVENT_ANIMUS_EXPLOSIVE_SLAM,
    EVENT_ANIMUS_SIPHON_ANIMA,
    EVENT_ANIMUS_TOUCH_OF_THE_ANIMUS,
    EVENT_ANIMUS_ANIMA_RING,

    EVENT_QON_THROW_SPEAR,
    EVENT_QON_IMPALE,
    EVENT_QON_UNLEASHED_FLAME,
    EVENT_QON_ARCING_LIGHTNING,
    EVENT_QON_FROST_SPIKE,

    EVENT_CONSORTS_COSMIC_BARRAGE,
    EVENT_CONSORTS_NUCLEAR_INFERNO,
    EVENT_CONSORTS_ICE_COMET,
    EVENT_CONSORTS_MOON_LOTUS,

    EVENT_LEISHEN_DECAPITATE,
    EVENT_LEISHEN_THUNDERSTRUCK,
    EVENT_LEISHEN_CRASHING_THUNDER,
    EVENT_LEISHEN_LIGHTNING_WHIP,

    EVENT_BEASTCALLER_CLEAVE,
    EVENT_BEASTCALLER_ENRAGE,

    EVENT_SKYSCREAMER_MULTI_SHOT,
    EVENT_SKYSCREAMER_SCREECH
};

class ThroneOfThunderInstanceScript : public InstanceScript
{
public:
    explicit ThroneOfThunderInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(ThroneOfThunderEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new ThroneOfThunderInstanceScript(pMapMgr); }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Jin'rokh the Breaker

class JinrokhAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new JinrokhAI(c); }
    explicit JinrokhAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_JINROKH, EncounterStates::InProgress);
        scriptEvents.addEvent(EVENT_JINROKH_THUNDERING_THROW, 6000);
        scriptEvents.addEvent(EVENT_JINROKH_STATIC_BURST, 10000);
        scriptEvents.addEvent(EVENT_JINROKH_LIGHTNING_STORM, 20000);
        scriptEvents.addEvent(EVENT_JINROKH_LIGHTNING_STRIKE, 14000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_JINROKH, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_JINROKH, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;
        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_JINROKH_THUNDERING_THROW:
                castSpellOnVictim(SPELL_JINROKH_THUNDERING_THROW);
                scriptEvents.addEvent(EVENT_JINROKH_THUNDERING_THROW, 16000);
                break;
            case EVENT_JINROKH_STATIC_BURST:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_JINROKH_STATIC_BURST);
                scriptEvents.addEvent(EVENT_JINROKH_STATIC_BURST, 18000);
                break;
            case EVENT_JINROKH_LIGHTNING_STORM:
                castSpellAOE(SPELL_JINROKH_LIGHTNING_STORM);
                scriptEvents.addEvent(EVENT_JINROKH_LIGHTNING_STORM, 26000);
                break;
            case EVENT_JINROKH_LIGHTNING_STRIKE:
                castSpellOnVictim(SPELL_JINROKH_LIGHTNING_STRIKE);
                scriptEvents.addEvent(EVENT_JINROKH_LIGHTNING_STRIKE, 20000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Horridon

class HorridonAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HorridonAI(c); }
    explicit HorridonAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "Horridon hungers!");
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_HORRIDON, EncounterStates::InProgress);
        scriptEvents.addEvent(EVENT_HORRIDON_TRIPLE_PUNCTURE, 8000);
        scriptEvents.addEvent(EVENT_HORRIDON_DOUBLE_SWIPE, 14000);
        scriptEvents.addEvent(EVENT_HORRIDON_DIRE_CALL, 22000);
        scriptEvents.addEvent(EVENT_HORRIDON_DIRE_FIXATION, 16000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_HORRIDON, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_HORRIDON, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;
        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_HORRIDON_TRIPLE_PUNCTURE:
                castSpellOnVictim(SPELL_HORRIDON_TRIPLE_PUNCTURE);
                scriptEvents.addEvent(EVENT_HORRIDON_TRIPLE_PUNCTURE, 18000);
                break;
            case EVENT_HORRIDON_DOUBLE_SWIPE:
                castSpellOnVictim(SPELL_HORRIDON_DOUBLE_SWIPE);
                scriptEvents.addEvent(EVENT_HORRIDON_DOUBLE_SWIPE, 20000);
                break;
            case EVENT_HORRIDON_DIRE_CALL:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Horridon lets loose a dire call!");
                castSpellAOE(SPELL_HORRIDON_DIRE_CALL);
                scriptEvents.addEvent(EVENT_HORRIDON_DIRE_CALL, 30000);
                break;
            case EVENT_HORRIDON_DIRE_FIXATION:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_HORRIDON_DIRE_FIXATION);
                scriptEvents.addEvent(EVENT_HORRIDON_DIRE_FIXATION, 24000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Council of Elders (simplified to Kazra'jin representing all four members)

class CouncilOfEldersAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CouncilOfEldersAI(c); }
    explicit CouncilOfEldersAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_COUNCIL_OF_ELDERS, EncounterStates::InProgress);
        scriptEvents.addEvent(EVENT_COUNCIL_DARK_POWER, 6000);
        scriptEvents.addEvent(EVENT_COUNCIL_FRIGID_ASSAULT, 12000);
        scriptEvents.addEvent(EVENT_COUNCIL_SAND_BOLT, 18000);
        scriptEvents.addEvent(EVENT_COUNCIL_WRATH_OF_THE_LOA, 24000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_COUNCIL_OF_ELDERS, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_COUNCIL_OF_ELDERS, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;
        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_COUNCIL_DARK_POWER:
                castSpellOnVictim(SPELL_COUNCIL_DARK_POWER);
                scriptEvents.addEvent(EVENT_COUNCIL_DARK_POWER, 20000);
                break;
            case EVENT_COUNCIL_FRIGID_ASSAULT:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_COUNCIL_FRIGID_ASSAULT);
                scriptEvents.addEvent(EVENT_COUNCIL_FRIGID_ASSAULT, 22000);
                break;
            case EVENT_COUNCIL_SAND_BOLT:
                castSpellOnVictim(SPELL_COUNCIL_SAND_BOLT);
                scriptEvents.addEvent(EVENT_COUNCIL_SAND_BOLT, 16000);
                break;
            case EVENT_COUNCIL_WRATH_OF_THE_LOA:
                castSpellAOE(SPELL_COUNCIL_WRATH_OF_THE_LOA);
                scriptEvents.addEvent(EVENT_COUNCIL_WRATH_OF_THE_LOA, 28000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Tortos

class TortosAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TortosAI(c); }
    explicit TortosAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_TORTOS, EncounterStates::InProgress);
        scriptEvents.addEvent(EVENT_TORTOS_SPINNING_SHELL, 10000);
        scriptEvents.addEvent(EVENT_TORTOS_KICK_SHELL, 16000);
        scriptEvents.addEvent(EVENT_TORTOS_FURIOUS_STONE_BREATH, 6000);
        scriptEvents.addEvent(EVENT_TORTOS_ROCKFALL, 22000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_TORTOS, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_TORTOS, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;
        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_TORTOS_SPINNING_SHELL:
                castSpellAOE(SPELL_TORTOS_SPINNING_SHELL);
                scriptEvents.addEvent(EVENT_TORTOS_SPINNING_SHELL, 24000);
                break;
            case EVENT_TORTOS_KICK_SHELL:
                castSpellOnVictim(SPELL_TORTOS_KICK_SHELL);
                scriptEvents.addEvent(EVENT_TORTOS_KICK_SHELL, 20000);
                break;
            case EVENT_TORTOS_FURIOUS_STONE_BREATH:
                castSpellOnVictim(SPELL_TORTOS_FURIOUS_STONE_BREATH);
                scriptEvents.addEvent(EVENT_TORTOS_FURIOUS_STONE_BREATH, 18000);
                break;
            case EVENT_TORTOS_ROCKFALL:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_TORTOS_ROCKFALL);
                scriptEvents.addEvent(EVENT_TORTOS_ROCKFALL, 26000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Megaera

class MegaeraAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MegaeraAI(c); }
    explicit MegaeraAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_MEGAERA, EncounterStates::InProgress);
        scriptEvents.addEvent(EVENT_MEGAERA_CINDERS, 8000);
        scriptEvents.addEvent(EVENT_MEGAERA_TORRENT_OF_ICE, 16000);
        scriptEvents.addEvent(EVENT_MEGAERA_ACID_RAIN, 12000);
        scriptEvents.addEvent(EVENT_MEGAERA_HYDRA_FRENZY, 24000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_MEGAERA, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_MEGAERA, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;
        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_MEGAERA_CINDERS:
                castSpellOnVictim(SPELL_MEGAERA_CINDERS);
                scriptEvents.addEvent(EVENT_MEGAERA_CINDERS, 20000);
                break;
            case EVENT_MEGAERA_TORRENT_OF_ICE:
                castSpellOnVictim(SPELL_MEGAERA_TORRENT_OF_ICE);
                scriptEvents.addEvent(EVENT_MEGAERA_TORRENT_OF_ICE, 22000);
                break;
            case EVENT_MEGAERA_ACID_RAIN:
                castSpellAOE(SPELL_MEGAERA_ACID_RAIN);
                scriptEvents.addEvent(EVENT_MEGAERA_ACID_RAIN, 18000);
                break;
            case EVENT_MEGAERA_HYDRA_FRENZY:
                castSpellOnSelf(SPELL_MEGAERA_HYDRA_FRENZY);
                scriptEvents.addEvent(EVENT_MEGAERA_HYDRA_FRENZY, 28000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Ji-Kun

class JikunAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new JikunAI(c); }
    explicit JikunAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_JIKUN, EncounterStates::InProgress);
        scriptEvents.addEvent(EVENT_JIKUN_TALON_STRIKE, 6000);
        scriptEvents.addEvent(EVENT_JIKUN_SCREECH, 20000);
        scriptEvents.addEvent(EVENT_JIKUN_TALON_RAKE, 12000);
        scriptEvents.addEvent(EVENT_JIKUN_QUILLS, 16000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_JIKUN, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_JIKUN, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;
        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_JIKUN_TALON_STRIKE:
                castSpellOnVictim(SPELL_JIKUN_TALON_STRIKE);
                scriptEvents.addEvent(EVENT_JIKUN_TALON_STRIKE, 16000);
                break;
            case EVENT_JIKUN_SCREECH:
                castSpellAOE(SPELL_JIKUN_SCREECH);
                scriptEvents.addEvent(EVENT_JIKUN_SCREECH, 28000);
                break;
            case EVENT_JIKUN_TALON_RAKE:
                castSpellOnVictim(SPELL_JIKUN_TALON_RAKE);
                scriptEvents.addEvent(EVENT_JIKUN_TALON_RAKE, 18000);
                break;
            case EVENT_JIKUN_QUILLS:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_JIKUN_QUILLS);
                scriptEvents.addEvent(EVENT_JIKUN_QUILLS, 20000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Durumu the Forgotten

class DurumuAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DurumuAI(c); }
    explicit DurumuAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "Who dares intrude on my domain?");
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_DURUMU, EncounterStates::InProgress);
        scriptEvents.addEvent(EVENT_DURUMU_DISINTEGRATION_BEAM, 12000);
        scriptEvents.addEvent(EVENT_DURUMU_GAZE, 8000);
        scriptEvents.addEvent(EVENT_DURUMU_BRIGHT_LIGHT, 18000);
        scriptEvents.addEvent(EVENT_DURUMU_LIFE_DRAIN, 24000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_DURUMU, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_DURUMU, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;
        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_DURUMU_DISINTEGRATION_BEAM:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Durumu's eye begins to glow with a Disintegration Beam!");
                castSpellOnVictim(SPELL_DURUMU_DISINTEGRATION_BEAM);
                scriptEvents.addEvent(EVENT_DURUMU_DISINTEGRATION_BEAM, 28000);
                break;
            case EVENT_DURUMU_GAZE:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_DURUMU_GAZE);
                scriptEvents.addEvent(EVENT_DURUMU_GAZE, 18000);
                break;
            case EVENT_DURUMU_BRIGHT_LIGHT:
                castSpellAOE(SPELL_DURUMU_BRIGHT_LIGHT);
                scriptEvents.addEvent(EVENT_DURUMU_BRIGHT_LIGHT, 24000);
                break;
            case EVENT_DURUMU_LIFE_DRAIN:
                castSpellOnVictim(SPELL_DURUMU_LIFE_DRAIN);
                scriptEvents.addEvent(EVENT_DURUMU_LIFE_DRAIN, 22000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Primordius

class PrimordiusAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new PrimordiusAI(c); }
    explicit PrimordiusAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_PRIMORDIUS, EncounterStates::InProgress);
        scriptEvents.addEvent(EVENT_PRIMORDIUS_PRIMORDIAL_STRIKE, 6000);
        scriptEvents.addEvent(EVENT_PRIMORDIUS_MALFORMED_BLOOD, 14000);
        scriptEvents.addEvent(EVENT_PRIMORDIUS_MUTAGENIC_POOL, 20000);
        scriptEvents.addEvent(EVENT_PRIMORDIUS_BLACK_BLOOD, 26000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_PRIMORDIUS, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_PRIMORDIUS, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;
        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_PRIMORDIUS_PRIMORDIAL_STRIKE:
                castSpellOnVictim(SPELL_PRIMORDIUS_PRIMORDIAL_STRIKE);
                scriptEvents.addEvent(EVENT_PRIMORDIUS_PRIMORDIAL_STRIKE, 16000);
                break;
            case EVENT_PRIMORDIUS_MALFORMED_BLOOD:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_PRIMORDIUS_MALFORMED_BLOOD);
                scriptEvents.addEvent(EVENT_PRIMORDIUS_MALFORMED_BLOOD, 22000);
                break;
            case EVENT_PRIMORDIUS_MUTAGENIC_POOL:
                castSpellAOE(SPELL_PRIMORDIUS_MUTAGENIC_POOL);
                scriptEvents.addEvent(EVENT_PRIMORDIUS_MUTAGENIC_POOL, 26000);
                break;
            case EVENT_PRIMORDIUS_BLACK_BLOOD:
                castSpellOnSelf(SPELL_PRIMORDIUS_BLACK_BLOOD);
                scriptEvents.addEvent(EVENT_PRIMORDIUS_BLACK_BLOOD, 30000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Dark Animus

class DarkAnimusAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DarkAnimusAI(c); }
    explicit DarkAnimusAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_DARK_ANIMUS, EncounterStates::InProgress);
        scriptEvents.addEvent(EVENT_ANIMUS_EXPLOSIVE_SLAM, 8000);
        scriptEvents.addEvent(EVENT_ANIMUS_SIPHON_ANIMA, 16000);
        scriptEvents.addEvent(EVENT_ANIMUS_TOUCH_OF_THE_ANIMUS, 12000);
        scriptEvents.addEvent(EVENT_ANIMUS_ANIMA_RING, 22000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_DARK_ANIMUS, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_DARK_ANIMUS, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;
        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_ANIMUS_EXPLOSIVE_SLAM:
                castSpellOnVictim(SPELL_ANIMUS_EXPLOSIVE_SLAM);
                scriptEvents.addEvent(EVENT_ANIMUS_EXPLOSIVE_SLAM, 20000);
                break;
            case EVENT_ANIMUS_SIPHON_ANIMA:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_ANIMUS_SIPHON_ANIMA);
                scriptEvents.addEvent(EVENT_ANIMUS_SIPHON_ANIMA, 24000);
                break;
            case EVENT_ANIMUS_TOUCH_OF_THE_ANIMUS:
                castSpellOnVictim(SPELL_ANIMUS_TOUCH_OF_THE_ANIMUS);
                scriptEvents.addEvent(EVENT_ANIMUS_TOUCH_OF_THE_ANIMUS, 18000);
                break;
            case EVENT_ANIMUS_ANIMA_RING:
                castSpellAOE(SPELL_ANIMUS_ANIMA_RING);
                scriptEvents.addEvent(EVENT_ANIMUS_ANIMA_RING, 26000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Iron Qon

class IronQonAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new IronQonAI(c); }
    explicit IronQonAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_IRON_QON, EncounterStates::InProgress);
        scriptEvents.addEvent(EVENT_QON_THROW_SPEAR, 6000);
        scriptEvents.addEvent(EVENT_QON_IMPALE, 12000);
        scriptEvents.addEvent(EVENT_QON_UNLEASHED_FLAME, 18000);
        scriptEvents.addEvent(EVENT_QON_ARCING_LIGHTNING, 24000);
        scriptEvents.addEvent(EVENT_QON_FROST_SPIKE, 30000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_IRON_QON, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_IRON_QON, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;
        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_QON_THROW_SPEAR:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_QON_THROW_SPEAR);
                scriptEvents.addEvent(EVENT_QON_THROW_SPEAR, 18000);
                break;
            case EVENT_QON_IMPALE:
                castSpellOnVictim(SPELL_QON_IMPALE);
                scriptEvents.addEvent(EVENT_QON_IMPALE, 20000);
                break;
            case EVENT_QON_UNLEASHED_FLAME:
                castSpellAOE(SPELL_QON_UNLEASHED_FLAME);
                scriptEvents.addEvent(EVENT_QON_UNLEASHED_FLAME, 26000);
                break;
            case EVENT_QON_ARCING_LIGHTNING:
                castSpellOnVictim(SPELL_QON_ARCING_LIGHTNING);
                scriptEvents.addEvent(EVENT_QON_ARCING_LIGHTNING, 22000);
                break;
            case EVENT_QON_FROST_SPIKE:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_QON_FROST_SPIKE);
                scriptEvents.addEvent(EVENT_QON_FROST_SPIKE, 24000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// The Twin Consorts (simplified to Lu'lin representing both Suen and Lu'lin)

class TwinConsortsAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TwinConsortsAI(c); }
    explicit TwinConsortsAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_TWIN_CONSORTS, EncounterStates::InProgress);
        scriptEvents.addEvent(EVENT_CONSORTS_COSMIC_BARRAGE, 8000);
        scriptEvents.addEvent(EVENT_CONSORTS_NUCLEAR_INFERNO, 16000);
        scriptEvents.addEvent(EVENT_CONSORTS_ICE_COMET, 22000);
        scriptEvents.addEvent(EVENT_CONSORTS_MOON_LOTUS, 28000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_TWIN_CONSORTS, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_TWIN_CONSORTS, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;
        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_CONSORTS_COSMIC_BARRAGE:
                castSpellAOE(SPELL_CONSORTS_COSMIC_BARRAGE);
                scriptEvents.addEvent(EVENT_CONSORTS_COSMIC_BARRAGE, 24000);
                break;
            case EVENT_CONSORTS_NUCLEAR_INFERNO:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_CONSORTS_NUCLEAR_INFERNO);
                scriptEvents.addEvent(EVENT_CONSORTS_NUCLEAR_INFERNO, 26000);
                break;
            case EVENT_CONSORTS_ICE_COMET:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_CONSORTS_ICE_COMET);
                scriptEvents.addEvent(EVENT_CONSORTS_ICE_COMET, 24000);
                break;
            case EVENT_CONSORTS_MOON_LOTUS:
                castSpellOnSelf(SPELL_CONSORTS_MOON_LOTUS);
                scriptEvents.addEvent(EVENT_CONSORTS_MOON_LOTUS, 30000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Lei Shen

class LeiShenAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new LeiShenAI(c); }
    explicit LeiShenAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "You dare stand against the Thunder King?!");
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_LEI_SHEN, EncounterStates::InProgress);
        scriptEvents.addEvent(EVENT_LEISHEN_DECAPITATE, 5000);
        scriptEvents.addEvent(EVENT_LEISHEN_THUNDERSTRUCK, 14000);
        scriptEvents.addEvent(EVENT_LEISHEN_CRASHING_THUNDER, 20000);
        scriptEvents.addEvent(EVENT_LEISHEN_LIGHTNING_WHIP, 26000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_LEI_SHEN, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "This is not... the end...");
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_LEI_SHEN, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;
        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_LEISHEN_DECAPITATE:
                castSpellOnVictim(SPELL_LEISHEN_DECAPITATE);
                scriptEvents.addEvent(EVENT_LEISHEN_DECAPITATE, 18000);
                break;
            case EVENT_LEISHEN_THUNDERSTRUCK:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_LEISHEN_THUNDERSTRUCK);
                scriptEvents.addEvent(EVENT_LEISHEN_THUNDERSTRUCK, 20000);
                break;
            case EVENT_LEISHEN_CRASHING_THUNDER:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Lei Shen unleashes crashing thunder!");
                castSpellAOE(SPELL_LEISHEN_CRASHING_THUNDER);
                scriptEvents.addEvent(EVENT_LEISHEN_CRASHING_THUNDER, 28000);
                break;
            case EVENT_LEISHEN_LIGHTNING_WHIP:
                castSpellOnVictim(SPELL_LEISHEN_LIGHTNING_WHIP);
                scriptEvents.addEvent(EVENT_LEISHEN_LIGHTNING_WHIP, 22000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Trash - Zandalari Beastcaller

class ZandalariBeastcallerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ZandalariBeastcallerAI(c); }
    explicit ZandalariBeastcallerAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_BEASTCALLER_CLEAVE, 6000);
        scriptEvents.addEvent(EVENT_BEASTCALLER_ENRAGE, 16000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_BEASTCALLER_CLEAVE:
                castSpellOnVictim(SPELL_BEASTCALLER_CLEAVE);
                scriptEvents.addEvent(EVENT_BEASTCALLER_CLEAVE, 12000);
                break;
            case EVENT_BEASTCALLER_ENRAGE:
                castSpellOnSelf(SPELL_BEASTCALLER_ENRAGE);
                scriptEvents.addEvent(EVENT_BEASTCALLER_ENRAGE, 26000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Trash - Zandalari Skyscreamer

class ZandalariSkyscreamerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ZandalariSkyscreamerAI(c); }
    explicit ZandalariSkyscreamerAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_SKYSCREAMER_MULTI_SHOT, 5000);
        scriptEvents.addEvent(EVENT_SKYSCREAMER_SCREECH, 14000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_SKYSCREAMER_MULTI_SHOT:
                castSpellOnVictim(SPELL_SKYSCREAMER_MULTI_SHOT);
                scriptEvents.addEvent(EVENT_SKYSCREAMER_MULTI_SHOT, 14000);
                break;
            case EVENT_SKYSCREAMER_SCREECH:
                castSpellAOE(SPELL_SKYSCREAMER_SCREECH);
                scriptEvents.addEvent(EVENT_SKYSCREAMER_SCREECH, 24000);
                break;
            default:
                break;
        }
    }
};

void SetupThroneOfThunder(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_THRONE_OF_THUNDER, &ThroneOfThunderInstanceScript::Create);

    mgr->register_creature_script(BOSS_JINROKH, &JinrokhAI::Create);
    mgr->register_creature_script(BOSS_HORRIDON, &HorridonAI::Create);
    mgr->register_creature_script(BOSS_KAZRAJIN, &CouncilOfEldersAI::Create);
    mgr->register_creature_script(BOSS_TORTOS, &TortosAI::Create);
    mgr->register_creature_script(BOSS_MEGAERA, &MegaeraAI::Create);
    mgr->register_creature_script(BOSS_JIKUN, &JikunAI::Create);
    mgr->register_creature_script(BOSS_DURUMU, &DurumuAI::Create);
    mgr->register_creature_script(BOSS_PRIMORDIUS, &PrimordiusAI::Create);
    mgr->register_creature_script(BOSS_DARK_ANIMUS, &DarkAnimusAI::Create);
    mgr->register_creature_script(BOSS_IRON_QON, &IronQonAI::Create);
    mgr->register_creature_script(BOSS_LULIN, &TwinConsortsAI::Create);
    mgr->register_creature_script(BOSS_LEI_SHEN, &LeiShenAI::Create);

    mgr->register_creature_script(NPC_ZANDALARI_BEASTCALLER, &ZandalariBeastcallerAI::Create);
    mgr->register_creature_script(NPC_ZANDALARI_SKYSCREAMER, &ZandalariSkyscreamerAI::Create);
}
