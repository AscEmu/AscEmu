/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>
#include <vector>

namespace AscEmu::Packets
{
    class SmsgCalendarFilterGuild : public ManagedPacket
    {
    public:
        std::vector<uint64_t> guids;

        SmsgCalendarFilterGuild() : SmsgCalendarFilterGuild(std::vector<uint64_t>{})
        {
        }

        SmsgCalendarFilterGuild(std::vector<uint64_t> guids) :
            ManagedPacket(SMSG_CALENDAR_FILTER_GUILD, 4 + guids.size() * 9),
            guids(std::move(guids))
        {
        }

    protected:
        size_t expectedSize() const override { return 4 + guids.size() * 9; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion <= WoW::Expansion::_Mop)
            {
                packet << uint32_t(guids.size());

                for (const auto& guid : guids)
                {
                    packet.appendPackGuid(guid);
                    packet << uint8_t(0);
                }

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
