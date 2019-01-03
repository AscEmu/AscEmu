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
    class CmsgLoot : public ManagedPacket
    {
    public:
        uint64_t guid;

        CmsgLoot() : CmsgLoot(0)
        {
        }

        CmsgLoot(uint64_t guid) :
            ManagedPacket(CMSG_LOOT, 8),
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
            packet >> guid;
            if (guid == 0)
                return false;

            return true;
        }
    };
}}
