/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Instance_ZulFarrak.h"

#include "Setup.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Utilities/Random.hpp"

class ZulFarrakInstanceScript : public InstanceScript
{
public:
    explicit ZulFarrakInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr) { }
    static InstanceScript* Create(WorldMap* pMapMgr) { return new ZulFarrakInstanceScript(pMapMgr); }
};

//Theka the Martyr
// casts the spell Theka Transform 11089 at %30  hp
// casts the spell fevered plague around each 17 second
/*
Fevered Plague 8600 = Inflicts 250 Nature damage to an enemy, then an additional 11 damage every 5 sec. for 3 min.
Fevered Plague 16186 =  Inflicts 72 to 78 Nature damage to an enemy, then an additional 10 damage every 3 sec. for 30 sec. */

class ThekaAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ThekaAI(c); }
    explicit ThekaAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        morph = sSpellMgr.getSpellInfo(ZulFarrak::SP_THEKA_TRANSFORM);
        plague = sSpellMgr.getSpellInfo(ZulFarrak::SP_THEKA_FEVERED_PLAGUE);

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
        if (plaguecount >= randomplague && getCreature()->getThreatManager().getCurrentVictim())
        {
            plaguecount = 0;
            Unit* target = getCreature()->getThreatManager().getCurrentVictim();
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
public:
    static CreatureAIScript* Create(Creature* c) { return new AntusulTriggerAI(c); }
    explicit AntusulTriggerAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* mTarget) override
    {
        getCreature()->setControlled(true, UNIT_STATE_ROOTED);
        _setMeleeDisabled(true);
        getCreature()->addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);

        Unit* antusul = getNearestCreature(1815.030029f, 686.817017f, 14.519000f, 8127);
        if (antusul)
        {
            if (antusul->isAlive())
            {
                antusul->getAIInterface()->onHostileAction(mTarget);
                antusul->sendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Lunch has arrived, my beutiful childern. Tear them to pieces!");
            }
        }
    }
};

/// \note healing ward and earthgrab ward commented out since they need time and work wich i dont have right now
class AntusulAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AntusulAI(c); }
    explicit AntusulAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        add1 = add2 = add3 = add4 = add5 = add6 = trigger = nullptr;
        spawns = spawns2 = attack = firstspawn = secondspawn = false;
        servant = sSpellMgr.getSpellInfo(ZulFarrak::SP_ANTUSUL_SERVANTS);

        secondspawncount = 0;
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        add1 = add2 = add3 = add4 = add5 = add6 = trigger = nullptr;
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
            getCreature()->sendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Rise and defend your master!");
            getCreature()->castSpell(getCreature(), servant, true);
        }
        if (getCreature()->getHealthPct() <= 25)
        {
            secondspawncount++;
            if (secondspawn)
            {
                secondspawn = false;
                getCreature()->sendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "The children of sul will protect their master. Rise once more Sul'lithuz!");
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
            Unit* Target = getCreature()->getThreatManager().getCurrentVictim();
            if (getCreature()->getThreatManager().getCurrentVictim())
            {
                if (add1 && Target)
                    add1->getAIInterface()->onHostileAction(Target);

                if (add2 && Target)
                    add2->getAIInterface()->onHostileAction(Target);

                if (add3 && Target)
                    add3->getAIInterface()->onHostileAction(Target);

                if (add4 && Target)
                    add4->getAIInterface()->onHostileAction(Target);

                if (add5 && Target)
                    add5->getAIInterface()->onHostileAction(Target);

                if (add6 && Target)
                    add6->getAIInterface()->onHostileAction(Target);
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
        spawnCreature(ZulFarrak::CN_SULLITHUZ_BROODLING, 1777.753540f, 741.063538f, 16.439308f, 6.197119f);
        spawnCreature(ZulFarrak::CN_SULLITHUZ_BROODLING, 1782.193481f, 751.190002f, 16.620836f, 5.174994f);
        spawnCreature(ZulFarrak::CN_SULLITHUZ_BROODLING, 1790.956299f, 754.666809f, 14.195786f, 5.174208f);
        spawnCreature(ZulFarrak::CN_SULLITHUZ_BROODLING, 1800.902710f, 755.723267f, 15.642491f, 4.545889f);
        spawnCreature(ZulFarrak::CN_SULLITHUZ_BROODLING, 1809.339722f, 749.212402f, 16.910545f, 4.109208f);
        spawnCreature(ZulFarrak::CN_SULLITHUZ_BROODLING, 1818.182129f, 744.702820f, 17.801855f, 3.899507f);
    }

    void addsdefine()
    {
        add1 = getNearestCreature(1777.753540f, 741.063538f, 16.439308f, ZulFarrak::CN_SULLITHUZ_BROODLING);
        add2 = getNearestCreature(1782.193481f, 751.190002f, 16.620836f, ZulFarrak::CN_SULLITHUZ_BROODLING);
        add3 = getNearestCreature(1790.956299f, 754.666809f, 14.195786f, ZulFarrak::CN_SULLITHUZ_BROODLING);
        add4 = getNearestCreature(1800.902710f, 755.723267f, 15.642491f, ZulFarrak::CN_SULLITHUZ_BROODLING);
        add5 = getNearestCreature(1809.339722f, 749.212402f, 16.910545f, ZulFarrak::CN_SULLITHUZ_BROODLING);
        add6 = getNearestCreature(1818.182129f, 744.702820f, 17.801855f, ZulFarrak::CN_SULLITHUZ_BROODLING);
    }

    void resettrigger()
    {
        trigger = getNearestCreature(1811.943726f, 714.839417f, 12.897189f, ZulFarrak::TRIGGER_ANTUSUL);
        if (trigger)
        {
            trigger->setControlled(false, UNIT_STATE_ROOTED);
            trigger->getAIInterface()->setMeleeDisabled(false);
        }
    }

    void deletespawns()
    {
        if (add1)
            add1->Despawn(1000, 0);

        if (add2)
            add2->Despawn(1000, 0);

        if (add3)
            add3->Despawn(1000, 0);

        if (add4)
            add4->Despawn(1000, 0);

        if (add5)
            add5->Despawn(1000, 0);

        if (add6)
            add6->Despawn(1000, 0);
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
    mgr->register_creature_script(ZulFarrak::CN_ANTUSUL, &AntusulAI::Create);
    mgr->register_creature_script(ZulFarrak::CN_THEKA , &ThekaAI::Create);
    mgr->register_creature_script(ZulFarrak::TRIGGER_ANTUSUL, &AntusulTriggerAI::Create);
}
