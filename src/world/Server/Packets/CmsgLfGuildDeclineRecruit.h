/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class CmsgLfGuildDeclineRecruit : public ManagedPacket
    {
    public:
        WoWGuid playerGuid;

        CmsgLfGuildDeclineRecruit() : ManagedPacket(CMSG_LF_GUILD_DECLINE_RECRUIT, 8)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isCata())
            {
                playerGuid[1] = packet.readBit();
                playerGuid[4] = packet.readBit();
                playerGuid[5] = packet.readBit();
                playerGuid[2] = packet.readBit();
                playerGuid[6] = packet.readBit();
                playerGuid[7] = packet.readBit();
                playerGuid[0] = packet.readBit();
                playerGuid[3] = packet.readBit();

                packet.readByteSeq(playerGuid[5]);
                packet.readByteSeq(playerGuid[7]);
                packet.readByteSeq(playerGuid[2]);
                packet.readByteSeq(playerGuid[3]);
                packet.readByteSeq(playerGuid[4]);
                packet.readByteSeq(playerGuid[1]);
                packet.readByteSeq(playerGuid[0]);
                packet.readByteSeq(playerGuid[6]);

                return true;
            }
            else if (m_protocol.isMop())
            {
                playerGuid[6] = packet.readBit();
                playerGuid[7] = packet.readBit();
                playerGuid[3] = packet.readBit();
                playerGuid[1] = packet.readBit();
                playerGuid[2] = packet.readBit();
                playerGuid[0] = packet.readBit();
                playerGuid[4] = packet.readBit();
                playerGuid[5] = packet.readBit();

                packet.readByteSeq(playerGuid[0]);
                packet.readByteSeq(playerGuid[7]);
                packet.readByteSeq(playerGuid[1]);
                packet.readByteSeq(playerGuid[6]);
                packet.readByteSeq(playerGuid[4]);
                packet.readByteSeq(playerGuid[3]);
                packet.readByteSeq(playerGuid[5]);
                packet.readByteSeq(playerGuid[2]);

                return true;
            }

            return false;
        }
    };
}
