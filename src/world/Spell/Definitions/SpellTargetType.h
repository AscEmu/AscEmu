/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

enum SpellTargetType
{
    TARGET_TYPE_NULL       = 0x0,
    TARGET_TYPE_BEAST      = 0x1,
    TARGET_TYPE_DRAGONKIN  = 0x2,
    TARGET_TYPE_DEMON      = 0x4,
    TARGET_TYPE_ELEMENTAL  = 0x8,
    TARGET_TYPE_GIANT      = 0x10,
    TARGET_TYPE_UNDEAD     = 0x20,
    TARGET_TYPE_HUMANOID   = 0x40,
    TARGET_TYPE_CRITTER    = 0x80,
    TARGET_TYPE_MECHANICAL = 0x100
};
