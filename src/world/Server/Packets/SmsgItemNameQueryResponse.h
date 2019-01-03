/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>
#include <utility>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgItemNameQueryResponse : public ManagedPacket
    {
    public:
        uint32_t itemEntry;
        const char* name;
        uint32_t inventoryType;

        SmsgItemNameQueryResponse() : SmsgItemNameQueryResponse(0, "", 0)
        {
        }

        SmsgItemNameQueryResponse(uint32_t itemEntry, const char* name, uint32_t inventoryType) :
            ManagedPacket(SMSG_ITEM_NAME_QUERY_RESPONSE, 100),
            itemEntry(itemEntry),
            name(name),
            inventoryType(inventoryType)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << itemEntry << name << inventoryType;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
