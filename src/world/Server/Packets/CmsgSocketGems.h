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
    class CmsgSocketGems : public ManagedPacket
    {
    public:
        uint64_t itemGuid;
        uint64_t gemGuid[3];

        CmsgSocketGems() : CmsgSocketGems(0)
        {
        }

        CmsgSocketGems(uint64_t itemGuid) :
            ManagedPacket(CMSG_SOCKET_GEMS, 0),
            itemGuid(itemGuid)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> itemGuid;

            for (auto i = 0; i < 3; ++i)
                packet >> gemGuid[i];

            return true;
        }
    };
}}
