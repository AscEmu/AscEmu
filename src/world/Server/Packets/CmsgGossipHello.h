/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class CmsgGossipHello : public ManagedPacket
    {
    public:
        WoWGuid guid;

        CmsgGossipHello() : CmsgGossipHello(0)
        {
        }

        CmsgGossipHello(uint64_t guid) :
            ManagedPacket(CMSG_GOSSIP_HELLO, 8),
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
            packet << guid;
            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            uint64_t unpackedGuid;
            packet >> unpackedGuid;
            guid.Init(unpackedGuid);
            return true;
        }
    };
}}
