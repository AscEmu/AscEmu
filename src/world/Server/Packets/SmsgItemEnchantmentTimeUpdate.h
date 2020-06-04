/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgItemEnchantmentTimeUpdate : public ManagedPacket
    {
    public:
        uint64_t itemGuid;
        uint32_t slot;
        uint32_t duration;
        uint64_t playerGuid;

        SmsgItemEnchantmentTimeUpdate() : SmsgItemEnchantmentTimeUpdate(0, 0, 0, 0)
        {
        }

        SmsgItemEnchantmentTimeUpdate(uint64_t itemGuid, uint32_t slot, uint32_t duration, uint64_t playerGuid) :
            ManagedPacket(SMSG_ITEM_ENCHANT_TIME_UPDATE, 24),
            itemGuid(itemGuid),
            slot(slot),
            duration(duration),
            playerGuid(playerGuid)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            packet << itemGuid << slot << duration << playerGuid;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
    };
}
