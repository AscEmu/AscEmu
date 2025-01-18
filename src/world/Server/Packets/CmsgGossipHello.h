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
            uint64_t unpackedGuid;
            packet >> unpackedGuid;
            guid.Init(unpackedGuid);
            return true;

            // uint64_t guid;

            // guid[2] = recvData.readBit();
            // guid[4] = recvData.readBit();
            // guid[0] = recvData.readBit();
            // guid[3] = recvData.readBit();
            // guid[6] = recvData.readBit();
            // guid[7] = recvData.readBit();
            // guid[5] = recvData.readBit();
            // guid[1] = recvData.readBit();

            // recvData.ReadByteSeq(guid[4]);
            // recvData.ReadByteSeq(guid[7]);
            // recvData.ReadByteSeq(guid[1]);
            // recvData.ReadByteSeq(guid[0]);
            // recvData.ReadByteSeq(guid[5]);
            // recvData.ReadByteSeq(guid[3]);
            // recvData.ReadByteSeq(guid[6]);
            // recvData.ReadByteSeq(guid[2]);
        }
    };
}
