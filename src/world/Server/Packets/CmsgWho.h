/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu { namespace Packets
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
            ManagedPacket(CMSG_WHO, 8),
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
        bool internalSerialise(WorldPacket& packet) override
        {
            packet << min_level;
            packet << max_level;
            packet << player_name;
            packet << guild_name;
            packet << race_mask;
            packet << class_mask;

            packet << zone_count;

            if (zone_count > 10)
                return true;

            for (uint32_t i = 0; i < zone_count; ++i)
                packet << zones[i];

            packet << name_count;

            if (name_count > 4)
                return true;

            for (uint32_t i = 0; i < name_count; ++i)
                packet << names[i];

            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
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

            return true;
        }
    };
}}
