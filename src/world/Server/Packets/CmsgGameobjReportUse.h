/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
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
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_WotLK)
            {
                uint64_t unpackedGuid;
                packet >> unpackedGuid;
                guid = WoWGuid(unpackedGuid);
                return true;
            }

            return false;
        }
    };
}
