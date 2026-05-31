/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"
#include "Spell/SpellCastTargets.hpp"

namespace AscEmu::Packets
{
    class CmsgUseItem : public ManagedPacket
    {
    public:
        uint8_t containerIndex;     // since 5875
        uint8_t inventorySlot;      // since 5875
#if VERSION_STRING <= TBC
        uint8_t spellIndex = 0;     // only for 5875 and 8606
#endif
        uint32_t spellId = 0;       // since 8606
        uint8_t castCount = 0;      // since 8606

        uint64_t itemGuidRaw = 0;   // just a helper
        WoWGuid itemGuid;           // since 12340

        uint32_t glyphIndex = 0;    // since 12340
        uint8_t castFlags = 0;      // since 12340

        SpellCastTargets targets;

        bool hasAdditionalData = false;

        float projectilePitch = 0.0f;
        float projectileSpeed = 0.0f;

        bool hasMovementData = false;

        bool hasSrcLocation = false;    // since 184141
        bool hasDestLocation = false;   // since 184141

        CmsgUseItem() : CmsgUseItem(0, 0)
        {
        }

        CmsgUseItem(uint8_t containerIndex, uint8_t inventorySlot) :
            ManagedPacket(CMSG_USE_ITEM, 12),
            containerIndex(containerIndex),
            inventorySlot(inventorySlot)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
#if VERSION_STRING < Mop
#if VERSION_STRING == Classic
            packet >> containerIndex >> inventorySlot >> spellIndex;
#endif
#if VERSION_STRING == TBC
            packet >> containerIndex >> inventorySlot >> spellIndex >> castCount >> itemGuidRaw;
            itemGuid.init(itemGuidRaw);
#endif
#if VERSION_STRING == WotLK
            packet >> containerIndex >> inventorySlot >> castCount >> spellId >> itemGuidRaw >> glyphIndex >> castFlags;
            itemGuid.init(itemGuidRaw);
#endif
#if VERSION_STRING == Cata
            packet >> containerIndex >> inventorySlot >> castCount >> spellId >> itemGuidRaw >> glyphIndex >> castFlags;
            itemGuid.init(itemGuidRaw);
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
            WoWGuid moveTransGuid = 0;
            WoWGuid moveGuid = 0;

            MovementInfo movementInfo;

            bool hasTransport = false;
            bool hasUnkField = false;
            uint32_t unkCounter = 0;
            uint32_t unkMoveCounter = 0;
            bool hasPitch = false;
            bool hasTimestamp = false;

            packet >> containerIndex >> inventorySlot;

            bool hasElevation = !packet.readBit();
            itemGuid[6] = packet.readBit();
            bool hasTargetString = !packet.readBit();
            itemGuid[1] = packet.readBit();
            bool hasCastFlags = !packet.readBit();
            bool hasDestLocation = packet.readBit();
            itemGuid[2] = packet.readBit();
            itemGuid[7] = packet.readBit();
            itemGuid[0] = packet.readBit();
            bool hasTargetMask = !packet.readBit();
            bool hasMissileSpeed = !packet.readBit();
            bool hasMovement = packet.readBit();
            bool hasCastCount = !packet.readBit();
            bool hasSpellId = !packet.readBit();
            packet.readBit();
            bool hasGlyphIndex = !packet.readBit();
            packet.readBit();
            itemGuid[4] = packet.readBit();
            bool hasSrcLocation = packet.readBit();
            itemGuid[3] = packet.readBit();
            itemGuid[5] = packet.readBit();

            uint8_t researchCount = packet.readBits(2);
            for (uint8_t i = 0; i < researchCount; ++i)
                packet.readBits(2);

            if (hasMovement)
            {
                movementInfo.status_info.hasPitch = !packet.readBit();
                hasTransport = packet.readBit();
                packet.readBit();

                if (hasTransport)
                {
                    moveTransGuid[7] = packet.readBit();
                    moveTransGuid[2] = packet.readBit();
                    moveTransGuid[4] = packet.readBit();
                    moveTransGuid[5] = packet.readBit();
                    moveTransGuid[6] = packet.readBit();
                    moveTransGuid[0] = packet.readBit();
                    moveTransGuid[1] = packet.readBit();
                    movementInfo.status_info.hasTransportTime3 = packet.readBit();
                    moveTransGuid[4] = packet.readBit();
                    movementInfo.status_info.hasTransportTime2 = packet.readBit();
                }

                moveGuid[6] = packet.readBit();
                moveGuid[2] = packet.readBit();
                moveGuid[1] = packet.readBit();
                unkCounter = packet.readBits(22);
                packet.readBit();
                bool hasMovementFlags2 = !packet.readBit();
                movementInfo.status_info.hasFallData = packet.readBit();
                moveGuid[5] = packet.readBit();
                movementInfo.status_info.hasSplineElevation = !packet.readBit();
                packet.readBit();
                moveGuid[7] = packet.readBit();
                moveGuid[0] = packet.readBit();

                if (movementInfo.status_info.hasFallData)
                    movementInfo.status_info.hasFallDirection = packet.readBit();

                movementInfo.status_info.hasOrientation = !packet.readBit();
                moveGuid[4] = packet.readBit();
                moveGuid[3] = packet.readBit();
                hasTimestamp = !packet.readBit();
                unkMoveCounter = !packet.readBit();
                bool hasMovementFlags = !packet.readBit();

                if (hasMovementFlags2)
                    movementInfo.flags2 = packet.readBits(13);

                if (hasMovement)
                    movementInfo.flags = packet.readBits(30);
            }

            if (hasSrcLocation)
            {
                srcTransGuid[3] = packet.readBit();
                srcTransGuid[1] = packet.readBit();
                srcTransGuid[7] = packet.readBit();
                srcTransGuid[4] = packet.readBit();
                srcTransGuid[2] = packet.readBit();
                srcTransGuid[0] = packet.readBit();
                srcTransGuid[6] = packet.readBit();
                srcTransGuid[5] = packet.readBit();
            }

            if (hasDestLocation)
            {
                destTransGuid[2] = packet.readBit();
                destTransGuid[4] = packet.readBit();
                destTransGuid[1] = packet.readBit();
                destTransGuid[7] = packet.readBit();
                destTransGuid[6] = packet.readBit();
                destTransGuid[0] = packet.readBit();
                destTransGuid[3] = packet.readBit();
                destTransGuid[5] = packet.readBit();
            }

            if (hasTargetString)
                targetStringLength = packet.readBits(7);

            targetGuid[1] = packet.readBit();
            targetGuid[0] = packet.readBit();
            targetGuid[5] = packet.readBit();
            targetGuid[3] = packet.readBit();
            targetGuid[6] = packet.readBit();
            targetGuid[4] = packet.readBit();
            targetGuid[7] = packet.readBit();
            targetGuid[2] = packet.readBit();

            itemTargetGuid[4] = packet.readBit();
            itemTargetGuid[5] = packet.readBit();
            itemTargetGuid[0] = packet.readBit();
            itemTargetGuid[1] = packet.readBit();
            itemTargetGuid[3] = packet.readBit();
            itemTargetGuid[7] = packet.readBit();
            itemTargetGuid[6] = packet.readBit();
            itemTargetGuid[2] = packet.readBit();

            if (hasCastFlags)
                castFlags = packet.readBits(5);

            if (hasTargetMask)
                targets.setTargetMask(packet.readBits(20));

            packet.ReadByteSeq(itemGuid[0]);
            packet.ReadByteSeq(itemGuid[5]);
            packet.ReadByteSeq(itemGuid[6]);
            packet.ReadByteSeq(itemGuid[3]);
            packet.ReadByteSeq(itemGuid[4]);
            packet.ReadByteSeq(itemGuid[2]);
            packet.ReadByteSeq(itemGuid[1]);

            for (uint8_t i = 0; i < researchCount; ++i)
            {
                packet.read_skip<uint32_t>();
                packet.read_skip<uint32_t>();
            }

            packet.ReadByteSeq(itemGuid[7]);

            if (hasMovement)
            {
                for (uint8_t i = 0; i != unkCounter; i++)
                    packet.read_skip<uint32_t>();

                if (hasTransport)
                {
                    packet >> movementInfo.transport_position.y;
                    packet >> movementInfo.transport_position.z;
                    packet.ReadByteSeq(moveTransGuid[1]);

                    if (movementInfo.status_info.hasTransportTime3)
                        packet >> movementInfo.transport_time3;

                    packet.ReadByteSeq(moveTransGuid[7]);
                    packet.ReadByteSeq(moveTransGuid[5]);
                    packet.ReadByteSeq(moveTransGuid[2]);
                    packet.ReadByteSeq(moveTransGuid[4]);
                    packet >> movementInfo.transport_position.x;
                    packet >> movementInfo.transport_position.o;
                    packet.ReadByteSeq(moveTransGuid[0]);
                    packet >> movementInfo.transport_seat;
                    packet >> movementInfo.transport_time;
                    packet.ReadByteSeq(moveTransGuid[6]);
                    packet.ReadByteSeq(moveTransGuid[3]);

                    if (movementInfo.status_info.hasTransportTime2)
                        packet >> movementInfo.transport_time2;
                }

                if (movementInfo.status_info.hasFallData)
                {
                    packet >> movementInfo.jump_info.velocity;

                    if (movementInfo.status_info.hasFallDirection)
                    {
                        packet >> movementInfo.jump_info.sinAngle;
                        packet >> movementInfo.jump_info.cosAngle;
                        packet >> movementInfo.jump_info.xyspeed;
                    }

                    packet >> movementInfo.fall_time;
                }

                packet.ReadByteSeq(moveTransGuid[3]);
                packet.ReadByteSeq(moveTransGuid[7]);
                packet.ReadByteSeq(moveTransGuid[6]);
                packet.ReadByteSeq(moveTransGuid[1]);
                packet >> movementInfo.position.y;

                if (movementInfo.status_info.hasSplineElevation)
                    packet >> movementInfo.spline_elevation;

                if (hasUnkField)
                    packet.read_skip<uint32_t>();

                if (movementInfo.status_info.hasOrientation)
                    packet >> movementInfo.position.o;

                packet.ReadByteSeq(moveTransGuid[2]);
                packet >> movementInfo.position.z;

                if (hasTimestamp)
                    packet >> movementInfo.update_time;

                packet >> movementInfo.position.x;
                packet.ReadByteSeq(moveTransGuid[5]);
                packet.ReadByteSeq(moveTransGuid[0]);

                if (movementInfo.status_info.hasPitch)
                    movementInfo.pitch_rate = G3D::wrap(packet.read<float>(), -AscEmu::Math::PiF, AscEmu::Math::PiF);

                packet.ReadByteSeq(moveTransGuid[4]);
            }

            if (hasDestLocation)
            {
                float x, y, z;

                packet.ReadByteSeq(destTransGuid[7]);
                packet >> x;
                packet.ReadByteSeq(destTransGuid[0]);
                packet.ReadByteSeq(destTransGuid[6]);
                packet.ReadByteSeq(destTransGuid[1]);
                packet.ReadByteSeq(destTransGuid[3]);
                packet >> y;
                packet.ReadByteSeq(destTransGuid[5]);
                packet >> z;
                packet.ReadByteSeq(destTransGuid[4]);
                packet.ReadByteSeq(destTransGuid[2]);

                targets.setDestination({x, y, z});
                targets.setTransportDestinationGuid(destTransGuid);
            }

            packet.ReadByteSeq(itemTargetGuid[6]);
            packet.ReadByteSeq(itemTargetGuid[7]);
            packet.ReadByteSeq(itemTargetGuid[2]);
            packet.ReadByteSeq(itemTargetGuid[0]);
            packet.ReadByteSeq(itemTargetGuid[3]);
            packet.ReadByteSeq(itemTargetGuid[4]);
            packet.ReadByteSeq(itemTargetGuid[1]);
            packet.ReadByteSeq(itemTargetGuid[5]);

            if (hasSrcLocation)
            {
                float x, y, z;

                packet.ReadByteSeq(srcTransGuid[7]);
                packet >> x;
                packet.ReadByteSeq(srcTransGuid[1]);
                packet.ReadByteSeq(srcTransGuid[5]);
                packet.ReadByteSeq(srcTransGuid[4]);
                packet >> z;
                packet.ReadByteSeq(srcTransGuid[6]);
                packet.ReadByteSeq(srcTransGuid[0]);
                packet.ReadByteSeq(srcTransGuid[3]);
                packet >> y;
                packet.ReadByteSeq(srcTransGuid[2]);

                targets.setSource({x, y, z});
                targets.setUnitTarget(srcTransGuid);
            }

            if (hasSpellId)
                packet >> spellId;

            packet.ReadByteSeq(targetGuid[1]);
            packet.ReadByteSeq(targetGuid[4]);
            packet.ReadByteSeq(targetGuid[3]);
            packet.ReadByteSeq(targetGuid[6]);
            packet.ReadByteSeq(targetGuid[2]);
            packet.ReadByteSeq(targetGuid[0]);
            packet.ReadByteSeq(targetGuid[7]);
            packet.ReadByteSeq(targetGuid[5]);

            if (hasTargetString)
                targets.setStringTarget(packet.ReadString(targetStringLength));

            if (hasElevation)
                packet >> projectilePitch;

            if (hasGlyphIndex)
                packet >> glyphIndex;

            if (hasMissileSpeed)
                packet >> projectileSpeed;

            if (hasCastCount)
                packet >> castCount;
#endif
            return true;
        }
    };
}
