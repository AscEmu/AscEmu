/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgPetActionFeedback : public ManagedPacket
    {
    public:
        uint8_t message;

        SmsgPetActionFeedback() : SmsgPetActionFeedback(0)
        {
        }

        SmsgPetActionFeedback(uint8_t message) :
            ManagedPacket(SMSG_PET_ACTION_FEEDBACK, 1),
            message(message)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << message;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
