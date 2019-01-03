/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class MsgRandomRoll : public ManagedPacket
    {
    public:
        uint32_t min;
        uint32_t max;
        uint32_t roll;
        uint64_t guid;

        MsgRandomRoll() : MsgRandomRoll(0, 0, 0, 0)
        {
        }

        MsgRandomRoll(uint32_t min, uint32_t max, uint32_t roll, uint64_t guid) :
            ManagedPacket(MSG_RANDOM_ROLL, 20),
            min(min),
            max(max),
            roll(roll),
            guid(guid)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            packet << min << max << roll << guid;
            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> min >> max;
            return true;
        }
    };
}}
