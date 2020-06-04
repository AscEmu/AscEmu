/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include <utility>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgSpellOrDamageImmune : public ManagedPacket
    {
    public:
        uint64_t casterGuid;
        uint64_t targetGuid;
        uint32_t spellId;
        uint8_t logEnabled;

        SmsgSpellOrDamageImmune() : SmsgSpellOrDamageImmune(0, 0, 0, 0)
        {
        }

        SmsgSpellOrDamageImmune(uint64_t casterGuid, uint64_t targetGuid, uint32_t spellId, uint8_t logEnabled = 0) :
            ManagedPacket(SMSG_SPELLORDAMAGE_IMMUNE, 8 + 8 + 4 + 1),
            casterGuid(casterGuid),
            targetGuid(targetGuid),
            spellId(spellId),
            logEnabled(logEnabled)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            packet << casterGuid << targetGuid << spellId << logEnabled;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
    };
}
