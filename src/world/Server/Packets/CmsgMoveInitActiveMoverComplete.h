/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgMoveInitActiveMoverComplete : public ManagedPacket
    {
    public:
        // client clock when its own create block arrived
        uint32_t ticks;

        CmsgMoveInitActiveMoverComplete() : CmsgMoveInitActiveMoverComplete(0)
        {
        }

        CmsgMoveInitActiveMoverComplete(uint32_t ticks) :
            ManagedPacket(CMSG_MOVE_INIT_ACTIVE_MOVER_COMPLETE, sizeof(uint32_t)),
            ticks(ticks)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> ticks;
            return true;
        }
    };
}
