/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgUiTime : public ManagedPacket
    {
    public:
        uint32_t currentTime;

        SmsgUiTime() : SmsgUiTime(0) {}

        explicit SmsgUiTime(uint32_t currentTime) :
            ManagedPacket(SMSG_UI_TIME, 4),
            currentTime(currentTime)
        {}

    protected:
        size_t expectedSize() const override { return 4; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << currentTime;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
