/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

#include "Spell/SpellCastTargets.hpp"
#include "ForeverSpellPacketUtils.hpp"

namespace AscEmu::Packets
{
    class CmsgCastSpell : public ManagedPacket
    {
    public:
        uint32_t spellId;
        uint8_t castCount;
        uint8_t castFlags;

        WoWGuid clientCastId = WoWGuid::createModernEmpty();
        uint32_t spellXSpellVisualId = 0;
        uint32_t scriptVisualId = 0;

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
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isForever())
            {
                using namespace ForeverSpellPacket;

                if (!readPackedGuid(packet, clientCastId))
                    return false;

                packet >> castFlags;

                // Misc[3] - present in the 70009 SpellCastRequest even when all values are zero.
                packet.readSkip<int32_t>();
                packet.readSkip<int32_t>();
                packet.readSkip<int32_t>();

                packet >> spellId;
                packet >> spellXSpellVisualId >> scriptVisualId;

                if (!readTargetData(packet, targets))
                    return false;

                packet >> projectilePitch >> projectileSpeed;

                WoWGuid craftingNpc;
                if (!readPackedGuid(packet, craftingNpc))
                    return false;

                uint32_t extraCurrencyCostCount = 0;
                uint32_t craftingReagentCount = 0;
                uint32_t removedReagentCount = 0;
                uint8_t craftingCastFlags = 0;
                packet >> extraCurrencyCostCount >> craftingReagentCount >> removedReagentCount >> craftingCastFlags;

                // The supplied 70009 gameplay captures use no crafting payload here. Keep the
                // parser strict until those variable-length structures are captured and verified.
                if (extraCurrencyCostCount != 0 || craftingReagentCount != 0 || removedReagentCount != 0)
                    return false;

                // Start of a new bit block. ByteBuffer does not expose a read-bit reset, so
                // consume the verified 70009 zero-weight header as its raw MSB-first byte.
                uint8_t optionalHeader = 0;
                packet >> optionalHeader;
                const bool hasReceiveTime = (optionalHeader & 0x80U) != 0;
                hasMovementData = (optionalHeader & 0x40U) != 0;
                const uint32_t weightCount = (optionalHeader >> 4U) & 0x03U;
                const bool hasCraftingOrderId = (optionalHeader & 0x08U) != 0;

                if (weightCount != 0 || hasMovementData)
                    return false;

                if (hasReceiveTime)
                    packet.readSkip<uint32_t>();
                if (hasCraftingOrderId)
                    packet.readSkip<uint64_t>();

                hasSrcLocation = (targets.getTargetMask() & TARGET_FLAG_SOURCE_LOCATION) != 0;
                hasDestLocation = (targets.getTargetMask() & TARGET_FLAG_DEST_LOCATION) != 0;
                castCount = static_cast<uint8_t>(clientCastId.getModernCounter() & 0xFFU);

                return !packet.hadReadFailure();
            }

            if (m_protocol.expansion != WoW::Expansion::_Mop)
            {
                if (m_protocol.expansion <= WoW::Expansion::_TBC)
                {
                    packet >> spellId >> castCount;
                }
                else if (m_protocol.expansion == WoW::Expansion::_WotLK)
                {
                    packet >> castCount >> spellId >> castFlags;
                }
                else if (m_protocol.expansion == WoW::Expansion::_Cata)
                {
                    packet >> castCount >> spellId >> glyphSlot >> castFlags;
                }

                targets.read(packet);

                if (castFlags & 0x02)
                {
                    hasAdditionalData = true;
                    packet >> projectilePitch >> projectileSpeed >> hasMovementData;
                }
            }
            else // Mop
            {
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
                uint8_t researchCount = static_cast<uint8_t>(packet.readBits(2));
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
                        movementInfo.flags2 = static_cast<uint16_t>(packet.readBits(13));
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
                    castFlags = static_cast<uint8_t>(packet.readBits(5));

                if (hasTargetString)
                    targetStringLength = packet.readBits(7);

                for (uint8_t i = 0; i < researchCount; ++i)
                {
                    packet.readSkip<uint32_t>();
                    packet.readSkip<uint32_t>();
                }

                if (hasMovement)
                {
                    packet >> movementInfo.position.x;
                    packet.readByteSeq(movementInfo.guid[0]);

                    if (hasTransport)
                    {
                        packet.readByteSeq(movementInfo.transport_guid[2]);
                        packet >> movementInfo.transport_seat;
                        packet.readByteSeq(movementInfo.transport_guid[3]);
                        packet.readByteSeq(movementInfo.transport_guid[7]);
                        packet >> movementInfo.transport_position.x;
                        packet.readByteSeq(movementInfo.transport_guid[5]);

                        if (movementInfo.status_info.hasTransportTime3)
                            packet >> movementInfo.transport_time3;

                        packet >> movementInfo.transport_position.z;
                        packet >> movementInfo.transport_position.y;

                        packet.readByteSeq(movementInfo.transport_guid[6]);
                        packet.readByteSeq(movementInfo.transport_guid[1]);
                        packet >> movementInfo.transport_position.o;

                        packet.readByteSeq(movementInfo.transport_guid[4]);

                        if (movementInfo.status_info.hasTransportTime2)
                            packet >> movementInfo.transport_time2;

                        packet.readByteSeq(movementInfo.transport_guid[0]);
                        packet >> movementInfo.transport_time;
                    }

                    packet.readByteSeq(movementInfo.guid[5]);

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

                    packet.readByteSeq(movementInfo.guid[6]);

                    if (hasUnkField)
                        packet.readSkip<uint32_t>();

                    packet.readByteSeq(movementInfo.guid[4]);

                    if (movementInfo.status_info.hasOrientation)
                        packet >> movementInfo.position.o;

                    if (movementInfo.status_info.hasTimeStamp)
                        packet >> movementInfo.update_time;

                    packet.readByteSeq(movementInfo.guid[1]);

                    if (movementInfo.status_info.hasPitch)
                        packet >> movementInfo.pitch_rate;

                    packet.readByteSeq(movementInfo.guid[3]);

                    for (uint32_t i = 0; i != unkCounter; i++)
                        packet.readSkip<uint32_t>();

                    packet >> movementInfo.position.y;
                    packet.readByteSeq(movementInfo.guid[7]);
                    packet >> movementInfo.position.z;
                    packet.readByteSeq(movementInfo.guid[2]);
                }

                packet.readByteSeq(itemTargetGuid[4]);
                packet.readByteSeq(itemTargetGuid[2]);
                packet.readByteSeq(itemTargetGuid[1]);
                packet.readByteSeq(itemTargetGuid[5]);
                packet.readByteSeq(itemTargetGuid[7]);
                packet.readByteSeq(itemTargetGuid[3]);
                packet.readByteSeq(itemTargetGuid[6]);
                packet.readByteSeq(itemTargetGuid[0]);

                if (hasDestLocation)
                {
                    float x, y, z;
                    packet.readByteSeq(destTransGuid[2]);
                    packet >> x;
                    packet.readByteSeq(destTransGuid[4]);
                    packet.readByteSeq(destTransGuid[1]);
                    packet.readByteSeq(destTransGuid[0]);
                    packet.readByteSeq(destTransGuid[3]);
                    packet >> y;
                    packet.readByteSeq(destTransGuid[7]);
                    packet >> z;
                    packet.readByteSeq(destTransGuid[5]);
                    packet.readByteSeq(destTransGuid[6]);

                    targets.setDestination({ x, y, z });
                    targets.setTransportDestinationGuid(destTransGuid);
                }

                packet.readByteSeq(targetGuid[3]);
                packet.readByteSeq(targetGuid[4]);
                packet.readByteSeq(targetGuid[7]);
                packet.readByteSeq(targetGuid[6]);
                packet.readByteSeq(targetGuid[2]);
                packet.readByteSeq(targetGuid[0]);
                packet.readByteSeq(targetGuid[1]);
                packet.readByteSeq(targetGuid[5]);

                if (hasSrcLocation)
                {
                    float x, y, z;
                    packet >> y;
                    packet.readByteSeq(srcTransGuid[5]);
                    packet.readByteSeq(srcTransGuid[1]);
                    packet.readByteSeq(srcTransGuid[7]);
                    packet.readByteSeq(srcTransGuid[6]);
                    packet >> x;
                    packet.readByteSeq(srcTransGuid[3]);
                    packet.readByteSeq(srcTransGuid[2]);
                    packet.readByteSeq(srcTransGuid[0]);
                    packet.readByteSeq(srcTransGuid[4]);
                    packet >> z;

                    targets.setSource({ x, y, z });
                    targets.setUnitTarget(srcTransGuid);
                }

                if (hasTargetString)
                    targets.setStringTarget(packet.readString(targetStringLength));

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
            }
            return true;
        }
    };
}
