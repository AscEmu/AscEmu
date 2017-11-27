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
#include "Instance_ShadowLabyrinth.h"
#include "Objects/Faction.h"


class CabalAcolyteAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(CabalAcolyteAI);
        CabalAcolyteAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto shadowProtection = addAISpell(SP_CABAL_ACOLYTE_SHADOW_PROTECTION, 6.0f, TARGET_SELF, 0, 0, false, true);
            shadowProtection->setAttackStopTimer(1000);

            auto heal = addAISpell(SP_CABAL_ACOLYTE_HEAL, 6.0f, TARGET_SELF, 0, 0, false, true);
            heal->setAttackStopTimer(1000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};

class CabalDeathswornAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(CabalDeathswornAI);
        CabalDeathswornAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto shadowCleave = addAISpell(SP_CABAL_DEATHSWORN_SHADOW_CLEAVE, 9.0f, TARGET_VARIOUS, 0, 0, false, true);
            shadowCleave->setAttackStopTimer(1000);

            auto knockback = addAISpell(SP_CABAL_DEATHSWORN_KNOCKBACK, 6.0f, TARGET_VARIOUS, 0, 0, false, true);
            knockback->setAttackStopTimer(1000);

            auto blackCleave = addAISpell(SP_CABAL_DEATHSWORN_BLACK_CLEAVE, 9.0f, TARGET_ATTACKING, 0, 0, false, true);
            blackCleave->setAttackStopTimer(1000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};

class CabalFanaticAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(CabalFanaticAI);
        CabalFanaticAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto fanaticFixate = addAISpell(SP_CABAL_FANATIC_FIXATE, 10.0f, TARGET_ATTACKING, 0, 0, false, true);
            fanaticFixate->setAttackStopTimer(1000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};

// CabalShadowPriestAI
class CabalShadowPriestAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(CabalShadowPriestAI);
        CabalShadowPriestAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto mindFlay = addAISpell(SP_CABAL_SHADOW_PRIEST_MIND_FLAY, 7.0f, TARGET_ATTACKING);
            mindFlay->setAttackStopTimer(1000);

            auto worPain = addAISpell(SP_CABAL_SHADOW_PRIEST_WORD_PAIN, 10.0f, TARGET_ATTACKING, 0, 0, false, true);
            worPain->setAttackStopTimer(1000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};

class CabalSpellbinderAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(CabalSpellbinderAI);
        CabalSpellbinderAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto mindControl = addAISpell(SP_CABAL_SPELLBINDER_MIND_CONTROL, 7.0f, TARGET_ATTACKING, 0, 0, false, true);
            mindControl->setAttackStopTimer(1000);

            auto earthShock = addAISpell(SP_CABAL_SPELLBINDER_EARTH_SHOCK, 10.0f, TARGET_ATTACKING, 0, 0, false, true);
            earthShock->setAttackStopTimer(1000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};

class CabalWarlockAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(CabalWarlockAI);
        CabalWarlockAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto shadowBolt = addAISpell(SP_CABAL_WARLOCK_SHADOW_BOLT, 13.0f, TARGET_ATTACKING);
            shadowBolt->setAttackStopTimer(1000);

            auto seedOfCorruption = addAISpell(SP_CABAL_WARLOCK_SEED_OF_CORRUPTION, 8.0f, TARGET_ATTACKING);
            seedOfCorruption->setAttackStopTimer(1000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};

class CabalZealotAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(CabalZealotAI);
        CabalZealotAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto shadowBolt = addAISpell(SP_CABAL_ZEALOT_SHADOW_BOLT, 13.0f, TARGET_ATTACKING);
            shadowBolt->setAttackStopTimer(1000);

            //never casted
            auto transformation = addAISpell(SP_CABAL_ZEALOT_TRANSFROMATION, 0.0f, TARGET_SELF, 0, 0, false, true);
            transformation->setAttackStopTimer(1000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};

/*
   #  Cabal Ritualist

    * with daggers - Gouges, hits about 1500 on cloth, low health.
    * with staff or single blade - 3 different types of casters

    1. Arcane - Arcane Missles, Addle Humanoid
    2. Fire - Fire Blast, Flame Buffet (small amount of damage and DoT)
    3. Frost - Frostbolt

    * Heroic: can dispel CC, such as Trap and Polymorph. Tank and kill away from other CC'd Ritualists.
    * Heroic: immune to MC

    */
class CabalRitualistAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(CabalRitualistAI);
        CabalRitualistAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto gouge = addAISpell(SP_CABAL_RITUALIST_GOUGE, 8.0f, TARGET_ATTACKING, 0, 0, false, true);
            gouge->setAttackStopTimer(1000);

            auto arcaneMissiles = addAISpell(SP_CABAL_RITUALIST_ARCANE_MISSILES, 8.0f, TARGET_ATTACKING, 0, 0, false, true);
            arcaneMissiles->setAttackStopTimer(1000);

            auto addleHumanoid = addAISpell(SP_CABAL_RITUALIST_ADDLE_HUMANOID, 7.0f, TARGET_ATTACKING);
            addleHumanoid->setAttackStopTimer(1000);

            auto fireBlast = addAISpell(SP_CABAL_RITUALIST_FIRE_BLAST, 9.0f, TARGET_ATTACKING, 0, 0, false, true);
            fireBlast->setAttackStopTimer(1000);

            auto flameBuffet = addAISpell(SP_CABAL_RITUALIST_FLAME_BUFFET, 9.0f, TARGET_VARIOUS);
            flameBuffet->setAttackStopTimer(1000);

            auto frostbolt = addAISpell(SP_CABAL_RITUALIST_FROSTBOLT, 9.0f, TARGET_VARIOUS);
            frostbolt->setAttackStopTimer(1000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};

class FelOverseerAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(FelOverseerAI);
        FelOverseerAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto shout = addAISpell(SP_FEL_OVERSEER_INTIMIDATING_SHOUT, 4.0f, TARGET_ATTACKING, 0, 0, false, true);
            shout->setAttackStopTimer(1000);

            auto charge = addAISpell(SP_FEL_OVERSEER_CHARGE_OVERSEER, 5.0f, TARGET_ATTACKING, 0, 0, false, true);
            charge->setAttackStopTimer(1000);

            heal = addAISpell(SP_FEL_OVERSEER_HEAL_OVERSEER, 0.0f, TARGET_SELF, 0, 0, false, true);
            heal->setAttackStopTimer(1000);

            auto mortalStrike = addAISpell(SP_FEL_OVERSEER_MORTAL_STRIKE, 12.0f, TARGET_ATTACKING, 0, 0, false, true);
            mortalStrike->setAttackStopTimer(1000);

            auto uppercut = addAISpell(SP_FEL_OVERSEER_UPPERCUT, 5.0f, TARGET_ATTACKING, 0, 0, false, true);
            uppercut->setAttackStopTimer(1000);

            HealCooldown = 0;
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            HealCooldown = 1;
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            HealCooldown = 1;
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }

        void OnDied(Unit* /*mKiller*/) override
        {
            HealCooldown = 1;
        }

        void AIUpdate() override
        {
            HealCooldown--;
            if (getCreature()->GetHealthPct() <= 25 && HealCooldown <= 0)
            {
                getCreature()->CastSpell(getCreature(), heal->mSpellInfo, true);
                HealCooldown = 60;
            }
        }

    protected:

        int HealCooldown;
        CreatureAISpells* heal;
};

class MaliciousInstructorAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(MaliciousInstructorAI);
        MaliciousInstructorAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto shadowNova = addAISpell(SP_MILICIOUS_INSTRUCT_SHADOW_NOVA, 12.0f, TARGET_VARIOUS, 0, 0, false, true);
            shadowNova->setAttackStopTimer(1000);

            auto markOfMalice = addAISpell(SP_MILICIOUS_INSTRUCT_MARK_OF_MALICE, 9.0f, TARGET_ATTACKING, 0, 0, false, true);
            markOfMalice->setAttackStopTimer(1000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};


// Boss AIs

class AmbassadorHellmawAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(AmbassadorHellmawAI);
        AmbassadorHellmawAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto corrosiveAcide = addAISpell(SP_AMBASSADOR_HELMAW_CORROSIVE_ACID, 10.0f, TARGET_VARIOUS, 0, 15);
            corrosiveAcide->setAttackStopTimer(1000);

            aoeFear = addAISpell(SP_AMBASSADOR_HELMAW_AOE_FEAR, 0.0f, TARGET_VARIOUS, 0, 25, false, true);
            aoeFear->setAttackStopTimer(1000);
            aoeFearTimerId = 0;

            addEmoteForEvent(Event_OnCombatStart, SAY_AMBASSADOR_HELMAW_02);
            addEmoteForEvent(Event_OnCombatStart, SAY_AMBASSADOR_HELMAW_03);
            addEmoteForEvent(Event_OnCombatStart, SAY_AMBASSADOR_HELMAW_04);
            addEmoteForEvent(Event_OnTargetDied, SAY_AMBASSADOR_HELMAW_06);
            addEmoteForEvent(Event_OnTargetDied, SAY_AMBASSADOR_HELMAW_07);
            addEmoteForEvent(Event_OnDied, SAY_AMBASSADOR_HELMAW_08);
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            aoeFearTimerId = _addTimer(25000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            _removeTimer(aoeFearTimerId);
        }

        void AIUpdate() override
        {
            if (_isTimerFinished(aoeFearTimerId) && !getCreature()->isCastingNonMeleeSpell())
            {
                getCreature()->CastSpell(getCreature(), aoeFear->mSpellInfo, true);

                _resetTimer(aoeFearTimerId, 25000);
            }
        }

    protected:

        CreatureAISpells* aoeFear;
        uint32_t aoeFearTimerId;
};


class BlackheartTheInciterAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(BlackheartTheInciterAI);
        BlackheartTheInciterAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto charge = addAISpell(SP_BLACKHEART_INCITER_CHARGE, 10.0f, TARGET_RANDOM_SINGLE, 0, 15, false, true);
            charge->setAttackStopTimer(1000);
            charge->setMinMaxDistance(0.0f, 40.0f);

            auto warStomp = addAISpell(SP_BLACKHEART_INCITER_WAR_STOMP, 10.0f, TARGET_VARIOUS, 0, 20, false, true);
            warStomp->setAttackStopTimer(1000);

            chaos = addAISpell(SP_BLACKHEART_INCITER_CHAOS, 0.0f, TARGET_RANDOM_SINGLE, 0, 40, false, true);
            chaos->setAttackStopTimer(1000);
            chaos->setMinMaxDistance(0.0f, 100.0f);
            chaosTimerId = 0;

            addEmoteForEvent(Event_OnCombatStart, SAY_BLACKHEART_INCITER_04);
            addEmoteForEvent(Event_OnCombatStart, SAY_BLACKHEART_INCITER_05);
            addEmoteForEvent(Event_OnCombatStart, SAY_BLACKHEART_INCITER_06);
            addEmoteForEvent(Event_OnTargetDied, SAY_BLACKHEART_INCITER_07);
            addEmoteForEvent(Event_OnTargetDied, SAY_BLACKHEART_INCITER_08);
            addEmoteForEvent(Event_OnDied, SAY_BLACKHEART_INCITER_10);
        }

        // sound corrections needed!
        void OnCombatStart(Unit* /*mTarget*/) override
        {
            chaosTimerId = _addTimer(20000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            _removeTimer(chaosTimerId);
        }

        void AIUpdate() override
        {
            if (_isTimerFinished(chaosTimerId) && !getCreature()->isCastingNonMeleeSpell())
            {
                getCreature()->CastSpell(getCreature(), chaos->mSpellInfo, true);
                _resetTimer(chaosTimerId, chaos->mCooldown);
            }
        }

    protected:

        uint32_t chaosTimerId;
        CreatureAISpells* chaos;
};

/*The fight starts as soon as one of the players moves close enough
to Vorpil to aggro him. Vorpil will immediately open the Void Rifts
around him, and Voidwalkers will start spawning, at an increasingly
faster rate as the battle progresses.*/
class GrandmasterVorpilAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(GrandmasterVorpilAI);
        GrandmasterVorpilAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto shadowBoltVolley = addAISpell(SP_GRDMASTER_VORPIL_SHADOW_BOLT_VOLLEY, 15.0f, TARGET_VARIOUS, 0, 10, false, true);
            shadowBoltVolley->setAttackStopTimer(1000);

            auto drawShadows = addAISpell(SP_GRDMASTER_VORPIL_DRAW_SHADOWS, 8.0f, TARGET_VARIOUS, 0, 25, false, true);
            drawShadows->setAttackStopTimer(1000);

            //addAISpell(SP_GRDMASTER_VORPIL_RAIN_OF_FIRE, 0.0f, TARGET_DESTINATION, 0, 25, false, true);
            //addAISpell(SP_GRDMASTER_VORPIL_VOID_PORTAL_VISUAL, 0.0f, TARGET_SELF, 0, 25, false, true);

            addEmoteForEvent(Event_OnCombatStart, SAY_GRD_VORPIL_02);
            addEmoteForEvent(Event_OnCombatStart, SAY_GRD_VORPIL_03);
            addEmoteForEvent(Event_OnCombatStart, SAY_GRD_VORPIL_04);
            addEmoteForEvent(Event_OnTargetDied, SAY_GRD_VORPIL_06);
            addEmoteForEvent(Event_OnTargetDied, SAY_GRD_VORPIL_07);
            addEmoteForEvent(Event_OnDied, SAY_GRD_VORPIL_08);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        }
};

class MurmurAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(MurmurAI);
        MurmurAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            enableCreatureAISpellSystem = true;

            auto boom1 = addAISpell(SP_MURMUR_SONIC_BOOM1, 10.0f, TARGET_SELF, 0, 25);
            boom1->setAttackStopTimer(1000);
            boom1->setAnnouncement("Murmur draws energy from the air...");

            auto shockwave = addAISpell(SP_MURMUR_SHOCKWAVE, 6.0f, TARGET_VARIOUS, 0, 15, false, true);
            shockwave->setAttackStopTimer(1000);

            auto touch = addAISpell(SP_MURMUR_MURMURS_TOUCH, 10.0f, TARGET_VARIOUS, 0, 20);
            touch->setAttackStopTimer(2000);

            resonance = addAISpell(SP_MURMUR_RESONANCE, 10.0f, TARGET_VARIOUS, 0, 5, false, true);
            resonance->setAttackStopTimer(1000);
            resonanceTimerId = 0;

            if (getCreature()->GetMapMgr() != NULL && !_isHeroic() && getCreature()->GetHealthPct() >= 41)
            {
                getCreature()->SetHealthPct(40);
            }

            SonicBoomTimerId = 0;
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            SonicBoomTimerId = _addTimer(5000);

            if (getCreature()->GetMapMgr() != NULL && !_isHeroic() && getCreature()->GetHealthPct() >= 41)
            {
                getCreature()->SetHealthPct(40);
            }
            getCreature()->setMoveRoot(true);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);

            if (getCreature()->GetMapMgr() != NULL && !_isHeroic() && getCreature()->GetHealthPct() >= 41)
            {
                getCreature()->SetHealthPct(40);
            }

            _removeTimer(SonicBoomTimerId);
        }

        void AIUpdate() override
        {
            if (_isTimerFinished(SonicBoomTimerId))
            {
                RemoveAIUpdateEvent();
                RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));

                getCreature()->interruptSpell();

                getCreature()->CastSpell(getCreature(), SP_MURMUR_SONIC_BOOM2, true);

                _resetTimer(SonicBoomTimerId, 25000);
            }
        }

    protected:

        uint32_t SonicBoomTimerId;
        uint32_t resonanceTimerId;
        CreatureAISpells* resonance;
};

void SetupShadowLabyrinth(ScriptMgr* mgr)
{
    //Creatures
    mgr->register_creature_script(CN_CABAL_ACOLYTE, &CabalAcolyteAI::Create);
    mgr->register_creature_script(CN_CABAL_DEATHSWORN, &CabalDeathswornAI::Create);
    mgr->register_creature_script(CN_CABAL_FANATIC, &CabalFanaticAI::Create);
    mgr->register_creature_script(CN_CABAL_SHADOW_PRIEST, &CabalShadowPriestAI::Create);
    mgr->register_creature_script(CN_CABAL_SPELLBINDER, &CabalSpellbinderAI::Create);
    mgr->register_creature_script(CN_CABAL_WARLOCK, &CabalWarlockAI::Create);
    mgr->register_creature_script(CN_CABAL_ZEALOT, &CabalZealotAI::Create);
    mgr->register_creature_script(CN_CABAL_RITUALIST, &CabalRitualistAI::Create);
    mgr->register_creature_script(CN_FEL_OVERSEER, &FelOverseerAI::Create);
    mgr->register_creature_script(CN_MALICIOUS_INSTRUCTOR, &MaliciousInstructorAI::Create);
    mgr->register_creature_script(CN_AMBASSADOR_HELLMAW, &AmbassadorHellmawAI::Create);
    mgr->register_creature_script(CN_BLACKHEART_THE_INCITER, &BlackheartTheInciterAI::Create);
    mgr->register_creature_script(CN_GRANDMASTER_VORPIL, &GrandmasterVorpilAI::Create);
    mgr->register_creature_script(CN_MURMUR, &MurmurAI::Create);
}
