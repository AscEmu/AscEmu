/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgCharacterEnumPreludeExtended final : public ManagedPacket
    {
    public:
        // [FOREVER-VERIFIED] Exact 0x0046021C wire shape from the working capture.
        uint64_t unknown0{2};
        uint64_t unknown1{0};
        uint64_t unknown2{0};

        SmsgCharacterEnumPreludeExtended() : ManagedPacket(SMSG_CHARACTER_ENUM_PRELUDE_EXTENDED, 24) {}

    protected:
        size_t expectedSize() const override { return 24; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (!m_protocol.isForever())
                return false;

            packet << unknown0 << unknown1 << unknown2;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
