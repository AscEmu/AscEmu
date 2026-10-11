/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgSuspendToken : public ManagedPacket
    {
    public:
        uint32_t sequenceIndex;
        uint8_t reason;

        SmsgSuspendToken(uint32_t sequenceIndex, uint8_t reason = 1) :
            ManagedPacket(SMSG_SUSPEND_TOKEN, 5),
            sequenceIndex(sequenceIndex),
            reason(reason)
        {
        }

    protected:
        size_t expectedSize() const override { return 5; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (!m_protocol.isForever())
                return false;

            packet << sequenceIndex;
            packet.writeBits(reason, 2);
            packet.flushBits();
            return true;
        }

        bool internalDeserialise(WorldPacket&) override { return false; }
    };
}
