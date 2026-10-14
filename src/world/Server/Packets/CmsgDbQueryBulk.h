/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <cstdint>
#include <vector>

namespace AscEmu::Packets
{
    class CmsgDbQueryBulk : public ManagedPacket
    {
    public:
        uint32_t tableHash{0};
        std::vector<uint32_t> recordIds;

        CmsgDbQueryBulk() : ManagedPacket(CMSG_DB_QUERY_BULK, 4) {}

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (!m_protocol.isForever() || packet.remaining() < sizeof(uint32_t))
                return false;

            packet >> tableHash;
            const uint32_t queryCount = packet.readBits(13);

            if (queryCount > packet.remaining() / sizeof(uint32_t))
                return false;

            recordIds.clear();
            recordIds.reserve(queryCount);

            for (uint32_t i = 0; i < queryCount; ++i)
            {
                uint32_t recordId = 0;
                packet >> recordId;
                recordIds.emplace_back(recordId);
            }

            return packet.remaining() == 0;
        }
    };
}
