/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "ForeverSpellPacketUtils.hpp"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgPetCastFailed : public ManagedPacket
    {
    public:
        uint8_t unk;
        uint32_t spellId;
        int32_t reason;
        WoWGuid castId = WoWGuid::createModernEmpty();
        uint32_t failedArg1 = 0;
        uint32_t failedArg2 = 0;

        SmsgPetCastFailed() : SmsgPetCastFailed(0, 0)
        {
        }

        SmsgPetCastFailed(uint32_t spellId, int32_t reason) :
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
            if (m_protocol.isForever())
            {
                ForeverSpellPacket::writePackedGuid(packet, castId);
                packet << static_cast<int32_t>(spellId);
                packet << static_cast<int32_t>(reason);
                packet << static_cast<int32_t>(failedArg1);
                packet << static_cast<int32_t>(failedArg2);
                return true;
            }

            packet << unk << spellId << static_cast<uint8_t>(reason);
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
