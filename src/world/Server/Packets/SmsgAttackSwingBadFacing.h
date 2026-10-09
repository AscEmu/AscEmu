/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

//\NOTE: This gets replaced in Mop by SMSG_ATTACKSWING_ERROR
namespace AscEmu::Packets
{
    class SmsgAttackSwingBadFacing : public ManagedPacket
    {
    public:

        SmsgAttackSwingBadFacing() :
            ManagedPacket(SMSG_ATTACKSWING_BADFACING, 0)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isWoD() || m_protocol.isLegion() || m_protocol.isBfA())
            {
                // one packet for every swing error; 6.x: cannot attack, not in range, bad facing, dead target,
                // 7.x: cannot attack, bad facing, not in range, dead target
                packet.initialize(SMSG_ATTACK_SWING_ERROR, 1);
                if (m_protocol.isBfA())
                    packet.writeBits(1, 3);             // 8.x: not in range, bad facing, cannot attack, dead target
                else
                    packet.writeBits(m_protocol.isWoD() ? 2 : 1, 2);
                packet.flushBits();
                return true;
            }

            if (m_protocol.isMop())
                return false;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
