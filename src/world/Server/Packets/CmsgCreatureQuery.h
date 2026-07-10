/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgCreatureQuery : public ManagedPacket
    {
    public:
        uint32_t entry;
        WoWGuid guid;

        CmsgCreatureQuery() : CmsgCreatureQuery(0, 0)
        {
        }

        CmsgCreatureQuery(uint32_t entry, uint64_t guid) :
            ManagedPacket(CMSG_CREATURE_QUERY, 4),
            entry(entry),
            guid(guid)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                uint64_t unpacked_guid;
                packet >> entry >> unpacked_guid;
                guid.init(unpacked_guid);
            }
            else // Mop
            {
                packet >> entry;
            }
            return true;
        }
    };
}
