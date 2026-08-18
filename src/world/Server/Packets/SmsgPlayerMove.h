/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Objects/MovementInfo.hpp"

namespace AscEmu::Packets
{
    class SmsgPlayerMove : public ManagedPacket
    {
    public:
        MovementInfo mi;
        bool withGuid = true;

        SmsgPlayerMove() : SmsgPlayerMove(MovementInfo())
        {
        }

        SmsgPlayerMove(MovementInfo mi, bool withGuid = true) :
            ManagedPacket(SMSG_PLAYER_MOVE, 0),
            mi(mi), withGuid(withGuid)
        {
        }

    protected:
        size_t expectedSize() const override { return sizeof(mi); }

        bool internalSerialise(WorldPacket& packet) override
        {
            mi.write(packet, withGuid);
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
