/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu { namespace Packets
{
    class CmsgStandStateChange : public ManagedPacket
    {
    public:
        uint8_t state;

        CmsgStandStateChange() : CmsgStandStateChange(0)
        {
        }

        CmsgStandStateChange(uint8_t state) :
            ManagedPacket(CMSG_STANDSTATECHANGE, 4),
            state(state)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            packet << state;
            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> state;
            return true;
        }
    };
}}
