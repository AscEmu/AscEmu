/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

// Same for SMSG_SPELL_START and SMSG_SPELL_GO
enum SpellPacketFlags : uint32_t
{
    SPELL_PACKET_FLAGS_NONE                     = 0x00,
    SPELL_PACKET_FLAGS_LOCK_PLAYER_CAST_ANIM    = 0x01, // also do not send standstate update
    SPELL_PACKET_FLAGS_DEFAULT                  = 0x02, // atm used as default flag
    SPELL_PACKET_FLAGS_UNK4                     = 0x04,
    SPELL_PACKET_FLAGS_UNK8                     = 0x08, // seems like all of these mean some spell anim state
    SPELL_PACKET_FLAGS_UNK10                    = 0x10,
    SPELL_PACKET_FLAGS_RANGED                   = 0x20, // Projectile visual
    SPELL_PACKET_FLAGS_UNK40                    = 0x40,
    SPELL_PACKET_FLAGS_UNK80                    = 0x80,
    SPELL_PACKET_FLAGS_ITEM_CASTER              = 0x100,
    SPELL_PACKET_FLAGS_UNK200                   = 0x200,
    SPELL_PACKET_FLAGS_EXTRA_MESSAGE            = 0x400, // TARGET MISSES AND OTHER MESSAGES LIKE "Resist"
    SPELL_PACKET_FLAGS_POWER_UPDATE             = 0x800, // seems to work hand in hand with some visual effect of update actually
    SPELL_PACKET_FLAGS_UNK1000                  = 0x1000,
    SPELL_PACKET_FLAGS_UNK2000                  = 0x2000,
    SPELL_PACKET_FLAGS_UNK4000                  = 0x4000,
    SPELL_PACKET_FLAGS_UNK8000                  = 0x8000, // seems to make server send extra 2 bytes before SPELL_GO_FLAGS_UNK1 and after SPELL_GO_FLAGS_UNK20000
#if VERSION_STRING >= WotLK
    SPELL_PACKET_FLAGS_UNK10000                 = 0x10000,
    SPELL_PACKET_FLAGS_UPDATE_MISSILE           = 0x20000,
    SPELL_PACKET_FLAGS_UNK40000                 = 0x40000, // related to cooldowns
    SPELL_PACKET_FLAGS_UNK80000                 = 0x80000, // 2 functions called (same ones as for ranged but different)
    SPELL_PACKET_FLAGS_UNK100000                = 0x100000,
    SPELL_PACKET_FLAGS_RUNE_UPDATE              = 0x200000, // 2 bytes for the rune cur and rune next flags
    SPELL_PACKET_FLAGS_UNK400000                = 0x400000, // seems to make server send an uint32 after m_targets.write
    SPELL_PACKET_FLAGS_UNK800000                = 0x800000,
    SPELL_PACKET_FLAGS_UNK1000000               = 0x1000000,
    SPELL_PACKET_FLAGS_UNK2000000               = 0x2000000,
    SPELL_PACKET_FLAGS_UNK4000000               = 0x4000000,
    SPELL_PACKET_FLAGS_UNK8000000               = 0x8000000,
#endif
#if VERSION_STRING >= Cata
    SPELL_PACKET_FLAGS_UNK10000000              = 0x10000000,
    SPELL_PACKET_FLAGS_UNK20000000              = 0x20000000,
    SPELL_PACKET_FLAGS_HEALTH_UPDATE            = 0x40000000, // used with healing spells
    SPELL_PACKET_FLAGS_UNK80000000              = 0x80000000,
#endif
};
