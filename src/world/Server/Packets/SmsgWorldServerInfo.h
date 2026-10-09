/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgWorldServerInfo : public ManagedPacket
    {
    public:
        SmsgWorldServerInfo() : ManagedPacket(SMSG_WORLD_SERVER_INFO, 4 + 4 + 1 + 1)
        {
        }

    protected:
        size_t expectedSize() const override { return 4 + 4 + 1 + 1; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isWoD())
            {
                // difficulty, tournament realm, weekly reset, cross realm pvp alert, restricted level, restricted money,
                // ineligible for loot, instance group size
                packet << uint32_t(0);
                packet << uint8_t(0);
                packet << uint32_t(0);
                packet.writeBit(false);
                packet.writeBit(false);
                packet.writeBit(false);
                packet.writeBit(false);
                packet.writeBit(false);
                packet.flushBits();
                return true;
            }

            if (m_protocol.isLegion() || m_protocol.isBfA())
            {
                // difficulty, tournament realm, cross realm pvp alert, 8.x: block exiting loading screen, restricted level,
                // restricted money, instance group size
                packet << uint32_t(0);
                packet << uint8_t(0);
                packet.writeBit(false);
                if (m_protocol.isBfA())
                    packet.writeBit(false);
                packet.writeBit(false);
                packet.writeBit(false);
                packet.writeBit(false);
                packet.flushBits();
                return true;
            }

            if (m_protocol.isMop())
            {
                packet.writeBit(0);
                packet.writeBit(0);
                packet.writeBit(0);
                packet.writeBit(0);
                packet.flushBits();

                packet << uint8_t(0);
                packet << uint32_t(0);       // reset weekly quest time
                packet << uint32_t(0);
            }
            else
            {
                return false;
            }

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
