/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgItemTextQueryResponse : public ManagedPacket
    {
    public:
        uint8_t result;
        uint64_t guid;
        std::string text;

        SmsgItemTextQueryResponse() : SmsgItemTextQueryResponse(0, 0, "")
        {
        }

        SmsgItemTextQueryResponse(uint8_t result, uint64_t guid, std::string text) :
            ManagedPacket(SMSG_ITEM_TEXT_QUERY_RESPONSE, 0),
            result(result),
            guid(guid),
            text(text)
        {
        }

    protected:
        size_t expectedSize() const override { return 1 + 8 + text.size(); }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << result;
            if (!result)
                packet << guid << text;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
    };
}}
