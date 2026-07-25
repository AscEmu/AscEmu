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
            packet << uint32_t(1) << guid << nearestNode;
            if (m_protocol.expansion >= WoW::Expansion::_Cata)
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
