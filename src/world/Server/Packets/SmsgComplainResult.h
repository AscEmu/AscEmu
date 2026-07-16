/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgComplainResult : public ManagedPacket
    {
    public:
        uint8_t result;

        SmsgComplainResult() : SmsgComplainResult(0)
        {
        }

        SmsgComplainResult(uint8_t result) :
            ManagedPacket(SMSG_COMPLAIN_RESULT, 1),
            result(result)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << result;

            if (m_protocol.expansion == WoW::Expansion::_Mop)
                packet << uint8_t(0);

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
