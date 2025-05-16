/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Instance_TheVioletHold.hpp"
#include "Zuramat.hpp"

#include "Movement/MovementManager.h"

//////////////////////////////////////////////////////////////////////////////////////////
//  Zuramat AI
ZuramatAI::ZuramatAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    // Instance Script
    mInstance = getInstanceScript();

    // Spells
    addAISpell(Zuramat::SPELL_SUMMON_VOID_SENTRY, 33.0f, 4, [this]() { return getCreature(); });
    addAISpell(Zuramat::SPELL_VOID_SHIFT, 33.0f, 9, [this]() { return getBestUnitTarget(TargetFilter_InRangeOnly, 0.0f, 60.0f); });
    addAISpell(Zuramat::SPELL_SHROUD_OF_DARKNESS, 33.0f, 18, [this]() { return getCreature(); });

    // Emotes
    addEmoteForEvent(Event_OnCombatStart, Zuramat::SAY_AGGRO);
    addEmoteForEvent(Event_OnTargetDied, Zuramat::SAY_SLAY1);
    addEmoteForEvent(Event_OnTargetDied, Zuramat::SAY_SLAY2);
    addEmoteForEvent(Event_OnTargetDied, Zuramat::SAY_SLAY3);
    addEmoteForEvent(Event_OnDied, Zuramat::SAY_DEATH);
}

CreatureAIScript* ZuramatAI::Create(Creature* pCreature) { return new ZuramatAI(pCreature); }

void ZuramatAI::OnLoad()
{
    getCreature()->getMovementManager()->moveTargetedHome();
    getCreature()->getAIInterface()->setImmuneToNPC(true);
    getCreature()->getAIInterface()->setImmuneToPC(true);
    getCreature()->addUnitFlags(UNIT_FLAG_IGNORE_CREATURE_COMBAT);
}

void ZuramatAI::OnCombatStop(Unit* /*_target*/)
{
    mVoidDance = true;
}

void ZuramatAI::OnDied(Unit* /*_killer*/)
{
    if (mInstance)
        mInstance->setBossState(DATA_ZURAMAT, Performed);
}

void ZuramatAI::OnSummonDies(Creature* summon, Unit* /*killer*/)
{
    if (summon->getEntry() == NPC_VOID_SENTRY)
        mVoidDance = false;
}

void ZuramatAI::OnSummonDespawn(Creature* summon)
{
    if (summon->getEntry() == NPC_VOID_SENTRY)
        summon->GetScript()->DoAction(Zuramat::ACTION_DESPAWN_VOID_SENTRY_BALL);
}

void ZuramatAI::justReachedSpawn()
{
    // Reset Cell Door
    if (mInstance)
        mInstance->setLocalData(DATA_HANDLE_CELLS, DATA_ZURAMAT);
}

uint32_t ZuramatAI::GetCreatureData(uint32_t type) const
{
    if (type == Zuramat::DATA_VOID_DANCE)
        return mVoidDance ? 1 : 0;

    return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////
//  Void Sentry AI
VoidSentryAI::VoidSentryAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    getCreature()->getAIInterface()->setReactState(REACT_PASSIVE);
}

CreatureAIScript* VoidSentryAI::Create(Creature* pCreature) { return new VoidSentryAI(pCreature); }

void VoidSentryAI::DoAction(int32_t actionId)
{
    if (actionId == Zuramat::ACTION_DESPAWN_VOID_SENTRY_BALL)
        summons.despawnAll();
}

void VoidSentryAI::OnSummon(Unit* /*summoner*/)
{
    getCreature()->castSpell(getCreature(), Zuramat::SPELL_SUMMON_VOID_SENTRY_BALL, true);
}

void VoidSentryAI::onSummonedCreature(Creature* summon)
{
    summons.summon(summon);
    summon->getAIInterface()->setReactState(REACT_PASSIVE);
}

void VoidSentryAI::OnDied(Unit* /*_killer*/)
{
    DoAction(Zuramat::ACTION_DESPAWN_VOID_SENTRY_BALL);
}

void VoidSentryAI::OnSummonDespawn(Creature* summon)
{
    summons.despawn(summon);
}

//////////////////////////////////////////////////////////////////////////////////////////
//  Void Sentry Achievement
bool achievement_void_dance::canCompleteCriteria(uint32_t /*criteriaID*/, Player* /*pPlayer*/, Object* target)
{
    if (!target)
        return false;

    if (Creature* Zuramat = target->ToCreature())
        if (Zuramat->GetScript() && Zuramat->GetScript()->GetCreatureData(Zuramat::DATA_VOID_DANCE))
            return true;

    return false;
}
