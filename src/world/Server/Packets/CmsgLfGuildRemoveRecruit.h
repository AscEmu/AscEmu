/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class CmsgLfGuildRemoveRecruit : public ManagedPacket
    {
    public:
        WoWGuid guildGuid;

        CmsgLfGuildRemoveRecruit() : ManagedPacket(CMSG_LF_GUILD_REMOVE_RECRUIT, 8)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_Cata)
            {
                guildGuid[0] = packet.readBit();
                guildGuid[4] = packet.readBit();
                guildGuid[3] = packet.readBit();
                guildGuid[5] = packet.readBit();
                guildGuid[7] = packet.readBit();
                guildGuid[6] = packet.readBit();
                guildGuid[2] = packet.readBit();
                guildGuid[1] = packet.readBit();

                packet.readByteSeq(guildGuid[4]);
                packet.readByteSeq(guildGuid[0]);
                packet.readByteSeq(guildGuid[3]);
                packet.readByteSeq(guildGuid[6]);
                packet.readByteSeq(guildGuid[5]);
                packet.readByteSeq(guildGuid[1]);
                packet.readByteSeq(guildGuid[2]);
                packet.readByteSeq(guildGuid[7]);

                return true;
            }

            return false;
        }
    };
}
