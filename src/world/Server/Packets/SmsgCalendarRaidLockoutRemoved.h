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
    // Covers both SMSG_CALENDAR_RAID_LOCKOUT_REMOVED and SMSG_CALENDAR_RAID_LOCKOUT_ADDED,
    // the opcode is switched at serialise time based on the `add` flag, matching the
    // original raw WorldPacket construction.
    class SmsgCalendarRaidLockoutRemoved : public ManagedPacket
    {
    public:
        bool add;
        time_t currTime;
        uint32_t mapId;
        uint32_t difficulty;
        uint32_t resetTimeDiff;
        uint64_t instanceId;

        SmsgCalendarRaidLockoutRemoved() : SmsgCalendarRaidLockoutRemoved(false, 0, 0, 0, 0, 0)
        {
        }

        SmsgCalendarRaidLockoutRemoved(bool add, time_t currTime, uint32_t mapId, uint32_t difficulty, uint32_t resetTimeDiff, uint64_t instanceId) :
            ManagedPacket(SMSG_CALENDAR_RAID_LOCKOUT_REMOVED, (4) + 4 + 4 + 4 + 8),
            add(add),
            currTime(currTime),
            mapId(mapId),
            difficulty(difficulty),
            resetTimeDiff(resetTimeDiff),
            instanceId(instanceId)
        {
        }

    protected:
        size_t expectedSize() const override { return (4) + 4 + 4 + 4 + 8; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion <= WoW::Expansion::_Mop)
            {
                if (add)
                {
                    packet.setOpcode(SMSG_CALENDAR_RAID_LOCKOUT_ADDED);
                    packet.appendPackedTime(currTime);
                }

                packet << mapId;
                packet << difficulty;
                packet << resetTimeDiff;
                packet << instanceId;

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
