/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgPlayerBound : public ManagedPacket
    {
    public:
        uint64_t casterGuid;
        uint32_t zoneId;

        SmsgPlayerBound() : SmsgPlayerBound(0, 0)
        {
        }

        SmsgPlayerBound(uint64_t casterGuid, uint32_t zoneId) :
            ManagedPacket(SMSG_PLAYERBOUND, 12),
            casterGuid(casterGuid),
            zoneId(zoneId)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
#if VERSION_STRING <= Cata
            packet << casterGuid << zoneId;
#else
            WoWGuid guid = casterGuid;
            packet.writeBit(guid[2]);
            packet.writeBit(guid[4]);
            packet.writeBit(guid[0]);
            packet.writeBit(guid[3]);
            packet.writeBit(guid[6]);
            packet.writeBit(guid[7]);
            packet.writeBit(guid[5]);
            packet.writeBit(guid[1]);

            packet.WriteByteSeq(guid[6]);
            packet.WriteByteSeq(guid[1]);
            packet.WriteByteSeq(guid[2]);
            packet.WriteByteSeq(guid[3]);
            packet.WriteByteSeq(guid[4]);
            packet.WriteByteSeq(guid[5]);
            packet.WriteByteSeq(guid[7]);
            packet.WriteByteSeq(guid[0]);

            packet << zoneId;
#endif
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
