/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class CmsgOptOutOfLoot : public ManagedPacket
    {
    public:
        uint32_t turnedOn;

        CmsgOptOutOfLoot() : CmsgOptOutOfLoot(0)
        {
        }

        CmsgOptOutOfLoot(uint32_t turnedOn) :
            ManagedPacket(CMSG_OPT_OUT_OF_LOOT, 0),
            turnedOn(turnedOn)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> turnedOn;
            return true;
        }
    };
}}
