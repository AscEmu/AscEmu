/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class CmsgGuildNewsUpdateSticky : public ManagedPacket
    {
    public:
        uint32_t newsId = 0;
        bool isSticky = false;
        WoWGuid guid;

        CmsgGuildNewsUpdateSticky() : ManagedPacket(CMSG_GUILD_NEWS_UPDATE_STICKY, 12)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isCata())
            {
                packet >> newsId;

                guid[2] = packet.readBit();
                guid[4] = packet.readBit();
                guid[3] = packet.readBit();
                guid[0] = packet.readBit();

                isSticky = packet.readBit();

                guid[6] = packet.readBit();
                guid[7] = packet.readBit();
                guid[1] = packet.readBit();
                guid[5] = packet.readBit();

                packet.readByteSeq(guid[6]);
                packet.readByteSeq(guid[2]);
                packet.readByteSeq(guid[1]);
                packet.readByteSeq(guid[0]);
                packet.readByteSeq(guid[5]);
                packet.readByteSeq(guid[3]);
                packet.readByteSeq(guid[7]);
                packet.readByteSeq(guid[4]);

                return true;
            }
            else if (m_protocol.isMop())
            {
                packet >> newsId;

                guid[6] = packet.readBit();
                guid[0] = packet.readBit();

                isSticky = packet.readBit();

                guid[2] = packet.readBit();
                guid[7] = packet.readBit();
                guid[5] = packet.readBit();
                guid[4] = packet.readBit();
                guid[3] = packet.readBit();
                guid[1] = packet.readBit();

                packet.readByteSeq(guid[5]);
                packet.readByteSeq(guid[4]);
                packet.readByteSeq(guid[0]);
                packet.readByteSeq(guid[1]);
                packet.readByteSeq(guid[6]);
                packet.readByteSeq(guid[2]);
                packet.readByteSeq(guid[3]);
                packet.readByteSeq(guid[7]);

                return true;
            }

            return false;
        }
    };
}
