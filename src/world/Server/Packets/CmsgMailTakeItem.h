/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgMailTakeItem : public ManagedPacket
    {
    public:
        WoWGuid gobjGuid;
        uint32_t messageId;
        uint32_t lowGuid;

        CmsgMailTakeItem() : CmsgMailTakeItem(0, 0, 0)
        {
        }

        CmsgMailTakeItem(uint64_t gobjGuid, uint32_t messageId, uint32_t lowGuid) :
            ManagedPacket(CMSG_MAIL_TAKE_ITEM, 16),
            gobjGuid(gobjGuid),
            messageId(messageId),
            lowGuid(lowGuid)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                packet >> messageId;
                packet >> lowGuid;

                gobjGuid[6] = packet.readBit();
                gobjGuid[5] = packet.readBit();
                gobjGuid[2] = packet.readBit();
                gobjGuid[3] = packet.readBit();
                gobjGuid[0] = packet.readBit();
                gobjGuid[1] = packet.readBit();
                gobjGuid[4] = packet.readBit();
                gobjGuid[7] = packet.readBit();

                packet.readByteSeq(gobjGuid[0]);
                packet.readByteSeq(gobjGuid[1]);
                packet.readByteSeq(gobjGuid[4]);
                packet.readByteSeq(gobjGuid[2]);
                packet.readByteSeq(gobjGuid[5]);
                packet.readByteSeq(gobjGuid[6]);
                packet.readByteSeq(gobjGuid[3]);
                packet.readByteSeq(gobjGuid[7]);

                return true;
            }
            else if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                uint64_t unpackedGuid;
                packet >> unpackedGuid >> messageId >> lowGuid;
                gobjGuid = WoWGuid(unpackedGuid);

                return true;
            }

            return false;
        }
    };
}
