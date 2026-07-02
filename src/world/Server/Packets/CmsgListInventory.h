/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class CmsgListInventory : public ManagedPacket
    {
    public:
        WoWGuid guid;

        CmsgListInventory() : CmsgListInventory(0)
        {
        }

        CmsgListInventory(uint64_t guid) :
            ManagedPacket(CMSG_LIST_INVENTORY, 8),
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
            guid.init(unpackedGuid);
#else // Mop
            guid[6] = packet.readBit();
            guid[7] = packet.readBit();
            guid[3] = packet.readBit();
            guid[1] = packet.readBit();
            guid[2] = packet.readBit();
            guid[0] = packet.readBit();
            guid[4] = packet.readBit();
            guid[5] = packet.readBit();

            packet.readByteSeq(guid[0]);
            packet.readByteSeq(guid[7]);
            packet.readByteSeq(guid[1]);
            packet.readByteSeq(guid[6]);
            packet.readByteSeq(guid[4]);
            packet.readByteSeq(guid[3]);
            packet.readByteSeq(guid[5]);
            packet.readByteSeq(guid[2]);
#endif
            return true;
        }
    };
}
