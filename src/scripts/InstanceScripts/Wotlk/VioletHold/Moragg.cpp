/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Instance_TheVioletHold.hpp"
#include "Moragg.hpp"

#include "Movement/MovementManager.h"

//////////////////////////////////////////////////////////////////////////////////////////
//  Moragg AI
MoraggAI::MoraggAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    // Instance Script
    mInstance = getInstanceScript();

    // Spells
    addAISpell(Moragg::SPELL_RAY_OF_PAIN, 66.0f, TARGET_SELF, 0, 15);
    addAISpell(Moragg::SPELL_RAY_OF_SUFFERING, 66.0f, TARGET_SELF, 0, 15);
    addAISpell(Moragg::SPELL_CORROSIVE_SALIVA, 66.0f, TARGET_ATTACKING, 0, 5);
    if (CreatureAISpells* opticLink = addAISpell(Moragg::SPELL_OPTIC_LINK, 66.0f, TARGET_RANDOM_SINGLE, 0, 25))
        opticLink->setMinMaxDistance(0.0f, 50.0f);
}

CreatureAIScript* MoraggAI::Create(Creature* pCreature) { return new MoraggAI(pCreature); }

void MoraggAI::OnLoad()
{
    getCreature()->getMovementManager()->moveTargetedHome();
    getCreature()->getAIInterface()->setImmuneToNPC(true);
    getCreature()->getAIInterface()->setImmuneToPC(true);
    getCreature()->addUnitFlags(UNIT_FLAG_IGNORE_CREATURE_COMBAT);
}

void MoraggAI::OnDied(Unit* /*_killer*/)
{
    mInstance->setBossState(DATA_MORAGG, Performed);
}

void MoraggAI::justReachedSpawn()
{
    // Reset Cell Door
    mInstance->setLocalData(DATA_HANDLE_CELLS, DATA_MORAGG);
}
