/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

#include "Spell/SpellCastTargets.hpp"
#include "Spell/Definitions/SpellCastTargetFlags.hpp"
#include "Objects/MovementInfo.hpp"
#include "Version/VersionRegistry.hpp"

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

        // 7.x and later: the id the client gave the cast, SMSG_SPELL_PREPARE links it to the cast id of the server
        WoWGuid128 clientCastGuid;

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

        // CMSG_USE_ITEM of 6.x and later clients carries the same cast request
        bool readCastRequest(WorldPacket& packet, WoW::ClientProtocol const& protocol)
        {
            setClientProtocol(protocol);
            return readGuid128Request(packet);
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isWoD() || m_protocol.isLegion() || m_protocol.isBfA() || m_protocol.isShadowlands() || m_protocol.isDragonflight() || m_protocol.isTheWarWithinOrLater())
                return readGuid128Request(packet);

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

    private:
        // 6.x and later: a target location is the transport and the position on it
        void readTargetLocation(WorldPacket& packet, bool isSource)
        {
            WoWGuid128 transportGuid;
            LocationVector location;
            packet >> transportGuid >> location;

            const uint64_t transport = WoWGuid::fromGuid128(transportGuid).getRawGuid();
            if (isSource)
            {
                targets.setSource(location);
                targets.setTransportSourceGuid(transport);
            }
            else
            {
                targets.setDestination(location);
                targets.setTransportDestinationGuid(transport);
            }
        }

        // 6.x to 10.x: the presence bits lead, 11.x and later: flags and guids lead
        void readTargetData(WorldPacket& packet)
        {
            uint32_t targetFlags = 0;
            uint32_t nameLength = 0;
            bool hasOrientation = false;
            bool hasMapId = false;
            WoWGuid128 objectGuid;
            WoWGuid128 itemGuid;

            if (m_protocol.isTheWarWithinOrLater())
            {
                WoWGuid128 extraGuid;                   // 11.x unknown, 12.x housing
                packet >> targetFlags >> objectGuid >> itemGuid >> extraGuid;

                packet.resetBitPos();
                packet.readBit();                       // 11.x unknown, 12.x housing resident
                hasSrcLocation = packet.readBit();
                hasDestLocation = packet.readBit();
                hasOrientation = packet.readBit();
                hasMapId = packet.readBit();
                nameLength = packet.readBits(7);
            }
            else
            {
                int32_t flagBits = 23;
                if (m_protocol.isLegion())
                    flagBits = 25;
                else if (m_protocol.isBfA() || m_protocol.isShadowlands())
                    flagBits = 26;
                else if (m_protocol.isDragonflight())
                    flagBits = 28;

                packet.resetBitPos();
                targetFlags = packet.readBits(flagBits);
                hasSrcLocation = packet.readBit();
                hasDestLocation = packet.readBit();
                hasOrientation = packet.readBit();
                if (!m_protocol.isWoD())
                    hasMapId = packet.readBit();
                nameLength = packet.readBits(7);

                packet >> objectGuid >> itemGuid;
            }

            if (hasSrcLocation)
                readTargetLocation(packet, true);

            if (hasDestLocation)
                readTargetLocation(packet, false);

            if (hasOrientation)
                packet.readSkip<float>();

            if (hasMapId)
                packet.readSkip<int32_t>();

            const std::string name = packet.readString(nameLength);

            targets.setTargetMask(targetFlags);

            // one guid for the unit or game object target
            const uint64_t object = WoWGuid::fromGuid128(objectGuid).getRawGuid();
            if (targetFlags & (TARGET_FLAG_OBJECT | TARGET_FLAG_OPEN_LOCK))
                targets.setGameObjectTarget(object);
            else
                targets.setUnitTarget(object);

            targets.setItemTarget(WoWGuid::fromGuid128(itemGuid).getRawGuid());

            if (!name.empty())
                targets.setStringTarget(name);
        }

        // crafting reagents (10.x and later) are not used by the server, they are only read past
        void skipCraftingReagent(WorldPacket& packet)
        {
            if (m_protocol.isMidnight())
            {
                packet.readSkip<int32_t>();             // slot
                packet.readSkip<int32_t>();             // quantity

                packet.resetBitPos();
                const bool hasItem = packet.readBit();
                const bool hasCurrency = packet.readBit();

                if (hasItem)
                    packet.readSkip<int32_t>();

                if (hasCurrency)
                    packet.readSkip<int32_t>();

                // the source bit shares the byte of the reagent bits unless a value came in between
                if (hasItem || hasCurrency)
                    packet.resetBitPos();
            }
            else
            {
                packet.readSkip<int32_t>();             // item
                packet.readSkip<int32_t>();             // data slot
                packet.readSkip<int32_t>();             // quantity
                packet.resetBitPos();
            }

            if (packet.readBit())
                packet.readSkip<uint8_t>();             // source
        }

        bool readGuid128Request(WorldPacket& packet)
        {
            const bool isWoD = m_protocol.isWoD();
            const bool isLegion = m_protocol.isLegion();
            const bool isBfA = m_protocol.isBfA();
            const bool isShadowlands = m_protocol.isShadowlands();
            const bool isDragonflight = m_protocol.isDragonflight();
            const bool isTheWarWithin = m_protocol.isTheWarWithin();
            const bool isMidnight = m_protocol.isMidnight();

            castCount = 0;
            castFlags = 0;

            uint32_t weightCount = 0;
            uint32_t currencyCount = 0;
            uint32_t reagentCount = 0;
            uint32_t removedCount = 0;
            bool hasMovement = false;
            bool hasCraftingOrderId = false;
            bool hasReceiveTime = false;

            // 6.x: cast count, 7.x and later: cast guid
            if (isWoD)
                packet >> castCount;
            else
                packet >> clientCastGuid;

            if (isTheWarWithin || isMidnight)
                packet >> castFlags;

            // misc values, 12.x has a third one
            packet.readSkip<int32_t>();
            packet.readSkip<int32_t>();
            if (isMidnight)
                packet.readSkip<int32_t>();

            packet >> spellId;

            // visual, 9.x and later followed by the script visual
            packet.readSkip<int32_t>();
            if (!isWoD && !isLegion && !isBfA)
                packet.readSkip<int32_t>();

            if (isWoD || isMidnight)
                readTargetData(packet);

            packet >> projectilePitch >> projectileSpeed;

            WoWGuid128 charmerOrCraftingNpc;
            packet >> charmerOrCraftingNpc;

            if (isShadowlands)
            {
                packet >> reagentCount >> currencyCount;
                if (reagentCount > 6 || currencyCount > 5)
                    return false;

                for (uint32_t i = 0; i < reagentCount; ++i)
                {
                    packet.readSkip<int32_t>();         // item
                    packet.readSkip<int32_t>();         // slot
                }

                for (uint32_t i = 0; i < currencyCount; ++i)
                {
                    packet.readSkip<int32_t>();         // currency
                    packet.readSkip<int32_t>();         // count
                }
            }
            else if (isDragonflight || isTheWarWithin || isMidnight)
            {
                packet >> currencyCount >> reagentCount >> removedCount;
                if (currencyCount > 5 || reagentCount > 6 || removedCount > 6)
                    return false;

                if (isTheWarWithin || isMidnight)
                    packet.readSkip<uint8_t>();         // crafting flags

                for (uint32_t i = 0; i < currencyCount; ++i)
                {
                    packet.readSkip<int32_t>();         // currency
                    packet.readSkip<int32_t>();         // count
                }
            }

            if (isMidnight)
            {
                for (uint32_t i = 0; i < reagentCount; ++i)
                    skipCraftingReagent(packet);

                for (uint32_t i = 0; i < removedCount; ++i)
                    skipCraftingReagent(packet);

                packet.resetBitPos();
                hasReceiveTime = packet.readBit();
                hasMovement = packet.readBit();
                weightCount = packet.readBits(2);
                hasCraftingOrderId = packet.readBit();

                if (hasReceiveTime)
                    packet.readSkip<uint32_t>();
            }
            else if (isTheWarWithin)
            {
                readTargetData(packet);

                packet.resetBitPos();
                hasMovement = packet.readBit();
                weightCount = packet.readBits(2);
                hasCraftingOrderId = packet.readBit();

                for (uint32_t i = 0; i < reagentCount; ++i)
                    skipCraftingReagent(packet);

                if (hasCraftingOrderId)
                    packet.readSkip<uint64_t>();

                for (uint32_t i = 0; i < removedCount; ++i)
                    skipCraftingReagent(packet);
            }
            else
            {
                packet.resetBitPos();
                castFlags = static_cast<uint8_t>(packet.readBits(5));
                hasMovement = packet.readBit();
                weightCount = packet.readBits(2);
                if (isDragonflight)
                    hasCraftingOrderId = packet.readBit();

                if (!isWoD)
                    readTargetData(packet);

                if (isDragonflight)
                {
                    if (hasCraftingOrderId)
                        packet.readSkip<uint64_t>();

                    for (uint32_t i = 0; i < reagentCount; ++i)
                        skipCraftingReagent(packet);

                    for (uint32_t i = 0; i < removedCount; ++i)
                        skipCraftingReagent(packet);
                }
            }

            // the movement update has the layout of a heartbeat, the server only reads past it
            if (hasMovement)
            {
                MovementInfo movementInfo;
                packet.resetBitPos();
                movementInfo.readMovementInfo(packet, Version::opcodeHexFor(MSG_MOVE_HEARTBEAT, m_protocol), m_protocol.expansion);
            }

            for (uint32_t i = 0; i < weightCount; ++i)
            {
                packet.resetBitPos();
                packet.readBits(2);                     // type
                packet.readSkip<int32_t>();             // id
                packet.readSkip<uint32_t>();            // quantity
            }

            if (isMidnight && hasCraftingOrderId)
                packet.readSkip<uint64_t>();

            return true;
        }
    };
}
