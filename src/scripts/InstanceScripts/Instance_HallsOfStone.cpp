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


//Dark Rune Stormcaller
class DarkRuneStormcallerAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(DarkRuneStormcallerAI, MoonScriptCreatureAI);
    DarkRuneStormcallerAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        AddSpell(STORMCALLER_LIGHTNINGBOLT, Target_RandomPlayer, 60, 3, 6);
        AddSpell(STORMCALLER_SHADOWWORD, Target_RandomPlayer, 16, 0, 12);
    }

};

//Iron Golem Custodian
class IronGolemCustodianAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(IronGolemCustodianAI, MoonScriptCreatureAI);
    IronGolemCustodianAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        AddSpell(CUSTODIAN_CRUSH_ARMOR, Target_Current, 50, 0, 5);
        AddSpell(CUSTODIAN_GROUND_SMASH, Target_ClosestPlayer, 20, 0, 14);
    }

};

//Dark Rune Protector
class DarkRuneProtectorAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(DarkRuneProtectorAI, MoonScriptCreatureAI);
    DarkRuneProtectorAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        AddSpell(PROTECTOR_CHARGE, Target_RandomPlayerNotCurrent, 20, 0, 14, 10);
        AddSpell(PROTECTOR_CLAVE, Target_Current, 35, 0, 8);
    }

};

//Lesser Air Elemental
class LesserAirElementalAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(LesserAirElementalAI, MoonScriptCreatureAI);
    LesserAirElementalAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        AddSpell(ELEMENTAL_LIGHTNING_BOLT, Target_RandomPlayerNotCurrent, 20, 3, 14);
    }

};

//Dark Rune Worker
class DarkRuneWorkerAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(DarkRuneWorkerAI, MoonScriptCreatureAI);
    DarkRuneWorkerAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        AddSpell(WORKER_ENRAGE, Target_Self, 5, 0, 60, 10);
        AddSpell(WORKER_PIERCE_ARMOR, Target_Current, 35, 0, 45);
    }

};

//Dark Rune Warrior
class DarkRuneWarriorAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(DarkRuneWarriorAI, MoonScriptCreatureAI);
    DarkRuneWarriorAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        AddSpell(WARRIOR_CLAVE, Target_Current, 15, 0, 8);
        AddSpell(WARRIOR_HEROIC_STRIKE, Target_Current, 35, 0, 12);
    }

};

//Dark Rune Theurgist
class DarkRuneTheurgistAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(DarkRuneTheurgistAI, MoonScriptCreatureAI);
    DarkRuneTheurgistAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        AddSpell(THEURGIST_BLAST_WAVE, Target_Self, 22, 0, 25);
        AddSpell(THEURGIST_FIREBOLT, Target_RandomPlayer, 40, 3, 6);
        AddSpell(THEURGIST_IRON_MIGHT, Target_Self, 5, 0, 60);
    }

};

//Dark Rune Shaper
class DarkRuneShaperAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(DarkRuneShaperAI, MoonScriptCreatureAI);
    DarkRuneShaperAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        AddSpell(SHAPER_RAY, Target_RandomPlayer, 35, 1.5, 12);
    }

};

//Dark Rune Scholar
class DarkRuneScholarAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(DarkRuneScholarAI, MoonScriptCreatureAI);
    DarkRuneScholarAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        AddSpell(SCHOLAR_SILANCE, Target_RandomPlayerNotCurrent, 35, 2.5, 12);
    }

};

//Dark Rune Giant
class DarkRuneGiantAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(DarkRuneGiantAI, MoonScriptCreatureAI);
    DarkRuneGiantAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        AddSpell(GIANT_FIST, Target_Self, 3, 2, 40);
        AddSpell(GIANT_STOMP, Target_RandomPlayer, 35, 0, 14, 0, 10);
    }

};

//Raging Construct
class DarkRuneConstructAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(DarkRuneConstructAI, MoonScriptCreatureAI);
    DarkRuneConstructAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        AddSpell(RAGING_POTENT_JOLT, Target_Self, 95, 0, 8);
        AddSpell(RAGING_CLAVE, Target_Current, 30, 0, 9, 0, 10);
    }

};

//Lightning Construct
class DarkLightningConstructAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(DarkLightningConstructAI, MoonScriptCreatureAI);
    DarkLightningConstructAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        AddSpell(LIGHTN_ELECTRICAL_OVERLOAD, Target_Self, 5, 1.5, 14);
        AddSpell(LIGHTN_CHAIN_LIGHTNING, Target_Current, 30, 3, 8, 0, 30);
    }

};

//Forged Iron Trogg
class ForgedIronTroggAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(ForgedIronTroggAI, MoonScriptCreatureAI);
    ForgedIronTroggAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        AddSpell(LIGHTN_CHAIN_LIGHTNING, Target_RandomPlayer, 30, 2, 8, 0, 10);
    }

};

//Maiden of Grief
class MaidenOfGriefAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(MaidenOfGriefAI, MoonScriptCreatureAI);
    MaidenOfGriefAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        AddSpell(MAIDEN_STORM_OF_GRIEF, Target_RandomPlayerDestination, 100, 0, 19);
        AddSpell(MAIDEN_PILLAR_OF_WOE, Target_RandomPlayerNotCurrent, 30, 0, 8);
        mShock = AddSpell(MAIDEN_SHOCK_OF_SORROW, Target_Self, 20, 0, 18);
        mShock->AddEmote("So much lost time... that you'll never get back!", CHAT_MSG_MONSTER_YELL, 13492);
    }

    void OnCombatStart(Unit* pTarget)
    {
        sendDBChatMessage(4367);     // You shouldn't have come... now you will die!

        mShock->TriggerCooldown();
        ParentClass::OnCombatStart(pTarget);
    }

    void OnTargetDied(Unit* pTarget)
    {
        switch (RandomUInt(3))
        {
            case 0:
                sendDBChatMessage(4368);     // Why must it be this way?
                break;
            case 1:
                sendDBChatMessage(4369);     // You had it coming!
                break;
            case 2:
                sendDBChatMessage(4370);     // My burden grows heavier...
                break;
            case 3:
                sendDBChatMessage(4371);     // This is your fault!
                break;
        }
    }

    void OnDied(Unit* pTarget)
    {
        sendDBChatMessage(4372);     // I hope you all rot! I never... wanted... this.
    }

    protected:

        SpellDesc* mShock;
};

// Krystallus
const uint32 STOMP_TIMER = 35000;
const uint32 SHATTER_TIMER = 4500;

class KrystallusAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(KrystallusAI, MoonScriptCreatureAI);
    KrystallusAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        AddSpell(KRYSTALLUS_BOULDER_TOSS, Target_ClosestPlayer, 35, 1, 4);
        mShatter = AddSpell(KRYSTALLUS_SHATTER, Target_Self, 0, 0, 0);
        mStomp = AddSpell(KRYSTALLUS_STOMP, Target_Self, 0, 0, 0);
        mShatter->AddEmote("Break.... you....", CHAT_MSG_MONSTER_YELL, 14178);

        mStompTimer = INVALIDATE_TIMER;
        mShatterTimer = INVALIDATE_TIMER;
    }

    void OnCombatStart(Unit* pTarget)
    {
        sendDBChatMessage(4363);      // Crush....

        mStompTimer = _addTimer(STOMP_TIMER);
        ParentClass::OnCombatStart(pTarget);
    }

    void AIUpdate()
    {
        if (_isTimerFinished(mStompTimer))
        {
            CastSpellNowNoScheduling(mStomp);
            setRooted(true);
            _resetTimer(mStompTimer, (STOMP_TIMER + SHATTER_TIMER));
            mShatterTimer = _addTimer(SHATTER_TIMER);

        }
        if (_isTimerFinished(mShatterTimer))
        {
            CastSpellNowNoScheduling(mShatter);
            setRooted(false);
            _removeTimer(mShatterTimer);
        }
    }

    void OnTargetDied(Unit* pTarget)
    {
        sendDBChatMessage(4365);     // Uuuuhhhhhhhhhh......
    }

    void OnDied(Unit* pTarget)
    {
        sendDBChatMessage(4364);     // 
    }

    protected:

        SpellDesc* mStomp;
        SpellDesc* mShatter;

        int32 mStompTimer;
        uint32 mShatterTimer;
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
