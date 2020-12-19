/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

enum SpellState : uint8_t
{
    SPELL_STATE_NULL = 0,
    SPELL_STATE_PREPARING,
    SPELL_STATE_CHANNELING,
    SPELL_STATE_TRAVELING,
    SPELL_STATE_FINISHED
};
