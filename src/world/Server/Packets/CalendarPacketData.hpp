/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Management/CalendarMgr.hpp"

#include <array>
#include <cstdint>
#include <ctime>
#include <string>
#include <vector>

namespace AscEmu::Packets
{
    // an event as the calendar packets describe it
    struct CalendarEventData
    {
        uint64_t eventId = 0;
        std::string title;
        std::string description;
        uint8_t type = 0;
        int32_t textureId = 0;          // dungeon of the event
        time_t date = 0;
        uint32_t flags = 0;
        uint32_t ownerGuid = 0;         // low guid of the owner
        uint32_t guildId = 0;           // 0 for an event without guild
    };

    // an invite as the calendar packets describe it
    struct CalendarInviteData
    {
        uint64_t inviteId = 0;
        uint64_t eventId = 0;
        uint32_t inviteeGuid = 0;
        uint32_t senderGuid = 0;
        uint8_t level = 0;
        uint8_t status = 0;
        uint8_t rank = 0;
        bool guildMember = false;       // the invitee belongs to the guild of a guild event
        bool guildEvent = false;        // the event of the invite is a guild event
        uint32_t ownerGuid = 0;         // owner of the event, 0 when the event is gone
        time_t responseTime = 0;
        std::string notes;
    };

    // a raid the player is saved to
    struct CalendarLockoutData
    {
        uint64_t instanceId = 0;
        uint32_t mapId = 0;
        uint32_t difficulty = 0;
        int32_t secondsLeft = 0;
    };

    // the next weekly or daily reset of a raid map
    struct CalendarRaidResetData
    {
        uint32_t mapId = 0;
        int32_t secondsLeft = 0;
    };

    // a holiday whose dates the server schedule replaces, the client keeps its own data for the rest
    struct CalendarHolidayData
    {
        uint32_t id = 0;
        uint32_t region = 0;
        uint32_t looping = 0;
        uint32_t priority = 0;
        int32_t filterType = 0;
        std::array<uint32_t, 26> dates = {};            // packed start dates
        std::array<uint32_t, 10> durations = {};        // hours of every stage
        std::array<uint32_t, 10> flags = {};
        std::string texture;
    };

    // the encodings the calendar packets share
    namespace CalendarWire
    {
        // the date as packed local time: minute, hour, weekday, day, month and year since 2000
        inline uint32_t packDate(time_t date)
        {
            const tm local = byteBufferDetail::toLocalTime(date);
            return ((static_cast<uint32_t>(local.tm_year - 100) & 0x1Fu) << 24u) |
                ((static_cast<uint32_t>(local.tm_mon) & 0x0Fu) << 20u) |
                ((static_cast<uint32_t>(local.tm_mday - 1) & 0x3Fu) << 14u) |
                ((static_cast<uint32_t>(local.tm_wday) & 0x07u) << 11u) |
                ((static_cast<uint32_t>(local.tm_hour) & 0x1Fu) << 6u) |
                (static_cast<uint32_t>(local.tm_min) & 0x3Fu);
        }

        // the clients from 10.x on read two unused flag bits above the year
        inline uint32_t dateValue(WoW::ClientProtocol const& protocol, time_t date)
        {
            uint32_t value = packDate(date);
            if (protocol.expansion >= WoW::Expansion::_Dragonflight)
                value |= 0x60000000u;

            return value;
        }

        inline void writeDate(WorldPacket& packet, WoW::ClientProtocol const& protocol, time_t date)
        {
            packet << dateValue(protocol, date);
        }

        // a date that is not set: 0 up to 9.x, the packed epoch from 10.x on
        inline void writeUnsetDate(WorldPacket& packet, WoW::ClientProtocol const& protocol)
        {
            packet << uint32_t(protocol.expansion >= WoW::Expansion::_Dragonflight ? 0x62002000u : 0u);
        }

        inline WoWGuid playerGuid(uint32_t lowGuid)
        {
            return WoWGuid(lowGuid, 0, HIGHGUID_TYPE_PLAYER);
        }

        inline WoWGuid guildGuid(uint32_t guildId)
        {
            return guildId != 0 ? WoWGuid(guildId, 0, HIGHGUID_TYPE_GUILD) : WoWGuid(uint64_t(0));
        }

        // a player as packed 64 bit guid up to 4.x and as 128 bit guid from 6.x on
        inline void writePlayer(WorldPacket& packet, WoW::ClientProtocol const& protocol, uint32_t lowGuid)
        {
            if (protocol.expansion >= WoW::Expansion::_WoD)
            {
                if (lowGuid != 0)
                    packet << playerGuid(lowGuid).toGuid128(protocol.realmId, 0);
                else
                    packet << WoWGuid128();
                return;
            }

            packet.appendPackGuid(lowGuid);
        }

        inline uint32_t readPlayer(WorldPacket& packet, WoW::ClientProtocol const& protocol)
        {
            if (protocol.expansion >= WoW::Expansion::_WoD)
            {
                WoWGuid128 guid;
                packet >> guid;
                return WoWGuid::fromGuid128(guid).getLowGuid();
            }

            return static_cast<uint32_t>(packet.unpackGuid());
        }

        // the guild of an event: 128 bit guid in 6.x and 7.x, the plain guild id (club) from 8.x on
        inline void writeEventGuild(WorldPacket& packet, WoW::ClientProtocol const& protocol, uint32_t guildId)
        {
            if (protocol.expansion >= WoW::Expansion::_BfA)
            {
                packet << uint64_t(guildId);
                return;
            }

            if (guildId != 0)
                packet << guildGuid(guildId).toGuid128(protocol.realmId, 0);
            else
                packet << WoWGuid128();
        }

        // the event flags are 16 bit from 11.x on
        inline void writeFlags(WorldPacket& packet, WoW::ClientProtocol const& protocol, uint32_t flags)
        {
            if (protocol.expansion >= WoW::Expansion::_TheWarWithin)
                packet << uint16_t(flags);
            else
                packet << uint32_t(flags);
        }

        // string bytes after a bit block, without terminator, the length went into the bit block
        inline void writeRaw(ByteBuffer& buffer, std::string const& value)
        {
            buffer.flushBits();
            buffer.writeString(value);
        }
    }
}
