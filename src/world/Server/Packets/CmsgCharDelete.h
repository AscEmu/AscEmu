/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class CmsgCharDelete : public ManagedPacket
    {
    public:
        WoWGuid guid;

        CmsgCharDelete() : CmsgCharDelete(0)
        {
        }

        CmsgCharDelete(uint64_t guid) :
#if VERSION_STRING < Mop
            ManagedPacket(CMSG_CHAR_DELETE, 8),
#else
            ManagedPacket(CMSG_CHAR_DELETE, 0),
#endif
            guid(guid)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
#if VERSION_STRING < Mop
            uint64_t unpackedGuid;
            packet >> unpackedGuid;
            guid.Init(unpackedGuid);
#else

            guid[1] = packet.readBit();
            guid[3] = packet.readBit();
            guid[2] = packet.readBit();
            guid[7] = packet.readBit();
            guid[4] = packet.readBit();
            guid[6] = packet.readBit();
            guid[0] = packet.readBit();
            guid[5] = packet.readBit();

            packet.ReadByteSeq(guid[7]);
            packet.ReadByteSeq(guid[1]);
            packet.ReadByteSeq(guid[6]);
            packet.ReadByteSeq(guid[0]);
            packet.ReadByteSeq(guid[3]);
            packet.ReadByteSeq(guid[4]);
            packet.ReadByteSeq(guid[2]);
            packet.ReadByteSeq(guid[5]);
#endif
            return true;
        }
    };
}
