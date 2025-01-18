/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu::Packets
{
    class CmsgBinderActivate : public ManagedPacket
    {
    public:
        WoWGuid guid;

        CmsgBinderActivate() : CmsgBinderActivate(0)
        {
        }

        CmsgBinderActivate(uint64_t guid) :
            ManagedPacket(CMSG_BINDER_ACTIVATE, 8),
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
#if VERSION_STRING <= Cata
            uint64_t unpackedGuid;
            packet >> unpackedGuid;
            guid.Init(unpackedGuid);
#elif VERSION_STRING == Mop
            ObjectGuid npcGuid;
            npcGuid[0] = packet.readBit();
            npcGuid[5] = packet.readBit();
            npcGuid[4] = packet.readBit();
            npcGuid[7] = packet.readBit();
            npcGuid[6] = packet.readBit();
            npcGuid[2] = packet.readBit();
            npcGuid[1] = packet.readBit();
            npcGuid[3] = packet.readBit();

            packet.ReadByteSeq(npcGuid[0]);
            packet.ReadByteSeq(npcGuid[4]);
            packet.ReadByteSeq(npcGuid[2]);
            packet.ReadByteSeq(npcGuid[3]);
            packet.ReadByteSeq(npcGuid[7]);
            packet.ReadByteSeq(npcGuid[1]);
            packet.ReadByteSeq(npcGuid[5]);
            packet.ReadByteSeq(npcGuid[6]);
#endif
            return true;
        }
    };
}
