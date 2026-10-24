/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ItemInstanceHelpers.h"
#include "ManagedPacket.h"
#include "Utilities/LocationVector.hpp"

#include <cstdint>

namespace AscEmu::Packets
{
    // a SpellVisual played from the source to a target unit or a position (Cata and later)
    class SmsgPlaySpellVisualTargeted : public ManagedPacket
    {
    public:
        uint64_t sourceGuid;
        uint64_t targetGuid;
        LocationVector targetPosition;
        uint32_t visualId;
        float travelSpeed;
        bool speedAsTime;
        uint16_t missReason;
        uint16_t reflectStatus;
        float launchDelay;

        SmsgPlaySpellVisualTargeted() : SmsgPlaySpellVisualTargeted(0, 0, LocationVector(), 0, 0.0f)
        {
        }

        SmsgPlaySpellVisualTargeted(uint64_t sourceGuid, uint64_t targetGuid, LocationVector const& targetPosition, uint32_t visualId, float travelSpeed,
            bool speedAsTime = false, uint16_t missReason = 0, uint16_t reflectStatus = 0, float launchDelay = 0.0f) :
            ManagedPacket(SMSG_PLAY_SPELL_VISUAL_TARGETED, 16 + 16 + 16 + 12 + 4 + 4 + 3 + 4 + 4 + 1),
            sourceGuid(sourceGuid),
            targetGuid(targetGuid),
            targetPosition(targetPosition),
            visualId(visualId),
            travelSpeed(travelSpeed),
            speedAsTime(speedAsTime),
            missReason(missReason),
            reflectStatus(reflectStatus),
            launchDelay(launchDelay)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_BfA)
            {
                // 8.x+: source, target, transport, position, visual, speed, hit/miss/reflect, launch delay, min duration, bit
                writeClientGuid(packet, m_protocol, m_receiverMapId, sourceGuid);
                writeClientGuid(packet, m_protocol, m_receiverMapId, targetGuid);
                writeClientGuid(packet, m_protocol, m_receiverMapId, 0);
                packet << targetPosition.x << targetPosition.y << targetPosition.z;
                packet << visualId;
                packet << travelSpeed;

                if (m_protocol.isTheWarWithinOrLater())
                {
                    packet << uint8_t(0);
                    packet << static_cast<uint8_t>(missReason);
                    packet << static_cast<uint8_t>(reflectStatus);
                }
                else
                {
                    packet << uint16_t(0);
                    packet << missReason;
                    packet << reflectStatus;
                }

                packet << launchDelay;
                packet << float(0.0f);
                packet.writeBit(speedAsTime);
                packet.flushBits();
                return true;
            }

            if (m_protocol.isWoD() || m_protocol.isLegion())
            {
                // 6.x/7.x: source, target, position, visual, speed, miss, reflect, orientation, bit
                writeClientGuid(packet, m_protocol, m_receiverMapId, sourceGuid);
                writeClientGuid(packet, m_protocol, m_receiverMapId, targetGuid);
                packet << targetPosition.x << targetPosition.y << targetPosition.z;
                packet << visualId;
                packet << travelSpeed;
                packet << missReason;
                packet << reflectStatus;
                packet << targetPosition.o;
                packet.writeBit(speedAsTime);
                packet.flushBits();
                return true;
            }

            const WoWGuid source = sourceGuid;
            const WoWGuid target = targetGuid;

            if (m_protocol.isMop())
            {
                packet.writeBit(target[4]);
                packet.writeBit(source[6]);
                packet.writeBit(source[4]);
                packet.writeBit(source[7]);
                packet.writeBit(target[6]);
                packet.writeBit(source[2]);
                packet.writeBit(source[0]);
                packet.writeBit(target[2]);
                packet.writeBit(targetGuid == 0);
                packet.writeBit(target[7]);
                packet.writeBit(source[3]);
                packet.writeBit(source[1]);
                packet.writeBit(target[0]);
                packet.writeBit(target[1]);
                packet.writeBit(source[5]);
                packet.writeBit(target[5]);
                packet.writeBit(target[3]);
                packet.flushBits();

                packet << targetPosition.z;
                packet.writeByteSeq(target[2]);
                packet.writeByteSeq(target[6]);
                packet.writeByteSeq(target[5]);
                packet.writeByteSeq(source[2]);
                packet.writeByteSeq(target[1]);
                packet << targetPosition.x;
                packet.writeByteSeq(target[3]);
                packet << reflectStatus;
                packet.writeByteSeq(source[4]);
                packet.writeByteSeq(source[7]);
                packet << travelSpeed;
                packet << targetPosition.y;
                packet.writeByteSeq(target[4]);
                packet.writeByteSeq(source[5]);
                packet << visualId;
                packet.writeByteSeq(source[1]);
                packet.writeByteSeq(target[7]);
                packet << missReason;
                packet.writeByteSeq(source[0]);
                packet.writeByteSeq(source[6]);
                packet.writeByteSeq(target[0]);
                packet.writeByteSeq(source[3]);
                return true;
            }

            if (m_protocol.isCata())
            {
                packet << targetPosition.z;
                packet << visualId;
                packet << missReason;
                packet << travelSpeed;
                packet << targetPosition.x;
                packet << reflectStatus;
                packet << targetPosition.y;

                packet.writeBit(target[1]);
                packet.writeBit(source[3]);
                packet.writeBit(source[0]);
                packet.writeBit(target[2]);
                packet.writeBit(target[5]);
                packet.writeBit(source[2]);
                packet.writeBit(source[4]);
                packet.writeBit(target[6]);
                packet.writeBit(target[0]);
                packet.writeBit(speedAsTime);
                packet.writeBit(source[6]);
                packet.writeBit(target[7]);
                packet.writeBit(source[5]);
                packet.writeBit(source[1]);
                packet.writeBit(source[7]);
                packet.writeBit(target[3]);
                packet.writeBit(target[4]);
                packet.flushBits();

                packet.writeByteSeq(source[7]);
                packet.writeByteSeq(source[4]);
                packet.writeByteSeq(target[7]);
                packet.writeByteSeq(source[1]);
                packet.writeByteSeq(source[3]);
                packet.writeByteSeq(source[0]);
                packet.writeByteSeq(source[6]);
                packet.writeByteSeq(target[0]);
                packet.writeByteSeq(target[4]);
                packet.writeByteSeq(source[5]);
                packet.writeByteSeq(target[1]);
                packet.writeByteSeq(target[5]);
                packet.writeByteSeq(target[6]);
                packet.writeByteSeq(target[2]);
                packet.writeByteSeq(source[2]);
                packet.writeByteSeq(target[3]);
                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
