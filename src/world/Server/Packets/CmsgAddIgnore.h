/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class CmsgAddIgnore : public ManagedPacket
    {
    public:
        std::string name;

        CmsgAddIgnore() : CmsgAddIgnore("")
        {
        }

        CmsgAddIgnore(std::string name) :
            ManagedPacket(CMSG_ADD_IGNORE, 4),
            name(name)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            // 6.x and later: the name with its length, 9.x sends the account of the player between both
            if (m_protocol.expansion >= WoW::Expansion::_WoD)
            {
                const uint32_t nameLength = packet.readBits(9);
                packet.resetBitPos();
                if (m_protocol.expansion >= WoW::Expansion::_Shadowlands)
                {
                    WoWGuid128 account;
                    packet >> account;
                }
                name = packet.readString(nameLength);
                return true;
            }

            if (m_protocol.expansion >= WoW::Expansion::_Classic)
            {
                packet >> name;
                return true;
            }

            return false;
        }
    };
}
