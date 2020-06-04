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
    class SmsgSpellDelayed : public ManagedPacket
    {
    public:
        WoWGuid casterGuid;
        uint32_t delay;

        SmsgSpellDelayed() : SmsgSpellDelayed(WoWGuid(), 0)
        {
        }

        SmsgSpellDelayed(WoWGuid casterGuid, uint32_t delay) :
            ManagedPacket(SMSG_SPELL_DELAYED, 8 + 4),
            casterGuid(casterGuid),
            delay(delay)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            packet << casterGuid << delay;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
    };
}
