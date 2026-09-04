/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

// The Stonecore - four bosses (verified against wowhead), hand-ported into AscEmu's own
// CreatureAIScript / InstanceScript framework.
//
// Simplifications versus the original fight design: Corborus' submerge/teleport/trashing-charge
// phase, Slabhide's flying phase and stalactite drops, and High Priestess Azil's second
// (platform/gravity-well) phase are all dropped - each boss instead runs a continuous ground
// rotation built from its core abilities. The instance is a strictly linear gauntlet (per
// wowhead: Corborus -> Slabhide -> Ozruk -> Azil), gated by the two Rock Wall objects that
// open together once Corborus is dead.

#include "Setup.h"
#include "Instance_The_Stonecore.hpp"

#include "Objects/GameObject.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

enum TheStonecoreEvents
{
    EVENT_CORBORUS_DAMPENING_WAVE = 1,
    EVENT_CORBORUS_CRYSTAL_BARRAGE,

    EVENT_SLABHIDE_LAVA_FISSURE,
    EVENT_SLABHIDE_SAND_BLAST,

    EVENT_OZRUK_ELEMENTIUM_BULWARK,
    EVENT_OZRUK_GROUND_SLAM,
    EVENT_OZRUK_ELEMENTIUM_SPIKE_SHIELD,
    EVENT_OZRUK_ENRAGE,

    EVENT_AZIL_CURSE_OF_BLOOD,
    EVENT_AZIL_FORCE_GRIP,
    EVENT_AZIL_SUMMON_GRAVITY_WELL,

    EVENT_RIFT_CONJURER_ARCANE_BOLT,
    EVENT_RIFT_CONJURER_ARCANE_BUBBLE,
    EVENT_STONECORE_BRUISER_SMASH,
    EVENT_WARBRINGER_SUNDER_ARMOR,
    EVENT_WARBRINGER_GORE,
    EVENT_MAGMALORD_LAVA_BURST,
    EVENT_MAGMALORD_MAGMA_SHIELD,
    EVENT_STONECORE_FLAYER_REND,
    EVENT_CRYSTALSPAWN_GIANT_SHATTER,
    EVENT_BERSERKER_WHIRLWIND,
    EVENT_BERSERKER_ENRAGE,
    EVENT_EARTHSHAPER_STONE_SPIKES,
    EVENT_EARTHSHAPER_QUAKE,
    EVENT_EARTHSHAPER_EMBEDDED_SPIKE,
    EVENT_EARTHSHAPER_UPHEAVAL
};

class TheStonecoreInstanceScript : public InstanceScript
{
public:
    explicit TheStonecoreInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(TheStonecoreEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new TheStonecoreInstanceScript(pMapMgr); }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        if (pGameObject->getEntry() == GO_ROCK_WALL)
            mRockWallGuids.push_back(pGameObject->getGuidLow());
    }

    void OpenRockWalls()
    {
        for (uint32_t guid : mRockWallGuids)
        {
            if (GameObject* pWall = GetGameObjectByGuid(guid))
                useDoorOrButton(pWall);
        }
    }

private:
    std::vector<uint32_t> mRockWallGuids;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Corborus

class CorborusAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CorborusAI(c); }
    explicit CorborusAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_CORBORUS, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_CORBORUS_DAMPENING_WAVE, 10000);
        scriptEvents.addEvent(EVENT_CORBORUS_CRYSTAL_BARRAGE, 15000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_CORBORUS, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_CORBORUS, EncounterStates::Performed);
        static_cast<TheStonecoreInstanceScript*>(getInstanceScript())->OpenRockWalls();
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_CORBORUS_DAMPENING_WAVE:
                castSpellAOE(SPELL_CORBORUS_DAMPENING_WAVE);
                scriptEvents.addEvent(EVENT_CORBORUS_DAMPENING_WAVE, 18000);
                break;
            case EVENT_CORBORUS_CRYSTAL_BARRAGE:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_CORBORUS_CRYSTAL_BARRAGE);
                scriptEvents.addEvent(EVENT_CORBORUS_CRYSTAL_BARRAGE, 20000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Slabhide

class SlabhideAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SlabhideAI(c); }
    explicit SlabhideAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_SLABHIDE, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_SLABHIDE_LAVA_FISSURE, 6000);
        scriptEvents.addEvent(EVENT_SLABHIDE_SAND_BLAST, 8000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_SLABHIDE, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_SLABHIDE, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_SLABHIDE_LAVA_FISSURE:
                castSpellAOE(SPELL_SLABHIDE_LAVA_FISSURE);
                scriptEvents.addEvent(EVENT_SLABHIDE_LAVA_FISSURE, 15000);
                break;
            case EVENT_SLABHIDE_SAND_BLAST:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_SLABHIDE_SAND_BLAST);
                scriptEvents.addEvent(EVENT_SLABHIDE_SAND_BLAST, 18000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Ozruk

class OzrukAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new OzrukAI(c); }
    explicit OzrukAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 21919, "None may pass into the World's Heart!");
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_OZRUK, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_OZRUK_ELEMENTIUM_BULWARK, 5000);
        scriptEvents.addEvent(EVENT_OZRUK_GROUND_SLAM, 10000);
        scriptEvents.addEvent(EVENT_OZRUK_ELEMENTIUM_SPIKE_SHIELD, 13000);
        scriptEvents.addEvent(EVENT_OZRUK_ENRAGE, 360000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_OZRUK, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 21922, "A protector has fallen. The World's Heart lies exposed!");
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_OZRUK, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_OZRUK_ELEMENTIUM_BULWARK:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Ozruk casts Elementium Bulwark!");
                castSpellOnSelf(SPELL_OZRUK_ELEMENTIUM_BULWARK);
                scriptEvents.addEvent(EVENT_OZRUK_ELEMENTIUM_BULWARK, 20000);
                break;
            case EVENT_OZRUK_GROUND_SLAM:
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 21921, "Break yourselves upon my body. Feel the strength of the earth!");
                castSpellAOE(SPELL_OZRUK_GROUND_SLAM);
                scriptEvents.addEvent(EVENT_OZRUK_GROUND_SLAM, 18000);
                break;
            case EVENT_OZRUK_ELEMENTIUM_SPIKE_SHIELD:
                castSpellOnSelf(SPELL_OZRUK_ELEMENTIUM_SPIKE_SHIELD);
                scriptEvents.addEvent(EVENT_OZRUK_ELEMENTIUM_SPIKE_SHIELD, 22000);
                break;
            case EVENT_OZRUK_ENRAGE:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Ozruk becomes enraged!");
                castSpellOnSelf(SPELL_OZRUK_ENRAGE, true);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// High Priestess Azil

class HighPriestessAzilAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HighPriestessAzilAI(c); }
    explicit HighPriestessAzilAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 21634, "The world will be reborn in flames!");
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_HIGH_PRIESTESS_AZIL, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_AZIL_CURSE_OF_BLOOD, 8000);
        scriptEvents.addEvent(EVENT_AZIL_FORCE_GRIP, 14000);
        scriptEvents.addEvent(EVENT_AZIL_SUMMON_GRAVITY_WELL, 20000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_HIGH_PRIESTESS_AZIL, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 21633, "For my death, countless more will fall. The burden is now yours to bear.");
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_HIGH_PRIESTESS_AZIL, EncounterStates::Performed);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_AZIL_CURSE_OF_BLOOD:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_AZIL_CURSE_OF_BLOOD);
                scriptEvents.addEvent(EVENT_AZIL_CURSE_OF_BLOOD, 20000);
                break;
            case EVENT_AZIL_FORCE_GRIP:
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 21628, "Witness the power bestowed upon me by Deathwing! Feel the fury of earth!");
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_AZIL_FORCE_GRIP);
                scriptEvents.addEvent(EVENT_AZIL_FORCE_GRIP, 18000);
                break;
            case EVENT_AZIL_SUMMON_GRAVITY_WELL:
                castSpellOnSelf(SPELL_AZIL_SUMMON_GRAVITY_WELL);
                scriptEvents.addEvent(EVENT_AZIL_SUMMON_GRAVITY_WELL, 25000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Trash

class StonecoreRiftConjurerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new StonecoreRiftConjurerAI(c); }
    explicit StonecoreRiftConjurerAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_RIFT_CONJURER_ARCANE_BOLT, 2000);
        scriptEvents.addEvent(EVENT_RIFT_CONJURER_ARCANE_BUBBLE, 10000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_RIFT_CONJURER_ARCANE_BOLT:
                castSpellOnVictim(SPELL_RIFT_CONJURER_ARCANE_BOLT);
                scriptEvents.addEvent(EVENT_RIFT_CONJURER_ARCANE_BOLT, 2000);
                break;
            case EVENT_RIFT_CONJURER_ARCANE_BUBBLE:
                castSpellOnSelf(SPELL_RIFT_CONJURER_ARCANE_BUBBLE);
                scriptEvents.addEvent(EVENT_RIFT_CONJURER_ARCANE_BUBBLE, 20000);
                break;
            default:
                break;
        }
    }
};

class StonecoreBruiserAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new StonecoreBruiserAI(c); }
    explicit StonecoreBruiserAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_STONECORE_BRUISER_SMASH, 14000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_STONECORE_BRUISER_SMASH)
        {
            castSpellOnSelf(SPELL_STONECORE_BRUISER_SMASH);
            scriptEvents.addEvent(EVENT_STONECORE_BRUISER_SMASH, 14000);
        }
    }
};

class StonecoreWarbringerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new StonecoreWarbringerAI(c); }
    explicit StonecoreWarbringerAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_WARBRINGER_SUNDER_ARMOR, 4000);
        scriptEvents.addEvent(EVENT_WARBRINGER_GORE, 8000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_WARBRINGER_SUNDER_ARMOR:
                castSpellOnVictim(SPELL_WARBRINGER_SUNDER_ARMOR);
                scriptEvents.addEvent(EVENT_WARBRINGER_SUNDER_ARMOR, 12000);
                break;
            case EVENT_WARBRINGER_GORE:
                castSpellOnVictim(SPELL_WARBRINGER_GORE);
                scriptEvents.addEvent(EVENT_WARBRINGER_GORE, 8000);
                break;
            default:
                break;
        }
    }
};

class StonecoreMagmalordAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new StonecoreMagmalordAI(c); }
    explicit StonecoreMagmalordAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_MAGMALORD_LAVA_BURST, 4000);
        scriptEvents.addEvent(EVENT_MAGMALORD_MAGMA_SHIELD, 8000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_MAGMALORD_LAVA_BURST:
                castSpellOnVictim(SPELL_MAGMALORD_LAVA_BURST);
                scriptEvents.addEvent(EVENT_MAGMALORD_LAVA_BURST, 10000);
                break;
            case EVENT_MAGMALORD_MAGMA_SHIELD:
                castSpellOnSelf(SPELL_MAGMALORD_MAGMA_SHIELD);
                scriptEvents.addEvent(EVENT_MAGMALORD_MAGMA_SHIELD, 10000);
                break;
            default:
                break;
        }
    }
};

class StonecoreFlayerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new StonecoreFlayerAI(c); }
    explicit StonecoreFlayerAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_STONECORE_FLAYER_REND, 2500);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_STONECORE_FLAYER_REND)
        {
            castSpellOnSelf(SPELL_STONECORE_FLAYER_REND);
            scriptEvents.addEvent(EVENT_STONECORE_FLAYER_REND, 10000);
        }
    }
};

class CrystalspawnGiantAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CrystalspawnGiantAI(c); }
    explicit CrystalspawnGiantAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_CRYSTALSPAWN_GIANT_SHATTER, 5000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_CRYSTALSPAWN_GIANT_SHATTER)
        {
            castSpellOnSelf(SPELL_CRYSTALSPAWN_GIANT_SHATTER);
            scriptEvents.addEvent(EVENT_CRYSTALSPAWN_GIANT_SHATTER, 12000);
        }
    }
};

class StonecoreBerserkerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new StonecoreBerserkerAI(c); }
    explicit StonecoreBerserkerAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_BERSERKER_WHIRLWIND, 10000);
        scriptEvents.addEvent(EVENT_BERSERKER_ENRAGE, 10000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_BERSERKER_WHIRLWIND:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_BERSERKER_WHIRLWIND);
                scriptEvents.addEvent(EVENT_BERSERKER_WHIRLWIND, 10000);
                break;
            case EVENT_BERSERKER_ENRAGE:
                castSpellOnSelf(SPELL_BERSERKER_ENRAGE);
                scriptEvents.addEvent(EVENT_BERSERKER_ENRAGE, 10000);
                break;
            default:
                break;
        }
    }
};

class StonecoreEarthshaperAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new StonecoreEarthshaperAI(c); }
    explicit StonecoreEarthshaperAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_EARTHSHAPER_STONE_SPIKES, 3000);
        scriptEvents.addEvent(EVENT_EARTHSHAPER_QUAKE, 9000);
        scriptEvents.addEvent(EVENT_EARTHSHAPER_EMBEDDED_SPIKE, 15000);
        scriptEvents.addEvent(EVENT_EARTHSHAPER_UPHEAVAL, 5000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_EARTHSHAPER_STONE_SPIKES:
                castSpellOnVictim(SPELL_EARTHSHAPER_STONE_SPIKES);
                scriptEvents.addEvent(EVENT_EARTHSHAPER_STONE_SPIKES, 10000);
                break;
            case EVENT_EARTHSHAPER_QUAKE:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Stonecore Earthshaper begins to transform into a Force of Earth!");
                castSpellOnSelf(SPELL_EARTHSHAPER_QUAKE);
                scriptEvents.addEvent(EVENT_EARTHSHAPER_QUAKE, 12000);
                break;
            case EVENT_EARTHSHAPER_EMBEDDED_SPIKE:
                castSpellOnSelf(SPELL_EARTHSHAPER_EMBEDDED_SPIKE);
                scriptEvents.addEvent(EVENT_EARTHSHAPER_EMBEDDED_SPIKE, 10000);
                break;
            case EVENT_EARTHSHAPER_UPHEAVAL:
                castSpellOnSelf(SPELL_EARTHSHAPER_UPHEAVAL);
                scriptEvents.addEvent(EVENT_EARTHSHAPER_UPHEAVAL, 15000);
                break;
            default:
                break;
        }
    }
};

void SetupTheStonecore(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_THE_STONECORE, &TheStonecoreInstanceScript::Create);

    mgr->register_creature_script(BOSS_CORBORUS, &CorborusAI::Create);
    mgr->register_creature_script(BOSS_SLABHIDE, &SlabhideAI::Create);
    mgr->register_creature_script(BOSS_OZRUK, &OzrukAI::Create);
    mgr->register_creature_script(BOSS_HIGH_PRIESTESS_AZIL, &HighPriestessAzilAI::Create);

    mgr->register_creature_script(NPC_STONECORE_RIFT_CONJURER, &StonecoreRiftConjurerAI::Create);
    mgr->register_creature_script(NPC_STONECORE_BRUISER, &StonecoreBruiserAI::Create);
    mgr->register_creature_script(NPC_STONECORE_WARBRINGER, &StonecoreWarbringerAI::Create);
    mgr->register_creature_script(NPC_STONECORE_MAGMALORD, &StonecoreMagmalordAI::Create);
    mgr->register_creature_script(NPC_STONECORE_FLAYER, &StonecoreFlayerAI::Create);
    mgr->register_creature_script(NPC_CRYSTALSPAWN_GIANT, &CrystalspawnGiantAI::Create);
    mgr->register_creature_script(NPC_STONECORE_BERSERKER, &StonecoreBerserkerAI::Create);
    mgr->register_creature_script(NPC_STONECORE_EARTHSHAPER, &StonecoreEarthshaperAI::Create);
}
