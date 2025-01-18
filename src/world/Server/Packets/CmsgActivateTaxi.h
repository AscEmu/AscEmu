/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu::Packets
{
    class CmsgActivateTaxi : public ManagedPacket
    {
    public:
        uint64_t guid;
        std::vector<uint32_t> nodes;

        CmsgActivateTaxi() : CmsgActivateTaxi(0, {0, 0})
        {
            nodes.resize(2);
        }

        CmsgActivateTaxi(uint64_t guid, std::vector<uint32_t> nodes) :
            ManagedPacket(CMSG_ACTIVATE_TAXI, 8 + 4 + 4),
            guid(guid),
            nodes(nodes)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> guid >> nodes[0] >> nodes[1];
            return true;
        }
    };
}
