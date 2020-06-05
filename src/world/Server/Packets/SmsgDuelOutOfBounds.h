/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgDuelOutOfBounds : public ManagedPacket
    {
    public:
        uint32_t timer;

        SmsgDuelOutOfBounds() : SmsgDuelOutOfBounds(0)
        {
        }

        SmsgDuelOutOfBounds(uint32_t timer) :
            ManagedPacket(SMSG_DUEL_OUTOFBOUNDS, 4),
            timer(timer)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << timer;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
