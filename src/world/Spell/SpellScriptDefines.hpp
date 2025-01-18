/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

enum class SpellScriptExecuteState : uint8_t
{
    EXECUTE_NOT_HANDLED = 0,    // Spell script is not found or not handled
    EXECUTE_OK,                 // Spell script is executed
    EXECUTE_PREVENT             // Spell script is executed but prevent default effect
};

enum class SpellScriptEffectDamage : uint8_t
{
    DAMAGE_DEFAULT = 0,         // Effect damage is using default calculations
    DAMAGE_NO_BONUSES,          // Effect damage gains no bonuses from spell power or attack power but will use effect modifiers
    DAMAGE_FULL_RECALCULATION   // Effect damage is completely recalculated, do not add any modifiers to damage
};

enum class SpellScriptCheckDummy : uint8_t
{
    DUMMY_NOT_HANDLED = 0,      // Default value, generates warning to debug log of unhandled dummy effect
    DUMMY_OK                    // Dummy effect handled, no warning to debug log
};

// Helpers for spell script
enum SpellEffects : uint8_t
{
    EFF_INDEX_0 = 0,
    EFF_INDEX_1,
    EFF_INDEX_2
};
