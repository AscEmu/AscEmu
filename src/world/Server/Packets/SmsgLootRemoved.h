/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgLootRemoved : public ManagedPacket
    {
    public:
        uint8_t slot;

        SmsgLootRemoved() : SmsgLootRemoved(0)
        {
        }

        SmsgLootRemoved(uint8_t slot) :
            ManagedPacket(SMSG_LOOT_REMOVED, 1),
            slot(slot)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << slot;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
