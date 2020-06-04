/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgEnchantmentLog : public ManagedPacket
    {
    public:
        uint64_t targetGuid;
        uint64_t casterGuid;
        uint32_t itemId;
        uint32_t enchantmentId;

        SmsgEnchantmentLog() : SmsgEnchantmentLog(0, 0, 0, 0)
        {
        }

        SmsgEnchantmentLog(uint64_t targetGuid, uint64_t casterGuid, uint32_t itemId, uint32_t enchantmentId) :
            ManagedPacket(SMSG_ENCHANTMENTLOG, 24),
            targetGuid(targetGuid),
            casterGuid(casterGuid),
            itemId(itemId),
            enchantmentId(enchantmentId)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            packet << targetGuid << casterGuid << itemId << enchantmentId;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
    };
}
