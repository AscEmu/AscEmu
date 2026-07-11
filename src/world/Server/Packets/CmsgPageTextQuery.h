/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgPageTextQuery : public ManagedPacket
    {
    public:
        uint32_t pageId;

        CmsgPageTextQuery() : CmsgPageTextQuery(0)
        {
        }

        CmsgPageTextQuery(uint32_t pageId) :
            ManagedPacket(CMSG_PAGE_TEXT_QUERY, 4),
            pageId(pageId)
        {
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> pageId;
            return true;
        }
    };
}
