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
    // a raid save of the player that was added (add) or removed
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
            ManagedPacket(add ? SMSG_CALENDAR_RAID_LOCKOUT_ADDED : SMSG_CALENDAR_RAID_LOCKOUT_REMOVED, 4 + 4 + 4 + 4 + 8),
            add(add),
            currTime(currTime),
            mapId(mapId),
            difficulty(difficulty),
            resetTimeDiff(resetTimeDiff),
            instanceId(instanceId)
        {
        }

    protected:
        size_t expectedSize() const override { return 4 + 4 + 4 + 4 + 8; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_Midnight)
            {
                // 12.x: map and 16 bit difficulty lead, the instance closes the packet
                if (add)
                    CalendarWire::writeDate(packet, m_protocol, currTime);
                packet << int32_t(mapId) << int16_t(difficulty);
                if (add)
                    packet << int32_t(resetTimeDiff);
                packet << instanceId;
                return true;
            }

            if (m_protocol.expansion >= WoW::Expansion::_WoD)
            {
                packet << instanceId;
                if (add)
                {
                    // the server time is packed in 7.x and from 10.x on, a plain unix time in 6.x, 8.x and 9.x
                    if (m_protocol.isLegion() || m_protocol.expansion >= WoW::Expansion::_Dragonflight)
                        CalendarWire::writeDate(packet, m_protocol, currTime);
                    else
                        packet << uint32_t(currTime);
                }
                packet << int32_t(mapId) << difficulty;
                if (add)
                    packet << int32_t(resetTimeDiff);
                return true;
            }

            if (m_protocol.isMop())
            {
                const WoWGuid instance(instanceId);
                if (add)
                {
                    packet.writeBit(instance[3]);
                    packet.writeBit(instance[1]);
                    packet.writeBit(instance[2]);
                    packet.writeBit(instance[0]);
                    packet.writeBit(instance[4]);
                    packet.writeBit(instance[7]);
                    packet.writeBit(instance[5]);
                    packet.writeBit(instance[6]);
                    packet.flushBits();

                    packet.writeByteSeq(instance[6]);
                    packet << difficulty;
                    packet.writeByteSeq(instance[0]);
                    packet.writeByteSeq(instance[4]);
                    packet.writeByteSeq(instance[5]);
                    packet.writeByteSeq(instance[3]);
                    packet.writeByteSeq(instance[2]);
                    packet << mapId;
                    packet.writeByteSeq(instance[1]);
                    packet.appendPackedTime(currTime);
                    packet << resetTimeDiff;
                    packet.writeByteSeq(instance[7]);
                    return true;
                }

                packet << difficulty << mapId;
                packet.writeBit(instance[2]);
                packet.writeBit(instance[0]);
                packet.writeBit(instance[4]);
                packet.writeBit(instance[6]);
                packet.writeBit(instance[5]);
                packet.writeBit(instance[7]);
                packet.writeBit(instance[3]);
                packet.writeBit(instance[1]);
                packet.flushBits();

                packet.writeByteSeq(instance[6]);
                packet.writeByteSeq(instance[1]);
                packet.writeByteSeq(instance[7]);
                packet.writeByteSeq(instance[3]);
                packet.writeByteSeq(instance[4]);
                packet.writeByteSeq(instance[5]);
                packet.writeByteSeq(instance[0]);
                packet.writeByteSeq(instance[2]);
                return true;
            }

            if (m_protocol.expansion >= WoW::Expansion::_WotLK && m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                if (add)
                    packet.appendPackedTime(currTime);

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
