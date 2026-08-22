/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgItemrefundrequest : public ManagedPacket
    {
    public:
        WoWGuid itemGuid;

        CmsgItemrefundrequest() : CmsgItemrefundrequest(0)
        {
        }

        CmsgItemrefundrequest(uint64_t itemGuid) :
            ManagedPacket(CMSG_ITEMREFUNDREQUEST, 8),
            itemGuid(itemGuid)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                itemGuid[2] = packet.readBit();
                itemGuid[4] = packet.readBit();
                itemGuid[1] = packet.readBit();
                itemGuid[6] = packet.readBit();
                itemGuid[3] = packet.readBit();
                itemGuid[0] = packet.readBit();
                itemGuid[5] = packet.readBit();
                itemGuid[7] = packet.readBit();

                packet.readByteSeq(itemGuid[3]);
                packet.readByteSeq(itemGuid[5]);
                packet.readByteSeq(itemGuid[6]);
                packet.readByteSeq(itemGuid[2]);
                packet.readByteSeq(itemGuid[7]);
                packet.readByteSeq(itemGuid[0]);
                packet.readByteSeq(itemGuid[1]);
                packet.readByteSeq(itemGuid[4]);

                return true;
            }
            else if (m_protocol.expansion >= WoW::Expansion::_WotLK)
            {
                uint64_t unpackedGuid;
                packet >> unpackedGuid;
                itemGuid.init(unpackedGuid);

                return true;
            }

            return false;
        }
    };
}
