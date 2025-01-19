/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <array>
#include <cstdint>

#include "AEVersion.hpp"
#include "Macros/PlayerMacros.hpp"
#include "ManagedPacket.h"

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
#if VERSION_STRING < Cata
        size_t expectedSize() const override { return 4 + 8 + 4 + 4 * DBC_TAXI_MASK_SIZE; }
#else   
        size_t expectedSize() const override { return 4 + 8 + 4 + 4 + DBC_TAXI_MASK_SIZE; }
#endif

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << uint32_t(1) << guid << nearestNode;
#if VERSION_STRING >= Cata
            packet << uint32_t(DBC_TAXI_MASK_SIZE);
#endif
            for (auto tMask : taxiMask)
            {
#if VERSION_STRING < Cata
                packet << static_cast<uint32_t>(tMask);
#else
                packet << static_cast<uint8_t>(tMask);
#endif
            }

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
