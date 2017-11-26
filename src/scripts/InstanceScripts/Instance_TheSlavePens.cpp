/*
 * ArcScripts for ArcEmu MMORPG Server
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
 * Copyright (C) 2007-2015 Moon++ Team <http://www.moonplusplus.info/>
 * Copyright (C) 2008-2015 Sun++ Team <http://www.sunplusplus.info/>
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
#include "Instance_TheSlavePens.h"
#include "Objects/Faction.h"


//////////////////////////////////////////////////////////////
// Boss AIs
//////////////////////////////////////////////////////////////

class TotemsAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(TotemsAI);
        TotemsAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            uint32 Despawn = 30000;
            uint32 AIUpdate = 1000;

            SpellID = 1;
            switch (getCreature()->GetEntry())
            {
                case CN_MENNUS_HEALING_WARD:
                    SpellID = 34977;
                    break;
                case CN_TAINED_EARTHGRAB_TOTEM:
                    SpellID = 20654;
                    AIUpdate = 5000;
                    break;
                case CN_TAINED_STONESKIN_TOTEM:
                    Despawn = 60000;
                    SpellID = 25509;    // temporary spell
                    AIUpdate = 0;
                    break;
                default:    // for Corrupted Nova Totem and it's also safe case
                    {
                        getCreature()->setUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_2);    // hax
                        Despawn = 6000;
                        SpellID = 33132;
                        AIUpdate = 5000;
                    }
            }

            if (AIUpdate != 0)
                RegisterAIUpdateEvent(AIUpdate);

            setAIAgent(AGENT_SPELL);
            getCreature()->Despawn(Despawn, 0);
            getCreature()->m_noRespawn = true;

            getCreature()->CastSpell(getCreature(), SpellID, true);
        }

        void AIUpdate() override
        {
            if (getCreature()->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_2))
                getCreature()->setUInt64Value(UNIT_FIELD_FLAGS, 0);

            getCreature()->CastSpell(getCreature(), SpellID, true);
        }

    protected:

        uint32 SpellID;
};

uint32 Totems[4] = { 20208, 18176, 18177, 14662 };

class MennuTheBetrayerAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(MennuTheBetrayerAI);

        bool SummonedTotems[4];

        MennuTheBetrayerAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            for (uint8 i = 0; i < 4; i++)
                SummonedTotems[i] = false;

            enableCreatureAISpellSystem = true;

            auto lighningBolt = addAISpell(LIGHTNING_BOLT, 10.0f, TARGET_ATTACKING, 0, 15);
            lighningBolt->setAttackStopTimer(5000);

            auto healingWard = addAISpell(MENNUS_HEALING_WARD, 10.0f, TARGET_SELF, 0, 20, false, true);
            healingWard->setAttackStopTimer(1000);

            earthgrabTotem = addAISpell(TAINTED_EARTHGRAB_TOTEM, 0.0f, TARGET_SELF, 0, 20, false, true);
            earthgrabTotem->setAttackStopTimer(1000);

            stoneskinTotem = addAISpell(TAINTED_STONESKIN_TOTEM, 0.0f, TARGET_SELF, 0, 20, false, true);
            stoneskinTotem->setAttackStopTimer(1000);

            novaTotem = addAISpell(CORRUPTED_NOVA_TOTEM, 0.0f, TARGET_SELF, 0, 20, false, true);
            novaTotem->setAttackStopTimer(1000);

            TotemCounter = 0;

            addEmoteForEvent(Event_OnCombatStart, SAY_MENNU_BETRAYER_01);
            addEmoteForEvent(Event_OnCombatStart, SAY_MENNU_BETRAYER_02);
            addEmoteForEvent(Event_OnCombatStart, SAY_MENNU_BETRAYER_03);
            addEmoteForEvent(Event_OnTargetDied, SAY_MENNU_BETRAYER_04);
            addEmoteForEvent(Event_OnTargetDied, SAY_MENNU_BETRAYER_05);
            addEmoteForEvent(Event_OnDied, SAY_MENNU_BETRAYER_06);
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            for (uint8 i = 0; i < 4; i++)
                SummonedTotems[i] = false;

            TotemCounter = 0;
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }

        void AIUpdate() override
        {
            if (getScriptPhase() == 1 && TotemCounter != 0 && !getCreature()->isCastingNonMeleeSpell())
            {
                TotemSpawning();
            }
        }

        // Random totem spawning
        void TotemSpawning()
        {
            getCreature()->setAttackTimer(1500, false);

            bool Spawned = false;
            uint32 Counter = 0;
            while(!Spawned)
            {
                if (Counter >= 2)
                {
                    for (uint8 i = 0; i < 4; i++)
                    {
                        if (!SummonedTotems[i])
                        {
                            spawnCreature(Totems[i], getCreature()->GetPosition());
                            getCreature()->CastSpell(getCreature(), earthgrabTotem->mSpellInfo, earthgrabTotem->mIsTriggered);
                            getCreature()->CastSpell(getCreature(), stoneskinTotem->mSpellInfo, stoneskinTotem->mIsTriggered);
                            getCreature()->CastSpell(getCreature(), novaTotem->mSpellInfo, novaTotem->mIsTriggered);
                            SummonedTotems[i] = true;
                            TotemCounter++;
                            break;
                        }
                    }

                    Spawned = true;
                }

                uint32 i = RandomUInt(3);
                if (SummonedTotems[i])
                    Counter++;
                else
                {
                    spawnCreature(Totems[i], getCreature()->GetPosition());
                    switch (i)
                    {
                        case 1:
                            getCreature()->CastSpell(getCreature(), earthgrabTotem->mSpellInfo, earthgrabTotem->mIsTriggered);
                            break;
                        case 2:
                            getCreature()->CastSpell(getCreature(), stoneskinTotem->mSpellInfo, stoneskinTotem->mIsTriggered);
                            break;
                        case 3:
                            getCreature()->CastSpell(getCreature(), novaTotem->mSpellInfo, novaTotem->mIsTriggered);
                            break;
                        default:
                            break;
                    }

                    SummonedTotems[i] = true;
                    TotemCounter++;
                    Spawned = true;
                }
            }

            if (TotemCounter == 4)
            {
                for (uint8 i = 0; i < 4; i++)
                    SummonedTotems[i] = false;

                TotemCounter = 0;
            }
        }

    protected:

        uint32 TotemCounter;
        CreatureAISpells* earthgrabTotem;
        CreatureAISpells* stoneskinTotem;
        CreatureAISpells* novaTotem;
};

class RokmarTheCracklerAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(RokmarTheCracklerAI);
        RokmarTheCracklerAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto grievousWound = addAISpell(GRIEVOUS_WOUND, 8.0f, TARGET_ATTACKING, 0, 20, false, true);
            grievousWound->setAttackStopTimer(5000);

            auto waterSpit = addAISpell(WATER_SPIT, 16.0f, TARGET_VARIOUS, 0, 10);
            waterSpit->setAttackStopTimer(5000);

            auto ensnaringMoss = addAISpell(ENSNARING_MOSS, 8.0f, TARGET_RANDOM_SINGLE, 0, 35);
            ensnaringMoss->setAttackStopTimer(5000);
            ensnaringMoss->setMinMaxDistance(0.0f, 30.0f);

            enrage = addAISpell(ENRAGE, 0.0f, TARGET_SELF, 0, 0, false, true);
            enrage->setAttackStopTimer(5000);

            Enraged = false;
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            Enraged = false;
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }

        void AIUpdate() override
        {
            if (getCreature()->GetHealthPct() <= 20 && !Enraged && getScriptPhase() == 1)
            {
                getCreature()->CastSpell(getCreature(), enrage->mSpellInfo, enrage->mIsTriggered);

                Enraged = true;
            }
        }

    protected:

        bool Enraged;
        CreatureAISpells* enrage;
};


// QuagmirranAI
class QuagmirranAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(QuagmirranAI);
        QuagmirranAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto acidGeyser = addAISpell(ACID_GEYSER, 10.0f, TARGET_RANDOM_SINGLE, 0, 20, false, true);
            acidGeyser->setAttackStopTimer(5000);
            acidGeyser->setMinMaxDistance(0.0f, 40.0f);

            auto poisonBoltVolley = addAISpell(POISON_BOLT_VOLLEY, 15.0f, TARGET_VARIOUS, 0, 10, false, true);
            poisonBoltVolley->setAttackStopTimer(2000);

            auto cleave = addAISpell(CLEAVE, 6.0f, TARGET_ATTACKING, 0, 15, false, true);
            cleave->setAttackStopTimer(2000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);;
        }
};


// \note Coilfang Slavemaster was already scripted in SteamVaults, so I haven't
// copied/pasted it here.
// Still many NPCs left and I don't have infos if any of those use any spell
void SetupTheSlavePens(ScriptMgr* mgr)
{
    mgr->register_creature_script(CN_MENNUS_HEALING_WARD, &TotemsAI::Create);
    mgr->register_creature_script(CN_TAINED_EARTHGRAB_TOTEM, &TotemsAI::Create);
    mgr->register_creature_script(CN_TAINED_STONESKIN_TOTEM, &TotemsAI::Create);
    mgr->register_creature_script(CN_CORRUPTED_NOVA_TOTEM, &TotemsAI::Create);
    mgr->register_creature_script(CN_MENNU_THE_BETRAYER, &MennuTheBetrayerAI::Create);
    mgr->register_creature_script(CN_ROKMAR_THE_CRACKLER, &RokmarTheCracklerAI::Create);
    mgr->register_creature_script(CN_QUAGMIRRAN, &QuagmirranAI::Create);
}
