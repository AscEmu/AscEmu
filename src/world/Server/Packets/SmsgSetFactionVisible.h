/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgSetFactionVisible : public ManagedPacket
    {
    public:
        uint32_t factionId;

        SmsgSetFactionVisible() : SmsgSetFactionVisible(0)
        {
        }

        SmsgSetFactionVisible(uint32_t factionId) :
            ManagedPacket(SMSG_SET_FACTION_VISIBLE, 4),
            factionId(factionId)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << factionId;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
