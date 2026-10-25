/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CalendarPacketData.hpp"

#include <cstdint>
#include <vector>

namespace AscEmu::Packets
{
    struct SmsgCalendarFilterGuildEntry
    {
        uint64_t guid = 0;
        uint8_t level = 0;
    };

    // the players the event planner invites at once: members of the guild or (3.x, 4.x) of an arena team
    class SmsgCalendarFilterGuild : public ManagedPacket
    {
    public:
        std::vector<SmsgCalendarFilterGuildEntry> guilds;
        bool arenaTeam = false;

        SmsgCalendarFilterGuild() : SmsgCalendarFilterGuild(std::vector<SmsgCalendarFilterGuildEntry>{})
        {
        }

        SmsgCalendarFilterGuild(std::vector<SmsgCalendarFilterGuildEntry> guilds, bool arenaTeam = false) :
            ManagedPacket(arenaTeam ? SMSG_CALENDAR_ARENA_TEAM : SMSG_CALENDAR_FILTER_GUILD, 4 + guilds.size() * 9),
            guilds(std::move(guilds)),
            arenaTeam(arenaTeam)
        {
        }

    protected:
        size_t expectedSize() const override { return 4 + guilds.size() * 9; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_WoD)
            {
                packet << uint32_t(guilds.size());
                for (const auto& g : guilds)
                {
                    packet << WoWGuid(g.guid).toGuid128(m_protocol.realmId, 0);
                    packet << g.level;
                }
                return true;
            }

            if (m_protocol.isMop())
            {
                ByteBuffer bufferData(guilds.size() * 9);

                packet.writeBits(guilds.size(), 23);
                for (const auto& g : guilds)
                {
                    const WoWGuid guid(g.guid);
                    packet.writeBit(guid[1]);
                    packet.writeBit(guid[7]);
                    packet.writeBit(guid[5]);
                    packet.writeBit(guid[0]);
                    packet.writeBit(guid[4]);
                    packet.writeBit(guid[3]);
                    packet.writeBit(guid[6]);
                    packet.writeBit(guid[2]);

                    bufferData << uint8_t(g.level);
                    bufferData.writeByteSeq(guid[3]);
                    bufferData.writeByteSeq(guid[5]);
                    bufferData.writeByteSeq(guid[4]);
                    bufferData.writeByteSeq(guid[6]);
                    bufferData.writeByteSeq(guid[7]);
                    bufferData.writeByteSeq(guid[0]);
                    bufferData.writeByteSeq(guid[2]);
                    bufferData.writeByteSeq(guid[1]);
                }

                packet.flushBits();
                packet.append(bufferData);
                return true;
            }

            if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet << uint32_t(guilds.size());
                for (const auto& g : guilds)
                {
                    packet.appendPackGuid(g.guid);
                    packet << g.level;
                }
                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
