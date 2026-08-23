/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <cstdint>
#include <string>
#include <vector>

namespace AscEmu::Packets
{
    struct WhoPlayerEntry
    {
        uint64_t guid = 0;
        std::string name;
        std::string guildName;
        uint32_t level = 0;
        uint32_t classId = 0;
        uint32_t raceId = 0;
        uint8_t gender = 0;
        uint32_t zoneId = 0;
    };

    class SmsgWho : public ManagedPacket
    {
    public:
        std::vector<WhoPlayerEntry> players;

        SmsgWho() : SmsgWho(std::vector<WhoPlayerEntry>{})
        {
        }

        SmsgWho(std::vector<WhoPlayerEntry> players) :
            ManagedPacket(SMSG_WHO, 0),
            players(std::move(players))
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 8 + players.size() * 40;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                const uint32_t count = static_cast<uint32_t>(players.size());

                packet << uint32_t(count);
                packet << uint32_t(count);

                //////////////////////////////////////////////////////////////////////////////////////////
                // Bit part
                for (const auto& player : players)
                {
                    WoWGuid guid = player.guid;

                    packet.writeBits(player.name.length(), 6);
                    packet.writeBits(player.guildName.length(), 6);

                    packet.writeBit(guid[3]);
                    packet.writeBit(guid[7]);
                    packet.writeBit(guid[2]);
                    packet.writeBit(guid[0]);
                    packet.writeBit(guid[1]);
                    packet.writeBit(guid[5]);
                    packet.writeBit(guid[6]);
                    packet.writeBit(guid[4]);
                }
                packet.flushBits();

                //////////////////////////////////////////////////////////////////////////////////////////
                // Data part
                for (const auto& player : players)
                {
                    WoWGuid guid = player.guid;

                    packet.writeByteSeq(guid[1]);
                    packet.writeByteSeq(guid[7]);
                    packet.writeByteSeq(guid[2]);
                    packet.writeByteSeq(guid[4]);
                    packet.writeByteSeq(guid[5]);
                    packet.writeByteSeq(guid[0]);
                    packet.writeByteSeq(guid[3]);
                    packet.writeByteSeq(guid[6]);

                    packet << uint32_t(0);

                    packet.append(player.name.c_str(), player.name.length());
                    packet.append(player.guildName.c_str(), player.guildName.length());

                    packet << uint32_t(player.level);
                    packet << uint32_t(player.classId);
                    packet << uint32_t(player.raceId);
                    packet << uint8_t(player.gender);
                    packet << uint32_t(player.zoneId);
                }

                return true;
            }
            else if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet << uint64_t(0); // placeholder, patched below

                for (const auto& player : players)
                {
                    packet << player.name.c_str();

                    if (!player.guildName.empty())
                        packet << player.guildName.c_str();
                    else
                        packet << uint8_t(0);

                    packet << uint32_t(player.level);
                    packet << uint32_t(player.classId);
                    packet << uint32_t(player.raceId);
                    packet << uint8_t(player.gender);
                    packet << uint32_t(player.zoneId);
                }

                const uint32_t count = static_cast<uint32_t>(players.size());
                packet.wpos(0);
                packet << count;
                packet << count;

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
