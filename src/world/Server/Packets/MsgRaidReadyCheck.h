/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu::Packets
{
    class MsgRaidReadyCheck : public ManagedPacket
    {
    public:
        uint8_t isReady;
        uint64_t guid;
        bool isRequest;

        MsgRaidReadyCheck() : MsgRaidReadyCheck(0, 0, false)
        {
        }

        MsgRaidReadyCheck(uint64_t guid, uint8_t isReady, bool isRequest) :
            ManagedPacket(MSG_RAID_READY_CHECK, 1),
            isReady(isReady),
            guid(guid),
            isRequest(isRequest)
        {
        }

    protected:
        size_t expectedSize() const override { return isRequest ? 8 : 9; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (isRequest)
            {
                packet << guid;
            }
            else
            {
                packet << guid << isReady;
            }
            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> isReady;
            return true;
        }
    };
}
