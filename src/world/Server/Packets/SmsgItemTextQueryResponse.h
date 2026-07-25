/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <cstdint>
#include <string>
#include <utility>

namespace AscEmu::Packets
{
    class SmsgItemTextQueryResponse : public ManagedPacket
    {
    public:
        uint8_t result;
        uint64_t guid;
        uint32_t textId;
        std::string text;

        SmsgItemTextQueryResponse() : SmsgItemTextQueryResponse(0, 0, "")
        {
        }

        SmsgItemTextQueryResponse(uint8_t result, uint64_t identifier, std::string text) :
            ManagedPacket(SMSG_ITEM_TEXT_QUERY_RESPONSE, 0),
            result(result),
            guid(identifier),
            textId(static_cast<uint32_t>(identifier)),
            text(std::move(text))
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return m_protocol.expansion > WoW::Expansion::_TBC ?
                1 + (result == 0 ? 8 + text.size() + 1 : 0) :
                4 + text.size() + 1;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion > WoW::Expansion::_TBC)
            {
                packet << result;
                if (result == 0)
                    packet << guid << text;
            }
            else
            {
                packet << textId << text;
            }

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
