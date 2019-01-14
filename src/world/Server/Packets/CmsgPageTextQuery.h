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

        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> pageId;
            return true;
        }
    };
}}
