/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Definitions/SpellFailure.h"
#include "Units/Unit.h"

#include "CommonTypes.hpp"

class Spell;

enum class SpellScriptExecuteState : uint8_t
{
    EXECUTE_NOT_HANDLED = 0,    // Spell script is not found or not handled
    EXECUTE_OK,                 // Spell script is executed
    EXECUTE_PREVENT,            // Spell script is executed but prevent default effect
};

class SERVER_DECL SpellScript
{
public:

    SpellScript() {}
    virtual ~SpellScript() {}

    // Called at the end of spell check cast function
    virtual SpellCastResult onCanCast(Spell* /*spell*/, uint32_t* /*parameter1*/, uint32_t* /*parameter2*/) const { return SPELL_CAST_SUCCESS; }
    // Called when cast bar is sent to client (NOT called for instant spells)
    virtual void doAtStartCasting(Spell* /*spell*/) {}
    // Called after spell targets for this effect have been initialized
    virtual void filterEffectTargets(Spell* /*spell*/, uint8_t /*effectIndex*/, std::vector<uint64_t>* /*effectTargets*/) {}
    // Called before spell effect is processed on effect targets
    virtual void doBeforeEffectHit(Spell* /*spell*/, uint8_t /*effectIndex*/) {}
    // Called after target missed/resisted the spell
    virtual void doAfterSpellMissed(Spell* /*spell*/, Unit* /*unitTarget*/) {}
    // Called before spell effect type handling
    virtual SpellScriptExecuteState beforeSpellEffect(Spell* /*spell*/, uint32_t /*effectType*/, uint8_t /*effectId*/) const { return SpellScriptExecuteState::EXECUTE_NOT_HANDLED; }
    // Called after spell effect type handling
    virtual void afterSpellEffect(Spell* /*spell*/, uint32_t /*effectType*/, uint8_t /*effectId*/) {}
};
