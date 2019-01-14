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
    class CmsgEnabletaxi : public ManagedPacket
    {
#if VERSION_STRING > TBC
    public:
        WoWGuid creatureGuid;

        CmsgEnabletaxi() : CmsgEnabletaxi(0)
        {
        }

        CmsgEnabletaxi(uint64_t creatureGuid) :
            ManagedPacket(CMSG_ENABLETAXI, 8),
            creatureGuid(creatureGuid)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            uint64_t unpackedGuid;
            packet >> unpackedGuid;
            creatureGuid.Init(unpackedGuid);
            return true;
        }
#endif
    };
}}
