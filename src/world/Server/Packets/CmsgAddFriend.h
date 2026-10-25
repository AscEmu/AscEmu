/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class CmsgAddFriend : public ManagedPacket
    {
    public:
        std::string name;
        std::string note;

        CmsgAddFriend() : CmsgAddFriend("", "")
        {
        }

        CmsgAddFriend(std::string name, std::string note) :
            ManagedPacket(CMSG_ADD_FRIEND, 4),
            name(name),
            note(note)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            // 6.x and later: the lengths of name and note, then both; the note takes 9 bits from 10.x on
            if (m_protocol.expansion >= WoW::Expansion::_WoD)
            {
                const uint32_t nameLength = packet.readBits(9);
                const uint32_t noteLength = packet.readBits(m_protocol.expansion >= WoW::Expansion::_Dragonflight ? 9 : 10);
                packet.resetBitPos();
                name = packet.readString(nameLength);
                note = packet.readString(noteLength);
                return true;
            }

            if (m_protocol.expansion >= WoW::Expansion::_Classic)
            {
                packet >> name >> note;
                return true;
            }

            return false;
        }
    };
}
