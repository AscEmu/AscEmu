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
    class SmsgShowBank : public ManagedPacket
    {
    public:
        uint64_t guid;

        SmsgShowBank() : SmsgShowBank(0)
        {
        }

        SmsgShowBank(uint64_t guid) :
            ManagedPacket(SMSG_SHOW_BANK, 8),
            guid(guid)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
#if VERSION_STRING <= Cata
            packet << guid;
#else

            WoWGuid wowGuid;
            wowGuid.Init(guid);
            packet.writeBit(wowGuid[2]);
            packet.writeBit(wowGuid[4]);
            packet.writeBit(wowGuid[3]);
            packet.writeBit(wowGuid[6]);
            packet.writeBit(wowGuid[5]);
            packet.writeBit(wowGuid[1]);
            packet.writeBit(wowGuid[7]);
            packet.writeBit(wowGuid[0]);

            packet.WriteByteSeq(wowGuid[7]);
            packet.WriteByteSeq(wowGuid[0]);
            packet.WriteByteSeq(wowGuid[5]);
            packet.WriteByteSeq(wowGuid[3]);
            packet.WriteByteSeq(wowGuid[6]);
            packet.WriteByteSeq(wowGuid[1]);
            packet.WriteByteSeq(wowGuid[4]);
            packet.WriteByteSeq(wowGuid[2]);
#endif
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
    };
}
