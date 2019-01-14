/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgClientControlUpdate : public ManagedPacket
    {
    public:
        WoWGuid targetGuid;
        uint8_t allowMove;

        SmsgClientControlUpdate() : SmsgClientControlUpdate(WoWGuid(), 0)
        {
        }

        SmsgClientControlUpdate(WoWGuid targetGuid, uint8_t allowMove) :
            ManagedPacket(SMSG_CLIENT_CONTROL_UPDATE, 0),
            targetGuid(targetGuid),
            allowMove(allowMove)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 8 + 1;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << targetGuid << allowMove;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
