/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>
#include <utility>

#include "ManagedPacket.h"
#include "Units/Creatures/Creature.h"

namespace AscEmu { namespace Packets
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
            packet << pageId << text << nextPageId;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
