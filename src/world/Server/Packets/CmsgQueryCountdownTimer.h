/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgQueryCountdownTimer : public ManagedPacket
    {
    public:
        // 0 pvp preparation, 1 challenge mode, 2 player countdown
        int32_t timerType;

        CmsgQueryCountdownTimer() : CmsgQueryCountdownTimer(0)
        {
        }

        CmsgQueryCountdownTimer(int32_t timerType) :
            ManagedPacket(CMSG_QUERY_COUNTDOWN_TIMER, sizeof(int32_t)),
            timerType(timerType)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> timerType;
            return true;
        }
    };
}
