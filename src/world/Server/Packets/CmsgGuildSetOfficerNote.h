/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class CmsgGuildSetOfficerNote : public ManagedPacket
    {
    public:
        std::string targetName;
        std::string note;

        CmsgGuildSetOfficerNote() : CmsgGuildSetOfficerNote("", "")
        {
        }

        CmsgGuildSetOfficerNote(std::string targetName, std::string note) :
            ManagedPacket(CMSG_GUILD_SET_OFFICER_NOTE, 1),
            targetName(targetName),
            note(note)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion <= WoW::Expansion::_WotLK)
            {
                packet >> targetName >> note;
                return true;
            }

            return false;
        }
    };
}
