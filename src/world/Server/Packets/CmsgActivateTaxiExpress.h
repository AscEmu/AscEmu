/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgActivateTaxiExpress : public ManagedPacket
    {
    public:
        WoWGuid guid;
        uint32_t nodeCount;
        std::vector<uint32_t> pathParts = {};

        CmsgActivateTaxiExpress() : CmsgActivateTaxiExpress(0, 0)
        {
        }

        CmsgActivateTaxiExpress(uint64_t guid, uint32_t nodeCount) :
            ManagedPacket(CMSG_ACTIVATE_TAXI_EXPRESS, 9),
            guid(guid),
            nodeCount(nodeCount)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                guid[6] = packet.readBit();
                guid[7] = packet.readBit();

                nodeCount = packet.readBits(22);

                guid[2] = packet.readBit();
                guid[0] = packet.readBit();
                guid[4] = packet.readBit();
                guid[3] = packet.readBit();
                guid[1] = packet.readBit();
                guid[5] = packet.readBit();

                packet.readByteSeq(guid[2]);
                packet.readByteSeq(guid[7]);
                packet.readByteSeq(guid[1]);

                if (nodeCount < 2 || nodeCount > 10)
                    return false;

                for (uint32_t i = 0; i < nodeCount; ++i)
                    pathParts.push_back(packet.read<uint32_t>());

                packet.readByteSeq(guid[0]);
                packet.readByteSeq(guid[5]);
                packet.readByteSeq(guid[3]);
                packet.readByteSeq(guid[6]);
                packet.readByteSeq(guid[4]);

                return true;
            }
            else if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                uint64_t unpackedGuid;
                packet >> unpackedGuid >> nodeCount;
                guid = WoWGuid(unpackedGuid);

                if (nodeCount < 2)
                    return false;

                if (nodeCount > 10)
                    return false;

                for (uint32_t i = 0; i < nodeCount; ++i)
                    pathParts.push_back(packet.read<uint32_t>());

                return true;
            }

            return false;
        }
    };
}
