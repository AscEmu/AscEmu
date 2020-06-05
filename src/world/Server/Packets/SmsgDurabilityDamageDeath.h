/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgDurabilityDamageDeath : public ManagedPacket
    {
    public:

        uint32_t percent;

        SmsgDurabilityDamageDeath(uint32_t percent = 0) :
            ManagedPacket(SMSG_DURABILITY_DAMAGE_DEATH, 0),
            percent(percent)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
#if VERSION_STRING > WotLK
            packet << percent;
#endif
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
