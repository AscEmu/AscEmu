/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "SpellScript.hpp"
#include "SpellDefines.hpp"
#include "SpellInfo.hpp"
#include "SpellProc.hpp"
#include "Objects/Object.hpp"

// Spell hooks

SpellCastResult SpellScript::onCanCast(Spell* /*spell*/, uint32_t* /*parameter1*/, uint32_t* /*parameter2*/)
{
    return SPELL_CAST_SUCCESS;
}

void SpellScript::doAtStartCasting(Spell* /*spell*/) {}

void SpellScript::filterEffectTargets(Spell* /*spell*/, uint8_t /*effectIndex*/, std::vector<uint64_t>* /*effectTargets*/) {}

void SpellScript::doBeforeEffectHit(Spell* /*spell*/, uint8_t /*effectIndex*/) {}

void SpellScript::doAfterSpellMissed(Spell* /*spell*/, Unit* /*unitTarget*/) {}

SpellScriptEffectDamage SpellScript::doCalculateEffect(Spell* /*spell*/, uint8_t /*effIndex*/, int32_t* /*damage*/)
{
    return SpellScriptEffectDamage::DAMAGE_DEFAULT;
}

SpellScriptExecuteState SpellScript::beforeSpellEffect(Spell* /*spell*/, uint8_t /*effectId*/)
{
    return SpellScriptExecuteState::EXECUTE_NOT_HANDLED;
}

SpellScriptCheckDummy SpellScript::onDummyOrScriptedEffect(Spell* /*spell*/, uint8_t /*effectIndex*/)
{
    return SpellScriptCheckDummy::DUMMY_NOT_HANDLED;
}

void SpellScript::afterSpellEffect(Spell* /*spell*/, uint8_t /*effectIndex*/) {}

// Aura hooks

void SpellScript::onAuraCreate(Aura* /*aur*/) {}

void SpellScript::onAuraApply(Aura* /*aur*/) {}

void SpellScript::onAuraRemove(Aura* /*aur*/, AuraRemoveMode /*mode*/) {}

void SpellScript::onAuraRefreshOrGainNewStack(Aura* /*aur*/, uint32_t /*newStackCount*/, uint32_t /*oldStackCount*/) {}

SpellScriptExecuteState SpellScript::beforeAuraEffect(Aura* /*aur*/, AuraEffectModifier* /*aurEff*/, bool /*apply*/)
{
    return SpellScriptExecuteState::EXECUTE_NOT_HANDLED;
}

SpellScriptCheckDummy SpellScript::onAuraDummyEffect(Aura* /*aur*/, AuraEffectModifier* /*aurEff*/, bool /*apply*/)
{
    return SpellScriptCheckDummy::DUMMY_NOT_HANDLED;
}

SpellScriptExecuteState SpellScript::onAuraPeriodicTick(Aura* /*aur*/, AuraEffectModifier* /*aurEff*/, float_t* /*damage*/)
{
    return SpellScriptExecuteState::EXECUTE_NOT_HANDLED;
}

// Spell proc

void SpellScript::onCreateSpellProc(SpellProc* /*spellProc*/, Object* /*obj*/) {}

bool SpellScript::canProc(SpellProc* /*spellProc*/, Unit* /*victim*/, SpellInfo const* /*castingSpell*/, DamageInfo /*damageInfo*/)
{
    return true;
}

bool SpellScript::onCheckProcFlags(SpellProc* spellProc, SpellProcFlags procFlags)
{
    return spellProc->getProcFlags() & procFlags;
}

bool SpellScript::canDeleteProc(SpellProc* spellProc, uint32_t spellId, uint64_t casterGuid, uint64_t /*misc = 0*/)
{
    if (spellProc->getSpell()->getId() == spellId && (casterGuid == 0 || spellProc->getCasterGuid() == casterGuid) && !spellProc->isDeleted())
        return true;

    return false;
}

SpellScriptExecuteState SpellScript::onDoProcEffect(SpellProc* /*spellProc*/, Unit* /*victim*/, SpellInfo const* /*castingSpell*/, DamageInfo /*damageInfo*/)
{
    return SpellScriptExecuteState::EXECUTE_NOT_HANDLED;
}

uint32_t SpellScript::calcProcChance(SpellProc* spellProc, Unit* victim, SpellInfo const* castingSpell)
{
    return spellProc->calcProcChance(victim, castingSpell);
}

bool SpellScript::canProcOnTriggered(SpellProc* spellProc, Unit* /*victim*/, SpellInfo const* /*castingSpell*/, Aura* /*triggeredFromAura*/)
{
    if (spellProc->getOriginalSpell() != nullptr && spellProc->getOriginalSpell()->getAttributesExC() & ATTRIBUTESEXC_CAN_PROC_ON_TRIGGERED)
        return true;

    return false;
}

SpellScriptExecuteState SpellScript::onCastProcSpell(SpellProc* /*spellProc*/, Unit* /*caster*/, Unit* /*victim*/, Spell* /*spellToProc*/)
{
    return SpellScriptExecuteState::EXECUTE_NOT_HANDLED;
}
