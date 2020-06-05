/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgZoneUnderAttack : public ManagedPacket
    {
    public:
        uint32_t areaId;

        SmsgZoneUnderAttack() : SmsgZoneUnderAttack(0)
        {
        }

        SmsgZoneUnderAttack(uint32_t areaId) :
            ManagedPacket(SMSG_ZONE_UNDER_ATTACK, 4),
            areaId(areaId)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << areaId;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
