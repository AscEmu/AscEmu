/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu { namespace Packets
{
    class CmsgActivatetaxiexpress : public ManagedPacket
    {
    public:
        uint64_t guid;
        uint32_t nodeCount;
        std::vector<uint32_t> pathParts = {};

        CmsgActivatetaxiexpress() : CmsgActivatetaxiexpress(0, 0)
        {
        }

        CmsgActivatetaxiexpress(uint64_t guid, uint32_t nodeCount) :
            ManagedPacket(CMSG_ACTIVATETAXIEXPRESS, 9),
            guid(guid),
            nodeCount(nodeCount)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> guid >> nodeCount;
            if (nodeCount < 2)
                return false;

            if (nodeCount > 10)
                return false;

            for (uint32_t i = 0; i < nodeCount; ++i)
                pathParts.push_back(packet.read<uint32_t>());

            return true;
        }
    };
}}
