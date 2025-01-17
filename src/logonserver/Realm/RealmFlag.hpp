/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

namespace AscEmu::Realm
{
    enum RealmFlag
    {
        NONE = 0x00,
        ///\todo Other emulators have VERSION_MISMATCH instead of INVALID
        INVALID = 0x01,
        OFFLINE = 0x02,
        SPECIFIC_BUILD = 0x04,     // client will show realm version in RealmList screen in form "RealmName (major.minor.revision.build)"
        UNKNOWN_1 = 0x08,
        UNKNOWN_2 = 0x10,
        ///\todo Other emulators have REALM_FLAG_NEW_PLAYERS and REALM_FLAG_RECOMMENDED swaped.
        NEW_PLAYERS = 0x20,
        RECOMMENDED = 0x40,
        FULL = 0x80
    };
}
