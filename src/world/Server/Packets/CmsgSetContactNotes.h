/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgSetContactNotes : public ManagedPacket
    {
    public:
        uint64_t guid;
        std::string note;

        CmsgSetContactNotes() : CmsgSetContactNotes(0, "")
        {
        }

        CmsgSetContactNotes(uint64_t guid, std::string note) :
            ManagedPacket(CMSG_SET_CONTACT_NOTES, 4),
            guid(guid),
            note(note)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            // 6.x and later: the contact, then the note with its length
            if (m_protocol.expansion >= WoW::Expansion::_WoD)
            {
                packet.readSkip<uint32_t>();        // realm of the contact
                WoWGuid128 player;
                packet >> player;
                guid = WoWGuid::fromGuid128(player).getRawGuid();
                note = packet.readString(packet.readBits(10));
                return true;
            }

            if (m_protocol.expansion >= WoW::Expansion::_Classic)
            {
                packet >> guid >> note;
                return true;
            }

            return false;
        }
    };
}
