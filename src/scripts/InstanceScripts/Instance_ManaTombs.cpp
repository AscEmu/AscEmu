/*
 * ArcScripts for ArcEmu MMORPG Server
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2008-2015 Sun++ Team <http://www.sunplusplus.info/>
 * Copyright (C) 2005-2007 Ascent Team
 * Copyright (C) 2007-2015 Moon++ Team <http://www.moonplusplus.info/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Setup.h"
#include "Instance_ManaTombs.h"
#include "Objects/Faction.h"


// EtherealDarkcasterAI
class EtherealDarkcasterAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(EtherealDarkcasterAI);
        EtherealDarkcasterAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto manaBurn = addAISpell(MANA_BURN, 10.0f, TARGET_DESTINATION, 0, 0, false, true);
            manaBurn->setAttackStopTimer(1000);

            auto shadowWordPain = addAISpell(SHADOW_WORD_PAIN, 10.0f, TARGET_ATTACKING, 0, 0, false, true);
            shadowWordPain->setAttackStopTimer(1000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};


// EtherealPriestAI
class EtherealPriestAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(EtherealPriestAI);
        EtherealPriestAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto heal = addAISpell(HEAL, 7.0f, TARGET_SELF);
            heal->setAttackStopTimer(1000);

            auto powerWordShield = addAISpell(POWER_WORD_SHIELD, 7.0f, TARGET_SELF, 0, 0, false, true);
            powerWordShield->setAttackStopTimer(1000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};

// EtherealTheurgistAI
class EtherealTheurgistAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(EtherealTheurgistAI);
        EtherealTheurgistAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto polymorph = addAISpell(POLYMORPH, 7.0f, TARGET_ATTACKING);
            polymorph->setAttackStopTimer(1000);

            auto blastWave = addAISpell(BLAST_WAVE, 10.0f, TARGET_SELF, 0, 0, false, true);
            blastWave->setAttackStopTimer(1000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};


// EtherealSorcererAI
class EtherealSorcererAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(EtherealSorcererAI);
        EtherealSorcererAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto arcaneMissiles = addAISpell(ARCANE_MISSILES, 10.0f, TARGET_ATTACKING, 0, 0, false, true);
            arcaneMissiles->setAttackStopTimer(1000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};

// NexusStalkerAI
class NexusStalkerAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(NexusStalkerAI);
        NexusStalkerAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto couge = addAISpell(GOUGE, 10.0f, TARGET_ATTACKING, 0, 0, false, true);
            couge->setAttackStopTimer(1000);

            auto poison = addAISpell(POISON, 7.0f, TARGET_ATTACKING, 0, 0, false, true);
            poison->setAttackStopTimer(1000);

            auto stealth = addAISpell(STEALTH, 5.0f, TARGET_SELF, 0, 0, false, true);
            stealth->setAttackStopTimer(1000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};

// NexusTerrorAI
class NexusTerrorAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(NexusTerrorAI);
        NexusTerrorAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto psychicScream = addAISpell(PSYCHIC_SCREAM, 3.0f, TARGET_VARIOUS, 0, 0, false, true);
            psychicScream->setAttackStopTimer(1000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};


// ManaLeechAI
class ManaLeechAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(ManaLeechAI);
        ManaLeechAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            arcaneExplosion = addAISpell(ARCANE_EXPLOSION, 0.0f, TARGET_VARIOUS, 0, 0, false, true);
            arcaneExplosion->setAttackStopTimer(1000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }

        void OnDied(Unit* /*mKiller*/) override
        {
            getCreature()->CastSpell(getCreature(), arcaneExplosion->mSpellInfo, true);
        }

    protected:

        CreatureAISpells* arcaneExplosion;
};

// EtherealSpellbinderAI
class EtherealSpellbinderAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(EtherealSpellbinderAI);
        EtherealSpellbinderAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto corruption = addAISpell(CORRUPTION, 7.0f, TARGET_ATTACKING, 0, 0, false, true);
            corruption->setAttackStopTimer(1000);

            auto immolate = addAISpell(IMMOLATE, 7.0f, TARGET_ATTACKING, 0, 0, false, true);
            immolate->setAttackStopTimer(1000);

            auto unstableAffliction = addAISpell(UNSTABLE_AFFLICTION, 7.0f, TARGET_ATTACKING, 0, 0, false, true);
            unstableAffliction->setAttackStopTimer(1000);

            auto summonEtherealWraith = addAISpell(SUMMON_ETHEREAL_WRAITH, 5.0f, TARGET_SELF, 0, 0, false, true);
            summonEtherealWraith->setAttackStopTimer(1000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};


// EtherealWraithAI
class EtherealWraithAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(EtherealWraithAI);
        EtherealWraithAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto shadowBoltVolley = addAISpell(SHADOW_BOLT_VOLLEY, 15.0f, TARGET_VARIOUS);
            shadowBoltVolley->setAttackStopTimer(1000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};


// \todo Needs to add Yor in a future

// PandemoniusAI
class PandemoniusAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(PandemoniusAI);
        PandemoniusAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

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

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};

// TavarokAI
/// \todo Strange... I couldn't find any sounds for this boss in DBC and in extracted from client sounds O_O
class TavarokAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(TavarokAI);
        TavarokAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto earthquake = addAISpell(EARTHQUAKE, 8.0f, TARGET_VARIOUS, 0, 20);
            earthquake->setAttackStopTimer(2000);

            auto crystalPrison = addAISpell(CRYSTAL_PRISON, 8.0f, TARGET_RANDOM_SINGLE, 0, 20);
            crystalPrison->setAttackStopTimer(2000);
            crystalPrison->setMinMaxDistance(0.0f, 40.0f);

            auto arcingSmash = addAISpell(ARCING_SMASH, 12.0f, TARGET_VARIOUS, 0, 10, false, true);
            arcingSmash->setAttackStopTimer(2000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};


// NexusPrinceShaffarAI
// \todo Work on beacons and find out if my current way of spawning them is correct
class NexusPrinceShaffarAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(NexusPrinceShaffarAI);
        NexusPrinceShaffarAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

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

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};

// YorAI
class YorAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(YorAI);
        YorAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto doubleBreath = addAISpell(DOUBLE_BREATH, 20.0f, TARGET_VARIOUS, 0, 15, false, true);
            doubleBreath->setAttackStopTimer(2000);

            auto stomp = addAISpell(STOMP, 7.0f, TARGET_VARIOUS, 0, 25, false, true);
            stomp->setAttackStopTimer(2000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};

void SetupManaTombs(ScriptMgr* mgr)
{
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
