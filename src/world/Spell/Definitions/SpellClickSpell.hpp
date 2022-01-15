/*
Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

enum SpellClickUserTypes
{
    SPELL_CLICK_USER_ANY = 0,
    SPELL_CLICK_USER_FRIEND = 1,
    SPELL_CLICK_USER_RAID = 2,
    SPELL_CLICK_USER_PARTY = 3,
    SPELL_CLICK_USER_MAX = 4
};

enum SpellClickCastFlags
{
    NPC_CLICK_CAST_CASTER_CLICKER = 0x01,
    NPC_CLICK_CAST_TARGET_CLICKER = 0x02,
    NPC_CLICK_CAST_ORIG_CASTER_OWNER = 0x04
};

struct SERVER_DECL SpellClickInfo
{
    uint32_t spellId;
    uint8_t castFlags;
    SpellClickUserTypes userType;

    // helpers
    bool isFitToRequirements(Unit* clicker, Unit* clickee);
};
