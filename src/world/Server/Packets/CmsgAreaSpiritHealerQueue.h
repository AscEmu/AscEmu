/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu { namespace Packets
{
    class CmsgAreaSpiritHealerQueue : public ManagedPacket
    {
    public:
        WoWGuid guid;

        CmsgAreaSpiritHealerQueue() : CmsgAreaSpiritHealerQueue(0)
        {
        }

        CmsgAreaSpiritHealerQueue(uint64_t guid) :
            ManagedPacket(CMSG_AREA_SPIRIT_HEALER_QUEUE, 8),
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
