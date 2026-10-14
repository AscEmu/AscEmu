/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgPing : public ManagedPacket
    {
    public:
        uint32_t serial{0};
        uint32_t latency{0};

        CmsgPing() : ManagedPacket(CMSG_PING, sizeof(uint32_t) * 2)
        {
        }

    protected:
        size_t expectedSize() const override { return sizeof(uint32_t) * 2; }

        bool internalSerialise(WorldPacket& /*packet*/) override { return false; }

        bool internalDeserialise(WorldPacket& packet) override
        {
            if (packet.remaining() != sizeof(uint32_t) * 2)
                return false;

            if (m_protocol.isForever() || m_protocol.expansion <= WoW::Expansion::_Cata)
                packet >> serial >> latency;
            else
                packet >> latency >> serial;

            return true;
        }
    };
}
