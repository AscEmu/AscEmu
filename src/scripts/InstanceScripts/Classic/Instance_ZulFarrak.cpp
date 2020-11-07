/*
 * Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
 * Copyright (c) 2008-2015 Sun++ Team <http://www.sunplusplus.info>
 * Copyright (c) 2007-2015 Moon++ Team <http://www.moonplusplus.info>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
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
#include "Instance_ZulFarrak.h"

class ZulFarrakInstanceScript : public InstanceScript
{
public:

    explicit ZulFarrakInstanceScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new ZulFarrakInstanceScript(pMapMgr); }


};


//Theka the Martyr
// casts the spell Theka Transform 11089 at %30  hp
// casts the spell fevered plague around each 17 second
/*
Fevered Plague 8600 = Inflicts 250 Nature damage to an enemy, then an additional 11 damage every 5 sec. for 3 min.
Fevered Plague 16186 =  Inflicts 72 to 78 Nature damage to an enemy, then an additional 10 damage every 3 sec. for 30 sec. */


class ThekaAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(ThekaAI)
    explicit ThekaAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        morph = sSpellMgr.getSpellInfo(SP_THEKA_TRANSFORM);
        plague = sSpellMgr.getSpellInfo(SP_THEKA_FEVERED_PLAGUE);

        plaguecount = 0;
        randomplague = 0;
        morphcheck = false;
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        morphcheck = true;
        plaguecount = 0;
        RegisterAIUpdateEvent(1000);
    }

    void OnCombatStop(Unit* /*mTarget*/) override
    {
        morphcheck = false;
        plaguecount = 0;
    }

    void OnDied(Unit* /*mKiller*/) override
    {
        morphcheck = false;
        plaguecount = 0;
    }

    void AIUpdate() override
    {
        plaguecount++;
        randomplague = 16 + Util::getRandomUInt(3);
        if (plaguecount >= randomplague && getCreature()->GetAIInterface()->getNextTarget())
        {
            plaguecount = 0;
            Unit* target = NULL;
            target = getCreature()->GetAIInterface()->getNextTarget();
            getCreature()->castSpell(target, plague, true);
        }
        else if (getCreature()->getHealthPct() <= 30 && morphcheck)
        {
            morphcheck = false;

            getCreature()->castSpell(getCreature(), morph, false);
        }
    }

protected:
    int plaguecount, randomplague;
    bool morphcheck;
    SpellInfo const* morph;
    SpellInfo const* plague;
};


// Antu'sul
/** \note
needs a aggro trigger outside cave

yells

Lunch has arrived, my beutiful childern. Tear them to pieces!   // on aggro
Rise and defend your master!  //  at 75% when his add spawn
The children of sul will protect their master. Rise once more Sul'lithuz! // at  25% when his adds start spawning again but he says it only once and adds keep on spawning

spells

Healing Ward 11889  random around 18 sec
Earthgrab Totem 8376 random around 18 sec

misc info

he summons 6 Sul'lithuz Broodling 8138 on aggro
he summons Servant of antu'sul 8156 75% with spell 11894
he summons Servant of antu'sul 8156 25% with spell 11894 each 15 second
*/
class AntusulTriggerAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(AntusulTriggerAI)
    explicit AntusulTriggerAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* mTarget) override
    {
        getCreature()->GetAIInterface()->m_canMove = false;
        _setMeleeDisabled(true);
        getCreature()->addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);

        Unit* antusul = NULL;
        antusul = getNearestCreature(1815.030029f, 686.817017f, 14.519000f, 8127);
        if (antusul)
        {
            if (antusul->isAlive())
            {
                antusul->GetAIInterface()->AttackReaction(mTarget, 0, 0);
                antusul->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Lunch has arrived, my beutiful childern. Tear them to pieces!");
            }
        }
    }
};


/// \note healing ward and earthgrab ward commented out since they need time and work wich i dont have right now
class AntusulAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(AntusulAI)
    explicit AntusulAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        add1 = add2 = add3 = add4 = add5 = add6 = trigger = NULL;
        spawns = spawns2 = attack = firstspawn = secondspawn = false;
        servant = sSpellMgr.getSpellInfo(SP_ANTUSUL_SERVANTS);

        secondspawncount = 0;
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        add1 = add2 = add3 = add4 = add5 = add6 = trigger = NULL;
        spawns = firstspawn = secondspawn = true;
        spawns2 = attack = false;

        secondspawncount = 0;
        RegisterAIUpdateEvent(1000);
    }

    void OnCombatStop(Unit* /*mTarget*/) override
    {
        spawns = spawns2 = attack = firstspawn = secondspawn = false;

        secondspawncount = 0;
        resettrigger();
        deletespawns();
    }

    void OnDied(Unit* /*mKiller*/) override
    {
        spawns = spawns2 = attack = firstspawn = secondspawn = false;

        secondspawncount = 0;
        trigger = getNearestCreature(1811.943726f, 714.839417f, 12.897189f, 133337);
        if (trigger)
            trigger->Despawn(100, 0);
    }

    void AIUpdate() override
    {
        if (getCreature()->getHealthPct() <= 75 && firstspawn)
        {
            firstspawn = false;
            getCreature()->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Rise and defend your master!");
            getCreature()->castSpell(getCreature(), servant, true);
        }
        if (getCreature()->getHealthPct() <= 25)
        {
            secondspawncount++;
            if (secondspawn)
            {
                secondspawn = false;
                getCreature()->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "The children of sul will protect their master. Rise once more Sul'lithuz!");
                getCreature()->castSpell(getCreature(), servant, true);
            }
            if (secondspawncount >= 15)
            {
                secondspawncount = 0;
                getCreature()->castSpell(getCreature(), servant, true);
            }

        }
        if (attack)
        {
            Unit* Target = NULL;
            Target = getCreature()->GetAIInterface()->getNextTarget();
            if (getCreature()->GetAIInterface()->getNextTarget())
            {
                if (add1 && Target)
                {
                    add1->GetAIInterface()->AttackReaction(Target, 0, 0);
                }
                if (add2 && Target)
                {
                    add2->GetAIInterface()->AttackReaction(Target, 0, 0);
                }
                if (add3 && Target)
                {
                    add3->GetAIInterface()->AttackReaction(Target, 0, 0);
                }
                if (add4 && Target)
                {
                    add4->GetAIInterface()->AttackReaction(Target, 0, 0);
                }
                if (add5 && Target)
                {
                    add5->GetAIInterface()->AttackReaction(Target, 0, 0);
                }
                if (add6 && Target)
                {
                    add6->GetAIInterface()->AttackReaction(Target, 0, 0);
                }
            }

            attack = false;
        }
        if (spawns2)
        {
            spawns2 = false;
            addsdefine();
            attack = true;
        }
        if (spawns)
        {
            spawns = false;
            spawnadds();
            spawns2 = true;
        }

    }

    void spawnadds()
    {
        spawnCreature(CN_SULLITHUZ_BROODLING, 1777.753540f, 741.063538f, 16.439308f, 6.197119f);
        spawnCreature(CN_SULLITHUZ_BROODLING, 1782.193481f, 751.190002f, 16.620836f, 5.174994f);
        spawnCreature(CN_SULLITHUZ_BROODLING, 1790.956299f, 754.666809f, 14.195786f, 5.174208f);
        spawnCreature(CN_SULLITHUZ_BROODLING, 1800.902710f, 755.723267f, 15.642491f, 4.545889f);
        spawnCreature(CN_SULLITHUZ_BROODLING, 1809.339722f, 749.212402f, 16.910545f, 4.109208f);
        spawnCreature(CN_SULLITHUZ_BROODLING, 1818.182129f, 744.702820f, 17.801855f, 3.899507f);
    }

    void addsdefine()
    {
        add1 = getNearestCreature(1777.753540f, 741.063538f, 16.439308f, CN_SULLITHUZ_BROODLING);
        add2 = getNearestCreature(1782.193481f, 751.190002f, 16.620836f, CN_SULLITHUZ_BROODLING);
        add3 = getNearestCreature(1790.956299f, 754.666809f, 14.195786f, CN_SULLITHUZ_BROODLING);
        add4 = getNearestCreature(1800.902710f, 755.723267f, 15.642491f, CN_SULLITHUZ_BROODLING);
        add5 = getNearestCreature(1809.339722f, 749.212402f, 16.910545f, CN_SULLITHUZ_BROODLING);
        add6 = getNearestCreature(1818.182129f, 744.702820f, 17.801855f, CN_SULLITHUZ_BROODLING);
    }

    void resettrigger()
    {
        trigger = getNearestCreature(1811.943726f, 714.839417f, 12.897189f, TRIGGER_ANTUSUL);
        if (trigger)
        {
            trigger->GetAIInterface()->m_canMove = true;
            trigger->GetAIInterface()->setMeleeDisabled(false);
        }
    }

    void deletespawns()
    {
        if (add1)
        {
            add1->Despawn(1000, 0);
        }
        if (add2)
        {
            add2->Despawn(1000, 0);
        }
        if (add3)
        {
            add3->Despawn(1000, 0);
        }
        if (add4)
        {
            add4->Despawn(1000, 0);
        }
        if (add5)
        {
            add5->Despawn(1000, 0);
        }
        if (add6)
        {
            add6->Despawn(1000, 0);
        }
    }

protected:
    bool spawns, spawns2, attack, firstspawn, secondspawn;
    int secondspawncount;

    Creature* add1;
    Creature* add2;
    Creature* add3;
    Creature* add4;
    Creature* add5;
    Creature* add6;
    Creature* trigger;

    SpellInfo const* servant;
};

void SetupZulFarrak(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_ZUL_FARAK, &ZulFarrakInstanceScript::Create);

    //Creatures
    mgr->register_creature_script(CN_ANTUSUL, &AntusulAI::Create);
    mgr->register_creature_script(CN_THEKA , &ThekaAI::Create);
    mgr->register_creature_script(TRIGGER_ANTUSUL, &AntusulTriggerAI::Create);
}
