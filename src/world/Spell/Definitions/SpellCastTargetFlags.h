/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

enum SpellCastTargetFlags
{
    TARGET_FLAG_SELF            = 0x00000,
    TARGET_FLAG_UNK1            = 0x00001,
    TARGET_FLAG_UNIT            = 0x00002,
    TARGET_FLAG_UNK2            = 0x00004,
    TARGET_FLAG_UNK3            = 0x00008,
    TARGET_FLAG_ITEM            = 0x00010,
    TARGET_FLAG_SOURCE_LOCATION = 0x00020,
    TARGET_FLAG_DEST_LOCATION   = 0x00040,
    TARGET_FLAG_OBJECT_CASTER   = 0x00080,
    TARGET_FLAG_UNIT_CASTER     = 0x00100,
    TARGET_FLAG_CORPSE          = 0x00200, // PvP Corpse
    TARGET_FLAG_UNIT_CORPSE     = 0x00400, // Gathering Corpse
    TARGET_FLAG_OBJECT          = 0x00800,
    TARGET_FLAG_TRADE_ITEM      = 0x01000,
    TARGET_FLAG_STRING          = 0x02000,
    TARGET_FLAG_OPEN_LOCK       = 0x04000,
    TARGET_FLAG_CORPSE2         = 0x08000, // Resurrection Spells
    TARGET_FLAG_GLYPH           = 0x20000
};
