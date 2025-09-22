/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgPetCastFailed : public ManagedPacket
    {
    public:
        uint8_t unk;
        uint32_t spellId;
        uint8_t reason;

        SmsgPetCastFailed() : SmsgPetCastFailed(0, 0)
        {
        }

        SmsgPetCastFailed(uint32_t spellId, uint8_t reason) :
            ManagedPacket(SMSG_PET_CAST_FAILED, 0),
            unk(0),
            spellId(spellId),
            reason(reason)
        {
        }

    protected:
        size_t expectedSize() const override { return static_cast<size_t>(1 + 4 + 1); }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << unk << spellId << reason;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
