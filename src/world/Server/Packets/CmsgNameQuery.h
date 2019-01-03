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
    class CmsgNameQuery : public ManagedPacket
    {
    public:
        WoWGuid guid;

        CmsgNameQuery() : CmsgNameQuery(0)
        {
        }

        CmsgNameQuery(uint64_t guid) :
            ManagedPacket(CMSG_NAME_QUERY, 8),
            guid(guid)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            packet << guid.GetOldGuid();
            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            uint64_t unpacked_guid;
            packet >> unpacked_guid;
            guid.Init(unpacked_guid);
            return true;
        }
    };
}}
