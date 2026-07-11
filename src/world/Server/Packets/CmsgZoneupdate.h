/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgZoneupdate : public ManagedPacket
    {
    public:
        uint32_t zoneId;

        CmsgZoneupdate() : CmsgZoneupdate(0)
        {
        }

        CmsgZoneupdate(uint32_t zoneId) :
            ManagedPacket(CMSG_ZONEUPDATE, 4),
            zoneId(zoneId)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> zoneId;
            return true;
        }
    };
}
