/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Objects/MovementInfo.hpp"

#include <cstdint>
#include <utility>

namespace AscEmu::Packets
{
    // movement info under any movement opcode, written with the descriptor of the receiving client
    class MsgMovementInfo : public ManagedPacket
    {
    public:
        MovementInfo mi;
        bool withGuid = true;

        MsgMovementInfo(uint16_t opcode, MovementInfo mi, bool withGuid = true) :
            ManagedPacket(opcode, 0),
            mi(std::move(mi)),
            withGuid(withGuid)
        {
        }

    protected:
        size_t expectedSize() const override { return sizeof(mi); }

        bool internalSerialise(WorldPacket& packet) override
        {
            mi.write(packet, m_protocol, withGuid);
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
