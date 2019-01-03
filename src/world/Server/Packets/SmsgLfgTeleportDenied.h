/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgLfgTeleportDenied : public ManagedPacket
    {
#if VERSION_STRING > TBC
    public:
        uint32_t error;

        SmsgLfgTeleportDenied() : SmsgLfgTeleportDenied(0)
        {
        }

        SmsgLfgTeleportDenied(uint32_t error) :
            ManagedPacket(SMSG_LFG_TELEPORT_DENIED, 0),
            error(error)
        {
        }

    protected:

        size_t expectedSize() const override { return 4; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << error;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
#endif
    };
}}
