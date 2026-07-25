/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

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

        bool internalSerialise([[maybe_unused]]WorldPacket& packet) override
        {
            if (m_protocol.expansion > WoW::Expansion::_WotLK)
            {
                packet << percent;
            }
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
