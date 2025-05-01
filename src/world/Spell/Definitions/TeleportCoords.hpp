/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#pragma pack(push,1)
struct TeleportCoords
{
    uint32_t id;
    uint32_t mapId;
    float x;
    float y;
    float z;
};
#pragma pack(pop)
