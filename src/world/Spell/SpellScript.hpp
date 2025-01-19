/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CommonTypes.hpp"
#include "Definitions/AuraRemoveMode.hpp"
#include "Definitions/ProcFlags.hpp"
#include "Definitions/SpellFailure.hpp"
#include "SpellScriptDefines.hpp"

#include <vector>
#include <math.h>

class Aura;
class Object;
class Spell;
class SpellInfo;
class SpellProc;
class Unit;

struct AuraEffectModifier;
struct DamageInfo;

class SERVER_DECL SpellScript
{
public:
    SpellScript() = default;
    virtual ~SpellScript() {}

    //////////////////////////////////////////////////////////////////////////////////////////
    // Spell execution

    // Called at the end of spell check cast function
    virtual SpellCastResult onCanCast(Spell* spell, uint32_t* parameter1, uint32_t* parameter2);
    // Called when cast bar is sent to client (NOT called for instant spells)
    virtual void doAtStartCasting(Spell* spell);
    // Called after spell targets for this effect have been initialized
    virtual void filterEffectTargets(Spell* spell, uint8_t effectIndex, std::vector<uint64_t>* effectTargets);
    // Called before spell effect is processed on effect targets
    virtual void doBeforeEffectHit(Spell* spell, uint8_t effectIndex);
    // Called after target missed/resisted the spell
    virtual void doAfterSpellMissed(Spell* spell, Unit* unitTarget);
    // Called when spell effect is calculated
    virtual SpellScriptEffectDamage doCalculateEffect(Spell* spell, uint8_t effIndex, int32_t* damage);
    // Called before spell effect is handled
    virtual SpellScriptExecuteState beforeSpellEffect(Spell* spell, uint8_t effIndex);
    // Called on a dummy or scripted spell effect
    virtual SpellScriptCheckDummy onDummyOrScriptedEffect(Spell* spell, uint8_t effIndex);
    // Called after spell effect is handled
    virtual void afterSpellEffect(Spell* spell, uint8_t effIndex);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Aura

    // Called when aura is created (Aura constructor)
    virtual void onAuraCreate(Aura* aur);
    // Called when aura is applied
    virtual void onAuraApply(Aura* aur);
    // Called when aura is removed
    virtual void onAuraRemove(Aura* aur, AuraRemoveMode mode);
    // Called when aura is refreshed or gains a new stack
    virtual void onAuraRefreshOrGainNewStack(Aura* aur, uint32_t newStackCount, uint32_t oldStackCount);
    // Called before aura effect is handled
    virtual SpellScriptExecuteState beforeAuraEffect(Aura* aur, AuraEffectModifier* aurEff, bool apply);
    // Called on a dummy aura effect, non-periodic and periodic
    virtual SpellScriptCheckDummy onAuraDummyEffect(Aura* aur, AuraEffectModifier* aurEff, bool apply);
    // Called when periodic tick happens
    virtual SpellScriptExecuteState onAuraPeriodicTick(Aura* aur, AuraEffectModifier* aurEff, float_t* damage);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Spell proc

    // Called after this object is created
    // Useful for initialize object members
    virtual void onCreateSpellProc(SpellProc* spellProc, Object* obj);
    // Returns true if this spell can proc, false otherwise
    virtual bool canProc(SpellProc* spellProc, Unit* victim, SpellInfo const* castingSpell, DamageInfo damageInfo);
    // Called when procFlags is to be compared
    // Return true on success, false otherwise
    virtual bool onCheckProcFlags(SpellProc* spellProc, SpellProcFlags procFlags);
    // Check if this object is identified by method arguments, so it can be deleted
    virtual bool canDeleteProc(SpellProc* spellProc, uint32_t spellId, uint64_t casterGuid, uint64_t misc = 0);
    // Called after proc chance is rolled
    // Return EXECUTE_OK so Unit::HandleProc execute subsequent statements, like cast the proc spell
    // Return EXECUTE_PREVENT if this handles everything, so Unit::HandleProc can skip to next iteration
    virtual SpellScriptExecuteState onDoProcEffect(SpellProc* spellProc, Unit* victim, SpellInfo const* castingSpell, DamageInfo damageInfo);
    // Calculate proc chance
    virtual uint32_t calcProcChance(SpellProc* spellProc, Unit* victim, SpellInfo const* castingSpell);
    // Called when trying to proc on a triggered spell
    // Return true allow proc, false otherwise
    virtual bool canProcOnTriggered(SpellProc* spellProc, Unit* victim, SpellInfo const* castingSpell, Aura* triggeredFromAura);
    // Called right before the proc spell is cast
    // Caster is not necessarily the unit who owns this spell proc
    virtual SpellScriptExecuteState onCastProcSpell(SpellProc* spellProc, Unit* caster, Unit* victim, Spell* spellToProc);
};
