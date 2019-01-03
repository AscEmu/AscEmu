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
    class CmsgLootRelease : public ManagedPacket
    {
    public:
        WoWGuid guid;

        CmsgLootRelease() : CmsgLootRelease(0)
        {
        }

        CmsgLootRelease(uint64_t guid) :
            ManagedPacket(CMSG_ACTIVATETAXI, 8),
            guid(guid)
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
            guid.Init(unpackedGuid);
            return true;
        }
    };
}}
