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
        int64_t lastFetchTime;

        explicit SmsgLastCatalogFetchResponse(int64_t lastFetchTime) :
            ManagedPacket(SMSG_LAST_CATALOG_FETCH_RESPONSE, 8),
            lastFetchTime(lastFetchTime)
        {
        }

    protected:
        size_t expectedSize() const override { return 8; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (!m_protocol.isForever())
                return false;

            // time of the last shop catalog fetch, 0 for none
            packet << lastFetchTime;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
