/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgDestroyObject : public ManagedPacket
    {
    public:

        WoWGuid guid;

        SmsgDestroyObject() : SmsgDestroyObject(0)
        {
        }

        SmsgDestroyObject(uint64_t guid) : 
        ManagedPacket(SMSG_DESTROY_OBJECT, 9),
            guid(guid)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
#if VERSION_STRING < Mop
            packet << guid;
#if VERSION_STRING >= WotLK
            packet << uint8_t(0);
#endif
#else

            packet.writeBit(guid[3]);
            packet.writeBit(guid[2]);
            packet.writeBit(guid[4]);
            packet.writeBit(guid[1]);

            packet << uint8_t(0);

            packet.writeBit(guid[7]);
            packet.writeBit(guid[0]);
            packet.writeBit(guid[6]);
            packet.writeBit(guid[5]);

            packet.flushBits();

            packet.WriteByteSeq(guid[0]);
            packet.WriteByteSeq(guid[4]);
            packet.WriteByteSeq(guid[7]);
            packet.WriteByteSeq(guid[2]);
            packet.WriteByteSeq(guid[6]);
            packet.WriteByteSeq(guid[3]);
            packet.WriteByteSeq(guid[1]);
            packet.WriteByteSeq(guid[5]);

#endif
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
    };
}
