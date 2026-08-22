/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <string>

namespace AscEmu::Packets
{
    class CmsgLfGuildAddRecruit : public ManagedPacket
    {
    public:
        uint32_t classRoles = 0;
        uint32_t guildInterests = 0;
        uint32_t availability = 0;
        WoWGuid guid;
        std::string comment;
        std::string playerName;

        CmsgLfGuildAddRecruit() : ManagedPacket(CMSG_LF_GUILD_ADD_RECRUIT, 20)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_Cata)
            {
                packet >> classRoles;
                packet >> guildInterests;
                packet >> availability;

                guid[3] = packet.readBit();
                guid[0] = packet.readBit();
                guid[6] = packet.readBit();
                guid[1] = packet.readBit();

                const uint16_t commentLength = static_cast<uint16_t>(packet.readBits(11));

                guid[5] = packet.readBit();
                guid[4] = packet.readBit();
                guid[7] = packet.readBit();

                const uint8_t nameLength = static_cast<uint8_t>(packet.readBits(7));

                guid[2] = packet.readBit();

                packet.readByteSeq(guid[4]);
                packet.readByteSeq(guid[5]);

                comment = packet.readString(commentLength);
                playerName = packet.readString(nameLength);

                packet.readByteSeq(guid[7]);
                packet.readByteSeq(guid[2]);
                packet.readByteSeq(guid[0]);
                packet.readByteSeq(guid[6]);
                packet.readByteSeq(guid[1]);
                packet.readByteSeq(guid[3]);

                return true;
            }

            return false;
        }
    };
}
