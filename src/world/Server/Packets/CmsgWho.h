/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgWho : public ManagedPacket
    {
    public:
        uint32_t min_level;
        uint32_t max_level;
        uint32_t class_mask;
        uint32_t race_mask;
        uint32_t zone_count;
        uint32_t name_count;
        std::string player_name;
        std::string guild_name;

        uint32_t zones[10];
        std::string names[4];

        CmsgWho() : CmsgWho(0, 0, 0, 0, 0, 0, "", "")
        {
        }

        CmsgWho(uint32_t min_level, uint32_t max_level, uint32_t class_mask, uint32_t race_mask,
            uint32_t zone_count, uint32_t name_count, std::string chatname, std::string guildname) :
            ManagedPacket(CMSG_WHO, 0),
            min_level(min_level),
            max_level(max_level),
            class_mask(class_mask),
            race_mask(race_mask),
            zone_count(zone_count),
            name_count(name_count),
            player_name(chatname),
            guild_name(guildname)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                packet >> min_level;
                packet >> max_level;
                packet >> player_name;
                packet >> guild_name;
                packet >> race_mask;
                packet >> class_mask;

                packet >> zone_count;

                if (zone_count > 10)
                    return true;

                for (uint32_t i = 0; i < zone_count; ++i)
                    packet >> zones[i];

                packet >> name_count;

                if (name_count > 4)
                    return true;

                for (uint32_t i = 0; i < name_count; ++i)
                    packet >> names[i];
            }
            else
            {
                packet >> min_level;
                packet >> max_level;

                uint8_t playerNameLen = static_cast<uint8_t>(packet.readBits(6));
                uint8_t guildNameLen = static_cast<uint8_t>(packet.readBits(6));

                packet >> race_mask;
                packet >> class_mask;

                zone_count = packet.readBits(4);
                name_count = packet.readBits(3);

                if (zone_count > 10)
                    zone_count = 10;

                if (name_count > 4)
                    name_count = 4;

                for (uint32_t i = 0; i < zone_count; ++i)
                    packet >> zones[i];

                for (uint32_t i = 0; i < name_count; ++i)
                {
                    uint8_t len = static_cast<uint8_t>(packet.readBits(6));
                    names[i] = packet.readString(len);
                }

                player_name = packet.readString(playerNameLen);
                guild_name = packet.readString(guildNameLen);
            }
            return true;
        }
    };
}
