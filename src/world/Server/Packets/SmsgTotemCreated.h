/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgTotemCreated : public ManagedPacket
    {
    public:
        uint8_t slot;
        uint64_t guid;
        uint32_t duration;
        uint32_t spellId;

        SmsgTotemCreated() : SmsgTotemCreated(0, 0, 0, 0)
        {
        }

        SmsgTotemCreated(uint8_t slot, uint64_t guid, uint32_t duration, uint32_t spellId) :
            ManagedPacket(SMSG_TOTEM_CREATED, 0),
            slot(slot),
            guid(guid),
            duration(duration),
            spellId(spellId)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 1 + 8 + 4 + 4;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << slot << guid << duration << spellId;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
