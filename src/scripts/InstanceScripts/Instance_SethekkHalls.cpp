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
#include "Instance_SethekkHalls.h"
#include "Objects/Faction.h"


class AvianDarkhawkAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(AvianDarkhawkAI);
        AvianDarkhawkAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto charge = addAISpell(SP_AVIAN_DARKHAWK_CHARGE, 5.0f, TARGET_ATTACKING, 0, 0, false, true);
            charge->setAttackStopTimer(1000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};

class AvianRipperAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(AvianRipperAI);
        AvianRipperAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto fleshRip = addAISpell(SP_AVIAN_RIPPER_FLESH_RIP, 15.0f, TARGET_ATTACKING, 0, 0, false, true);
            fleshRip->setAttackStopTimer(3000);
        }
        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};

class AvianWarhawkAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(AvianWarhawkAI);
        AvianWarhawkAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto cleave = addAISpell(SP_AVIAN_WARHAWK_CLEAVE, 12.0f, TARGET_VARIOUS, 0, 0, false, true);
            cleave->setAttackStopTimer(1000);

            auto charge = addAISpell(SP_AVIAN_WARHAWK_CHARGE, 5.0f, TARGET_ATTACKING, 0, 0, false, true);
            charge->setAttackStopTimer(1000);

            auto bite = addAISpell(SP_AVIAN_WARHAWK_BITE, 12.0f, TARGET_ATTACKING, 0, 0, false, true);
            bite->setAttackStopTimer(1000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};

class CobaltSerpentAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(CobaltSerpentAI);
        CobaltSerpentAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto wingBuffet = addAISpell(SP_COBALT_SERPENT_WING_BUFFET, 7.0f, TARGET_VARIOUS);
            wingBuffet->setAttackStopTimer(1000);

            auto frostbolt = addAISpell(SP_COBALT_SERPENT_FROSTBOLT, 15.0f, TARGET_ATTACKING);
            frostbolt->setAttackStopTimer(1000);

            auto chainLightning = addAISpell(SP_COBALT_SERPENT_CHAIN_LIGHTNING, 9.0f, TARGET_ATTACKING);
            chainLightning->setAttackStopTimer(1000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};

class TimeLostControllerAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(TimeLostControllerAI);
        TimeLostControllerAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto shrink = addAISpell(SP_TL_CONTROLLER_SHIRNK, 10.0f, TARGET_ATTACKING, 0, 0, false, true);
            shrink->setAttackStopTimer(1000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};

class TimeLostScryerAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(TimeLostScryerAI);
        TimeLostScryerAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto flashHeal = addAISpell(SP_TL_SCRYER_FLASH_HEAL, 5.0f, TARGET_SELF, 0, 0, false, true);
            flashHeal->setAttackStopTimer(1000);

            auto arcaneMissiles = addAISpell(SP_TL_SCRYER_ARCANE_MISSILES, 12.0f, TARGET_ATTACKING, 0, 0, false, true);
            arcaneMissiles->setAttackStopTimer(1000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};

class TimeLostShadowmageAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(TimeLostShadowmageAI);
        TimeLostShadowmageAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto curseOfDarkTalon = addAISpell(SP_TL_CURSE_OF_THE_DARK_TALON, 10.0f, TARGET_ATTACKING, 0, 0, false, true);
            curseOfDarkTalon->setAttackStopTimer(1000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};

class SethekkGuardAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(SethekkGuardAI);
        SethekkGuardAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto thunderclap = addAISpell(SP_SETHEKK_GUARD_THUNDERCLAP, 12.0f, TARGET_VARIOUS, 0, 0, false, true);
            thunderclap->setAttackStopTimer(1000);

            auto sunderArmor = addAISpell(SP_SETHEKK_GUARD_SUNDER_ARMOR, 8.0f, TARGET_ATTACKING, 0, 0, false, true);
            sunderArmor->setAttackStopTimer(1000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};

class SethekkInitiateAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(SethekkInitiateAI);
        SethekkInitiateAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto magicReflection = addAISpell(SP_SETHEKK_INIT_MAGIC_REFLECTION, 10.0f, TARGET_SELF, 0, 0, false, true);
            magicReflection->setAttackStopTimer(1000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};

class SethekkOracleAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(SethekkOracleAI);
        SethekkOracleAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto faeriFire = addAISpell(SP_SETHEKK_ORACLE_FAERIE_FIRE, 8.0f, TARGET_ATTACKING, 0, 0, false, true);
            faeriFire->setAttackStopTimer(1000);

            auto arcaneLighning = addAISpell(SP_SETHEKK_ORACLE_ARCANE_LIGHTNING, 15.0f, TARGET_ATTACKING);
            arcaneLighning->setAttackStopTimer(1000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};

class SethekkProphetAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(SethekkProphetAI);
        SethekkProphetAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto prophetFear = addAISpell(SP_SETHEKK_PROPHET_FEAR, 8.0f, TARGET_ATTACKING, 0, 0, false, true);
            prophetFear->setAttackStopTimer(1000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};

class SethekkRavenguardAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(SethekkRavenguardAI);
        SethekkRavenguardAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto bloodthirst = addAISpell(SP_SETHEKK_RAVENG_BLOODTHIRST, 10.0f, TARGET_ATTACKING, 0, 0, false, true);
            bloodthirst->setAttackStopTimer(1000);

            auto howlingScreech = addAISpell(SP_SETHEKK_RAVENG_HOWLING_SCREECH, 8.0f, TARGET_VARIOUS, 0, 0, false, true);
            howlingScreech->setAttackStopTimer(1000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};

class SethekkShamanAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(SethekkShamanAI);
        SethekkShamanAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto summonDarkVortex = addAISpell(SP_SETHEKK_SHAMAN_SUM_DARK_VORTEX, 8.0f, TARGET_SELF, 0, 0, false, true);
            summonDarkVortex->setAttackStopTimer(1000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};

class SethekkTalonLordAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(SethekkTalonLordAI);
        SethekkTalonLordAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto ofJustice = addAISpell(SP_SETHEKK_TALON_OF_JUSTICE, 10.0f, TARGET_ATTACKING, 0, 0, false, true);
            ofJustice->setAttackStopTimer(1000);

            auto avengersShield = addAISpell(SP_SETHEKK_TALON_AVENGERS_SHIELD, 7.0f, TARGET_ATTACKING, 0, 0, false, true);
            avengersShield->setAttackStopTimer(1000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};

////////////////////////////////////////////////////
// Lakka AI
static Movement::LocationWithFlag LakkaWaypoint[] =
{
    {},
    { -157.200f, 159.922f, 0.010f, 0.104f, Movement::WP_MOVE_TYPE_WALK },
    { -128.318f, 172.483f, 0.009f, 0.222f, Movement::WP_MOVE_TYPE_WALK },
    { -73.749f, 173.171f, 0.009f, 6.234f, Movement::WP_MOVE_TYPE_WALK },
};

class LakkaAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(LakkaAI);
        LakkaAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            SetWaypointMoveType(Movement::WP_MOVEMENT_SCRIPT_NONE);

            //WPs
            for (uint8 i = 1; i < 4; ++i)
            {
                AddWaypoint(CreateWaypoint(i, 0, LakkaWaypoint[i].wp_flag, LakkaWaypoint[i].wp_location));
            }
        }

        void OnReachWP(uint32 iWaypointId, bool /*bForwards*/) override
        {
            switch (iWaypointId)
            {
                case 1:
                {
                    SetWaypointMoveType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                    SetWaypointToMove(2);
                    Player* pPlayer = NULL;
                    QuestLogEntry* pQuest = NULL;
                    for (std::set<Object*>::iterator itr = getCreature()->GetInRangeSetBegin(); itr != getCreature()->GetInRangeSetEnd(); ++itr)
                    {
                        if ((*itr)->IsPlayer())
                        {
                            pPlayer = static_cast<Player*>((*itr));
                            if (pPlayer != NULL)
                            {
                                pQuest = pPlayer->GetQuestLogForEntry(10097);
                                if (pQuest != NULL && pQuest->GetMobCount(1) < 1)
                                {
                                    pQuest->SetMobCount(1, 1);
                                    pQuest->SendUpdateAddKill(1);
                                    pQuest->UpdatePlayerFields();
                                }
                            }
                        }
                    }
                }
                break;
                case 3:
                {
                    despawn(100, 0);
                }
                break;
                default:
                {
                    SetWaypointMoveType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                    SetWaypointToMove(1);
                }
            }
        }
};

////////////////////////////////////////////////////
// Boss AIs

class DarkweaverSythAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(DarkweaverSythAI);
        DarkweaverSythAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto frostShock = addAISpell(SP_DARKW_SYNTH_FROST_SHOCK, 8.0f, TARGET_ATTACKING, 0, 15, false, true);
            frostShock->setAttackStopTimer(2000);

            auto flameShock = addAISpell(SP_DARKW_SYNTH_FLAME_SHOCK, 8.0f, TARGET_ATTACKING, 0, 15, false, true);
            flameShock->setAttackStopTimer(2000);

            auto shadowShock = addAISpell(SP_DARKW_SYNTH_SHADOW_SHOCK, 8.0f, TARGET_ATTACKING, 0, 15, false, true);
            shadowShock->setAttackStopTimer(2000);

            auto arcaneShock = addAISpell(SP_DARKW_SYNTH_ARCANE_SHOCK, 8.0f, TARGET_ATTACKING, 0, 15, false, true);
            arcaneShock->setAttackStopTimer(2000);

            auto chainLighning = addAISpell(SP_DARKW_SYNTH_CHAIN_LIGHTNING, 10.0f, TARGET_ATTACKING, 0, 15, false, true);
            chainLighning->setAttackStopTimer(2000);

            summonFireEle = addAISpell(SP_DARKW_SYNTH_SUM_FIRE_ELEMENTAL, 0.0f, TARGET_SELF, 0, 10);
            summonFireEle->setAttackStopTimer(1000);

            summonFrostEle = addAISpell(SP_DARKW_SYNTH_SUM_FROST_ELEMENTAL, 0.0f, TARGET_SELF, 0, 10);
            summonFrostEle->setAttackStopTimer(1000);

            summonArcaneEle = addAISpell(SP_DARKW_SYNTH_SUM_ARCANE_ELEMENTAL, 0.0f, TARGET_SELF, 0, 10);
            summonArcaneEle->setAttackStopTimer(1000);

            summonShadowEle = addAISpell(SP_DARKW_SYNTH_SUM_SHADOW_ELEMENTAL, 0.0f, TARGET_SELF, 0, 10);
            summonShadowEle->setAttackStopTimer(1000);

            Summons = 0;

            addEmoteForEvent(Event_OnCombatStart, SAY_DARKW_SYNTH_02);
            addEmoteForEvent(Event_OnCombatStart, SAY_DARKW_SYNTH_03);
            addEmoteForEvent(Event_OnCombatStart, SAY_DARKW_SYNTH_04);
            addEmoteForEvent(Event_OnTargetDied, SAY_DARKW_SYNTH_05);
            addEmoteForEvent(Event_OnTargetDied, SAY_DARKW_SYNTH_05);
            addEmoteForEvent(Event_OnDied, SAY_DARKW_SYNTH_07);
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            Summons = 0;
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            Summons = 0;
        }

        void OnDied(Unit* /*mKiller*/) override
        {
            GameObject* LakkasCage = getNearestGameObject(-160.813f, 157.043f, 0.194095f, 183051);
            Creature* mLakka = getNearestCreature(-160.813f, 157.043f, 0.194095f, 18956);

            if (LakkasCage != NULL)
            {
                LakkasCage->SetState(GO_STATE_OPEN);
                LakkasCage->SetFlags(LakkasCage->GetFlags() - 1);
            }

            if (mLakka != NULL && mLakka->GetScript())
            {
                CreatureAIScript* pLakkaAI = static_cast< CreatureAIScript* >(mLakka->GetScript());
                mLakka->GetAIInterface()->setAiState(AI_STATE_SCRIPTMOVE);
                pLakkaAI->SetWaypointMoveType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                pLakkaAI->SetWaypointToMove(1);
                pLakkaAI->setAIAgent(AGENT_NULL);
            }
        }

        void AIUpdate() override
        {
            if (((getCreature()->GetHealthPct() <= 75 && getScriptPhase() == 1) || (getCreature()->GetHealthPct() <= 50 && getScriptPhase() == 2) || (getCreature()->GetHealthPct() <= 25 && getScriptPhase() == 3)))
            {
                getCreature()->setAttackTimer(1500, false);
                getCreature()->GetAIInterface()->StopMovement(1000);    // really?

                SummonElementalWave();

                setScriptPhase(getScriptPhase() + 1);
            }
        }

        void SummonElementalWave()
        {
            sendDBChatMessage(SAY_DARKW_SYNTH_01);

            getCreature()->CastSpell(getCreature(), summonFireEle->mSpellInfo, true);
            getCreature()->CastSpell(getCreature(), summonFrostEle->mSpellInfo, true);
            getCreature()->CastSpell(getCreature(), summonArcaneEle->mSpellInfo, true);
            getCreature()->CastSpell(getCreature(), summonShadowEle->mSpellInfo, true);
        }

    protected:

        uint32 Summons;
        CreatureAISpells* summonFireEle;
        CreatureAISpells* summonFrostEle;
        CreatureAISpells* summonArcaneEle;
        CreatureAISpells* summonShadowEle;
};

class TalonKingIkissAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(TalonKingIkissAI);
        TalonKingIkissAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            arcaneVolley = addAISpell(SP_TALRON_K_IKISS_ARCANE_VOLLEY, 12.0f, TARGET_VARIOUS, 0, 10);
            arcaneVolley->setAttackStopTimer(1000);

            auto blink = addAISpell(SP_TALRON_K_IKISS_BLINK, 7.0f, TARGET_SELF, 15, 25, false, true);
            blink->setAttackStopTimer(2500);

            auto polymorph = addAISpell(SP_TALRON_K_IKISS_POLYMORPH, 9.0f, TARGET_RANDOM_SINGLE, 0, 15, false, true);
            polymorph->setAttackStopTimer(1000);
            polymorph->setMinMaxDistance(0.0f, 40.0f);

            arcaneExplosion = addAISpell(SP_TALRON_K_IKISS_ARCANE_EXPLOSION, 0.0f, TARGET_VARIOUS);
            arcaneExplosion->setAttackStopTimer(1000);

            Blink = false;

            addEmoteForEvent(Event_OnCombatStart, SAY_TALRON_K_IKISS_02);
            addEmoteForEvent(Event_OnCombatStart, SAY_TALRON_K_IKISS_03);
            addEmoteForEvent(Event_OnCombatStart, SAY_TALRON_K_IKISS_04);
            addEmoteForEvent(Event_OnTargetDied, SAY_TALRON_K_IKISS_05);
            addEmoteForEvent(Event_OnTargetDied, SAY_TALRON_K_IKISS_06);
            addEmoteForEvent(Event_OnDied, SAY_TALRON_K_IKISS_07);
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            if (!getCreature()->isCastingNonMeleeSpell())
            {
                getCreature()->GetAIInterface()->StopMovement(1);
                getCreature()->setAttackTimer(3000, false);
                getCreature()->CastSpell(getCreature(), arcaneVolley->mSpellInfo, true);
            }

            Blink = false;
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }

        void OnDied(Unit* /*mKiller*/) override
        {
            GameObject* IkissDoor = getNearestGameObject(43.079f, 149.505f, 0.034f, 183398);
            if (IkissDoor != nullptr)
                IkissDoor->SetState(GO_STATE_OPEN);
        }

        void AIUpdate() override
        {
            if (Blink)
            {
                getCreature()->GetAIInterface()->StopMovement(2000);
                getCreature()->setAttackTimer(6500, false);

                getCreature()->interruptSpell();

                getCreature()->CastSpell(getCreature(), arcaneExplosion->mSpellInfo, true);

                Blink = false;
            }
        }

        void OnCastSpell(uint32 spellId) override
        {
            if (spellId == SP_TALRON_K_IKISS_BLINK)
                Blink = true;
        }

    protected:

        bool Blink;
        CreatureAISpells* arcaneVolley;
        CreatureAISpells* arcaneExplosion;
};

class ANZUAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(ANZUAI);
        ANZUAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto bomb = addAISpell(SP_ANZU_SPELL_BOMB, 10.0f, TARGET_ATTACKING, 0, 0, false, true);
            bomb->setAttackStopTimer(1000);

            auto cycloneOfFeathers = addAISpell(SP_ANZU_CYCLONE_OF_FEATHERS, 10.0f, TARGET_ATTACKING);
            cycloneOfFeathers->setAttackStopTimer(1000);

            auto screech = addAISpell(SP_ANZU_PARALYZING_SCREECH, 10.0f, TARGET_VARIOUS);
            screech->setAttackStopTimer(1000);

            banish = addAISpell(SP_ANZU_BANISH, 0.0f, TARGET_SELF, 0, 0, false, true);
            banish->setAttackStopTimer(1000);

            ravenGod = addAISpell(SP_ANZU_SUMMON_RAVEN_GOD, 0.0f, TARGET_SELF, 0, 60, true, true);
            ravenGod->setAttackStopTimer(1000);

            Banished = false;
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            getCreature()->CastSpell(getCreature(), ravenGod->mSpellInfo, true);

            Banished = false;
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            getCreature()->RemoveAura(SP_ANZU_BANISH);

            Banished = false;
        }

        void AIUpdate() override
        {
            if ((getCreature()->GetHealthPct() <= 66 && getScriptPhase() == 1) || (getCreature()->GetHealthPct() <= 33 && getScriptPhase() == 1))
            {
                SummonPhase();
                setScriptPhase(getScriptPhase() + 1);
            }

            if (Banished)
            {
                getCreature()->RemoveAura(SP_ANZU_BANISH);
                Banished = false;
            }
        }

        void SummonPhase()
        {
            getCreature()->CastSpell(getCreature(), banish->mSpellInfo, true);
        }

        void OnCastSpell(uint32 spellId) override
        {
            if (spellId == SP_ANZU_BANISH)
                Banished = true;
        }

    protected:

        bool Banished;

        CreatureAISpells* banish;
        CreatureAISpells* ravenGod;
};

void SetupSethekkHalls(ScriptMgr* mgr)
{
    //Creatures
    mgr->register_creature_script(CN_AVIAN_DARKHAWK, &AvianDarkhawkAI::Create);
    mgr->register_creature_script(CN_AVIAN_RIPPER, &AvianRipperAI::Create);
    mgr->register_creature_script(CN_AVIAN_WARHAWK, &AvianWarhawkAI::Create);
    mgr->register_creature_script(CN_COBALT_SERPENT, &CobaltSerpentAI::Create);
    mgr->register_creature_script(CN_TIME_LOST_CONTROLLER, &TimeLostControllerAI::Create);
    mgr->register_creature_script(CN_TIME_LOST_SCRYER, &TimeLostScryerAI::Create);
    mgr->register_creature_script(CN_TIME_LOST_SHADOWMAGE, &TimeLostShadowmageAI::Create);
    mgr->register_creature_script(CN_SETHEKK_GUARD, &SethekkGuardAI::Create);
    mgr->register_creature_script(CN_SETHEKK_INITIATE, &SethekkInitiateAI::Create);
    mgr->register_creature_script(CN_SETHEKK_ORACLE, &SethekkOracleAI::Create);
    mgr->register_creature_script(CN_SETHEKK_PROPHET, &SethekkProphetAI::Create);
    mgr->register_creature_script(CN_SETHEKK_RAVENGUARD, &SethekkRavenguardAI::Create);
    mgr->register_creature_script(CN_SETHEKK_SHAMAN, &SethekkShamanAI::Create);
    mgr->register_creature_script(CN_SETHEKK_TALON_LORD, &SethekkTalonLordAI::Create);
    mgr->register_creature_script(CN_DARKWEAVER_SYTH, &DarkweaverSythAI::Create);
    mgr->register_creature_script(CN_TALON_KING_IKISS, &TalonKingIkissAI::Create);
    mgr->register_creature_script(CN_LAKKA, &LakkaAI::Create);
    //mgr->register_creature_script(CN_ANZU, &AnzuAI::Create); /// \todo Can't check Anzu he is in the DB right now
}
