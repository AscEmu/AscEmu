/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
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
#if VERSION_STRING <= Cata
            uint64_t unpackedGuid;
            packet >> unpackedGuid;
            guid.Init(unpackedGuid);
#else

            WoWGuid guid;
            guid[2] = packet.readBit();
            guid[4] = packet.readBit();
            guid[0] = packet.readBit();
            guid[3] = packet.readBit();
            guid[6] = packet.readBit();
            guid[7] = packet.readBit();
            guid[5] = packet.readBit();
            guid[1] = packet.readBit();

            packet.ReadByteSeq(guid[4]);
            packet.ReadByteSeq(guid[7]);
            packet.ReadByteSeq(guid[1]);
            packet.ReadByteSeq(guid[0]);
            packet.ReadByteSeq(guid[5]);
            packet.ReadByteSeq(guid[3]);
            packet.ReadByteSeq(guid[6]);
            packet.ReadByteSeq(guid[2]);
#endif
            return true;
        }
    };
}
