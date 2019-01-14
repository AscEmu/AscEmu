/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class CmsgNpcTextQuery : public ManagedPacket
    {
    public:
        uint32_t text_id;
        uint64_t guid;

        CmsgNpcTextQuery() : CmsgNpcTextQuery(0, 0)
        {
        }

        CmsgNpcTextQuery(uint32_t text_id, uint64_t guid) :
            ManagedPacket(CMSG_NPC_TEXT_QUERY, 0),
            text_id(text_id),
            guid(guid)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return m_minimum_size;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << text_id << guid;
            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> text_id >> guid;
            return true;
        }
    };
}}
