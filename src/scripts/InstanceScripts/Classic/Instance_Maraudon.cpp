/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Maraudon.h"

#include "Server/Script/CreatureAIScript.h"

class MaraudonInstanceScript : public InstanceScript
{
public:

    explicit MaraudonInstanceScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new MaraudonInstanceScript(pMapMgr); }


};

class CelebrasTheCursedAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(CelebrasTheCursedAI)
    explicit CelebrasTheCursedAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto wrath = addAISpell(21667, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        wrath->setAttackStopTimer(3000);

        auto entanglingRoots = addAISpell(21331, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        entanglingRoots->setAttackStopTimer(3000);

        auto twistedTranquility = addAISpell(21793, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        twistedTranquility->setAttackStopTimer(3000);
    }
};


class LordVyletongueAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(LordVyletongueAI)
    explicit LordVyletongueAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto putridBreath = addAISpell(21080, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        putridBreath->setAttackStopTimer(3000);

        auto smokeBomb = addAISpell(8817, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        smokeBomb->setAttackStopTimer(3000);
    }
};


class MeshlokTheHarvesterAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(MeshlokTheHarvesterAI)
    explicit MeshlokTheHarvesterAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto warStomp = addAISpell(24375, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        warStomp->setAttackStopTimer(3000);

        auto strike = addAISpell(15580, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        strike->setAttackStopTimer(3000);
    }
};


class PrincessTheradrasAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(PrincessTheradrasAI)
    explicit PrincessTheradrasAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto duelField = addAISpell(21909, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        duelField->setAttackStopTimer(3000);

        auto boulder = addAISpell(21832, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        boulder->setAttackStopTimer(3000);

        auto knockdown = addAISpell(19128, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        knockdown->setAttackStopTimer(3000);

        auto repulsiveGaze = addAISpell(21869, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        repulsiveGaze->setAttackStopTimer(3000);
    }
};


class RazorlashAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(RazorlashAI)
    explicit RazorlashAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto puncture = addAISpell(21911, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        puncture->setAttackStopTimer(3000);

        auto unknown = addAISpell(15584, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        unknown->setAttackStopTimer(3000);

        auto unknown2 = addAISpell(21749, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        unknown2->setAttackStopTimer(3000);
    }
};


class TinkererGizlockAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(TinkererGizlockAI)
    explicit TinkererGizlockAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto goblinDragonGun = addAISpell(21833, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        goblinDragonGun->setAttackStopTimer(3000);

        auto bomb = addAISpell(22334, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        bomb->setAttackStopTimer(3000);
    }
};


class NoxxionAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(NoxxionAI)
    explicit NoxxionAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto toxicVolley = addAISpell(21687, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        toxicVolley->setAttackStopTimer(3000);

        auto sporeCloud = addAISpell(21547, 20.0f, TARGET_ATTACKING, 0, 10, false, true);
        sporeCloud->setAttackStopTimer(3000);
    }
};

void SetupMaraudon(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_MARAUDON, &MaraudonInstanceScript::Create);
    mgr->register_creature_script(CN_CELEBRAS_THE_CURESE, &CelebrasTheCursedAI::Create);
    mgr->register_creature_script(CN_LORD_VYLETONGUE, &LordVyletongueAI::Create);
    mgr->register_creature_script(CN_MESHLOCK_THE_HARVESTER, &MeshlokTheHarvesterAI::Create);
    mgr->register_creature_script(CN_PRINCESS_THERADRAS, &PrincessTheradrasAI::Create);
    mgr->register_creature_script(CN_RAZORLASH, &RazorlashAI::Create);
    mgr->register_creature_script(CN_TRINKERER_GIZLOCK, &TinkererGizlockAI::Create);
    mgr->register_creature_script(CN_NOXXION, &NoxxionAI::Create);
}
