/*
 * ArcScripts for ArcEmu MMORPG Server
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
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
#include "Instance_HallsOfStone.h"


class DarkRuneStormcallerAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(DarkRuneStormcallerAI);
    DarkRuneStormcallerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(STORMCALLER_LIGHTNINGBOLT, 60.0f, TARGET_RANDOM_SINGLE, 3, 6);
        addAISpell(STORMCALLER_SHADOWWORD, 16.0f, TARGET_RANDOM_SINGLE, 0, 12);
    }
};

class IronGolemCustodianAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(IronGolemCustodianAI);
    IronGolemCustodianAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(CUSTODIAN_CRUSH_ARMOR, 50.0f, TARGET_ATTACKING, 0, 5);
        addAISpell(CUSTODIAN_GROUND_SMASH, 20.0f, TARGET_ATTACKING, 0, 14);
    }
};

class DarkRuneProtectorAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(DarkRuneProtectorAI);
    DarkRuneProtectorAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(PROTECTOR_CHARGE, 20.0f, TARGET_RANDOM_SINGLE, 0, 14);
        addAISpell(PROTECTOR_CLAVE, 35.0f, TARGET_ATTACKING, 0, 8);
    }
};

class LesserAirElementalAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(LesserAirElementalAI);
    LesserAirElementalAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(ELEMENTAL_LIGHTNING_BOLT, 20.0f, TARGET_RANDOM_SINGLE, 3, 14);
    }
};

class DarkRuneWorkerAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(DarkRuneWorkerAI);
    DarkRuneWorkerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(WORKER_ENRAGE, 5.0f, TARGET_SELF, 0, 60);
        addAISpell(WORKER_PIERCE_ARMOR, 35.0f, TARGET_ATTACKING, 0, 45);
    }
};

class DarkRuneWarriorAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(DarkRuneWarriorAI);
    DarkRuneWarriorAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(WARRIOR_CLAVE, 15.0f, TARGET_ATTACKING, 0, 8);
        addAISpell(WARRIOR_HEROIC_STRIKE, 35.0f, TARGET_ATTACKING, 0, 12);
    }
};

class DarkRuneTheurgistAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(DarkRuneTheurgistAI);
    DarkRuneTheurgistAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(THEURGIST_BLAST_WAVE, 22.0f, TARGET_SELF, 0, 25);
        addAISpell(THEURGIST_FIREBOLT, 40.0f, TARGET_RANDOM_SINGLE, 3, 6);
        addAISpell(THEURGIST_IRON_MIGHT, 5.0f, TARGET_SELF, 0, 60);
    }
};

class DarkRuneShaperAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(DarkRuneShaperAI);
    DarkRuneShaperAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(SHAPER_RAY, 35.0f, TARGET_RANDOM_SINGLE, 2, 12);
    }
};

class DarkRuneScholarAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(DarkRuneScholarAI);
    DarkRuneScholarAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(SCHOLAR_SILANCE, 35.0f, TARGET_RANDOM_SINGLE, 3, 12);
    }
};

class DarkRuneGiantAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(DarkRuneGiantAI);
    DarkRuneGiantAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(GIANT_FIST, 3.0f, TARGET_SELF, 2, 40);
        auto stomp = addAISpell(GIANT_STOMP, 35.0f, TARGET_RANDOM_SINGLE, 0, 14);
        stomp->setMinMaxDistance(0.0f, 10.0f);
    }
};

class DarkRuneConstructAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(DarkRuneConstructAI);
    DarkRuneConstructAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(RAGING_POTENT_JOLT, 95.0f, TARGET_SELF, 0, 8);
        auto cleave = addAISpell(RAGING_CLAVE, 30.0f, TARGET_ATTACKING, 0, 9);
        cleave->setMinMaxDistance(0.0f, 10.0f);
    }
};

class DarkLightningConstructAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(DarkLightningConstructAI);
    DarkLightningConstructAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(LIGHTN_ELECTRICAL_OVERLOAD, 5.0f, TARGET_SELF, 2, 14);
        auto chainLightning = addAISpell(LIGHTN_CHAIN_LIGHTNING, 30.0f, TARGET_ATTACKING, 3, 8);
        chainLightning->setMinMaxDistance(0.0f, 30.0f);
    }
};

class ForgedIronTroggAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(ForgedIronTroggAI);
    ForgedIronTroggAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto chainLightning = addAISpell(LIGHTN_CHAIN_LIGHTNING, 30.0f, TARGET_ATTACKING, 3, 8);
        chainLightning->setMinMaxDistance(0.0f, 30.0f);
    }
};

class MaidenOfGriefAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(MaidenOfGriefAI);
    MaidenOfGriefAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(MAIDEN_STORM_OF_GRIEF, 100.0f, TARGET_RANDOM_DESTINATION, 0, 19);
        addAISpell(MAIDEN_PILLAR_OF_WOE, 30.0f, TARGET_RANDOM_SINGLE, 0, 8);

        mShock = addAISpell(MAIDEN_SHOCK_OF_SORROW, 20.0f, TARGET_SELF, 0, 18);
        mShock->addEmote("So much lost time... that you'll never get back!", CHAT_MSG_MONSTER_YELL, 13492);
        mShock->setAttackStopTimer(0);

        addEmoteForEvent(Event_OnCombatStart, 4367);     // You shouldn't have come... now you will die!
        addEmoteForEvent(Event_OnTargetDied, 4368);     // Why must it be this way?
        addEmoteForEvent(Event_OnTargetDied, 4369);     // You had it coming!
        addEmoteForEvent(Event_OnTargetDied, 4370);     // My burden grows heavier...
        addEmoteForEvent(Event_OnTargetDied, 4371);     // This is your fault!
        addEmoteForEvent(Event_OnDied, 4372);     // I hope you all rot! I never... wanted... this.
    }

    protected:

        CreatureAISpells* mShock;
};

// Krystallus
const uint32 STOMP_TIMER = 35000;
const uint32 SHATTER_TIMER = 4500;

class KrystallusAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(KrystallusAI);
    KrystallusAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(KRYSTALLUS_BOULDER_TOSS, 35.0f, TARGET_ATTACKING, 1, 4);
        mShatter = addAISpell(KRYSTALLUS_SHATTER, 0.0f, TARGET_SELF, 0, 0);
        mStomp = addAISpell(KRYSTALLUS_STOMP, 0.0f, TARGET_SELF, 0, 0);
        mShatter->addEmote("Break.... you....", CHAT_MSG_MONSTER_YELL, 14178);

        mStompTimerId = 0;
        mShatterTimerId = 0;

        addEmoteForEvent(Event_OnCombatStart, 4363);    // Crush....
        addEmoteForEvent(Event_OnTargetDied, 4365);     // Uuuuhhhhhhhhhh......
        addEmoteForEvent(Event_OnDied, 4364);
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        mStompTimerId = _addTimer(STOMP_TIMER);
    }

    void AIUpdate() override
    {
        if (_isTimerFinished(mStompTimerId))
        {
            _castAISpell(mStomp);
            setRooted(true);
            _resetTimer(mStompTimerId, (STOMP_TIMER + SHATTER_TIMER));
            mShatterTimerId = _addTimer(SHATTER_TIMER);

        }
        if (_isTimerFinished(mShatterTimerId))
        {
            _castAISpell(mShatter);
            setRooted(false);
            _removeTimer(mShatterTimerId);
        }
    }

    protected:

        CreatureAISpells* mStomp;
        CreatureAISpells* mShatter;

        uint32_t mStompTimerId;
        uint32_t mShatterTimerId;
};


void SetupHallsOfStone(ScriptMgr* mgr)
{
    //Trash
    mgr->register_creature_script(CN_DR_STORMCALLER, &DarkRuneStormcallerAI::Create);
    mgr->register_creature_script(CN_GOLEM_CUSTODIAN, &IronGolemCustodianAI::Create);
    mgr->register_creature_script(CN_DR_PROTECTOR, &DarkRuneProtectorAI::Create);
    mgr->register_creature_script(CN_LASSER_AIR_ELEMENTAL, &LesserAirElementalAI::Create);
    mgr->register_creature_script(CN_DR_WORKER, &DarkRuneWorkerAI::Create);
    mgr->register_creature_script(CN_DR_THEURGIST, &DarkRuneTheurgistAI::Create);
    mgr->register_creature_script(CN_DR_SHAPER, &DarkRuneShaperAI::Create);
    mgr->register_creature_script(CN_DR_SCHOLAR, &DarkRuneScholarAI::Create);
    mgr->register_creature_script(CN_DR_GIANT, &DarkRuneGiantAI::Create);
    mgr->register_creature_script(CN_RAGING_CONSTRUCT, &DarkRuneConstructAI::Create);
    mgr->register_creature_script(CN_LIGHTNING_CONSTRUCT, &DarkLightningConstructAI::Create);
    mgr->register_creature_script(CN_FI_TRAGG, &ForgedIronTroggAI::Create);

    //Bosses
    mgr->register_creature_script(BOSS_MAIDEN_OF_GRIEF, &MaidenOfGriefAI::Create);
    mgr->register_creature_script(BOSS_KRYSTALLUS, &KrystallusAI::Create);
}
