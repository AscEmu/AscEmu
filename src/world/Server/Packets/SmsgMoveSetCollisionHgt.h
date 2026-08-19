/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    // reason for a collision height change - used by the Cata wire format of SMSG_MOVE_SET_COLLISION_HGT
    // (0 = Mount, 1 = Scale; AscEmu currently only ever sends Mount, Scale is provided for future use)
    enum class CollisionHeightChangeReason : uint8_t
    {
        Mount = 0,
        Scale = 1
    };

    class SmsgMoveSetCollisionHgt : public ManagedPacket
    {
    public:
        WoWGuid guid;
        uint32_t packetCounter;
        float collisionHeight;
        uint32_t mountDisplayId;
        CollisionHeightChangeReason reason;

        SmsgMoveSetCollisionHgt() : SmsgMoveSetCollisionHgt(WoWGuid(), 0, 0.0f)
        {
        }

        SmsgMoveSetCollisionHgt(WoWGuid guid, uint32_t packetCounter, float collisionHeight,
            uint32_t mountDisplayId = 0, CollisionHeightChangeReason reason = CollisionHeightChangeReason::Mount) :
            ManagedPacket(SMSG_MOVE_SET_COLLISION_HGT, 16),
            guid(guid), packetCounter(packetCounter), collisionHeight(collisionHeight),
            mountDisplayId(mountDisplayId), reason(reason)
        {
        }

    protected:
        size_t expectedSize() const override { return sizeof(uint64_t) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(float); }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                const bool hasMountDisplayId = mountDisplayId != 0;

                packet.writeBit(guid[7]);
                packet.writeBit(guid[0]);
                packet.writeBit(guid[1]);
                packet.writeBit(!hasMountDisplayId);
                packet.writeBit(guid[3]);
                packet.writeBit(0);
                packet.writeBit(0);
                packet.writeBit(guid[2]);
                packet.writeBit(guid[6]);
                packet.writeBit(guid[5]);
                packet.writeBit(guid[4]);

                packet.flushBits();

                packet << float(collisionHeight); // Height
                if (hasMountDisplayId)
                    packet << uint32_t(mountDisplayId);
                packet.writeByteSeq(guid[3]);
                packet.writeByteSeq(guid[2]);
                packet.writeByteSeq(guid[5]);
                packet.writeByteSeq(guid[6]);
                packet << uint32_t(packetCounter);
                packet << float(1.0f); // Scale
                packet.writeByteSeq(guid[7]);
                packet.writeByteSeq(guid[1]);
                packet.writeByteSeq(guid[4]);
                packet.writeByteSeq(guid[0]);

                return true;
            }
            else if (m_protocol.isCata())
            {
                packet.writeBits(static_cast<uint8_t>(reason), 2);
                packet.writeBit(guid[6]);
                packet.writeBit(guid[1]);
                packet.writeBit(guid[4]);
                packet.writeBit(guid[7]);
                packet.writeBit(guid[5]);
                packet.writeBit(guid[2]);
                packet.writeBit(guid[0]);
                packet.writeBit(guid[3]);

                packet.flushBits();

                packet.writeByteSeq(guid[6]);
                packet.writeByteSeq(guid[0]);
                packet.writeByteSeq(guid[4]);
                packet.writeByteSeq(guid[3]);
                packet.writeByteSeq(guid[5]);
                packet << uint32_t(packetCounter);
                packet.writeByteSeq(guid[1]);
                packet.writeByteSeq(guid[2]);
                packet.writeByteSeq(guid[7]);
                packet << float(collisionHeight);

                return true;
            }
            else
            {
                if (m_protocol.isWotlk())
                {
                    ByteBuffer guidData;
                    guidData << guid;

                    packet.append(guidData);
                    packet << uint32_t(packetCounter); // Packet counter
                    packet << collisionHeight;

                    return true;
                }
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
