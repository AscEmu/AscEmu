/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

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
            ManagedPacket(MSG_CHANNEL_UPDATE, 0),
            casterGuid(casterGuid),
            time(time)
        {
        }

    protected:
        size_t expectedSize() const override { return static_cast<size_t>(8 + 4); }

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
