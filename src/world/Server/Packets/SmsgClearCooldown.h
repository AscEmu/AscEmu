/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgClearCooldown : public ManagedPacket
    {
    public:
        uint32_t spellId;
        uint64_t guid;
        
        SmsgClearCooldown() : SmsgClearCooldown(0, 0)
        {
        }

        SmsgClearCooldown(uint32_t spellId, uint64_t guid) :
            ManagedPacket(SMSG_CLEAR_COOLDOWN, 12),
            spellId(spellId),
            guid(guid)
        {
        }

    protected:
        size_t expectedSize() const override { return 12; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << spellId << guid;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
