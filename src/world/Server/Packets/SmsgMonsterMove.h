/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    // Covers the simple "single-point" SMSG_MONSTER_MOVE construction shared by several
    // ad-hoc callers (Lua scripting, legacy spell effects, creature AI scripting). This is
    // distinct from the spline-driven SMSG_MONSTER_MOVE built by MoveSplineInit, which writes
    // a variable-length list of spline points via PacketBuilder::WriteMonsterMove and is not
    // covered by this class.
    class SmsgMonsterMove : public ManagedPacket
    {
    public:
        WoWGuid guid;
        float posX = 0.0f;
        float posY = 0.0f;
        float posZ = 0.0f;
        uint32_t moveTime = 0;
        bool hasOrientation = false;
        float orientation = 0.0f;
        uint32_t moveFlags = 0;
        uint32_t duration = 0;
        float destX = 0.0f;
        float destY = 0.0f;
        float destZ = 0.0f;

        SmsgMonsterMove() : SmsgMonsterMove(WoWGuid(), 0.0f, 0.0f, 0.0f, 0, 0, 0, 0.0f, 0.0f, 0.0f)
        {
        }

        SmsgMonsterMove(WoWGuid guid, float posX, float posY, float posZ, uint32_t moveTime,
            uint32_t moveFlags, uint32_t duration, float destX, float destY, float destZ,
            bool hasOrientation = false, float orientation = 0.0f) :
            ManagedPacket(SMSG_MONSTER_MOVE, 60),
            guid(guid), posX(posX), posY(posY), posZ(posZ), moveTime(moveTime),
            hasOrientation(hasOrientation), orientation(orientation),
            moveFlags(moveFlags), duration(duration),
            destX(destX), destY(destY), destZ(destZ)
        {
        }

    protected:
        size_t expectedSize() const override { return 60; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet << guid;

                // Present for every version except TBC (see Unit::setFacing's TBC vs non-TBC split).
                if (!m_protocol.isTbc())
                    packet << uint8_t(0); // vehicle seat index (unused by these simple single-point moves)

                packet << posX << posY << posZ;
                packet << moveTime;

                if (hasOrientation)
                {
                    packet << uint8_t(4);
                    packet << orientation;
                }
                else
                {
                    packet << uint8_t(0);
                }

                packet << moveFlags;
                packet << duration;
                packet << uint32_t(1); // point count, always 1 for this simple single-point variant
                packet << destX << destY << destZ;

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
