/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgSetPlayerDeclinedNamesResult : public ManagedPacket
    {
    public:
        uint8_t result;

        enum Result : uint8_t
        {
            OK              = 0,
            ERROR_INVALID   = 1,
            // ERROR_NOT_FOUND = 2
        };

        SmsgSetPlayerDeclinedNamesResult() : SmsgSetPlayerDeclinedNamesResult(OK)
        {
        }

        SmsgSetPlayerDeclinedNamesResult(uint8_t result) :
            ManagedPacket(SMSG_SET_PLAYER_DECLINED_NAMES_RESULT, 1),
            result(result)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            packet << result;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
    };
}
