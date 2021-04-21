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

        uint64_t guid;

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
            ObjectGuid oGuid = guid;

            packet.writeBit(oGuid[3]);
            packet.writeBit(oGuid[2]);
            packet.writeBit(oGuid[4]);
            packet.writeBit(oGuid[1]);

            packet << uint8_t(0);

            packet.writeBit(oGuid[7]);
            packet.writeBit(oGuid[0]);
            packet.writeBit(oGuid[6]);
            packet.writeBit(oGuid[5]);

            packet.flushBits();

            packet.WriteByteSeq(oGuid[0]);
            packet.WriteByteSeq(oGuid[4]);
            packet.WriteByteSeq(oGuid[7]);
            packet.WriteByteSeq(oGuid[2]);
            packet.WriteByteSeq(oGuid[6]);
            packet.WriteByteSeq(oGuid[3]);
            packet.WriteByteSeq(oGuid[1]);
            packet.WriteByteSeq(oGuid[5]);

#endif
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
    };
}
