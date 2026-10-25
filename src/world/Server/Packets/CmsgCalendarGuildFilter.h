/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CalendarPacketData.hpp"

#include <cstdint>

namespace AscEmu::Packets
{
    // the members of the guild the event planner invites at once
    class CmsgCalendarGuildFilter : public ManagedPacket
    {
    public:
        uint32_t minLevel = 1;
        uint32_t maxLevel = 0;
        uint32_t maxRank = 0;               // the lowest guild rank invited, 0 is the guild master

        CmsgCalendarGuildFilter() :
            ManagedPacket(CMSG_CALENDAR_GUILD_FILTER, 3)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet >> minLevel >> maxLevel >> maxRank;
                return true;
            }

            if (m_protocol.expansion >= WoW::Expansion::_WoD)
            {
                // 8.x and later name the club first
                if (m_protocol.expansion >= WoW::Expansion::_BfA)
                    packet.readSkip<uint64_t>();

                uint8_t min;
                uint8_t max;
                uint8_t rank;
                packet >> min >> max >> rank;

                minLevel = min;
                maxLevel = max;
                maxRank = rank;
                return true;
            }

            return false;
        }
    };
}
