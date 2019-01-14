/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu { namespace Packets
{
    class CmsgCreatureQuery : public ManagedPacket
    {
    public:
        uint32_t entry;
        WoWGuid guid;

        CmsgCreatureQuery() : CmsgCreatureQuery(0, 0)
        {
        }

        CmsgCreatureQuery(uint32_t entry, uint64_t guid) :
            ManagedPacket(CMSG_CREATURE_QUERY, 12),
            entry(entry),
            guid(guid)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            packet << entry << guid;
            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            uint64_t unpacked_guid;
            packet >> entry >> unpacked_guid;
            guid.Init(unpacked_guid);
            return true;
        }
    };
}}
