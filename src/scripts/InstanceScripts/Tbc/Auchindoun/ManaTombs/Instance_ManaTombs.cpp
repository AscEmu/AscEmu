/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Instance_ManaTombs.h"

#include "Setup.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"

class ManaTombsInstanceScript : public InstanceScript
{
public:
    explicit ManaTombsInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr){}
    static InstanceScript* Create(WorldMap* pMapMgr) { return new ManaTombsInstanceScript(pMapMgr); }
};

class EtherealDarkcasterAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EtherealDarkcasterAI(c); }
    explicit EtherealDarkcasterAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto manaBurn = addAISpell(MANA_BURN, 10.0f, TARGET_DESTINATION, 0, 0, false, true);
        manaBurn->setAttackStopTimer(1000);

        auto shadowWordPain = addAISpell(SHADOW_WORD_PAIN, 10.0f, TARGET_ATTACKING, 0, 0, false, true);
        shadowWordPain->setAttackStopTimer(1000);
    }
};

class EtherealPriestAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EtherealPriestAI(c); }
    explicit EtherealPriestAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto heal = addAISpell(HEAL, 7.0f, TARGET_SELF);
        heal->setAttackStopTimer(1000);

        auto powerWordShield = addAISpell(POWER_WORD_SHIELD, 7.0f, TARGET_SELF, 0, 0, false, true);
        powerWordShield->setAttackStopTimer(1000);
    }
};

class EtherealTheurgistAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EtherealTheurgistAI(c); }
    explicit EtherealTheurgistAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto polymorph = addAISpell(POLYMORPH, 7.0f, TARGET_ATTACKING);
        polymorph->setAttackStopTimer(1000);

        auto blastWave = addAISpell(BLAST_WAVE, 10.0f, TARGET_SELF, 0, 0, false, true);
        blastWave->setAttackStopTimer(1000);
    }
};

class EtherealSorcererAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EtherealSorcererAI(c); }
    explicit EtherealSorcererAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto arcaneMissiles = addAISpell(ARCANE_MISSILES, 10.0f, TARGET_ATTACKING, 0, 0, false, true);
        arcaneMissiles->setAttackStopTimer(1000);
    }
};

class NexusStalkerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NexusStalkerAI(c); }
    explicit NexusStalkerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto couge = addAISpell(GOUGE, 10.0f, TARGET_ATTACKING, 0, 0, false, true);
        couge->setAttackStopTimer(1000);

        auto poison = addAISpell(POISON, 7.0f, TARGET_ATTACKING, 0, 0, false, true);
        poison->setAttackStopTimer(1000);

        auto stealth = addAISpell(STEALTH, 5.0f, TARGET_SELF, 0, 0, false, true);
        stealth->setAttackStopTimer(1000);
    }
};

class NexusTerrorAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NexusTerrorAI(c); }
    explicit NexusTerrorAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto psychicScream = addAISpell(PSYCHIC_SCREAM, 3.0f, TARGET_VARIOUS, 0, 0, false, true);
        psychicScream->setAttackStopTimer(1000);
    }
};

class ManaLeechAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ManaLeechAI(c); }
    explicit ManaLeechAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        arcaneExplosion = addAISpell(ARCANE_EXPLOSION, 0.0f, TARGET_VARIOUS, 0, 0, false, true);
        arcaneExplosion->setAttackStopTimer(1000);
    }

    void OnDied(Unit* /*mKiller*/) override
    {
        getCreature()->castSpell(getCreature(), arcaneExplosion->mSpellInfo, true);
    }

protected:
    CreatureAISpells* arcaneExplosion;
};

class EtherealSpellbinderAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EtherealSpellbinderAI(c); }
    explicit EtherealSpellbinderAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto corruption = addAISpell(CORRUPTION, 7.0f, TARGET_ATTACKING, 0, 0, false, true);
        corruption->setAttackStopTimer(1000);

        auto immolate = addAISpell(IMMOLATE, 7.0f, TARGET_ATTACKING, 0, 0, false, true);
        immolate->setAttackStopTimer(1000);

        auto unstableAffliction = addAISpell(UNSTABLE_AFFLICTION, 7.0f, TARGET_ATTACKING, 0, 0, false, true);
        unstableAffliction->setAttackStopTimer(1000);

        auto summonEtherealWraith = addAISpell(SUMMON_ETHEREAL_WRAITH, 5.0f, TARGET_SELF, 0, 0, false, true);
        summonEtherealWraith->setAttackStopTimer(1000);
    }
};

class EtherealWraithAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EtherealWraithAI(c); }
    explicit EtherealWraithAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto shadowBoltVolley = addAISpell(SHADOW_BOLT_VOLLEY, 15.0f, TARGET_VARIOUS);
        shadowBoltVolley->setAttackStopTimer(1000);
    }
};

class PandemoniusAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new PandemoniusAI(c); }
    explicit PandemoniusAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto darkShell = addAISpell(DARK_SHELL, 20.0f, TARGET_SELF, 0, 20);
        darkShell->setAttackStopTimer(2000);

        auto voidBlast = addAISpell(VOID_BLAST, 0.0f, TARGET_SELF, 0, 5);
        voidBlast->setAttackStopTimer(1500);
        voidBlast->setMinMaxDistance(0.0f, 40.0f);

        addEmoteForEvent(Event_OnCombatStart, SAY_PANDEMONIUS_01);
        addEmoteForEvent(Event_OnCombatStart, SAY_PANDEMONIUS_02);
        addEmoteForEvent(Event_OnCombatStart, SAY_PANDEMONIUS_03);
        addEmoteForEvent(Event_OnTargetDied, SAY_PANDEMONIUS_04);
        addEmoteForEvent(Event_OnTargetDied, SAY_PANDEMONIUS_05);
        addEmoteForEvent(Event_OnDied, SAY_PANDEMONIUS_06);
    }
};

// \todo Strange... I couldn't find any sounds for this boss in DBC and in extracted from client sounds O_O
class TavarokAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TavarokAI(c); }
    explicit TavarokAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto earthquake = addAISpell(EARTHQUAKE, 8.0f, TARGET_VARIOUS, 0, 20);
        earthquake->setAttackStopTimer(2000);

        auto crystalPrison = addAISpell(CRYSTAL_PRISON, 8.0f, TARGET_RANDOM_SINGLE, 0, 20);
        crystalPrison->setAttackStopTimer(2000);
        crystalPrison->setMinMaxDistance(0.0f, 40.0f);

        auto arcingSmash = addAISpell(ARCING_SMASH, 12.0f, TARGET_VARIOUS, 0, 10, false, true);
        arcingSmash->setAttackStopTimer(2000);
    }
};

// \todo Work on beacons and find out if my current way of spawning them is correct
class NexusPrinceShaffarAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NexusPrinceShaffarAI(c); }
    explicit NexusPrinceShaffarAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto fireball = addAISpell(FIREBALL, 35.0f, TARGET_RANDOM_SINGLE, 0, 20);
        fireball->setAttackStopTimer(2000);
        fireball->setMinMaxDistance(0.0f, 40.0f);

        auto frostball = addAISpell(FROSTBOLT, 35.0f, TARGET_RANDOM_SINGLE, 0, 5);
        frostball->setAttackStopTimer(2000);
        frostball->setMinMaxDistance(0.0f, 40.0f);

        auto frostNova = addAISpell(FROST_NOVA, 15.0f, TARGET_VARIOUS, 0, 15, false, true);
        frostNova->setAttackStopTimer(1000);

        auto blink = addAISpell(BLINK, 5.0f, TARGET_SELF, 0, 20, false, true);
        blink->setAttackStopTimer(1000);

        auto summonEterealBecon = addAISpell(SUMMON_ETEREAL_BECON, 0.0f, TARGET_SELF, 0, 10, false, true);
        summonEterealBecon->setAttackStopTimer(1000);

        addEmoteForEvent(Event_OnCombatStart, SAY_NEXUSPRINCE_02);
        addEmoteForEvent(Event_OnCombatStart, SAY_NEXUSPRINCE_03);
        addEmoteForEvent(Event_OnCombatStart, SAY_NEXUSPRINCE_04);
        addEmoteForEvent(Event_OnTargetDied, SAY_NEXUSPRINCE_06);
        addEmoteForEvent(Event_OnTargetDied, SAY_NEXUSPRINCE_05);
        addEmoteForEvent(Event_OnDied, SAY_NEXUSPRINCE_08);
    }
};

class YorAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new YorAI(c); }
    explicit YorAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto doubleBreath = addAISpell(DOUBLE_BREATH, 20.0f, TARGET_VARIOUS, 0, 15, false, true);
        doubleBreath->setAttackStopTimer(2000);

        auto stomp = addAISpell(STOMP, 7.0f, TARGET_VARIOUS, 0, 25, false, true);
        stomp->setAttackStopTimer(2000);
    }
};

void SetupManaTombs(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_AUCHENAI_MANA_TOMBS, &ManaTombsInstanceScript::Create);

    mgr->register_creature_script(CN_ETHEREAL_DARKCASTER, &EtherealDarkcasterAI::Create);
    mgr->register_creature_script(CN_ETHEREAL_PRIEST, &EtherealPriestAI::Create);
    mgr->register_creature_script(CN_ETHEREAL_SPELLBINDER, &EtherealSpellbinderAI::Create);
    mgr->register_creature_script(CN_ETHEREAL_THEURGIST, &EtherealTheurgistAI::Create);
    mgr->register_creature_script(CN_ETHEREAL_SORCERER, &EtherealSorcererAI::Create);
    mgr->register_creature_script(CN_ETHEREAL_WRAITH, &EtherealWraithAI::Create);
    mgr->register_creature_script(CN_NEXUS_STALKER, &NexusStalkerAI::Create);
    mgr->register_creature_script(CN_NEXUS_TERROR, &NexusTerrorAI::Create);
    mgr->register_creature_script(CN_MANA_LEECH, &ManaLeechAI::Create);
    mgr->register_creature_script(CN_PANDEMONIUS, &PandemoniusAI::Create);
    mgr->register_creature_script(CN_TAVAROK, &TavarokAI::Create);
    mgr->register_creature_script(CN_NEXUS_PRINCE_SHAFFAR, &NexusPrinceShaffarAI::Create);
    mgr->register_creature_script(CN_YOR, &YorAI::Create);
}
