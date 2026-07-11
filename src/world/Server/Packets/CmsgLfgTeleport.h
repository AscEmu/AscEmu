/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class CmsgLfgTeleport : public ManagedPacket
    {
    public:
        bool teleportOut;

        CmsgLfgTeleport() : CmsgLfgTeleport(false)
        {
        }

        CmsgLfgTeleport(bool teleportOut) :
            ManagedPacket(CMSG_LFG_TELEPORT, 1),
            teleportOut(teleportOut)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> teleportOut;
            return true;
        }
    };
}
