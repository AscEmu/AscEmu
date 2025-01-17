/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgDestructibleBuildingDamage : public ManagedPacket
    {
#if VERSION_STRING > TBC
    public:
        WoWGuid goGuid;
        WoWGuid attackerGuid;
        WoWGuid controllerGuid;
        uint32_t damage;
        uint32_t spellId;

        SmsgDestructibleBuildingDamage() : SmsgDestructibleBuildingDamage(WoWGuid(), WoWGuid(), WoWGuid(), 0, 0)
        {
        }

        SmsgDestructibleBuildingDamage(WoWGuid goGuid, WoWGuid attackerGuid, WoWGuid controllerGuid, uint32_t damage, uint32_t spellId) :
            ManagedPacket(SMSG_DESTRUCTIBLE_BUILDING_DAMAGE, 8 + 8 + 8 + 4 + 4),
            goGuid(goGuid),
            attackerGuid(attackerGuid),
            controllerGuid(controllerGuid),
            damage(damage),
            spellId(spellId)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << goGuid << attackerGuid << controllerGuid << damage << spellId;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
#endif
    };
}
