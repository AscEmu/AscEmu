/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

enum SpellGoFlags
{
    SPELL_GO_FLAGS_LOCK_PLAYER_CAST_ANIM = 0x01, //also do not send standstate update
    //0x02
    //0x04
    //0x08 //seems like all of these mean some spell anim state
    //0x10
    SPELL_GO_FLAGS_RANGED                = 0x20,
    //0x40
    //0x80
    SPELL_GO_FLAGS_ITEM_CASTER           = 0x100,
    SPELL_GO_FLAGS_UNK200                = 0x200,
    SPELL_GO_FLAGS_EXTRA_MESSAGE         = 0x400, // TARGET MISSES AND OTHER MESSAGES LIKE "Resist"
    SPELL_GO_FLAGS_POWER_UPDATE          = 0x800, // seems to work hand in hand with some visual effect of update actually
    //0x1000
    SPELL_GO_FLAGS_UNK2000               = 0x2000,
    SPELL_GO_FLAGS_UNK1000               = 0x1000,
    //0x4000
    SPELL_GO_FLAGS_UNK8000               = 0x8000, // seems to make server send extra 2 bytes before SPELL_GO_FLAGS_UNK1 and after SPELL_GO_FLAGS_UNK20000
    SPELL_GO_FLAGS_UNK20000              = 0x20000, // seems to make server send an uint32 after m_targets.write
    SPELL_GO_FLAGS_UNK40000              = 0x40000,
    SPELL_GO_FLAGS_UNK80000              = 0x80000, // 2 functions called (same ones as for ranged but different)
    SPELL_GO_FLAGS_RUNE_UPDATE           = 0x200000, // 2 bytes for the rune cur and rune next flags
    SPELL_GO_FLAGS_UNK400000             = 0x400000, // seems to make server send an uint32 after m_targets.write
};
