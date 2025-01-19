/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgLoginSetTimeSpeed : public ManagedPacket
    {
    public:
        uint32_t time;
        float gameSpeed;
        
        SmsgLoginSetTimeSpeed() : SmsgLoginSetTimeSpeed(0, 0)
        {
        }

        SmsgLoginSetTimeSpeed(uint32_t time, float gameSpeed) :
            ManagedPacket(SMSG_LOGIN_SET_TIME_SPEED, 8),
            time(time),
            gameSpeed(gameSpeed)
        {
        }

    protected:
        size_t expectedSize() const override { return 8; }

        bool internalSerialise(WorldPacket& packet) override
        {
#if VERSION_STRING < Mop
            packet << time << gameSpeed;
#if VERSION_STRING > TBC
            packet << uint32_t(0);
#endif
#else
            packet << uint32_t(0) << time << uint32_t(0) << time << gameSpeed;
#endif

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
