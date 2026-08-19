/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Objects/MovementInfo.hpp"

namespace AscEmu::Packets
{
    // SMSG_MOVE_UPDATE_COLLISION_HEIGHT - broadcast to nearby observers so they also learn about a
    // unit's new collision height (e.g. after mount/dismount). Does not exist before Cata (opcode is
    // 0x0000 for Classic/TBC/WotLK in Opcodes.hpp) - internalSerialise returns false for those versions.
    class SmsgMoveUpdateCollisionHeight : public ManagedPacket
    {
    public:
        MovementInfo mi;
        float collisionHeight;

        SmsgMoveUpdateCollisionHeight() : SmsgMoveUpdateCollisionHeight(MovementInfo(), 0.0f)
        {
        }

        SmsgMoveUpdateCollisionHeight(MovementInfo mi, float collisionHeight) :
            ManagedPacket(SMSG_MOVE_UPDATE_COLLISION_HEIGHT, 0),
            mi(mi), collisionHeight(collisionHeight)
        {
        }

    protected:
        size_t expectedSize() const override { return sizeof(mi) + sizeof(float); }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (!m_protocol.isCata() && !m_protocol.isMop())
                return false;

            mi.collisionHeight = collisionHeight;
            mi.write(packet, true);

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
