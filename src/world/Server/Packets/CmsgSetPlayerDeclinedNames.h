/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu::Packets
{
    class CmsgSetPlayerDeclinedNames : public ManagedPacket
    {
    public:
        std::array<std::string, 5/*MAX_DECLINED_NAME_CASES*/> declined;

        CmsgSetPlayerDeclinedNames() : ManagedPacket(CMSG_SET_PLAYER_DECLINED_NAMES, 0) {}

        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            for (uint8_t i = 0; i < 5/*MAX_DECLINED_NAME_CASES*/; ++i)
                packet >> declined[i];

            return true;
        }
    };
}
