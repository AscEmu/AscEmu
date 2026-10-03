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
            if (m_protocol.isLegion())
            {
                // one packet for every swing error: cannot attack, bad facing, not in range, dead target
                packet.initialize(SMSG_ATTACK_SWING_ERROR, 1);
                packet.writeBits(1, 2);
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
