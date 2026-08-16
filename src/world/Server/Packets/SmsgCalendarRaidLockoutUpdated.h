/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>
#include <ctime>

namespace AscEmu::Packets
{
    class SmsgCalendarRaidLockoutUpdated : public ManagedPacket
    {
    public:
        time_t currTime;
        uint32_t mapId;
        uint32_t difficulty;
        uint32_t resetTimeDiff;

        SmsgCalendarRaidLockoutUpdated() : SmsgCalendarRaidLockoutUpdated(0, 0, 0, 0)
        {
        }

        SmsgCalendarRaidLockoutUpdated(time_t currTime, uint32_t mapId, uint32_t difficulty, uint32_t resetTimeDiff) :
            ManagedPacket(SMSG_CALENDAR_RAID_LOCKOUT_UPDATED, 4 + 4 + 4 + 4 + 8),
            currTime(currTime),
            mapId(mapId),
            difficulty(difficulty),
            resetTimeDiff(resetTimeDiff)
        {
        }

    protected:
        size_t expectedSize() const override { return 4 + 4 + 4 + 4 + 8; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion <= WoW::Expansion::_Mop)
            {
                packet.appendPackedTime(currTime);
                packet << mapId;
                packet << difficulty;
                packet << uint32_t(0);    // Amount of seconds that has changed to the reset time
                packet << resetTimeDiff;

                return true;
            }
            else if (m_protocol.isMop())
            {
                packet << resetTimeDiff;
                packet << mapId;
                packet << difficulty;
                packet.appendPackedTime(currTime);
                packet << uint32_t(0);    // Amount of seconds that has changed to the reset time

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
