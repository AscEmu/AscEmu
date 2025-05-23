/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Erekem.hpp"

#include "Movement/MovementManager.h"
#include "Movement/MovementGenerators/PointMovementGenerator.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"

//////////////////////////////////////////////////////////////////////////////////////////
//  Erekem AI
ErekemAI::ErekemAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    // Instance Script
    mInstance = getInstanceScript();

    // Spells
    addAISpell(Erekem::SPELL_EARTH_SHIELD, 100.0f, TARGET_SELF);
    addAISpell(Erekem::SPELL_BLOODLUST, 100.0f, TARGET_SELF);
    addAISpell(Erekem::SPELL_LIGHTNING_BOLT, 44.0f, TARGET_RANDOM_SINGLE, 0, 2);
    addAISpell(Erekem::SPELL_EARTH_SHOCK, 44.0f, TARGET_RANDOM_SINGLE, 0, 8);

    // Only Casted When hes Guards are Dead so make Chance 0 and Handle in Update
    if (windfury = addAISpell(Erekem::SPELL_WINDFURY, 0.0f, TARGET_ATTACKING))
    {
        windfury->mIsTriggered = true;
        windfury->setAvailableForScriptPhase({ 1 });
    }

    breakBonds = addAISpell(Erekem::SPELL_BREAK_BONDS, 0.0f, TARGET_SELF);

    // Only Cast This on The Lowest Health In Range Friendly Target
    addAISpell(Erekem::SPELL_CHAIN_HEAL, 33.0f, 3, [this]() { return getBestUnitTarget(TargetFilter_WoundedFriendlyLowestHealthInRange, 0.0f, 40.0f); });
    addAISpell(Erekem::SPELL_EARTH_SHIELD, 100.0f, 20, [this]() { return getBestUnitTarget(TargetFilter_WoundedFriendlyLowestHealthInRange, 0.0f, 30.0f); });

    if (CreatureAISpells* stormstrike = addAISpell(Erekem::SPELL_STORMSTRIKE, 100.0f, TARGET_ATTACKING))
        stormstrike->setAvailableForScriptPhase({ 2 });

    // Emotes
    addEmoteForEvent(Event_OnCombatStart, Erekem::SAY_AGGRO);
    addEmoteForEvent(Event_OnTargetDied, Erekem::SAY_SLAY1);
    addEmoteForEvent(Event_OnTargetDied, Erekem::SAY_SLAY2);
    addEmoteForEvent(Event_OnTargetDied, Erekem::SAY_SLAY3);
    addEmoteForEvent(Event_OnDied, Erekem::SAY_DEATH);
}

CreatureAIScript* ErekemAI::Create(Creature* pCreature) { return new ErekemAI(pCreature); }

void ErekemAI::OnLoad()
{
    getCreature()->getMovementManager()->moveTargetedHome();
    getCreature()->getAIInterface()->setImmuneToNPC(true);
    getCreature()->getAIInterface()->setImmuneToPC(true);
    getCreature()->addUnitFlags(UNIT_FLAG_IGNORE_CREATURE_COMBAT);
}

void ErekemAI::AIUpdate(unsigned long /*time_passed*/)
{
    if (mInstance)
    {
        for (uint32_t i = DATA_EREKEM_GUARD_1; i <= DATA_EREKEM_GUARD_2; ++i)
        {
            Creature* guard = mInstance->GetCreatureByGuid(mInstance->getLocalData(i));

            if (guard && guard->isAlive() && checkGuardAuras(guard))
            {
                _castAISpell(breakBonds);
                return;
            }
        }
    }

    if (getScriptPhase() == 1 && !checkGuardsAlive())
    {
        setScriptPhase(2);
        _castAISpell(windfury);
    }
}

bool ErekemAI::checkGuardsAlive()
{
    if (mInstance)
    {
        for (uint32_t i = DATA_EREKEM_GUARD_1; i <= DATA_EREKEM_GUARD_2; ++i)
        {
            if (Creature* guard = mInstance->GetCreatureByGuid(mInstance->getLocalData(i)))
                if (guard->isAlive())
                    return true;
        }
    }

    return false;
}

bool ErekemAI::checkGuardAuras(Creature* guard)
{
    uint32_t mechanicMask = MOVEMENT_IMPAIRMENTS_AND_LOSS_OF_CONTROL_MASK;

    static std::list<AuraEffect> const AuraImmunityList =
    {
        SPELL_AURA_MOD_STUN,
        SPELL_AURA_MOD_DECREASE_SPEED,
        SPELL_AURA_MOD_ROOT,
        SPELL_AURA_MOD_CONFUSE,
        SPELL_AURA_MOD_FEAR
    };

    if (guard->hasAuraWithMechanic(SpellMechanic(mechanicMask)))
        return true;

    for (AuraEffect type : AuraImmunityList)
        if (guard->hasAuraWithAuraEffect(type))
            return true;

    return false;
}

void ErekemAI::OnDied(Unit* /*_killer*/)
{
    if (mInstance)
        mInstance->setBossState(DATA_EREKEM, Performed);
}

void ErekemAI::OnReachWP(uint32_t type, uint32_t pointId)
{
    if (type == EFFECT_MOTION_TYPE && pointId == POINT_INTRO)
        getCreature()->setFacingTo(4.921828f);
}

void ErekemAI::justReachedSpawn()
{
    // Reset Cell Door
    if (mInstance)
        mInstance->setLocalData(DATA_HANDLE_CELLS, DATA_EREKEM);
}

//////////////////////////////////////////////////////////////////////////////////////////
//  ErekemGuard AI
ErekemGuardAI::ErekemGuardAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    // Spells
    addAISpell(Erekem::SPELL_GUSHING_WOUND, 33.0f, TARGET_ATTACKING, 0, 7);
    addAISpell(Erekem::SPELL_HOWLING_SCREECH, 33.0f, TARGET_SELF, 0, 8);
    addAISpell(Erekem::SPELL_STRIKE, 33.0f, TARGET_ATTACKING, 0, 4);
}

CreatureAIScript* ErekemGuardAI::Create(Creature* pCreature) { return new ErekemGuardAI(pCreature); }
