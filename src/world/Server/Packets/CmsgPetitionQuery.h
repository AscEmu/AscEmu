/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgPetitionQuery : public ManagedPacket
    {
    public:
        uint32_t charterId;
        WoWGuid itemGuid;

        CmsgPetitionQuery() : CmsgPetitionQuery(0, 0)
        {
        }

        CmsgPetitionQuery(uint32_t charterId, uint64_t itemGuid) :
            ManagedPacket(CMSG_PETITION_QUERY, 12),
            charterId(charterId),
            itemGuid(itemGuid)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                uint64_t unpackedGuid;
                packet >> charterId >> unpackedGuid;
                itemGuid.init(unpackedGuid);
                return true;
            }
            else if (m_protocol.isMop())
            {
                packet >> charterId;

                itemGuid[2] = packet.readBit();
                itemGuid[3] = packet.readBit();
                itemGuid[1] = packet.readBit();
                itemGuid[0] = packet.readBit();
                itemGuid[4] = packet.readBit();
                itemGuid[7] = packet.readBit();
                itemGuid[6] = packet.readBit();
                itemGuid[5] = packet.readBit();

                packet.readByteSeq(itemGuid[0]);
                packet.readByteSeq(itemGuid[4]);
                packet.readByteSeq(itemGuid[7]);
                packet.readByteSeq(itemGuid[5]);
                packet.readByteSeq(itemGuid[1]);
                packet.readByteSeq(itemGuid[6]);
                packet.readByteSeq(itemGuid[3]);
                packet.readByteSeq(itemGuid[2]);
                return true;
            }

            return false;
        }
    };
}
