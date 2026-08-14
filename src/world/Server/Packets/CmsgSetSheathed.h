/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgSetSheathed : public ManagedPacket
    {
    public:
        uint32_t type;
        bool hasData = false;
        CmsgSetSheathed() : CmsgSetSheathed(0)
        {
        }

        CmsgSetSheathed(uint32_t type) :
            ManagedPacket(CMSG_SETSHEATHED, sizeof(uint32_t)),
            type(type)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> type;

            if (m_protocol.isMop())
                hasData = packet.readBit();

            return true;
        }
    };
}
