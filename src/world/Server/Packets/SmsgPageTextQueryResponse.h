/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgPageTextQueryResponse : public ManagedPacket
    {
    public:
        uint32_t pageId;
        const char* text;
        uint32_t nextPageId;

        SmsgPageTextQueryResponse() : SmsgPageTextQueryResponse(0, "", 0)
        {
        }

        SmsgPageTextQueryResponse(uint32_t pageId, const char* text, uint32_t nextPageId) :
            ManagedPacket(SMSG_PAGE_TEXT_QUERY_RESPONSE, 1000),
            pageId(pageId),
            text(text),
            nextPageId(nextPageId)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isLegion())
            {
                // page, allow, then the pages: id, next page, player condition, flags, text
                const std::string pageText = text != nullptr ? text : "";

                packet << pageId;
                packet.writeBit(true);
                packet.flushBits();
                packet << uint32_t(1);
                packet << pageId;
                packet << nextPageId;
                packet << int32_t(0);
                packet << uint8_t(0);
                packet.writeBits(static_cast<uint32_t>(pageText.length()), 12);
                packet.flushBits();
                packet.writeString(pageText);
                return true;
            }

            packet << pageId << text << nextPageId;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
