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
#include "Instance_Scholomance.h"


class DoctorTheolenKrastinovAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(DoctorTheolenKrastinovAI);
        DoctorTheolenKrastinovAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto rend = addAISpell(SP_DR_THEOL_REND, 20.0f, TARGET_ATTACKING, 0, 0, false, true);
            rend->setAttackStopTimer(1000);

            auto cleave = addAISpell(SP_DR_THEOL_KRASTINOVCLEAVE, 9.0f, TARGET_ATTACKING, 0, 0, false, true);
            cleave->setAttackStopTimer(1000);

            frenzy = addAISpell(SP_DR_THEOL_FRENZY, 0.0f, TARGET_SELF, 0, 0, false, true);
            frenzy->setAttackStopTimer(1000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }

        void AIUpdate() override
        {
            if (getCreature()->GetHealthPct() <= 50 && getScriptPhase() == 1)
            {
                getCreature()->CastSpell(getCreature(), frenzy->mSpellInfo, true);
                setScriptPhase(2);
            }
        }

    protected:

        CreatureAISpells* frenzy;
};

class InstructorMaliciaAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(InstructorMaliciaAI);
        InstructorMaliciaAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto callOfGrave = addAISpell(SP_MALICIA_CALL_OF_GRAVE, 10.0f, TARGET_DESTINATION, 0, 10, false, true);
            callOfGrave->setAttackStopTimer(1000);

            auto corruption = addAISpell(SP_MALICIA_CORRUPTION, 9.0f, TARGET_ATTACKING, 0, 0, false, true);
            corruption->setAttackStopTimer(1000);

            auto flashHeal = addAISpell(SP_MALICIA_FLASH_HEAL, 5.0f, TARGET_SELF, 0, 0, false, true);
            flashHeal->setAttackStopTimer(1000);

            auto renew = addAISpell(SP_MALICIA_RENEW, 4.0f, TARGET_SELF, 0, 0, false, true);
            renew->setAttackStopTimer(1000);

            auto heal = addAISpell(SP_MALICIA_HEAL, 5.0f, TARGET_SELF, 0, 0, false, true);
            heal->setAttackStopTimer(1000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};

class TheRavenianAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(TheRavenianAI);
        TheRavenianAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto trample = addAISpell(SP_RAVENIAN_TRAMPLE, 8.0f, TARGET_ATTACKING, 0, 0, false, true);
            trample->setAttackStopTimer(1000);

            auto cleave = addAISpell(SP_RAVENIAN_RAVENIANCLEAVE, 9.0f, TARGET_ATTACKING, 0, 0, false, true);
            cleave->setAttackStopTimer(1000);

            auto sunderCleave = addAISpell(SP_RAVENIAN_SUNDERINCLEAVE, 20.0f, TARGET_ATTACKING, 0, 0, false, true);
            sunderCleave->setAttackStopTimer(1000);

            auto knockaway = addAISpell(SP_RAVENIAN_KNOCKAWAY, 11.0f, TARGET_ATTACKING, 0, 0, false, true);
            knockaway->setAttackStopTimer(1000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};

class LadyIlluciaBarovAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(LadyIlluciaBarovAI);
        LadyIlluciaBarovAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto curseOfAgony = addAISpell(SP_ILLUCIA_CURSE_OF_AGONY, 8.0f, TARGET_VARIOUS, 0, 0, false, true);
            curseOfAgony->setAttackStopTimer(1000);

            auto shadowShock = addAISpell(SP_ILLUCIA_SHADOW_SHOCK, 12.0f, TARGET_VARIOUS, 0, 0, false, true);
            shadowShock->setAttackStopTimer(1000);

            auto silence = addAISpell(SP_ILLUCIA_SILENCE, 8.0f, TARGET_ATTACKING, 0, 0, false, true);
            silence->setAttackStopTimer(1000);

            auto fear = addAISpell(SP_ILLUCIA_FEAR, 4.0f, TARGET_ATTACKING, 0, 0, false, true);
            fear->setAttackStopTimer(1000);

            auto dominantMind = addAISpell(SP_ILLUCIA_DOMINATE_MIND, 4.0f, TARGET_ATTACKING, 0, 0, false, true);
            dominantMind->setAttackStopTimer(1000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};

class RasForstwhisperAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(RasForstwhisperAI);
        RasForstwhisperAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto frostbolt = addAISpell(SP_RAS_FORTH_FROSTBOLT, 14.0f, TARGET_ATTACKING);
            frostbolt->setAttackStopTimer(2000);

            IceArmor = addAISpell(SP_RAS_FORTH_ICE_ARMOR, 0.0f, TARGET_SELF, 0, 0, false, true);
            IceArmor->setAttackStopTimer(1000);

            auto freeze = addAISpell(SP_RAS_FORTH_FREEZE, 11.0f, TARGET_ATTACKING);
            freeze->setAttackStopTimer(4000);

            auto fear = addAISpell(SP_RAS_FORTH_FEAR, 9.0f, TARGET_ATTACKING, 0, 0, false, true);
            fear->setAttackStopTimer(2000);

            auto chillNova = addAISpell(SP_RAS_FORTH_CHILL_NOVA, 8.0f, TARGET_VARIOUS, 0, 0, false, true);
            chillNova->setAttackStopTimer(1000);

            auto frostVolley = addAISpell(SP_RAS_FORTH_FROSTB_VOLLEY, 13.0f, TARGET_VARIOUS, 0, 0, false, true);
            frostVolley->setAttackStopTimer(2000);
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            getCreature()->CastSpell(getCreature(), IceArmor->mSpellInfo, true);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }

    protected:

        CreatureAISpells* IceArmor;
};

class JandiceBarovAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(JandiceBarovAI);
        JandiceBarovAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto curseOfBlood = addAISpell(SP_JANDICE_CURSE_OF_BLOOD, 8.0f, TARGET_DESTINATION, 0, 0, false, true);
            curseOfBlood->setAttackStopTimer(1000);

            auto banish = addAISpell(SP_JANDICE_BANISH, 8.0f, TARGET_ATTACKING, 0, 0, false, true);
            banish->setAttackStopTimer(1000);

            auto summonIllusion = addAISpell(SP_JANDICE_SUMMON_ILLUSION, 5.0f, TARGET_SELF, 0, 0, false, true);
            summonIllusion->setAttackStopTimer(1000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};

class KormokAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(KormokAI);
        KormokAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto shadowVolley = addAISpell(SP_KORMOK_SHADOW_B_VOLLEY, 11.0f, TARGET_VARIOUS, 0, 0, false, true);
            shadowVolley->setAttackStopTimer(1000);

            boneShield = addAISpell(SP_KORMOK_BONE_SHIELD, 0.0f, TARGET_SELF, 0, 0, false, true);
            boneShield->setAttackStopTimer(1000);

            auto summonLackey = addAISpell(SP_KORMOK_SUM_RISEY_LACKEY, 4.0f, TARGET_SELF, 0, 0, false, true);
            summonLackey->setAttackStopTimer(1000);
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            getCreature()->CastSpell(getCreature(), boneShield->mSpellInfo, true);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }

    protected:

        CreatureAISpells* boneShield;
};

class VectusAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(VectusAI);
        VectusAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto blastWave = addAISpell(SP_VECTUS_BLAST_WAVE, 18.0f, TARGET_ATTACKING, 0, 0, false, true);
            blastWave->setAttackStopTimer(1000);

            fireShield = addAISpell(SP_VECTUS_FIRE_SHIELD, 5.0f, TARGET_SELF, 0, 0, false, true);
            fireShield->setAttackStopTimer(1000);

            frenzy = addAISpell(SP_VECTUS_FRENZY, 0.0f, TARGET_SELF, 0, 0, false, true);
            frenzy->setAttackStopTimer(1000);
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
            getCreature()->CastSpell(getCreature(), fireShield->mSpellInfo, true);
        }

        void AIUpdate() override
        {
            if (getCreature()->GetHealthPct() <= 25 && getScriptPhase() == 1)
            {
                getCreature()->CastSpell(getCreature(), frenzy->mSpellInfo, true);
                setScriptPhase(2);
            }
        }

    protected:

        CreatureAISpells* fireShield;
        CreatureAISpells* frenzy;
};

class LordAlexeiBarovAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(LordAlexeiBarovAI);
        LordAlexeiBarovAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            unholyAura = addAISpell(SP_ALEXEI_UNHOLY_AURA, 0.0f, TARGET_SELF, 0, 0, false, true);
            unholyAura->setAttackStopTimer(1000);

            auto immolate = addAISpell(SP_ALEXEI_IMMOLATE, 10.0f, TARGET_ATTACKING, 0, 0, false, true);
            immolate->setAttackStopTimer(1000);

            auto veilOfShadow = addAISpell(SP_ALEXEI_VEIL_OF_SHADOW, 8.0f, TARGET_VARIOUS, 0, 0, false, true);
            veilOfShadow->setAttackStopTimer(2000);
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            getCreature()->CastSpell(getCreature(), unholyAura->mSpellInfo, true);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }

    protected:

        CreatureAISpells* unholyAura;
};

class LorekeeperPolkeltAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(LorekeeperPolkeltAI);
        LorekeeperPolkeltAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto volatileInfection = addAISpell(SP_LORE_VOLATILE_INFECTION, 6.0f, TARGET_ATTACKING, 0, 0, false, true);
            volatileInfection->setAttackStopTimer(1000);

            auto darkPlague = addAISpell(SP_LORE_DARK_PLAGUE, 8.0f, TARGET_ATTACKING, 0, 0, false, true);
            darkPlague->setAttackStopTimer(1000);

            auto corrosiveAcid = addAISpell(SP_LORE_CORROSIVE_ACID, 12.0f, TARGET_ATTACKING, 0, 0, false, true);
            corrosiveAcid->setAttackStopTimer(1000);

            auto noxiousCatalyst = addAISpell(SP_LORE_NOXIOUS_CATALYST, 10.0f, TARGET_ATTACKING, 0, 0, false, true);
            noxiousCatalyst->setAttackStopTimer(1000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};

class DarkmasterGandlingAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(DarkmasterGandlingAI);
        DarkmasterGandlingAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto arcaneMissiles = addAISpell(SP_GANDLING_ARCANE_MISSILES, 12.0f, TARGET_ATTACKING, 0, 0, false, true);
            arcaneMissiles->setAttackStopTimer(1000);

            auto cotDarkmaster = addAISpell(SP_GANDLING_COT_DARKMASTER, 7.0f, TARGET_ATTACKING);
            cotDarkmaster->setAttackStopTimer(2000);

            shadowShield = addAISpell(SP_GANDLING_SHADOW_SHIELD, 0.0f, TARGET_SELF, 0, 0, false, true);
            shadowShield->setAttackStopTimer(1000);
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            getCreature()->CastSpell(getCreature(), shadowShield->mSpellInfo, true);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }

    protected:

        CreatureAISpells* shadowShield;
};

void SetupScholomance(ScriptMgr* mgr)
{
    //Creatures
    mgr->register_creature_script(CN_DOCTOR_THEOLEN_KRASTINOV, &DoctorTheolenKrastinovAI::Create);
    mgr->register_creature_script(CN_INSTRUCTOR_MALICIA, &InstructorMaliciaAI::Create);
    mgr->register_creature_script(CN_THE_RAVENIAN, &TheRavenianAI::Create);
    mgr->register_creature_script(CN_LADY_ILLUCIA_BAROV, &LadyIlluciaBarovAI::Create);
    mgr->register_creature_script(CN_RAS_FORSTWHISPER, &RasForstwhisperAI::Create);
    mgr->register_creature_script(CN_JANDICE_BAROV, &JandiceBarovAI::Create);
    mgr->register_creature_script(CN_KORMOK, &KormokAI::Create);
    mgr->register_creature_script(CN_VECTUS, &VectusAI::Create);
    mgr->register_creature_script(CN_LORD_ALEXEI_BAROV, &LordAlexeiBarovAI::Create);
    mgr->register_creature_script(CN_LOREKEEPER_POLKELT, &LorekeeperPolkeltAI::Create);
    mgr->register_creature_script(CN_DARKMASTER_GANDLING, &DarkmasterGandlingAI::Create);
}
