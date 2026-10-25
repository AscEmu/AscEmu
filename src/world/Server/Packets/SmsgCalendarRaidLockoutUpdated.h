/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CalendarPacketData.hpp"

#include <cstdint>
#include <ctime>

namespace AscEmu::Packets
{
    // the reset of a raid save moved (extended save), the remaining seconds before and after the change
    class SmsgCalendarRaidLockoutUpdated : public ManagedPacket
    {
    public:
        time_t currTime;
        uint32_t mapId;
        uint32_t difficulty;
        int32_t oldSecondsLeft;
        int32_t newSecondsLeft;

        SmsgCalendarRaidLockoutUpdated() : SmsgCalendarRaidLockoutUpdated(0, 0, 0, 0, 0)
        {
        }

        SmsgCalendarRaidLockoutUpdated(time_t currTime, uint32_t mapId, uint32_t difficulty, int32_t oldSecondsLeft, int32_t newSecondsLeft) :
            ManagedPacket(SMSG_CALENDAR_RAID_LOCKOUT_UPDATED, 4 + 4 + 4 + 4 + 8),
            currTime(currTime),
            mapId(mapId),
            difficulty(difficulty),
            oldSecondsLeft(oldSecondsLeft),
            newSecondsLeft(newSecondsLeft)
        {
        }

    protected:
        size_t expectedSize() const override { return 4 + 4 + 4 + 4 + 8; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_Shadowlands)
            {
                // 9.x and later: packed server time, the old remaining time before the new one
                CalendarWire::writeDate(packet, m_protocol, currTime);
                packet << int32_t(mapId);
                if (m_protocol.expansion >= WoW::Expansion::_Midnight)
                    packet << int16_t(difficulty);
                else
                    packet << difficulty;
                packet << oldSecondsLeft << newSecondsLeft;
                return true;
            }

            if (m_protocol.expansion >= WoW::Expansion::_WoD)
            {
                packet << uint32_t(currTime);
                packet << int32_t(mapId) << difficulty;
                packet << newSecondsLeft << oldSecondsLeft;
                return true;
            }

            if (m_protocol.isMop())
            {
                packet << newSecondsLeft << mapId << difficulty;
                packet.appendPackedTime(currTime);
                packet << oldSecondsLeft;
                return true;
            }

            if (m_protocol.expansion >= WoW::Expansion::_WotLK && m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet.appendPackedTime(currTime);
                packet << mapId << difficulty;
                packet << int32_t(newSecondsLeft - oldSecondsLeft);   // shift of the reset
                packet << newSecondsLeft;
                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
