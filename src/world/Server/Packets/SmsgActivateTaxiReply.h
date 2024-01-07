/*
Copyright (c) 2014-2024 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgActivateTaxiReply : public ManagedPacket
    {
    public:
        uint32_t error;

        SmsgActivateTaxiReply() : SmsgActivateTaxiReply(0)
        {
        }

        SmsgActivateTaxiReply(uint32_t error) :
            ManagedPacket(SMSG_ACTIVATE_TAXI_REPLY, 0),
            error(error)
        {
        }

    protected:
        size_t expectedSize() const override { return 4; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << error;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
