/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgWorldStateUiTimerUpdate : public ManagedPacket
    {
#if VERSION_STRING > TBC
    public:
        uint32_t unixtime;

        SmsgWorldStateUiTimerUpdate() : SmsgWorldStateUiTimerUpdate(0)
        {
        }

        SmsgWorldStateUiTimerUpdate(uint32_t unixtime) :
            ManagedPacket(SMSG_WORLD_STATE_UI_TIMER_UPDATE, 4),
            unixtime(unixtime)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            packet << unixtime;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
#endif
    };
}}
