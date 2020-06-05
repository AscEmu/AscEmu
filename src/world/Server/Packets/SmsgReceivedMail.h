/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgReceivedMail : public ManagedPacket
    {
    public:
        uint32_t unknown;

        SmsgReceivedMail(uint32_t unknown = 0) :
            ManagedPacket(SMSG_RECEIVED_MAIL, 4),
            unknown(unknown)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << unknown;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
