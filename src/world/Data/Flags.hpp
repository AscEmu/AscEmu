/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Server/ClientProtocol.hpp"

#include <cstdint>

// state of a trainer spell for the player, the trainer list packet writes the value of the client version
enum TrainerSpellState : uint8_t
{
    TRAINER_SPELL_GRAY                  = 0,
    TRAINER_SPELL_GREEN                 = 1,
    TRAINER_SPELL_RED                   = 2,
    TRAINER_SPELL_GREEN_DISABLED        = 10
};

// what the create update of an object carries, clientUpdateFlags() gives the value of the client version
enum ObjectUpdateFlags : uint16_t
{
    UPDATEFLAG_NONE                     = 0x0000,
    UPDATEFLAG_SELF                     = 0x0001,
    UPDATEFLAG_TRANSPORT                = 0x0002,
    UPDATEFLAG_HAS_TARGET               = 0x0004,
    UPDATEFLAG_LIVING                   = 0x0008,
    UPDATEFLAG_HAS_POSITION             = 0x0010, // stationary position
    UPDATEFLAG_VEHICLE                  = 0x0020,
    UPDATEFLAG_POSITION                 = 0x0040, // transport position
    UPDATEFLAG_ROTATION                 = 0x0080,
    UPDATEFLAG_ANIM_KITS                = 0x0100,
    UPDATEFLAG_AREATRIGGER              = 0x0200,
    UPDATEFLAG_PLAY_HOVER_ANIM          = 0x0400,
    UPDATEFLAG_SUPPRESSED_GREETINGS     = 0x0800,
    UPDATEFLAG_NO_BIRTH_ANIM            = 0x1000,
    UPDATEFLAG_ENABLE_PORTALS           = 0x2000,
    UPDATEFLAG_LEGACY_GUID              = 0x4000  // guid block of the clients up to wotlk
};

// the clients up to wotlk read the flags as one value with their own bits, the later clients get the bits by name
inline uint16_t clientUpdateFlags(uint16_t updateFlags, WoW::Expansion expansion)
{
    if (expansion >= WoW::Expansion::_Cata)
        return updateFlags;

    struct ClientBit
    {
        uint16_t flag;
        uint16_t classic;
        uint16_t tbc;
        uint16_t wotlk;
    };
    static constexpr ClientBit bits[] =
    {
        { UPDATEFLAG_SELF,           0x01, 0x01, 0x0001 },
        { UPDATEFLAG_TRANSPORT,      0x02, 0x02, 0x0002 },
        { UPDATEFLAG_HAS_TARGET,     0x04, 0x04, 0x0004 },
        { UPDATEFLAG_LEGACY_GUID,    0x10, 0x18, 0x0010 },
        { UPDATEFLAG_LIVING,         0x20, 0x20, 0x0020 },
        { UPDATEFLAG_HAS_POSITION,   0x40, 0x40, 0x0040 },
        { UPDATEFLAG_VEHICLE,        0x00, 0x00, 0x0080 },
        { UPDATEFLAG_POSITION,       0x00, 0x00, 0x0100 },
        { UPDATEFLAG_ROTATION,       0x00, 0x00, 0x0200 },
        { UPDATEFLAG_ANIM_KITS,      0x00, 0x00, 0x0800 },
        { UPDATEFLAG_ENABLE_PORTALS, 0x00, 0x00, 0x2000 }
    };

    uint16_t value = 0;
    for (const auto& bit : bits)
    {
        if ((updateFlags & bit.flag) == 0)
            continue;

        value |= expansion == WoW::Expansion::_Classic ? bit.classic : expansion == WoW::Expansion::_TBC ? bit.tbc : bit.wotlk;
    }

    return value;
}
