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
    class CmsgGameobjReportUse : public ManagedPacket
    {
    public:
        WoWGuid guid;

        CmsgGameobjReportUse() : CmsgGameobjReportUse(0)
        {
        }

        CmsgGameobjReportUse(uint64_t guid) :
            ManagedPacket(CMSG_GAMEOBJ_REPORT_USE, 8),
            guid(guid)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            uint64_t unpackedGuid;
            packet >> unpackedGuid;
            guid = WoWGuid(unpackedGuid);
            return true;
        }
    };
}}
