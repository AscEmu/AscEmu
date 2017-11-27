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
#include "Instance_BlackrockSpire.h"


// General Drakkisath AI by Soulshifter
class GeneralDrakkisathAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(GeneralDrakkisathAI);
        GeneralDrakkisathAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto firenova = addAISpell(SPELL_FIRENOVA, 15.0f, TARGET_VARIOUS);
            firenova->setAttackStopTimer(1000);

            auto cleave = addAISpell(SPELL_CLEAVE1, 20.0f, TARGET_ATTACKING);
            cleave->setAttackStopTimer(1000);

            auto conflagration = addAISpell(SPELL_CONFLAGRATION, 20.0f, TARGET_ATTACKING);
            conflagration->setAttackStopTimer(1000);

            auto thunderclap = addAISpell(SPELL_THUNDERCLAP, 15.0f, TARGET_VARIOUS);
            thunderclap->setAttackStopTimer(1000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};


// Pyroguard Embersser AI by Soulshifter
class PyroguardEmbersserAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(PyroguardEmbersserAI);
        PyroguardEmbersserAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto firenova = addAISpell(SPELL_FIRENOVA, 15.0f, TARGET_VARIOUS);
            firenova->setAttackStopTimer(1000);

            auto flamebuffet = addAISpell(SPELL_FLAMEBUFFET, 25.0f, TARGET_VARIOUS);
            flamebuffet->setAttackStopTimer(1000);

            auto pyroblast = addAISpell(SPELL_PYROBLAST, 30.0f, TARGET_ATTACKING);
            pyroblast->setAttackStopTimer(1000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};

// Warchief Rend Blackhand AI by Soulshifter
///\todo  PHASES. D:
class RendBlackhandAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(RendBlackhandAI);
        RendBlackhandAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto whirlwind = addAISpell(SPELL_WHIRLWIND, 30.0f, TARGET_VARIOUS);
            whirlwind->setAttackStopTimer(1000);

            auto cleave = addAISpell(SPELL_CLEAVE2, 30.0f, TARGET_VARIOUS);
            cleave->setAttackStopTimer(1000);

            auto thunderclap = addAISpell(SPELL_THUNDERCLAP_WR, 30.0f, TARGET_ATTACKING);
            thunderclap->setAttackStopTimer(1000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};


// Gyth AI by Soulshifter
class GythAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(GythAI);
        GythAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto corrosiveacid = addAISpell(SPELL_CORROSIVEACID, 24.0f, TARGET_VARIOUS);
            corrosiveacid->setAttackStopTimer(1000);

            auto freeze = addAISpell(SPELL_FREEZE, 30.0f, TARGET_ATTACKING);
            freeze->setAttackStopTimer(1000);

            auto flameBreath = addAISpell(SPELL_FLAMEBREATH, 20.0f, TARGET_VARIOUS);
            flameBreath->setAttackStopTimer(1000);

            HasSummoned = false;
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            HasSummoned = false;
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }

        void AIUpdate() override
        {
            if (!HasSummoned && getCreature()->GetHealthPct() <= 8)
            {
                Unit* Warchief = spawnCreature(CN_REND_BLACKHAND, 157.366516f, -419.779358f, 110.472336f, 3.056772f);
                if (Warchief != NULL)
                {
                    if (getCreature()->GetAIInterface()->getNextTarget() != NULL)
                    {
                        Warchief->GetAIInterface()->AttackReaction(getCreature()->GetAIInterface()->getNextTarget(), 1, 0);
                    }
                }

                HasSummoned = true;
            }
        }

    protected:

        bool HasSummoned;
};


// The Beast AI by Soulshifter
class TheBeastAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(TheBeastAI);
        TheBeastAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto flameBreak = addAISpell(SPELL_FLAMEBREAK, 20.0f, TARGET_VARIOUS);
            flameBreak->setAttackStopTimer(1000);

            auto immolate = addAISpell(SPELL_IMMOLATE, 15.0f, TARGET_ATTACKING);
            immolate->setAttackStopTimer(1000);

            auto terrifyingroar = addAISpell(SPELL_TERRIFYINGROAR, 20.0f, TARGET_VARIOUS);
            terrifyingroar->setAttackStopTimer(1000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};


// Highlord Omokk AI by Soulshifter
class HighlordOmokkAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(HighlordOmokkAI);
        HighlordOmokkAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto warstomp = addAISpell(SPELL_WARSTOMP, 20.0f, TARGET_VARIOUS);
            warstomp->setAttackStopTimer(1000);

            auto cleave = addAISpell(SPELL_CLEAVE3, 10.0f, TARGET_ATTACKING);
            cleave->setAttackStopTimer(1000);

            auto strike = addAISpell(SPELL_STRIKE, 30.0f, TARGET_ATTACKING);
            strike->setAttackStopTimer(1000);

            auto rend = addAISpell(SPELL_REND, 25.0f, TARGET_ATTACKING);
            rend->setAttackStopTimer(1000);

            auto sunderarmor = addAISpell(SPELL_SUNDERARMOR, 20.0f, TARGET_ATTACKING);
            sunderarmor->setAttackStopTimer(1000);

            auto knockaway = addAISpell(SPELL_KNOCKAWAY, 20.0f, TARGET_VARIOUS);
            knockaway->setAttackStopTimer(1000);

            auto slow = addAISpell(SPELL_SLOW, 20.0f, TARGET_VARIOUS);
            slow->setAttackStopTimer(1000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};


// Shadow Hunter Vosh'gajin AI by Soulshifter
class ShadowHunterVoshAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(ShadowHunterVoshAI);
        ShadowHunterVoshAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto curseOfBlood = addAISpell(SPELL_CURSEOFBLOOD, 15.0f, TARGET_VARIOUS);
            curseOfBlood->setAttackStopTimer(1000);

            auto hex = addAISpell(SPELL_HEX, 20.0f, TARGET_ATTACKING);
            hex->setAttackStopTimer(1000);

            auto cleave = addAISpell(SPELL_CLEAVE4, 30.0f, TARGET_ATTACKING);
            cleave->setAttackStopTimer(1000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};


// War Master Voone AI by Soulshifter
class WarMasterVooneAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(WarMasterVooneAI);
    WarMasterVooneAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        enableCreatureAISpellSystem = true;

        auto snapkick = addAISpell(SPELL_SNAPKICK, 20.0f, TARGET_VARIOUS);
        snapkick->setAttackStopTimer(1000);

        auto cleave = addAISpell(SPELL_CLEAVE_WM, 10.0f, TARGET_ATTACKING);
        cleave->setAttackStopTimer(1000);

        auto uppercut = addAISpell(SPELL_UPPERCUT, 20.0f, TARGET_ATTACKING);
        uppercut->setAttackStopTimer(1000);

        auto mortalstrike = addAISpell(SPELL_MORTALSTRIKE, 20.0f, TARGET_ATTACKING);
        mortalstrike->setAttackStopTimer(1000);

        auto pummel = addAISpell(SPELL_PUMMEL, 20.0f, TARGET_ATTACKING);
        pummel->setAttackStopTimer(1000);

        auto throwax = addAISpell(SPELL_THROWAXE, 30.0f, TARGET_ATTACKING);
        throwax->setAttackStopTimer(1000);
    }

    void OnCombatStop(Unit* /*mTarget*/) override
    {
        setAIAgent(AGENT_NULL);
        getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
    }
};


// Mother Smolderweb AI by Soulshifter
class MotherSmolderwebAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(MotherSmolderwebAI);
        MotherSmolderwebAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto crystalize = addAISpell(SPELL_CRYSTALIZE, 25.0f, TARGET_VARIOUS);
            crystalize->setAttackStopTimer(1000);

            auto mothersmilk = addAISpell(SPELL_MOTHERSMILK, 20.0f, TARGET_ATTACKING);
            mothersmilk->setAttackStopTimer(1000);

            auto poison = addAISpell(SPELL_POISON, 20.0f, TARGET_ATTACKING);
            poison->setAttackStopTimer(1000);

            auto webexplosion = addAISpell(SPELL_WEBEXPLOSION, 20.0f, TARGET_ATTACKING);
            webexplosion->setAttackStopTimer(1000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};


// Urok Doomhowl AI by Soulshifter
class UrokDoomhowlAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(UrokDoomhowlAI);
        UrokDoomhowlAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto warstomp = addAISpell(SPELL_WARSTOMP_UD, 20.0f, TARGET_VARIOUS);
            warstomp->setAttackStopTimer(1000);

            auto cleave = addAISpell(SPELL_CLEAVE_UD, 10.0f, TARGET_ATTACKING);
            cleave->setAttackStopTimer(1000);

            auto strike = addAISpell(SPELL_STRIKE_UD, 20.0f, TARGET_ATTACKING);
            strike->setAttackStopTimer(1000);

            auto rend = addAISpell(SPELL_REND_UD, 20.0f, TARGET_ATTACKING);
            rend->setAttackStopTimer(1000);

            auto sunderArmor = addAISpell(SPELL_SUNDERARMOR_UD, 20.0f, TARGET_ATTACKING);
            sunderArmor->setAttackStopTimer(1000);

            auto knockaway = addAISpell(SPELL_KNOCKAWAY_UD, 20.0f, TARGET_VARIOUS);
            knockaway->setAttackStopTimer(1000);

            auto slow = addAISpell(SPELL_SLOW_UD, 10.0f, TARGET_VARIOUS);
            slow->setAttackStopTimer(1000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};


// Quartermaster Zigris AI by Soulshifter
class QuartermasterZigrisAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(QuartermasterZigrisAI);
        QuartermasterZigrisAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto shoot = addAISpell(SPELL_SHOOT, 40.0f, TARGET_VARIOUS);
            shoot->setAttackStopTimer(1000);

            auto stunbomb = addAISpell(SPELL_STUNBOMB, 20.0f, TARGET_ATTACKING);
            stunbomb->setAttackStopTimer(1000);

            auto hooked = addAISpell(SPELL_HOOKEDNET, 20.0f, TARGET_ATTACKING);
            hooked->setAttackStopTimer(1000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};


// Halycon AI by Soulshifter
class HalyconAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(HalyconAI);
        HalyconAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto crowdpummel = addAISpell(SPELL_CROWDPUMMEL, 25.0f, TARGET_VARIOUS);
            crowdpummel->setAttackStopTimer(1000);

            auto mightyBlow = addAISpell(SPELL_MIGHTYBLOW, 25.0f, TARGET_ATTACKING);
            mightyBlow->setAttackStopTimer(1000);

            HasSummoned = false;
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            HasSummoned = false;
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }

        void AIUpdate() override
        {
            if (!HasSummoned && getCreature()->GetHealthPct() <= 25)
            {
                Unit* cGizrul = spawnCreature(CN_GIZRUL, -195.100006f, -321.970001f, 65.424400f, 0.016500f);
                if (cGizrul != NULL)
                {
                    if (getCreature()->GetAIInterface()->getNextTarget() != NULL)
                    {
                        cGizrul->GetAIInterface()->AttackReaction(getCreature()->GetAIInterface()->getNextTarget(), 1, 0);
                    }
                }

                HasSummoned = true;
            }
        }

    protected:

        bool HasSummoned;
};


// Overlord Wyrmthalak AI by Soulshifter
class OverlordWyrmthalakAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(OverlordWyrmthalakAI);
        OverlordWyrmthalakAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto blastWave = addAISpell(SPELL_BLASTWAVE, 25.0f, TARGET_VARIOUS);
            blastWave->setAttackStopTimer(1000);

            auto shout = addAISpell(SPELL_SHOUT, 20.0f, TARGET_ATTACKING);
            shout->setAttackStopTimer(1000);

            auto cleaves = addAISpell(SPELL_CLEAVE5, 25.0f, TARGET_VARIOUS);
            cleaves->setAttackStopTimer(1000);

            auto knockaway = addAISpell(SPELL_KNOCKAWAY_OW, 25.0f, TARGET_VARIOUS);
            knockaway->setAttackStopTimer(1000);

            HasSummoned = false;
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            HasSummoned = false;
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }

        void AIUpdate() override
        {
            if (!HasSummoned && getCreature()->GetHealthPct() <= 50)
            {
                Unit* Warlord1 = spawnCreature(CN_SPIRESTONE_WARLORD, -30.675352f, -493.231750f, 90.610725f, 3.123542f);
                Unit* Warlord2 = spawnCreature(CN_SPIRESTONE_WARLORD, -30.433489f, -479.833923f, 90.535606f, 3.123542f);
                if (getCreature()->GetAIInterface()->getNextTarget() != NULL)
                {
                    if (Warlord1 != NULL)
                    {
                        Warlord1->GetAIInterface()->AttackReaction(getCreature()->GetAIInterface()->getNextTarget(), 1, 0);
                    }
                    if (Warlord2 != NULL)
                    {
                        Warlord2->GetAIInterface()->AttackReaction(getCreature()->GetAIInterface()->getNextTarget(), 1, 0);
                    }
                }

                HasSummoned = true;    //Indicates that the spawns have been summoned
            }
        }

    protected:

        bool HasSummoned;
};

void SetupBlackrockSpire(ScriptMgr* mgr)
{
    mgr->register_creature_script(CN_GENERAL_DRAKKISATH, &GeneralDrakkisathAI::Create);
    mgr->register_creature_script(CN_PYROGUARD_EMBERSSER, &PyroguardEmbersserAI::Create);
    mgr->register_creature_script(CN_REND_BLACKHAND, &RendBlackhandAI::Create);
    mgr->register_creature_script(CN_GYTH, &GythAI::Create);
    mgr->register_creature_script(CN_THE_BEAST, &TheBeastAI::Create);
    mgr->register_creature_script(CN_HIGHLORD_OMOKK, &HighlordOmokkAI::Create);
    mgr->register_creature_script(CN_SHADOW_HUNTER_VOSH, &ShadowHunterVoshAI::Create);
    mgr->register_creature_script(CN_WAR_MASTER_VOONE, &WarMasterVooneAI::Create);
    mgr->register_creature_script(CN_MOTHER_SMOLDERWEB, &MotherSmolderwebAI::Create);
    mgr->register_creature_script(CN_UROK_DOOMHOWL, &UrokDoomhowlAI::Create);
    mgr->register_creature_script(CN_QUARTERMASTER_ZIGRIS, &QuartermasterZigrisAI::Create);
    mgr->register_creature_script(CN_HALYCON, &HalyconAI::Create);
    mgr->register_creature_script(CN_OVERLORD_WYRMTHALAK, &OverlordWyrmthalakAI::Create);
}
