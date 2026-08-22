/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgDuelComplete : public ManagedPacket
    {
    public:
        uint8_t isCompleted;

        SmsgDuelComplete() : SmsgDuelComplete(0)
        {
        }

        SmsgDuelComplete(uint8_t isCompleted) :
            ManagedPacket(SMSG_DUEL_COMPLETE, 1),
            isCompleted(isCompleted)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                packet.writeBit(isCompleted);
                packet.flushBits();
                return true;
            }
            else if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                packet << isCompleted;
                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
