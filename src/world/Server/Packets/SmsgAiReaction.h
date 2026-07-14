/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgAiReaction : public ManagedPacket
    {
    public:
        uint64_t unpackedGuid;
        uint32_t reaction;

        SmsgAiReaction() : SmsgAiReaction(0, 0)
        {
        }

        SmsgAiReaction(uint64_t unpackedGuid, uint32_t reaction) :
            ManagedPacket(SMSG_AI_REACTION, 12),
            unpackedGuid(unpackedGuid),
            reaction(reaction)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet << unpackedGuid << reaction;
            }
            else if (m_protocol.expansion == WoW::Expansion::_Mop)
            {
                WoWGuid guid = unpackedGuid;
                packet.writeBit(guid[5]);
                packet.writeBit(guid[7]);
                packet.writeBit(guid[0]);
                packet.writeBit(guid[4]);
                packet.writeBit(guid[6]);
                packet.writeBit(guid[2]);
                packet.writeBit(guid[3]);
                packet.writeBit(guid[1]);

                packet.writeByteSeq(guid[4]);
                packet.writeByteSeq(guid[6]);
                packet.writeByteSeq(guid[5]);
                packet << reaction;
                packet.writeByteSeq(guid[7]);
                packet.writeByteSeq(guid[1]);
                packet.writeByteSeq(guid[2]);
                packet.writeByteSeq(guid[0]);
                packet.writeByteSeq(guid[3]);
            }
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
