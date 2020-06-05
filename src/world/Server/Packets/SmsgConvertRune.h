/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgConvertRune : public ManagedPacket
    {
#if VERSION_STRING > TBC
    public:
        uint8_t slot;
        uint8_t type;

        SmsgConvertRune() : SmsgConvertRune(0, 0)
        {
        }

        SmsgConvertRune(uint8_t slot, uint8_t type) :
            ManagedPacket(SMSG_CONVERT_RUNE, 2),
            slot(slot),
            type(type)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << slot << type;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
#endif
    };
}
