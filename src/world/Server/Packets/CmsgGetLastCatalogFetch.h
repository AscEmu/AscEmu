/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class CmsgGetLastCatalogFetch : public ManagedPacket
    {
    public:
        CmsgGetLastCatalogFetch() : ManagedPacket(CMSG_GET_LAST_CATALOG_FETCH, 0) {}

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            return m_protocol.isForever() && packet.remaining() == 0;
        }
    };
}
