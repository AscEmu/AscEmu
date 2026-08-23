/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Macros/PlayerMacros.hpp"

#include <array>
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgShowTaxiNodes : public ManagedPacket
    {
    public:
        uint64_t guid;
        uint32_t nearestNode;
        std::array<uint8_t, DBC_TAXI_MASK_SIZE> taxiMask;

        SmsgShowTaxiNodes(uint64_t guid, uint32_t nearestNode, std::array<uint8_t, DBC_TAXI_MASK_SIZE> const& taxiMask) :
            ManagedPacket(SMSG_SHOWTAXINODES, 0),
            guid(guid),
            nearestNode(nearestNode),
            taxiMask(taxiMask)
        {
        }

    protected:
size_t expectedSize() const override
        {
            return m_protocol.expansion < WoW::Expansion::_Cata ?
                4 + 8 + 4 + (4 * DBC_TAXI_MASK_SIZE) :
                4 + 8 + 4 + 4 + DBC_TAXI_MASK_SIZE;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                const WoWGuid npcGuid(guid);

                packet.writeBit(true); // unk

                packet.writeBit(npcGuid[3]);
                packet.writeBit(npcGuid[0]);
                packet.writeBit(npcGuid[4]);
                packet.writeBit(npcGuid[2]);
                packet.writeBit(npcGuid[1]);
                packet.writeBit(npcGuid[7]);
                packet.writeBit(npcGuid[6]);
                packet.writeBit(npcGuid[5]);

                packet.writeBits(DBC_TAXI_MASK_SIZE, 24);
                packet.flushBits();

                packet.writeByteSeq(npcGuid[0]);
                packet.writeByteSeq(npcGuid[3]);

                packet << nearestNode;

                packet.writeByteSeq(npcGuid[5]);
                packet.writeByteSeq(npcGuid[2]);
                packet.writeByteSeq(npcGuid[6]);
                packet.writeByteSeq(npcGuid[1]);
                packet.writeByteSeq(npcGuid[7]);
                packet.writeByteSeq(npcGuid[4]);

                for (auto tMask : taxiMask)
                    packet << static_cast<uint8_t>(tMask);

                return true;
            }

            packet << uint32_t(1) << guid << nearestNode;
            if (m_protocol.isCata())
            {
                packet << uint32_t(DBC_TAXI_MASK_SIZE);
            }
            for (auto tMask : taxiMask)
            {
                if (m_protocol.expansion < WoW::Expansion::_Cata)
                {
                    packet << static_cast<uint32_t>(tMask);
                }
                else
                {
                    packet << static_cast<uint8_t>(tMask);
                }
            }

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
