/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgPowerUpdate : public ManagedPacket
    {
#if VERSION_STRING > TBC
    public:
        WoWGuid guid;
        uint8_t powerType;
        uint32_t power;

        SmsgPowerUpdate() : SmsgPowerUpdate(WoWGuid(), 0, 0)
        {
        }

        SmsgPowerUpdate(WoWGuid guid, uint8_t powerType, uint32_t power) :
            ManagedPacket(SMSG_POWER_UPDATE, 0),
            guid(guid),
            powerType(powerType),
            power(power)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 8 + 4;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << guid;

#if VERSION_STRING == Cata
            packet << uint32_t(1);
#endif
            packet << powerType << power;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
#endif
    };
}}
