/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

#include "Spell/SpellCastTargets.hpp"

namespace AscEmu::Packets
{
    class CmsgPetCastSpell : public ManagedPacket
    {
    public:
        WoWGuid petGuid;
        uint8_t castCount;
        uint32_t spellId;
        uint8_t castFlags;

        SpellCastTargets targets;

        bool hasAdditionalData = false;

        float projectilePitch = 0.0f;
        float projectileSpeed = 0.0f;

        bool hasMovementData = false;

        bool hasSrcLocation = false; // since 184141
        bool hasDestLocation = false; // since 184141

        CmsgPetCastSpell() : CmsgPetCastSpell(0, 0, 0, 0)
        {
        }

        CmsgPetCastSpell(uint64_t petGuid, uint8_t castCount, uint32_t spellId, uint8_t castFlags) :
            ManagedPacket(CMSG_PET_CAST_SPELL, 12),
            petGuid(petGuid),
            castCount(castCount),
            spellId(spellId),
            castFlags(castFlags)
        {
        }

    protected:
        bool internalDeserialise([[maybe_unused]] WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                uint64_t rawGuid = 0;
                packet >> rawGuid >> castCount >> spellId >> castFlags;
                petGuid.init(rawGuid);

                targets.read(packet);

                if (castFlags & 0x02)
                {
                    hasAdditionalData = true;
                    packet >> projectilePitch >> projectileSpeed >> hasMovementData;
                }

                return true;
            }
            else // Mop
            {
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
                [[maybe_unused]] bool hasPitch = false;
                bool hasTimestamp = false;

                hasDestLocation = packet.readBit();
                petGuid[7] = packet.readBit();
                bool hasMissileSpeed = !packet.readBit();
                hasSrcLocation = packet.readBit();
                petGuid[1] = packet.readBit();
                const uint8_t archeologyCount = static_cast<uint8_t>(packet.readBits(2));
                bool hasTargetMask = !packet.readBit();
                petGuid[4] = packet.readBit();
                packet.readBit();
                petGuid[6] = packet.readBit();
                bool hasTargetString = !packet.readBit();
                packet.readBit();
                bool hasMovement = packet.readBit();
                bool hasCastFlags = !packet.readBit();
                bool hasSpellId = !packet.readBit();
                petGuid[0] = packet.readBit();
                petGuid[5] = packet.readBit();
                petGuid[2] = packet.readBit();

                for (uint8_t i = 0; i < archeologyCount; ++i)
                    packet.readBits(2);

                petGuid[3] = packet.readBit();
                bool hasGlyphIndex = !packet.readBit();
                bool hasCastCount = !packet.readBit();
                bool hasElevation = !packet.readBit();

                if (hasMovement)
                {
                    movementInfo.status_info.hasOrientation = !packet.readBit();
                    movementInfo.status_info.hasSplineElevation = !packet.readBit();
                    packet.readBit();
                    moveGuid[5] = packet.readBit();
                    moveGuid[7] = packet.readBit();
                    bool hasMovementFlags2 = !packet.readBit();
                    hasTimestamp = !packet.readBit();
                    movementInfo.status_info.hasFallData = packet.readBit();
                    bool hasMovementFlags = !packet.readBit();
                    hasUnkField = !packet.readBit();

                    if (hasMovementFlags)
                        movementInfo.flags = static_cast<uint32_t>(packet.readBits(30));

                    packet.readBit();
                    moveGuid[6] = packet.readBit();
                    hasTransport = packet.readBit();
                    moveGuid[0] = packet.readBit();
                    unkCounter = packet.readBits(22);

                    if (hasTransport)
                    {
                        movementInfo.status_info.hasTransportTime2 = packet.readBit();
                        movementInfo.status_info.hasTransportTime3 = packet.readBit();
                        moveTransGuid[5] = packet.readBit();
                        moveTransGuid[6] = packet.readBit();
                        moveTransGuid[4] = packet.readBit();
                        moveTransGuid[0] = packet.readBit();
                        moveTransGuid[1] = packet.readBit();
                        moveTransGuid[2] = packet.readBit();
                        moveTransGuid[7] = packet.readBit();
                        moveTransGuid[3] = packet.readBit();
                    }

                    moveGuid[1] = packet.readBit();

                    if (hasMovementFlags2)
                        movementInfo.flags2 = static_cast<uint16_t>(packet.readBits(13));

                    moveGuid[3] = packet.readBit();
                    moveGuid[2] = packet.readBit();
                    packet.readBit();
                    hasPitch = !packet.readBit();
                    moveGuid[4] = packet.readBit();

                    if (movementInfo.status_info.hasFallData)
                        movementInfo.status_info.hasFallDirection = packet.readBit();
                }

                if (hasDestLocation)
                {
                    destTransGuid[2] = packet.readBit();
                    destTransGuid[0] = packet.readBit();
                    destTransGuid[1] = packet.readBit();
                    destTransGuid[4] = packet.readBit();
                    destTransGuid[5] = packet.readBit();
                    destTransGuid[6] = packet.readBit();
                    destTransGuid[3] = packet.readBit();
                    destTransGuid[7] = packet.readBit();
                }

                if (hasCastFlags)
                    castFlags = static_cast<uint8_t>(packet.readBits(5));

                itemTargetGuid[2] = packet.readBit();
                itemTargetGuid[4] = packet.readBit();
                itemTargetGuid[7] = packet.readBit();
                itemTargetGuid[0] = packet.readBit();
                itemTargetGuid[6] = packet.readBit();
                itemTargetGuid[1] = packet.readBit();
                itemTargetGuid[5] = packet.readBit();
                itemTargetGuid[3] = packet.readBit();

                if (hasTargetMask)
                    targets.setTargetMask(packet.readBits(20));

                if (hasTargetString)
                    targetStringLength = packet.readBits(7);

                if (hasSrcLocation)
                {
                    srcTransGuid[2] = packet.readBit();
                    srcTransGuid[0] = packet.readBit();
                    srcTransGuid[3] = packet.readBit();
                    srcTransGuid[1] = packet.readBit();
                    srcTransGuid[6] = packet.readBit();
                    srcTransGuid[7] = packet.readBit();
                    srcTransGuid[4] = packet.readBit();
                    srcTransGuid[5] = packet.readBit();
                }

                targetGuid[6] = packet.readBit();
                targetGuid[0] = packet.readBit();
                targetGuid[3] = packet.readBit();
                targetGuid[4] = packet.readBit();
                targetGuid[2] = packet.readBit();
                targetGuid[1] = packet.readBit();
                targetGuid[5] = packet.readBit();
                targetGuid[7] = packet.readBit();

                packet.readByteSeq(petGuid[2]);
                packet.readByteSeq(petGuid[6]);
                packet.readByteSeq(petGuid[3]);

                for (uint8_t i = 0; i < archeologyCount; ++i)
                {
                    packet.readSkip<uint32_t>();
                    packet.readSkip<uint32_t>();
                }

                packet.readByteSeq(petGuid[1]);
                packet.readByteSeq(petGuid[7]);
                packet.readByteSeq(petGuid[0]);
                packet.readByteSeq(petGuid[4]);
                packet.readByteSeq(petGuid[5]);

                if (hasDestLocation)
                {
                    float x, y, z;
                    packet.readByteSeq(destTransGuid[4]);
                    packet.readByteSeq(destTransGuid[1]);
                    packet.readByteSeq(destTransGuid[7]);
                    packet >> z;
                    packet >> y;
                    packet.readByteSeq(destTransGuid[6]);
                    packet.readByteSeq(destTransGuid[3]);
                    packet >> x;
                    packet.readByteSeq(destTransGuid[2]);
                    packet.readByteSeq(destTransGuid[5]);
                    packet.readByteSeq(destTransGuid[0]);

                    targets.setDestination({ x, y, z });
                    targets.setTransportDestinationGuid(destTransGuid);
                }

                if (hasMovement)
                {
                    if (hasPitch)
                        packet.readSkip<float>();

                    if (hasTransport)
                    {
                        if (movementInfo.status_info.hasTransportTime3)
                            packet >> movementInfo.transport_time3;

                        if (movementInfo.status_info.hasTransportTime2)
                            packet >> movementInfo.transport_time2;

                        packet >> movementInfo.transport_seat;
                        packet >> movementInfo.transport_position.o;
                        packet >> movementInfo.transport_position.z;
                        packet.readByteSeq(moveTransGuid[2]);
                        packet >> movementInfo.transport_time;
                        packet.readByteSeq(moveTransGuid[3]);
                        packet >> movementInfo.transport_position.x;
                        packet.readByteSeq(moveTransGuid[6]);
                        packet.readByteSeq(moveTransGuid[5]);
                        packet.readByteSeq(moveTransGuid[7]);
                        packet.readByteSeq(moveTransGuid[0]);
                        packet >> movementInfo.transport_position.y;
                        packet.readByteSeq(moveTransGuid[4]);
                        packet.readByteSeq(moveTransGuid[1]);
                    }

                    if (hasUnkField)
                        packet.readSkip<int32_t>();

                    for (uint32_t i = 0; i < unkCounter; ++i)
                        packet.readSkip<uint32_t>();

                    packet.readByteSeq(moveGuid[3]);

                    if (movementInfo.status_info.hasOrientation)
                        packet >> movementInfo.position.o;

                    packet.readByteSeq(moveGuid[5]);

                    if (movementInfo.status_info.hasFallData)
                    {
                        packet >> movementInfo.jump_info.velocity;

                        if (movementInfo.status_info.hasFallDirection)
                        {
                            packet >> movementInfo.jump_info.cosAngle;
                            packet >> movementInfo.jump_info.xyspeed;
                            packet >> movementInfo.jump_info.sinAngle;
                        }

                        packet >> movementInfo.fall_time;
                    }

                    if (hasTimestamp)
                        packet >> movementInfo.update_time;

                    packet.readByteSeq(moveGuid[6]);
                    packet >> movementInfo.position.x;
                    packet.readByteSeq(moveGuid[1]);
                    packet >> movementInfo.position.z;
                    packet.readByteSeq(moveGuid[2]);
                    packet.readByteSeq(moveGuid[7]);
                    packet.readByteSeq(moveGuid[0]);
                    packet >> movementInfo.position.y;
                    packet.readByteSeq(moveGuid[4]);

                    if (movementInfo.status_info.hasSplineElevation)
                        packet >> movementInfo.spline_elevation;
                }

                if (hasSrcLocation)
                {
                    float x, y, z;
                    packet.readByteSeq(srcTransGuid[3]);
                    packet.readByteSeq(srcTransGuid[4]);
                    packet.readByteSeq(srcTransGuid[2]);
                    packet.readByteSeq(srcTransGuid[1]);
                    packet.readByteSeq(srcTransGuid[0]);
                    packet.readByteSeq(srcTransGuid[7]);
                    packet >> z;
                    packet.readByteSeq(srcTransGuid[6]);
                    packet.readByteSeq(srcTransGuid[5]);
                    packet >> x;
                    packet >> y;

                    targets.setSource({ x, y, z });
                    targets.setUnitTarget(srcTransGuid);
                }

                if (hasMissileSpeed)
                    packet >> projectileSpeed;

                packet.readByteSeq(itemTargetGuid[1]);
                packet.readByteSeq(itemTargetGuid[2]);
                packet.readByteSeq(itemTargetGuid[5]);
                packet.readByteSeq(itemTargetGuid[7]);
                packet.readByteSeq(itemTargetGuid[4]);
                packet.readByteSeq(itemTargetGuid[6]);
                packet.readByteSeq(itemTargetGuid[3]);
                packet.readByteSeq(itemTargetGuid[0]);

                packet.readByteSeq(targetGuid[1]);
                packet.readByteSeq(targetGuid[5]);
                packet.readByteSeq(targetGuid[7]);
                packet.readByteSeq(targetGuid[3]);
                packet.readByteSeq(targetGuid[0]);
                packet.readByteSeq(targetGuid[2]);
                packet.readByteSeq(targetGuid[4]);
                packet.readByteSeq(targetGuid[6]);

                if (hasElevation)
                    packet >> projectilePitch;

                if (hasCastCount)
                    packet >> castCount;

                if (hasTargetString)
                    targets.setStringTarget(packet.readString(targetStringLength));

                if (hasGlyphIndex)
                    packet.readSkip<int32_t>();

                if (hasSpellId)
                    packet >> spellId;

                return true;
            }
        }
    };
}
