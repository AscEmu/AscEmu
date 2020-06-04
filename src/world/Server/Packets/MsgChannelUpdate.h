/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include <utility>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class MsgChannelUpdate : public ManagedPacket
    {
    public:
        WoWGuid casterGuid;
        uint32_t time;

        MsgChannelUpdate() : MsgChannelUpdate(WoWGuid(), 0)
        {
        }

        MsgChannelUpdate(WoWGuid casterGuid, uint32_t time) :
            ManagedPacket(MSG_CHANNEL_UPDATE, 8 + 4),
            casterGuid(casterGuid),
            time(time)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            packet << casterGuid << time;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
    };
}
