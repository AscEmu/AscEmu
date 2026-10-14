/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgLastCatalogFetchResponse : public ManagedPacket
    {
    public:
        explicit SmsgLastCatalogFetchResponse(int64_t lastCatalogFetch) :
            ManagedPacket(SMSG_LAST_CATALOG_FETCH_RESPONSE, sizeof(int64_t)),
            lastCatalogFetch(lastCatalogFetch)
        {
        }

    protected:
        size_t expectedSize() const override { return sizeof(int64_t); }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (!m_protocol.isForever())
                return false;

            packet << lastCatalogFetch;
            return true;
        }

        bool internalDeserialise(WorldPacket&) override { return false; }

    private:
        int64_t lastCatalogFetch;
    };
}
