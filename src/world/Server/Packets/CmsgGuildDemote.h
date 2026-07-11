/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class CmsgGuildDemote : public ManagedPacket
    {
    public:
        std::string name;
        WoWGuid guid;

        CmsgGuildDemote() : CmsgGuildDemote(0)
        {
        }

        CmsgGuildDemote(WoWGuid guid) :
            ManagedPacket(CMSG_GUILD_DEMOTE, 8),
            guid(guid)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion <= WoW::Expansion::_WotLK)
            {
                packet >> name;
            }
            else
            {
                guid[7] = packet.readBit();
                guid[1] = packet.readBit();
                guid[5] = packet.readBit();
                guid[6] = packet.readBit();
                guid[2] = packet.readBit();
                guid[3] = packet.readBit();
                guid[0] = packet.readBit();
                guid[4] = packet.readBit();

                packet.readByteSeq(guid[1]);
                packet.readByteSeq(guid[2]);
                packet.readByteSeq(guid[7]);
                packet.readByteSeq(guid[5]);
                packet.readByteSeq(guid[6]);
                packet.readByteSeq(guid[0]);
                packet.readByteSeq(guid[4]);
                packet.readByteSeq(guid[3]);
            }
            return true;
        }
    };
}
