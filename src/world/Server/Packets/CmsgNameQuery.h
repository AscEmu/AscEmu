/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu::Packets
{
    class CmsgNameQuery : public ManagedPacket
    {
    public:
        WoWGuid guid;
        bool bit14 = false;
        bool bit1C = false;
        uint32_t unk = 0;
        uint32_t unk1 = 0;

        CmsgNameQuery() : ManagedPacket(CMSG_NAME_QUERY, 0)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
#if VERSION_STRING == Mop
            return false;
#else
            packet << guid.getRawGuid();
            return true;
#endif
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
#if VERSION_STRING == Mop
            guid[4] = packet.readBit();
            bit14 = packet.readBit();
            guid[6] = packet.readBit();
            guid[0] = packet.readBit();
            guid[7] = packet.readBit();
            guid[1] = packet.readBit();
            bit1C = packet.readBit();
            guid[5] = packet.readBit();
            guid[2] = packet.readBit();
            guid[3] = packet.readBit();

            packet.ReadByteSeq(guid[7]);
            packet.ReadByteSeq(guid[5]);
            packet.ReadByteSeq(guid[1]);
            packet.ReadByteSeq(guid[2]);
            packet.ReadByteSeq(guid[6]);
            packet.ReadByteSeq(guid[3]);
            packet.ReadByteSeq(guid[0]);
            packet.ReadByteSeq(guid[4]);

            if (bit14)
                packet >> unk;

            if (bit1C)
                packet >> unk1;

            return true;
#else
            uint64_t unpacked_guid;
            packet >> unpacked_guid;
            guid.init(unpacked_guid);
            return true;
#endif
        }
    };
}
