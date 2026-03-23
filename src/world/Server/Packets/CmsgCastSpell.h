/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"
#include "Spell/SpellCastTargets.hpp"

namespace AscEmu::Packets
{
    class CmsgCastSpell : public ManagedPacket
    {
    public:
        uint32_t spellId;
        uint8_t castCount;
        uint8_t castFlags;

        SpellCastTargets targets;

        uint32_t glyphSlot = 0;     // since 15595

        bool hasAdditionalData = false;

        float projectilePitch = 0.0f;
        float projectileSpeed = 0.0f;

        bool hasMovementData = false;

        bool hasSrcLocation = false;    // since 184141
        bool hasDestLocation = false;   // since 184141

        CmsgCastSpell() : CmsgCastSpell(0, 0, 0)
        {
        }

        CmsgCastSpell(uint32_t _spellId, uint8_t _castCount, uint8_t _flags) :
            ManagedPacket(CMSG_CAST_SPELL, 0),
            spellId(_spellId),
            castCount(_castCount),
            castFlags(_flags)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return m_minimum_size;
        }

        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
#if VERSION_STRING < Mop
#if VERSION_STRING <= TBC
            packet >> spellId >> castCount;
#elif VERSION_STRING == WotLK
            packet >> castCount >> spellId >> castFlags;
#elif VERSION_STRING == Cata
            packet >> castCount >> spellId >> glyphSlot >> castFlags;
#endif
            targets.read(packet);

            if (castFlags & 0x02)
            {
                hasAdditionalData = true;
                packet >> projectilePitch >> projectileSpeed >> hasMovementData;
            }
#else // Mop

            uint32_t targetStringLength = 0;
            
            WoWGuid targetGuid = 0;
            WoWGuid itemTargetGuid = 0;
            WoWGuid destTransGuid = 0;
            WoWGuid srcTransGuid = 0;

            MovementInfo movementInfo;

            bool hasTransport = false;
            bool hasUnkField = false;
            uint32_t unkCounter = 0;

            packet.readBit(); // unk

            bool hasTargetString = !packet.readBit();

            packet.readBit(); // unk

            bool hasCastCount = !packet.readBit();

            hasSrcLocation = packet.readBit();
            hasDestLocation = packet.readBit();

            bool hasSpellId = !packet.readBit();
            uint8_t researchCount = packet.readBits(2);
            bool hasTargetFlags = !packet.readBit();
            bool hasMissileSpeed = !packet.readBit();

            for (uint8_t i = 0; i < researchCount; ++i)
                packet.readBits(2);

            bool hasGlyphIndex = !packet.readBit();
            bool hasMovement = packet.readBit();
            bool hasElevation = !packet.readBit();
            bool hasCastFlags = !packet.readBit();

            targetGuid[5] = packet.readBit();
            targetGuid[4] = packet.readBit();
            targetGuid[2] = packet.readBit();
            targetGuid[7] = packet.readBit();
            targetGuid[1] = packet.readBit();
            targetGuid[6] = packet.readBit();
            targetGuid[3] = packet.readBit();
            targetGuid[0] = packet.readBit();

            if (hasDestLocation)
            {
                destTransGuid[1] = packet.readBit();
                destTransGuid[3] = packet.readBit();
                destTransGuid[5] = packet.readBit();
                destTransGuid[0] = packet.readBit();
                destTransGuid[2] = packet.readBit();
                destTransGuid[6] = packet.readBit();
                destTransGuid[7] = packet.readBit();
                destTransGuid[4] = packet.readBit();
            }

            if (hasMovement)
            {
                unkCounter = packet.readBits(22);
                packet.readBit();
                movementInfo.guid[4] = packet.readBit();
                hasTransport = packet.readBit();

                if (hasTransport)
                {
                    movementInfo.status_info.hasTransportTime2 = packet.readBit();
                    movementInfo.transport_guid[7] = packet.readBit();
                    movementInfo.transport_guid[4] = packet.readBit();
                    movementInfo.transport_guid[1] = packet.readBit();
                    movementInfo.transport_guid[0] = packet.readBit();
                    movementInfo.transport_guid[6] = packet.readBit();
                    movementInfo.transport_guid[3] = packet.readBit();
                    movementInfo.transport_guid[5] = packet.readBit();
                    movementInfo.status_info.hasTransportTime3 = packet.readBit();
                    movementInfo.transport_guid[2] = packet.readBit();
                }

                packet.readBit();
                movementInfo.guid[7] = packet.readBit();
                movementInfo.status_info.hasOrientation = !packet.readBit();
                movementInfo.guid[6] = packet.readBit();
                movementInfo.status_info.hasSplineElevation = !packet.readBit();
                movementInfo.status_info.hasPitch = !packet.readBit();
                movementInfo.guid[0] = packet.readBit();

                packet.readBit();

                bool hasMovementFlags = !packet.readBit();
                movementInfo.status_info.hasTimeStamp = !packet.readBit();
                hasUnkField = !packet.readBit();

                if (hasMovementFlags)
                    movementInfo.flags = packet.readBits(30);

                movementInfo.guid[1] = packet.readBit();
                movementInfo.guid[3] = packet.readBit();
                movementInfo.guid[2] = packet.readBit();
                movementInfo.guid[5] = packet.readBit();
                movementInfo.status_info.hasFallData = packet.readBit();

                if (movementInfo.status_info.hasFallData)
                    movementInfo.status_info.hasFallDirection = packet.readBit();

                bool hasMovementFlags2 = !packet.readBit();

                if (hasMovementFlags2)
                    movementInfo.flags2 = packet.readBits(13);
            }

            itemTargetGuid[1] = packet.readBit();
            itemTargetGuid[0] = packet.readBit();
            itemTargetGuid[7] = packet.readBit();
            itemTargetGuid[4] = packet.readBit();
            itemTargetGuid[6] = packet.readBit();
            itemTargetGuid[5] = packet.readBit();
            itemTargetGuid[3] = packet.readBit();
            itemTargetGuid[2] = packet.readBit();

            if (hasSrcLocation)
            {
                srcTransGuid[4] = packet.readBit();
                srcTransGuid[5] = packet.readBit();
                srcTransGuid[3] = packet.readBit();
                srcTransGuid[0] = packet.readBit();
                srcTransGuid[7] = packet.readBit();
                srcTransGuid[1] = packet.readBit();
                srcTransGuid[6] = packet.readBit();
                srcTransGuid[2] = packet.readBit();
            }

            if (hasTargetFlags)
                targets.setTargetMask(packet.readBits(20));

            if (hasCastFlags)
                castFlags = packet.readBits(5);

            if (hasTargetString)
                targetStringLength = packet.readBits(7);

            for (uint8_t i = 0; i < researchCount; ++i)
            {
                packet.read_skip<uint32_t>();
                packet.read_skip<uint32_t>();
            }

            if (hasMovement)
            {
                packet >> movementInfo.position.x;
                packet.ReadByteSeq(movementInfo.guid[0]);

                if (hasTransport)
                {
                    packet.ReadByteSeq(movementInfo.transport_guid[2]);
                    packet >> movementInfo.transport_seat;
                    packet.ReadByteSeq(movementInfo.transport_guid[3]);
                    packet.ReadByteSeq(movementInfo.transport_guid[7]);
                    packet >> movementInfo.transport_position.x;
                    packet.ReadByteSeq(movementInfo.transport_guid[5]);

                    if (movementInfo.status_info.hasTransportTime3)
                        packet >> movementInfo.transport_time3;

                    packet >> movementInfo.transport_position.z;
                    packet >> movementInfo.transport_position.y;

                    packet.ReadByteSeq(movementInfo.transport_guid[6]);
                    packet.ReadByteSeq(movementInfo.transport_guid[1]);
                    packet >> movementInfo.transport_position.o;

                    packet.ReadByteSeq(movementInfo.transport_guid[4]);

                    if (movementInfo.status_info.hasTransportTime2)
                        packet >> movementInfo.transport_time2;

                    packet.ReadByteSeq(movementInfo.transport_guid[0]);
                    packet >> movementInfo.transport_time;
                }

                packet.ReadByteSeq(movementInfo.guid[5]);

                if (movementInfo.status_info.hasFallData)
                {
                    packet >> movementInfo.fall_time;
                    packet >> movementInfo.jump_info.velocity;

                    if (movementInfo.status_info.hasFallDirection)
                    {
                        packet >> movementInfo.jump_info.sinAngle;
                        packet >> movementInfo.jump_info.xyspeed;
                        packet >> movementInfo.jump_info.cosAngle;
                    }
                }

                if (movementInfo.status_info.hasSplineElevation)
                    packet >> movementInfo.spline_elevation;

                packet.ReadByteSeq(movementInfo.guid[6]);

                if (hasUnkField)
                    packet.read_skip<uint32_t>();

                packet.ReadByteSeq(movementInfo.guid[4]);

                if (movementInfo.status_info.hasOrientation)
                    movementInfo.position.o;

                if (movementInfo.status_info.hasTimeStamp)
                    packet >> movementInfo.update_time;

                packet.ReadByteSeq(movementInfo.guid[1]);

                if (movementInfo.status_info.hasPitch)
                    packet >> movementInfo.pitch_rate;

                packet.ReadByteSeq(movementInfo.guid[3]);

                for (uint8_t i = 0; i != unkCounter; i++)
                    packet.read_skip<uint32_t>();

                packet >> movementInfo.position.y;
                packet.ReadByteSeq(movementInfo.guid[7]);
                packet >> movementInfo.position.z;
                packet.ReadByteSeq(movementInfo.guid[2]);
            }

            packet.ReadByteSeq(itemTargetGuid[4]);
            packet.ReadByteSeq(itemTargetGuid[2]);
            packet.ReadByteSeq(itemTargetGuid[1]);
            packet.ReadByteSeq(itemTargetGuid[5]);
            packet.ReadByteSeq(itemTargetGuid[7]);
            packet.ReadByteSeq(itemTargetGuid[3]);
            packet.ReadByteSeq(itemTargetGuid[6]);
            packet.ReadByteSeq(itemTargetGuid[0]);

            if (hasDestLocation)
            {
                float x, y, z;
                packet.ReadByteSeq(destTransGuid[2]);
                packet >> x;
                packet.ReadByteSeq(destTransGuid[4]);
                packet.ReadByteSeq(destTransGuid[1]);
                packet.ReadByteSeq(destTransGuid[0]);
                packet.ReadByteSeq(destTransGuid[3]);
                packet >> y;
                packet.ReadByteSeq(destTransGuid[7]);
                packet >> z;
                packet.ReadByteSeq(destTransGuid[5]);
                packet.ReadByteSeq(destTransGuid[6]);

                targets.setDestination({x, y, z});
                targets.setTransportDestinationGuid(destTransGuid);
            }

            packet.ReadByteSeq(targetGuid[3]);
            packet.ReadByteSeq(targetGuid[4]);
            packet.ReadByteSeq(targetGuid[7]);
            packet.ReadByteSeq(targetGuid[6]);
            packet.ReadByteSeq(targetGuid[2]);
            packet.ReadByteSeq(targetGuid[0]);
            packet.ReadByteSeq(targetGuid[1]);
            packet.ReadByteSeq(targetGuid[5]);

            if (hasSrcLocation)
            {
                float x, y, z;
                packet >> y;
                packet.ReadByteSeq(srcTransGuid[5]);
                packet.ReadByteSeq(srcTransGuid[1]);
                packet.ReadByteSeq(srcTransGuid[7]);
                packet.ReadByteSeq(srcTransGuid[6]);
                packet >> x;
                packet.ReadByteSeq(srcTransGuid[3]);
                packet.ReadByteSeq(srcTransGuid[2]);
                packet.ReadByteSeq(srcTransGuid[0]);
                packet.ReadByteSeq(srcTransGuid[4]);
                packet >> z;

                targets.setSource({x, y, z});
                targets.setUnitTarget(srcTransGuid);
            }

            if (hasTargetString)
                targets.setStringTarget(packet.ReadString(targetStringLength));

            if (hasMissileSpeed)
                packet >> projectileSpeed;

            if (hasElevation)
                packet >> projectilePitch;

            if (hasCastCount)
                packet >> castCount;

            if (hasSpellId)
                packet >> spellId;

            if (hasGlyphIndex)
                packet >> glyphSlot;

            targets.setUnitTarget(targetGuid);
            targets.setItemTarget(itemTargetGuid);

#endif
            return true;
        }
    };
}
