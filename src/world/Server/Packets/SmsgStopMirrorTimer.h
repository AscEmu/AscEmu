/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgStopMirrorTimer : public ManagedPacket
    {
    public:
        uint32_t type;

        SmsgStopMirrorTimer() : SmsgStopMirrorTimer(0)
        {
        }

        SmsgStopMirrorTimer(uint32_t type) :
            ManagedPacket(SMSG_STOP_MIRROR_TIMER, 0),
            type(type)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 4;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << type;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
