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
    struct SmsgCalendarFilterGuildEntry
    {
        uint64_t guid = 0;
        uint8_t level = 0; // Mop only: level of the member at the time of the mass invite
    };

    class SmsgCalendarFilterGuild : public ManagedPacket
    {
    public:
        std::vector<SmsgCalendarFilterGuildEntry> guilds;

        SmsgCalendarFilterGuild() : SmsgCalendarFilterGuild(std::vector<SmsgCalendarFilterGuildEntry>{})
        {
        }

        SmsgCalendarFilterGuild(std::vector<SmsgCalendarFilterGuildEntry> guilds) :
            ManagedPacket(SMSG_CALENDAR_FILTER_GUILD, 4 + guilds.size() * 9),
            guilds(std::move(guilds))
        {
        }

    protected:
        size_t expectedSize() const override { return 4 + guilds.size() * 9; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                packet << uint32_t(guilds.size());

                for (const auto& g : guilds)
                {
                    packet.appendPackGuid(g.guid);
                    packet << uint8_t(0);
                }

                return true;
            }
            else if (m_protocol.isMop())
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

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
