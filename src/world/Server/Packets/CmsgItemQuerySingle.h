/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class CmsgItemQuerySingle : public ManagedPacket
    {
    public:
        uint32_t item_id;

        CmsgItemQuerySingle() : CmsgItemQuerySingle(0)
        {
        }

        CmsgItemQuerySingle(uint32_t item_id) :
            ManagedPacket(CMSG_ITEM_QUERY_SINGLE, 4),
            item_id(item_id)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return m_minimum_size;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << item_id;
            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> item_id;
            return true;
        }
    };
}}
