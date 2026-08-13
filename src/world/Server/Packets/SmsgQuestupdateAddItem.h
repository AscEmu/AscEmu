/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgQuestupdateAddItem : public ManagedPacket
    {
    public:
        uint32_t itemEntry;
        uint32_t count;

        SmsgQuestupdateAddItem() : SmsgQuestupdateAddItem(0, 0)
        {}

        SmsgQuestupdateAddItem(uint32_t itemEntry, uint32_t count) :
            ManagedPacket(SMSG_QUESTUPDATE_ADD_ITEM, 8),
            itemEntry(itemEntry),
            count(count)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << itemEntry << count;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
