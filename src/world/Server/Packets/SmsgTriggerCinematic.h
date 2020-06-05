/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgTriggerCinematic : public ManagedPacket
    {
    public:
        uint32_t cinematicId;

        SmsgTriggerCinematic() : SmsgTriggerCinematic(0)
        {
        }

        SmsgTriggerCinematic(uint32_t cinematicId) :
            ManagedPacket(SMSG_TRIGGER_CINEMATIC, 4),
            cinematicId(cinematicId)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << cinematicId;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
