/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>
#include <ctime>
#include <vector>

namespace AscEmu::Packets
{
    struct GuildNewsEntryData
    {
        uint64_t playerGuid = 0;
        uint32_t flags = 0;
        uint32_t value = 0;
        uint32_t guid = 0;
        uint32_t type = 0;
        time_t timestamp = 0;
    };

    class SmsgGuildNewsUpdate : public ManagedPacket
    {
    public:
        std::vector<GuildNewsEntryData> entries;

        SmsgGuildNewsUpdate() : SmsgGuildNewsUpdate(std::vector<GuildNewsEntryData>{})
        {
        }

        SmsgGuildNewsUpdate(std::vector<GuildNewsEntryData> entries) :
            ManagedPacket(SMSG_GUILD_NEWS_UPDATE, (21 + entries.size() * (26 + 8)) / 8 + (8 + 6 * 4) * entries.size()),
            entries(std::move(entries))
        {
        }

    protected:
        size_t expectedSize() const override { return (21 + entries.size() * (26 + 8)) / 8 + (8 + 6 * 4) * entries.size(); }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Cata)
            {
                return false;
            }
            else if (m_protocol.isCata())
            {
                packet.writeBits(entries.size(), 21);

                for (const auto& entry : entries)
                {
                    WoWGuid guid = entry.playerGuid;

                    packet.writeBits(0, 26); // guild achievements

                    packet.writeBit(guid[7]);
                    packet.writeBit(guid[0]);
                    packet.writeBit(guid[6]);
                    packet.writeBit(guid[5]);
                    packet.writeBit(guid[4]);
                    packet.writeBit(guid[3]);
                    packet.writeBit(guid[1]);
                    packet.writeBit(guid[2]);
                }

                packet.flushBits();

                for (const auto& entry : entries)
                {
                    WoWGuid guid = entry.playerGuid;
                    packet.writeByteSeq(guid[5]);

                    packet << uint32_t(entry.flags);
                    packet << uint32_t(entry.value);
                    packet << uint32_t(0);

                    packet.writeByteSeq(guid[7]);
                    packet.writeByteSeq(guid[6]);
                    packet.writeByteSeq(guid[2]);
                    packet.writeByteSeq(guid[3]);
                    packet.writeByteSeq(guid[0]);
                    packet.writeByteSeq(guid[4]);
                    packet.writeByteSeq(guid[1]);

                    packet << uint32_t(entry.guid);
                    packet << uint32_t(entry.type);

                    packet.appendPackedTime(entry.timestamp);
                }

                return true;
            }
            else if (m_protocol.isMop())
            {
                packet.writeBits(entries.size(), 19);

                for (const auto& entry : entries)
                {
                    WoWGuid guid = entry.playerGuid;

                    packet.writeBit(guid[3]);
                    packet.writeBits(0, 24); // guild achievements
                    packet.writeBit(guid[4]);
                    packet.writeBit(guid[5]);
                    packet.writeBit(guid[6]);
                    packet.writeBit(guid[0]);
                    packet.writeBit(guid[7]);
                    packet.writeBit(guid[2]);
                    packet.writeBit(guid[1]);
                }

                packet.flushBits();

                for (const auto& entry : entries)
                {
                    WoWGuid guid = entry.playerGuid;

                    packet.writeByteSeq(guid[2]);
                    packet << uint32_t(entry.value);
                    packet.writeByteSeq(guid[1]);
                    packet.writeByteSeq(guid[7]);
                    packet.writeByteSeq(guid[4]);
                    packet.writeByteSeq(guid[3]);
                    packet.writeByteSeq(guid[5]);
                    packet << uint32_t(0);
                    packet << uint32_t(entry.type);
                    packet << uint32_t(entry.flags);
                    packet.writeByteSeq(guid[6]);
                    packet.appendPackedTime(entry.timestamp);
                    packet << uint32_t(entry.guid);
                    packet.writeByteSeq(guid[0]);
                }

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
