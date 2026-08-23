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
            if (m_protocol.expansion >= WoW::Expansion::_Classic)
            {
                packet >> guid >> note;
                return true;
            }

            return false;
        }
    };
}
