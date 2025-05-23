/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Instance_TheVioletHold.hpp"
#include "Lavanthor.hpp"

#include "Movement/MovementManager.h"

//////////////////////////////////////////////////////////////////////////////////////////
//  Lavanthor AI
LavanthorAI::LavanthorAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    // Instance Script
    mInstance = getInstanceScript();

    // Spells
    addAISpell(Lavanthor::SPELL_CAUTERIZING_FLAMES, 33.0f, TARGET_VARIOUS, 0, 10, false, false, true);

    addAISpell(Lavanthor::SPELL_FIREBOLT, 45.0f, TARGET_SELF, 0, 5);
    addAISpell(Lavanthor::SPELL_FLAME_BREATH, 45.0f, TARGET_RANDOM_SINGLE, 0, 15);
    addAISpell(Lavanthor::SPELL_LAVA_BURN, 33.0f, TARGET_ATTACKING, 0, 10);
}

CreatureAIScript* LavanthorAI::Create(Creature* pCreature) { return new LavanthorAI(pCreature); }

void LavanthorAI::OnLoad()
{
    getCreature()->getMovementManager()->moveTargetedHome();
    getCreature()->getAIInterface()->setImmuneToNPC(true);
    getCreature()->getAIInterface()->setImmuneToPC(true);
    getCreature()->addUnitFlags(UNIT_FLAG_IGNORE_CREATURE_COMBAT);
}

void LavanthorAI::OnDied(Unit* /*_killer*/)
{
    if (mInstance)
        mInstance->setBossState(DATA_LAVANTHOR, Performed);
}

void LavanthorAI::justReachedSpawn()
{
    // Reset Cell Door
    if (mInstance)
        mInstance->setLocalData(DATA_HANDLE_CELLS, DATA_LAVANTHOR);
}
