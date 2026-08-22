/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class CmsgLfgLeave : public ManagedPacket
    {
    public:
        uint32_t ticketType = 0;
        uint32_t ticketId = 0;
        uint32_t ticketTime = 0;
        uint32_t slotOrQueueId = 0;
        WoWGuid requesterGuid;

        CmsgLfgLeave() : ManagedPacket(CMSG_LFG_LEAVE, 0)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                if (packet.remaining() < sizeof(uint32_t) * 4)
                    return false;

                packet >> ticketType;
                packet >> ticketId;
                packet >> ticketTime;
                packet >> slotOrQueueId;

                requesterGuid[6] = packet.readBit();
                requesterGuid[0] = packet.readBit();
                requesterGuid[2] = packet.readBit();
                requesterGuid[3] = packet.readBit();
                requesterGuid[1] = packet.readBit();
                requesterGuid[5] = packet.readBit();
                requesterGuid[4] = packet.readBit();
                requesterGuid[7] = packet.readBit();

                packet.readByteSeq(requesterGuid[2]);
                packet.readByteSeq(requesterGuid[0]);
                packet.readByteSeq(requesterGuid[4]);
                packet.readByteSeq(requesterGuid[6]);
                packet.readByteSeq(requesterGuid[3]);
                packet.readByteSeq(requesterGuid[1]);
                packet.readByteSeq(requesterGuid[5]);
                packet.readByteSeq(requesterGuid[7]);

                return true;
            }
            else if (m_protocol.isCata())
            {
                packet.readSkip<uint32_t>();
                packet.readSkip<uint32_t>();
                packet.readSkip<uint32_t>();
                packet.readSkip<uint32_t>();

                requesterGuid[4] = packet.readBit();
                requesterGuid[5] = packet.readBit();
                requesterGuid[0] = packet.readBit();
                requesterGuid[6] = packet.readBit();
                requesterGuid[2] = packet.readBit();
                requesterGuid[7] = packet.readBit();
                requesterGuid[1] = packet.readBit();
                requesterGuid[3] = packet.readBit();

                packet.readByteSeq(requesterGuid[7]);
                packet.readByteSeq(requesterGuid[4]);
                packet.readByteSeq(requesterGuid[3]);
                packet.readByteSeq(requesterGuid[2]);
                packet.readByteSeq(requesterGuid[6]);
                packet.readByteSeq(requesterGuid[0]);
                packet.readByteSeq(requesterGuid[1]);
                packet.readByteSeq(requesterGuid[5]);

                return true;
            }
            else if (m_protocol.expansion >= WoW::Expansion::_WotLK)
            {
                return true;
            }

            return false;
        }
    };
}
