/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Network/WorldPacket.hpp"
#include <cstdint>

namespace AscEmu::Packets
{
    // 6.x and later: item packets start with the inventory slots the client changed, the server does not need them
    inline void skipInvUpdate(WorldPacket& packet)
    {
        const uint32_t count = packet.readBits(2);
        packet.resetBitPos();

        for (uint32_t i = 0; i < count; ++i)
        {
            packet.readSkip<uint8_t>();         // container slot
            packet.readSkip<uint8_t>();         // slot
        }
    }
}
