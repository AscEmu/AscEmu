/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_TheUnderbog.h"

#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Utilities/Random.hpp"

class TheUnderbogInstanceScript : public InstanceScript
{
public:
    explicit TheUnderbogInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr){}
    static InstanceScript* Create(WorldMap* pMapMgr) { return new TheUnderbogInstanceScript(pMapMgr); }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Boss AIs
class HungarfenAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HungarfenAI(c); }
    explicit HungarfenAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
    }

    void OnDied(Unit* /*mKiller*/) override
    {
    }

    void AIUpdate() override
    {
    }
};

class GhazanAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new GhazanAI(c); }
    explicit GhazanAI(Creature* pCreature) : CreatureAIScript(pCreature)
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
        if (getCreature()->getHealthPct() <= 20 && !Enraged && !getCreature()->isCastingSpell())
        {
            getCreature()->castSpell(getCreature(), enrage->mSpellInfo, enrage->mIsTriggered);

            Enraged = true;
        }
    }

protected:
    bool Enraged;
    CreatureAISpells* enrage;
};

class ClawAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ClawAI(c); }
    explicit ClawAI(Creature* pCreature) : CreatureAIScript(pCreature)
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
public:
    static CreatureAIScript* Create(Creature* c) { return new SwamplordMuselekAI(c); }
    explicit SwamplordMuselekAI(Creature* pCreature) : CreatureAIScript(pCreature)
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
            Bear->getAIInterface()->onHostileAction(mTarget);
    }

    void AIUpdate() override
    {
        if (getCreature()->getThreatManager().getCurrentVictim())
        {
            Unit* target = getCreature()->getThreatManager().getCurrentVictim();
            if (getCreature()->GetDistance2dSq(target) >= 100.0f && getCreature()->getDistanceSq(target) <= 900.0f && Util::getRandomUInt(3) != 1)
            {
                getCreature()->pauseMovement(2000);
                if (!getCreature()->isCastingSpell())
                {
                    uint32_t RangedSpell = Util::getRandomUInt(100);
                    if (RangedSpell <= 20 && _isTimerFinished(aimedShot->mCooldownTimer->isTimePassed()))
                    {
                        getCreature()->castSpell(target, aimedShot->mSpellInfo, true);
                        getCreature()->setAttackTimer(MELEE, aimedShot->getAttackStopTimer());
                        aimedShot->mCooldownTimer->resetInterval(aimedShot->mCooldown);
                    }

                    if (RangedSpell > 20 && RangedSpell <= 40 && _isTimerFinished(multiShot->mCooldownTimer->isTimePassed()))
                    {
                        getCreature()->castSpell(target, multiShot->mSpellInfo, true);
                        getCreature()->setAttackTimer(MELEE, multiShot->getAttackStopTimer());
                        multiShot->mCooldownTimer->resetInterval(multiShot->mCooldown);
                    }
                    else
                    {
                        if (_isTimerFinished(shot->mCooldownTimer->isTimePassed()))
                        {
                            getCreature()->castSpell(target, shot->mSpellInfo, true);
                            getCreature()->setAttackTimer(MELEE, shot->getAttackStopTimer());
                            shot->mCooldownTimer->resetInterval(shot->mCooldown);
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
public:
    static CreatureAIScript* Create(Creature* c) { return new TheBlackStalkerAI(c); }
    explicit TheBlackStalkerAI(Creature* pCreature) : CreatureAIScript(pCreature)
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
    mgr->register_instance_script(MAP_CF_THE_UNDERBOG, &TheUnderbogInstanceScript::Create);

    //Creatures
    mgr->register_creature_script(CN_HUNGARFEN, &HungarfenAI::Create);
    mgr->register_creature_script(CN_GHAZAN, &GhazanAI::Create);
    mgr->register_creature_script(CN_CLAW, &ClawAI::Create);
    mgr->register_creature_script(CN_SWAMPLORD_MUSELEK, &SwamplordMuselekAI::Create);
    mgr->register_creature_script(CN_THE_BLACK_STALKER, &TheBlackStalkerAI::Create);
}
