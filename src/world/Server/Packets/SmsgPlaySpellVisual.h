/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgPlaySpellVisual : public ManagedPacket
    {
    public:
        uint64_t rawGuid;
        uint32_t visualId;
        uint32_t type;

        SmsgPlaySpellVisual() : SmsgPlaySpellVisual(0, 0, 0)
        {
        }

        SmsgPlaySpellVisual(uint64_t rawGuid, uint32_t visualId, uint32_t type) :
            ManagedPacket(SMSG_PLAY_SPELL_VISUAL, 4 + 4 + 4 + 8),
            rawGuid(rawGuid),
            visualId(visualId),
            type(type)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
#if VERSION_STRING < Cata
            packet << rawGuid << visualId;
#elif VERSION_STRING == Cata
            packet << uint32_t(0) << visualId << type;

            WoWGuid guid = rawGuid;
            packet.writeBit(guid[4]);
            packet.writeBit(guid[7]);
            packet.writeBit(guid[5]);
            packet.writeBit(guid[3]);
            packet.writeBit(guid[1]);
            packet.writeBit(guid[2]);
            packet.writeBit(guid[0]);
            packet.writeBit(guid[6]);

            packet.flushBits();

            packet.WriteByteSeq(guid[0]);
            packet.WriteByteSeq(guid[4]);
            packet.WriteByteSeq(guid[1]);
            packet.WriteByteSeq(guid[6]);
            packet.WriteByteSeq(guid[7]);
            packet.WriteByteSeq(guid[2]);
            packet.WriteByteSeq(guid[3]);
            packet.WriteByteSeq(guid[5]);

#elif VERSION_STRING == Mop
            WoWGuid guid = rawGuid;
            packet.writeBit(guid[4]);
            packet.writeBit(guid[2]);
            packet.writeBit(guid[6]);
            packet.writeBit(guid[5]);
            packet.writeBit(guid[1]);
            packet.writeBit(guid[3]);
            packet.writeBit(guid[0]);
            packet.writeBit(guid[7]);

            packet.WriteByteSeq(guid[5]);
            packet.WriteByteSeq(guid[7]);
            packet << uint32_t(0);
            packet.WriteByteSeq(guid[1]);
            packet.WriteByteSeq(guid[0]);
            packet.WriteByteSeq(guid[6]);
            packet << type;
            packet.WriteByteSeq(guid[4]);
            packet.WriteByteSeq(guid[2]);
            packet.WriteByteSeq(guid[3]);
            packet << visualId;
#endif

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
