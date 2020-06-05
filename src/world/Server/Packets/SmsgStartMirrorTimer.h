/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgStartMirrorTimer : public ManagedPacket
    {
    public:
        uint32_t type;
        uint32_t current;
        uint32_t max;
        int32_t regen;
        uint8_t paused;
        uint32_t spellId;

        SmsgStartMirrorTimer() : SmsgStartMirrorTimer(0, 0, 0, 0)
        {
        }

        SmsgStartMirrorTimer(uint32_t type, uint32_t current, uint32_t max, int32_t regen, uint8_t paused = 0, uint32_t spellId = 0) :
            ManagedPacket(SMSG_START_MIRROR_TIMER, 21),
            type(type),
            current(current),
            max(max),
            regen(regen),
            paused(paused),
            spellId(spellId)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
#if VERSION_STRING < Mop
            packet << type << current << max << regen << paused << spellId;
#else
            packet << max << spellId << type << regen << current;
            packet.writeBit(paused);
            packet.flushBits();
#endif
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
