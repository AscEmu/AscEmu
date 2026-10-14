/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgWaitQueueFinish : public ManagedPacket
    {
    public:
        SmsgWaitQueueFinish() : ManagedPacket(SMSG_WAIT_QUEUE_FINISH, 0) {}

    protected:
        bool internalSerialise(WorldPacket&) override { return true; }
        bool internalDeserialise(WorldPacket&) override { return false; }
    };
}
