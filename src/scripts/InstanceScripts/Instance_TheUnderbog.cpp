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

// \todo move most defines to enum, text to db (use SendScriptTextChatMessage(ID))
#include "Setup.h"
#include "Instance_TheUnderbog.h"
#include "Objects/Faction.h"


///////////////////////////////////////////////////////////
// Boss AIs

class HungarfenAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(HungarfenAI);
        HungarfenAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            auto mushroom = addAISpell(UNDERBOG_MUSHROOM, 0.0f, TARGET_RANDOM_DESTINATION, 0, 15, false, true);
            mushroom->setAttackStopTimer(1000);

            spores = addAISpell(FOUL_SPORES, 0.0f, TARGET_VARIOUS);
            spores->setAttackStopTimer(1000);

            FourSpores = false;
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            FourSpores = false;
        }

        void OnDied(Unit* /*mKiller*/) override
        {
            FourSpores = false;
        }

        void AIUpdate() override
        {
            if (getCreature()->GetHealthPct() <= 20 && !FourSpores)
            {
                getCreature()->GetAIInterface()->StopMovement(11000);
                getCreature()->setAttackTimer(1200, false);

                getCreature()->CastSpell(getCreature(), spores->mSpellInfo, spores->mIsTriggered);

                FourSpores = true;
            }
            else if (!getCreature()->getAuraWithId(FOUL_SPORES))
            {
                // Not yet added
                //CastSpellOnRandomTarget(0, 0.0f, 40.0f, 0, 100);
            }
        }

    protected:

        bool FourSpores;
        CreatureAISpells* spores;
};

class GhazanAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(GhazanAI);
        GhazanAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            auto acidSpit = addAISpell(ACID_SPIT, 8.0f, TARGET_VARIOUS, 0, 20, false, true);
            acidSpit->setAttackStopTimer(1000);

            auto tailSweep = addAISpell(TAIL_SWEEP, 7.0f, TARGET_VARIOUS, 0, 25, false, true);
            tailSweep->setAttackStopTimer(1000);

            auto acidBreath = addAISpell(ACID_BREATH, 10.0f, TARGET_VARIOUS, 0, 15, false, true);
            acidBreath->setAttackStopTimer(1000);

            enrage = addAISpell(ENRAGE, 0.0f, TARGET_SELF, 0, 160, false, true);
            enrage->setAttackStopTimer(1000);

            Enraged = false;
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            Enraged = false;
        }

        void AIUpdate() override
        {
            if (getCreature()->GetHealthPct() <= 20 && !Enraged && !getCreature()->isCastingNonMeleeSpell())
            {
                getCreature()->CastSpell(getCreature(), enrage->mSpellInfo, enrage->mIsTriggered);

                Enraged = true;
            }
        }

    protected:

        bool Enraged;
        CreatureAISpells* enrage;
};

class ClawAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(ClawAI);
        ClawAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            auto maul = addAISpell(MAUL, 15.0f, TARGET_ATTACKING, 0, 15, false, true);
            maul->setAttackStopTimer(1000);

            auto echoingRoar = addAISpell(CL_ECHOING_ROAR, 8.0f, TARGET_VARIOUS, 0, 30, false, true);
            echoingRoar->setAttackStopTimer(1000);

            auto feralCharge = addAISpell(FERAL_CHARGE, 18.0f, TARGET_RANDOM_SINGLE, 0, 3, false, true);
            feralCharge->setAttackStopTimer(1000);
            feralCharge->setMinMaxDistance(0.0f, 40.0f);

            auto enrage = addAISpell(CL_ENRAGE, 15.0f, TARGET_SELF, 0, 240, false, true);
            enrage->setAttackStopTimer(1000);
        }
};

class SwamplordMuselekAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(SwamplordMuselekAI);
        SwamplordMuselekAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            auto freezingTrap = addAISpell(THROW_FREEZING_TRAP, 8.0f, TARGET_RANDOM_SINGLE, 0, 30, false, true);
            freezingTrap->setAttackStopTimer(1000);
            freezingTrap->setMinMaxDistance(0.0f, 40.0f);

            auto knockAway = addAISpell(KNOCK_AWAY_MUSELEK, 12.0f, TARGET_ATTACKING, 0, 20, false, true);
            knockAway->setAttackStopTimer(1000);

            aimedShot = addAISpell(AIMED_SHOT, 35.0f, TARGET_RANDOM_SINGLE, 0, 20);
            aimedShot->setAttackStopTimer(6500);

            multiShot = addAISpell(MULTI_SHOT, 35.0f, TARGET_RANDOM_SINGLE, 0, 15, false, true);
            multiShot->setAttackStopTimer(1000);

            shot = addAISpell(SHOT, 35.0f, TARGET_RANDOM_SINGLE, 0, 20, false, true);
            shot->setAttackStopTimer(1000);

            addEmoteForEvent(Event_OnCombatStart, SAY_SWAMPLORD_MUSEL_02);
            addEmoteForEvent(Event_OnCombatStart, SAY_SWAMPLORD_MUSEL_03);
            addEmoteForEvent(Event_OnCombatStart, SAY_SWAMPLORD_MUSEL_04);
            addEmoteForEvent(Event_OnTargetDied, SAY_SWAMPLORD_MUSEL_05);
            addEmoteForEvent(Event_OnTargetDied, SAY_SWAMPLORD_MUSEL_06);
            addEmoteForEvent(Event_OnDied, SAY_SWAMPLORD_MUSEL_07);
        }

        void OnCombatStart(Unit* mTarget) override
        {
            Unit* Bear = getNearestCreature(getCreature()->GetPositionX(), getCreature()->GetPositionY(), getCreature()->GetPositionZ(), 17827);
            if (Bear && Bear->isAlive())
                Bear->GetAIInterface()->AttackReaction(mTarget, 1, 0);
        }

        void AIUpdate() override
        {
            if (getCreature()->GetAIInterface()->getNextTarget())
            {
                Unit* target = getCreature()->GetAIInterface()->getNextTarget();
                if (getCreature()->GetDistance2dSq(target) >= 100.0f && getCreature()->getDistanceSq(target) <= 900.0f && RandomUInt(3) != 1)
                {
                    getCreature()->GetAIInterface()->StopMovement(2000);
                    if (!getCreature()->isCastingNonMeleeSpell())
                    {
                        uint32 RangedSpell = RandomUInt(100);
                        if (RangedSpell <= 20 && _isTimerFinished(aimedShot->mCooldownTimerId))
                        {
                            getCreature()->CastSpell(target, aimedShot->mSpellInfo, true);
                            getCreature()->setAttackTimer(aimedShot->getAttackStopTimer(), false);
                            _resetTimer(aimedShot->mCooldownTimerId, aimedShot->mCooldown);
                        }

                        if (RangedSpell > 20 && RangedSpell <= 40 && _isTimerFinished(multiShot->mCooldownTimerId))
                        {
                            getCreature()->CastSpell(target, multiShot->mSpellInfo, true);
                            getCreature()->setAttackTimer(multiShot->getAttackStopTimer(), false);
                            _resetTimer(multiShot->mCooldownTimerId, multiShot->mCooldown);
                        }
                        else
                        {
                            if (_isTimerFinished(shot->mCooldownTimerId))
                            {
                                getCreature()->CastSpell(target, shot->mSpellInfo, true);
                                getCreature()->setAttackTimer(shot->getAttackStopTimer(), false);
                                _resetTimer(shot->mCooldownTimerId, shot->mCooldown);
                            }
                        }
                    }
                }
            }
        }

    private:
        CreatureAISpells* aimedShot;
        CreatureAISpells* multiShot;
        CreatureAISpells* shot;
};


class TheBlackStalkerAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(TheBlackStalkerAI);
        TheBlackStalkerAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            auto chainLighning = addAISpell(CHAIN_LIGHTNING, 12.0f, TARGET_RANDOM_SINGLE, 0, 15);
            chainLighning->setAttackStopTimer(1000);
            chainLighning->setMinMaxDistance(0.0f, 40.0f);

            auto levitate = addAISpell(LEVITATE, 8.0f, TARGET_RANDOM_SINGLE, 0, 30, false, true);
            levitate->setAttackStopTimer(1000);
            levitate->setMinMaxDistance(0.0f, 40.0f);

            auto staticCharge = addAISpell(STATIC_CHARGE, 8.0f, TARGET_RANDOM_SINGLE, 0, 25, false, true);
            staticCharge->setAttackStopTimer(1000);
            staticCharge->setMinMaxDistance(0.0f, 40.0f);

            auto summonSporeStrider = addAISpell(SUMMON_SPORE_STRIDER, 0.0f, TARGET_SELF, 0, 10, false, true);
            summonSporeStrider->setAttackStopTimer(1000);
        }
};

// \note Wasp/Stinger must be checked. Please check it (because for sure
// many spells/creatures with spells are missing and also you will find some dupes.
// No spells found for: Windcaller Claw, Spore Spider, Earthbinder Rayge
// Left Underbog Mushroom.
void SetupTheUnderbog(ScriptMgr* mgr)
{
    //Creatures
    mgr->register_creature_script(CN_HUNGARFEN, &HungarfenAI::Create);
    mgr->register_creature_script(CN_GHAZAN, &GhazanAI::Create);
    mgr->register_creature_script(CN_CLAW, &ClawAI::Create);
    mgr->register_creature_script(CN_SWAMPLORD_MUSELEK, &SwamplordMuselekAI::Create);
    mgr->register_creature_script(CN_THE_BLACK_STALKER, &TheBlackStalkerAI::Create);
}
