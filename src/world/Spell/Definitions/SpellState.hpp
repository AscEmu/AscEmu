/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

enum SpellState : uint8_t
{
    SPELL_STATE_NULL = 0,
    SPELL_STATE_PREPARING,      // Spell is being created and prepared for spell cast
    SPELL_STATE_CASTING,        // Spell cast has started and cast bar is sent to client
    SPELL_STATE_CASTED,         // Spell cast has finished and targets are being initalized for each effect
    SPELL_STATE_CHANNELING,     // After CASTED state channeled spells start channeling
    SPELL_STATE_TRAVELING,      // After CASTED state non-channeled spells send spell projectiles to targets
    SPELL_STATE_FINISHED        // All spell effects and targets have been handled and spell is prepared for remove
};
