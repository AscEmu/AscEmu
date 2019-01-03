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
    class CmsgGmReportLag : public ManagedPacket
    {
#if VERSION_STRING > TBC
    public:
        uint32_t lagType;
        uint32_t mapId;
        LocationVector location;

        CmsgGmReportLag() : CmsgGmReportLag(0, 0, { 0.0f, 0.0f, 0.0f })
        {
        }

        CmsgGmReportLag(uint32_t lagType, uint32_t mapId, LocationVector location) :
            ManagedPacket(CMSG_GM_REPORT_LAG, 4 + 4 + 4 * 3),
            lagType(lagType),
            mapId(mapId),
            location(location)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> lagType >> mapId >> location.x >> location.y >> location.z;

            return true;
        }
#endif
    };
}}
