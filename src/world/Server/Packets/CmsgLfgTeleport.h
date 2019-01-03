/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu { namespace Packets
{
    class CmsgLfgTeleport : public ManagedPacket
    {
#if VERSION_STRING > TBC
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
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> teleportOut;
            return true;
        }
#endif
    };
}}
