/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgSpellFailedOther : public ManagedPacket
    {
    public:
        WoWGuid casterGuid;
        uint8_t castNumber;
        uint32_t spellId;
        uint8_t result;

        SmsgSpellFailedOther() : SmsgSpellFailedOther(WoWGuid(), 0, 0, 0)
        {
        }

        SmsgSpellFailedOther(WoWGuid casterGuid, uint8_t castNumber, uint32_t spellId, uint8_t result) :
            ManagedPacket(SMSG_SPELL_FAILED_OTHER, 8 + 4),
            casterGuid(casterGuid),
            castNumber(castNumber),
            spellId(spellId),
            result(result)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            packet << casterGuid;
#if VERSION_STRING > TBC
            packet << castNumber;
#endif
            packet << spellId << result;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
    };
}
