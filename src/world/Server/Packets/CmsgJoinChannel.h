/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgJoinChannel : public ManagedPacket
    {
    public:
        utf8_string channelName;
        std::string password;
        uint32_t dbcId;
        uint16_t unk;

        CmsgJoinChannel() : CmsgJoinChannel(0, 0)
        {
        }

        CmsgJoinChannel(uint16_t unk, uint32_t dbcId, std::string channelName = "", std::string password = "") :
            ManagedPacket(CMSG_JOIN_CHANNEL, 4),
            channelName(channelName),
            password(password),
            dbcId(dbcId),
            unk(unk)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion <= WoW::Expansion::_WotLK)
            {
                packet >> dbcId >> unk >> channelName >> password;
            }
            else if (m_protocol.expansion == WoW::Expansion::_Cata)
            {
                packet >> dbcId;

                packet.readBit();       // has voice
                packet.readBit();       // zone update

                const uint32_t channelLength = packet.readBits(8);
                const uint32_t passwordLength = packet.readBits(8);

                channelName = packet.readString(channelLength);
                password = packet.readString(passwordLength);
            }
            else if (m_protocol.expansion == WoW::Expansion::_Mop)
            {
                packet >> dbcId;

                packet.readBit();       // has voice

                const uint32_t channelLength = packet.readBits(7);
                const uint32_t passwordLength = packet.readBits(7);

                packet.readBit();       // zone update

                channelName = packet.readString(channelLength);
                password = packet.readString(passwordLength);
            }

            return true;
        }
    };
}
