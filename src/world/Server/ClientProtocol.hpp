/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

//\NOTE:    This file is part of an attempt to replace version specific code in files.
//          It defines the client version across all packet handling stages. Do not
//          use or work with this file unless you are able to understand what is
//          happening here ;)

#pragma once

#include <cstdint>

enum class ClientVersion : uint8_t
{
    _Classic,
    _TBC,
    _WotLK,
    _Cata,
    _Mop,
    _WoD,
    _Legion,
    _BfA,
    _Shadowlands,
    _Dragonflight,
    _TWW,
    _Midnight,
    _Unknown
};

struct ClientProtocolState
{

    ClientVersion version{ ClientVersion::_Unknown };

    int versionId() const
    {
        switch (version)
        {
            case ClientVersion::_Classic:       return 0;
            case ClientVersion::_TBC:           return 1;
            case ClientVersion::_WotLK:         return 2;
            case ClientVersion::_Cata:          return 3;
            case ClientVersion::_Mop:           return 4;
            case ClientVersion::_WoD:           return 5;
            case ClientVersion::_Legion:        return 6;
            case ClientVersion::_BfA:           return 7;
            case ClientVersion::_Shadowlands:   return 8;
            case ClientVersion::_Dragonflight:  return 9;
            case ClientVersion::_TWW:           return 10;
            case ClientVersion::_Midnight:      return 11;
            default:                            return -1;
        }
    }
};
